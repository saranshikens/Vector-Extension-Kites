#include <gtest/gtest.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "assembler/assembler.h"
#include "processor/rv5s/rv5s_processor_h_f.h"
#include "utils/utils.h"

namespace
{

constexpr uint64_t kMatrixABase = 0x2000;
constexpr uint64_t kMatrixBTransposeBase = 0x3000;
constexpr uint64_t kMatrixOutputBase = 0x4000;
constexpr uint64_t kMatrixLowerBase = 0x5000;
constexpr uint64_t kMatrixUpperBase = 0x6000;
constexpr uint64_t kMatrixUpperTransposeBase = 0x7000;
constexpr uint64_t kSeedBase = 20261108;
constexpr size_t kDefaultSamplesPerDimension = 100;
constexpr std::array<size_t, 9> kDimensions = {2, 3, 4, 5, 6, 8, 10, 12, 16};

using Matrix = std::vector<uint64_t>;

struct KernelMetrics
{
    uint64_t instructions{};
    uint64_t cycles{};
};

struct BenchmarkRow
{
    size_t dimension{};
    size_t sample{};
    uint64_t seed{};
    std::string method;
    KernelMetrics scalar;
    KernelMetrics vector;
};

class NullStreamBuffer : public std::streambuf
{
  protected:
    int overflow(int character) override
    {
        return traits_type::not_eof(character);
    }
};

class ScopedCoutSilencer
{
  public:
    ScopedCoutSilencer() : old_buffer_(std::cout.rdbuf(&null_buffer_)) {}
    ~ScopedCoutSilencer()
    {
        std::cout.rdbuf(old_buffer_);
    }

    ScopedCoutSilencer(const ScopedCoutSilencer &) = delete;
    ScopedCoutSilencer &operator=(const ScopedCoutSilencer &) = delete;

  private:
    NullStreamBuffer null_buffer_;
    std::streambuf *old_buffer_;
};

std::string readExample(const std::string &filename)
{
    std::ifstream input("../examples/" + filename);
    if (!input)
        throw std::runtime_error("Unable to read example assembly: " + filename);
    std::ostringstream contents;
    contents << input.rdbuf();
    return contents.str();
}

std::string setDimension(std::string source, size_t dimension)
{
    const std::string immediate = "li t";
    const std::string value = ", 4";
    const size_t marker = source.find(immediate);
    if (marker == std::string::npos)
        throw std::runtime_error("Could not locate matrix dimension in assembly");

    const size_t value_start = source.find(value, marker);
    if (value_start == std::string::npos)
        throw std::runtime_error("Could not locate matrix dimension immediate");

    source.replace(value_start + 2, 1, std::to_string(dimension));
    return source;
}

void replaceAll(std::string &source, const std::string &from, const std::string &to)
{
    size_t position = 0;
    while ((position = source.find(from, position)) != std::string::npos)
    {
        source.replace(position, from.size(), to);
        position += to.size();
    }
}

std::string loadScalarMatrixMultiply(size_t dimension)
{
    std::string source = setDimension(readExample("matrix_multiply_scalar_4x4.s"), dimension);
    replaceAll(source, "li t1, 0x2100", "li t1, 0x3000");
    replaceAll(source, "li t2, 0x2200", "li t2, 0x4000");
    return source;
}

std::string loadVectorMatrixMultiply(size_t dimension)
{
    std::string source = setDimension(readExample("matrix_multiply_vector_4x4.s"), dimension);
    replaceAll(source, "li t1, 0x2100", "li t1, 0x3000");
    replaceAll(source, "li t2, 0x2200", "li t2, 0x4000");
    replaceAll(source, "slli s4, s0, 5", "mul s4, s0, t3\n    slli s4, s4, 3");
    replaceAll(source, "slli s4, s1, 5", "mul s4, s1, t3\n    slli s4, s4, 3");
    return source;
}

std::string loadScalarCholesky(size_t dimension)
{
    std::string source = setDimension(readExample("cholesky_scalar_4x4.s"), dimension);
    replaceAll(source, "li t1, 0x2200", "li t1, 0x5000");
    return source;
}

std::string loadVectorCholesky(size_t dimension)
{
    std::string source = setDimension(readExample("cholesky_4x4.s"), dimension);
    replaceAll(source, "li t1, 0x2200", "li t1, 0x5000");
    replaceAll(source, "slli s4, s0, 5", "mul s4, s0, t2\n    slli s4, s4, 3");
    replaceAll(source, "slli s4, s1, 5", "mul s4, s1, t2\n    slli s4, s4, 3");
    return source;
}

std::string loadScalarLu(size_t dimension)
{
    std::string source = setDimension(readExample("lu_scalar_4x4.s"), dimension);
    replaceAll(source, "li t1, 0x2200", "li t1, 0x5000");
    replaceAll(source, "li t2, 0x2400", "li t2, 0x6000");
    return source;
}

std::string loadVectorLu(size_t dimension)
{
    std::string source = setDimension(readExample("lu_4x4.s"), dimension);
    replaceAll(source, "li t1, 0x2200", "li t1, 0x5000");
    replaceAll(source, "li t2, 0x2400", "li t2, 0x6000");
    replaceAll(source, "li t3, 0x2500", "li t3, 0x7000");
    replaceAll(source, "slli s4, s0, 5", "mul s4, s0, t4\n    slli s4, s4, 3");
    replaceAll(source, "slli s4, s1, 5", "mul s4, s1, t4\n    slli s4, s4, 3");
    return source;
}

Matrix transpose(const Matrix &matrix, size_t dimension)
{
    Matrix result(matrix.size());
    for (size_t row = 0; row < dimension; ++row)
        for (size_t column = 0; column < dimension; ++column)
            result[row * dimension + column] = matrix[column * dimension + row];
    return result;
}

void writeMatrix(Kites::MemoryController &memory, uint64_t base, const Matrix &matrix)
{
    for (size_t index = 0; index < matrix.size(); ++index)
        memory.writeDoubleWord_d(base + index * sizeof(uint64_t), matrix[index]);
}

Matrix readMatrix(Kites::MemoryController &memory, uint64_t base, size_t dimension)
{
    memory.getL1Cache()->flush();
    memory.getL2Cache()->flush();
    Matrix matrix(dimension * dimension);
    for (size_t index = 0; index < matrix.size(); ++index)
        matrix[index] = memory.readDoubleWord_d(base + index * sizeof(uint64_t));
    return matrix;
}

size_t benchmarkSamplesPerDimension()
{
    const char *configured_samples = std::getenv("KITES_MATRIX_BENCHMARK_SAMPLES");
    if (configured_samples == nullptr)
        return kDefaultSamplesPerDimension;

    const size_t samples = std::stoull(configured_samples);
    if (samples == 0)
        throw std::invalid_argument("KITES_MATRIX_BENCHMARK_SAMPLES must be positive");
    return samples;
}

template <typename Setup>
std::unique_ptr<Kites::RV5StageProcessorHF> runKernel(const std::string &source, Setup setup)
{
    auto vm = std::make_unique<Kites::RV5StageProcessorHF>();
    setup(vm->memory_controller_);
    std::istringstream assembly(source);
    vm->LoadProgram(Kites::assemble(assembly));
    vm->DebugRun();
    return vm;
}

std::pair<Matrix, Matrix> makeMultiplyInput(size_t dimension, uint64_t seed)
{
    std::mt19937_64 random(seed);
    std::uniform_int_distribution<int64_t> value(-8, 8);
    Matrix a(dimension * dimension);
    Matrix b(dimension * dimension);
    for (uint64_t &element : a)
        element = static_cast<uint64_t>(value(random));
    for (uint64_t &element : b)
        element = static_cast<uint64_t>(value(random));
    return {std::move(a), std::move(b)};
}

Matrix multiply(const Matrix &a, const Matrix &b, size_t dimension)
{
    Matrix result(dimension * dimension, 0);
    for (size_t row = 0; row < dimension; ++row)
        for (size_t column = 0; column < dimension; ++column)
            for (size_t k = 0; k < dimension; ++k)
                result[row * dimension + column] +=
                    a[row * dimension + k] * b[k * dimension + column];
    return result;
}

Matrix makeCholeskyInput(size_t dimension, uint64_t seed, Matrix &expected_l)
{
    std::mt19937_64 random(seed);
    std::uniform_int_distribution<int64_t> off_diagonal(-2, 2);
    std::uniform_int_distribution<int64_t> diagonal(2, 4);
    expected_l.assign(dimension * dimension, 0);
    for (size_t row = 0; row < dimension; ++row)
    {
        expected_l[row * dimension + row] = static_cast<uint64_t>(diagonal(random));
        for (size_t column = 0; column < row; ++column)
            expected_l[row * dimension + column] =
                static_cast<uint64_t>(off_diagonal(random));
    }
    return multiply(expected_l, transpose(expected_l, dimension), dimension);
}

std::pair<Matrix, Matrix> makeLuInput(size_t dimension, uint64_t seed)
{
    std::mt19937_64 random(seed);
    std::uniform_int_distribution<int64_t> lower_value(-1, 1);
    std::uniform_int_distribution<int64_t> upper_value(-2, 2);
    std::uniform_int_distribution<int64_t> diagonal(2, 4);
    Matrix lower(dimension * dimension, 0);
    Matrix upper(dimension * dimension, 0);
    for (size_t row = 0; row < dimension; ++row)
    {
        lower[row * dimension + row] = 1;
        upper[row * dimension + row] = static_cast<uint64_t>(diagonal(random));
        for (size_t column = 0; column < row; ++column)
            lower[row * dimension + column] = static_cast<uint64_t>(lower_value(random));
        for (size_t column = row + 1; column < dimension; ++column)
            upper[row * dimension + column] = static_cast<uint64_t>(upper_value(random));
    }
    return {multiply(lower, upper, dimension), lower};
}

KernelMetrics metrics(const Kites::RV5StageProcessorHF &vm)
{
    return {vm.instructions_retired_, vm.cycle_s_};
}

void addRow(std::vector<BenchmarkRow> &rows, size_t dimension, size_t sample,
            uint64_t seed, const std::string &method,
            const Kites::RV5StageProcessorHF &scalar,
            const Kites::RV5StageProcessorHF &vector)
{
    rows.push_back({dimension, sample, seed, method, metrics(scalar), metrics(vector)});
}

void benchmarkMultiply(std::vector<BenchmarkRow> &rows, size_t dimension, size_t sample)
{
    const uint64_t seed = kSeedBase + dimension * 10000 + sample * 3;
    const auto [a, b] = makeMultiplyInput(dimension, seed);
    const Matrix b_transposed = transpose(b, dimension);
    const Matrix expected = multiply(a, b, dimension);
    const auto setup = [&](Kites::MemoryController &memory)
    {
        writeMatrix(memory, kMatrixABase, a);
        writeMatrix(memory, kMatrixBTransposeBase, b_transposed);
        writeMatrix(memory, kMatrixOutputBase, Matrix(dimension * dimension, 0));
    };

    auto scalar = runKernel(loadScalarMatrixMultiply(dimension), setup);
    auto vector = runKernel(loadVectorMatrixMultiply(dimension), setup);
    EXPECT_EQ(readMatrix(scalar->memory_controller_, kMatrixOutputBase, dimension), expected);
    EXPECT_EQ(readMatrix(vector->memory_controller_, kMatrixOutputBase, dimension), expected);
    addRow(rows, dimension, sample, seed, "matrix_multiply", *scalar, *vector);
}

void benchmarkCholesky(std::vector<BenchmarkRow> &rows, size_t dimension, size_t sample)
{
    const uint64_t seed = kSeedBase + dimension * 10000 + sample * 3 + 1;
    Matrix expected_l;
    const Matrix input = makeCholeskyInput(dimension, seed, expected_l);
    const auto setup = [&](Kites::MemoryController &memory)
    {
        writeMatrix(memory, kMatrixABase, input);
        writeMatrix(memory, kMatrixLowerBase, Matrix(dimension * dimension, 0));
    };

    auto scalar = runKernel(loadScalarCholesky(dimension), setup);
    auto vector = runKernel(loadVectorCholesky(dimension), setup);
    EXPECT_EQ(readMatrix(scalar->memory_controller_, kMatrixLowerBase, dimension), expected_l);
    EXPECT_EQ(readMatrix(vector->memory_controller_, kMatrixLowerBase, dimension), expected_l);
    addRow(rows, dimension, sample, seed, "cholesky", *scalar, *vector);
}

void benchmarkLu(std::vector<BenchmarkRow> &rows, size_t dimension, size_t sample)
{
    const uint64_t seed = kSeedBase + dimension * 10000 + sample * 3 + 2;
    const auto [input, expected_l] = makeLuInput(dimension, seed);
    const Matrix expected_u(dimension * dimension, 0);
    const auto setup = [&](Kites::MemoryController &memory)
    {
        writeMatrix(memory, kMatrixABase, input);
        writeMatrix(memory, kMatrixLowerBase, Matrix(dimension * dimension, 0));
        writeMatrix(memory, kMatrixUpperBase, expected_u);
        writeMatrix(memory, kMatrixUpperTransposeBase, expected_u);
    };

    Matrix lower = expected_l;
    Matrix upper(dimension * dimension, 0);
    const auto calculateExpected = [&](const Matrix &matrix)
    {
        Matrix expected_lower(dimension * dimension, 0);
        Matrix expected_upper(dimension * dimension, 0);
        for (size_t i = 0; i < dimension; ++i)
        {
            for (size_t j = i; j < dimension; ++j)
            {
                int64_t value = static_cast<int64_t>(matrix[i * dimension + j]);
                for (size_t k = 0; k < i; ++k)
                    value -= static_cast<int64_t>(expected_lower[i * dimension + k]) *
                             static_cast<int64_t>(expected_upper[k * dimension + j]);
                expected_upper[i * dimension + j] = static_cast<uint64_t>(value);
            }
            expected_lower[i * dimension + i] = 1;
            for (size_t j = i + 1; j < dimension; ++j)
            {
                int64_t value = static_cast<int64_t>(matrix[j * dimension + i]);
                for (size_t k = 0; k < i; ++k)
                    value -= static_cast<int64_t>(expected_lower[j * dimension + k]) *
                             static_cast<int64_t>(expected_upper[k * dimension + i]);
                const int64_t pivot =
                    static_cast<int64_t>(expected_upper[i * dimension + i]);
                if (value % pivot != 0)
                    throw std::runtime_error("Generated LU input has an inexact pivot division");
                expected_lower[j * dimension + i] =
                    static_cast<uint64_t>(value / pivot);
            }
        }
        return std::pair<Matrix, Matrix>{std::move(expected_lower),
                                         std::move(expected_upper)};
    };
    std::tie(lower, upper) = calculateExpected(input);

    auto scalar = runKernel(loadScalarLu(dimension), setup);
    auto vector = runKernel(loadVectorLu(dimension), setup);
    EXPECT_EQ(readMatrix(scalar->memory_controller_, kMatrixLowerBase, dimension), lower);
    EXPECT_EQ(readMatrix(scalar->memory_controller_, kMatrixUpperBase, dimension), upper);
    EXPECT_EQ(readMatrix(vector->memory_controller_, kMatrixLowerBase, dimension), lower);
    EXPECT_EQ(readMatrix(vector->memory_controller_, kMatrixUpperBase, dimension), upper);
    addRow(rows, dimension, sample, seed, "lu", *scalar, *vector);
}

void writeReport(const std::vector<BenchmarkRow> &rows, size_t samples_per_dimension,
                 double runtime_seconds)
{
    std::ofstream report("../report.txt", std::ios::trunc);
    if (!report)
        throw std::runtime_error("Unable to create report.txt from the test directory");

    report << "KITES MATRIX KERNEL SCALING BENCHMARK\n"
              "=====================================\n\n"
              "Purpose\n"
              "-------\n"
              "Measure scalar versus vector-reduced matrix multiplication, Cholesky,\n"
              "and LU kernels as matrix dimension increases. Every input is run on\n"
              "the five-stage RV5 pipeline with hazard detection and forwarding.\n"
              "All results are simulator counters, not wall-clock timings.\n\n"
              "Methodology\n"
              "-----------\n"
              "Dimensions: 2, 3, 4, 5, 6, 8, 10, 12, 16.\n"
              "Samples per dimension and method: "
           << samples_per_dimension
           << "\nInput cases: " << kDimensions.size() * samples_per_dimension
           << " independently seeded inputs per method; each input runs both\n"
              "the scalar and vector implementation. Inputs are deterministic.\n"
              "Seed formula: multiply = 20261108 + N*10000 + sample*3;\n"
              "Cholesky adds 1; LU adds 2. Sample is zero-based.\n"
              "Multiply elements are uniform integers in [-8,8]. Cholesky inputs\n"
              "are generated as L*transpose(L), with random integral L and\n"
              "diagonal entries in [2,4]. LU inputs are generated as L*U with\n"
              "unit-lower L, upper-triangular U, and diagonal pivots in [2,4].\n"
              "The matrices and expected factors/results are checked for every run.\n"
              "Average speedup is the arithmetic mean of per-input scalar_cycles /\n"
              "vector_cycles. The ratio of mean cycles is also reported. Instruction\n"
              "speedup is calculated the same way using retired instruction counts.\n"
              "Random values use C++ mt19937_64 and uniform_int_distribution; the\n"
              "seed and generation order are defined by the benchmark source. For\n"
              "bit-for-bit input regeneration, use the same standard-library\n"
              "implementation as the run that produced this report.\n"
              "Build preset: test-only (C++20, Qt 6.9.3, MinGW GCC 13.1.0).\n"
              "Observed benchmark runtime: "
           << std::setprecision(1) << runtime_seconds << " seconds.\n"
           << "Memory bases (disjoint at the largest tested size): A=0x2000,\n"
              "B-transpose=0x3000, C=0x4000, L=0x5000, U=0x6000,\n"
              "U-transpose scratch=0x7000.\n"
              "Set KITES_MATRIX_BENCHMARK_SAMPLES before running the test to change\n"
              "the sample count; the default and count used here are 100.\n"
              "Inputs are generated by test/matrix_scaling_benchmark_test.cpp; their\n"
              "seed and all measured counters are included in the CSV below.\n\n"
              "Summary: mean simulator cycles and mean per-input speedup\n"
              "---------------------------------------------------------\n"
              "Method,N,Samples,Scalar_mean_cycles,Vector_mean_cycles,"
              "Mean_individual_cycle_speedup,Speedup_of_mean_cycles,"
              "Scalar_mean_instructions,Vector_mean_instructions,"
              "Mean_individual_instruction_speedup\n";

    report << std::fixed << std::setprecision(4);
    for (const char *method : {"matrix_multiply", "cholesky", "lu"})
    {
        for (size_t dimension : kDimensions)
        {
            uint64_t scalar_cycles = 0;
            uint64_t vector_cycles = 0;
            uint64_t scalar_instructions = 0;
            uint64_t vector_instructions = 0;
            double cycle_speedup_sum = 0.0;
            double instruction_speedup_sum = 0.0;
            size_t count = 0;
            for (const BenchmarkRow &row : rows)
            {
                if (row.method != method || row.dimension != dimension)
                    continue;
                scalar_cycles += row.scalar.cycles;
                vector_cycles += row.vector.cycles;
                scalar_instructions += row.scalar.instructions;
                vector_instructions += row.vector.instructions;
                cycle_speedup_sum += static_cast<double>(row.scalar.cycles) /
                                     static_cast<double>(row.vector.cycles);
                instruction_speedup_sum +=
                    static_cast<double>(row.scalar.instructions) /
                    static_cast<double>(row.vector.instructions);
                ++count;
            }
            if (count == 0)
                throw std::runtime_error("Missing benchmark rows for summary");
            const double n = static_cast<double>(count);
            report << method << ',' << dimension << ',' << count << ','
                   << static_cast<double>(scalar_cycles) / n << ','
                   << static_cast<double>(vector_cycles) / n << ','
                   << cycle_speedup_sum / n << ','
                   << static_cast<double>(scalar_cycles) /
                          static_cast<double>(vector_cycles)
                   << ',' << static_cast<double>(scalar_instructions) / n << ','
                   << static_cast<double>(vector_instructions) / n << ','
                   << instruction_speedup_sum / n << '\n';
        }
    }

    const auto meanCycleSpeedup = [&](const char *method, size_t dimension)
    {
        double speedup_sum = 0.0;
        size_t count = 0;
        for (const BenchmarkRow &row : rows)
        {
            if (row.method == method && row.dimension == dimension)
            {
                speedup_sum += static_cast<double>(row.scalar.cycles) /
                               static_cast<double>(row.vector.cycles);
                ++count;
            }
        }
        if (count == 0)
            throw std::runtime_error("Missing benchmark rows for speedup interpretation");
        return speedup_sum / static_cast<double>(count);
    };

    report << "\nInterpretation highlights\n"
              "-------------------------\n"
              "At N=16, average cycle speedups are "
           << meanCycleSpeedup("matrix_multiply", 16) << "x for matrix multiply, "
           << meanCycleSpeedup("cholesky", 16) << "x for Cholesky, and "
           << meanCycleSpeedup("lu", 16) << "x for LU.\n"
              "At N=2, Cholesky is "
           << meanCycleSpeedup("cholesky", 2) << "x and LU is "
           << meanCycleSpeedup("lu", 2)
           << "x; values below 1.0 mean vector setup/load overhead outweighs\n"
              "the shorter reductions at that size. The complete per-dimension\n"
              "tables below show how these averages change as N increases.\n";

    report << "\nPer-input raw measurements (CSV)\n"
              "--------------------------------\n"
              "method,dimension,sample,seed,scalar_instructions,vector_instructions,"
              "scalar_cycles,vector_cycles,cycle_speedup,instruction_speedup\n";
    for (const BenchmarkRow &row : rows)
    {
        report << row.method << ',' << row.dimension << ',' << row.sample << ','
               << row.seed << ',' << row.scalar.instructions << ','
               << row.vector.instructions << ',' << row.scalar.cycles << ','
               << row.vector.cycles << ','
               << static_cast<double>(row.scalar.cycles) /
                      static_cast<double>(row.vector.cycles)
               << ','
               << static_cast<double>(row.scalar.instructions) /
                      static_cast<double>(row.vector.instructions)
               << '\n';
    }

    report << "\nNotes and interpretation\n"
              "------------------------\n"
              "Speedup above 1.0 means the vector implementation used fewer cycles\n"
              "(or instructions) than its scalar counterpart; below 1.0 means it\n"
              "was slower for that metric. Per-input speedups are averaged so that\n"
              "each generated matrix has equal weight. Cycle counts include pipeline\n"
              "fill/drain, branches, address arithmetic, and any NOPs in the kernels.\n"
              "The current integer Cholesky kernel requires an SPD matrix with exact\n"
              "integer Cholesky factors and diagonal square roots. The LU kernel uses\n"
              "Doolittle factorization without pivoting. All generated matrices meet\n"
              "these preconditions and all integer divisions are exact. Results are\n"
              "specific to this simulator, implementation, compiler-independent\n"
              "assembly workload, and tested dimensions; they are not hardware timing\n"
              "claims. The results can be regenerated with:\n\n"
              "    cmake --build --preset test-only\n"
              "    ctest --preset test-only -R MatrixScalingBenchmark\n";

    report.flush();
    if (!report)
        throw std::runtime_error("Failed while writing benchmark report.txt");
}

TEST(MatrixScalingBenchmark, CorrectnessAndPerformanceAcrossDimensions)
{
    Kites::setupVmStateDirectory();
    ScopedCoutSilencer silence;
    const size_t samples_per_dimension = benchmarkSamplesPerDimension();
    const auto start_time = std::chrono::steady_clock::now();
    std::vector<BenchmarkRow> rows;
    rows.reserve(kDimensions.size() * samples_per_dimension * 3);

    for (size_t dimension : kDimensions)
    {
        for (size_t sample = 0; sample < samples_per_dimension; ++sample)
        {
            SCOPED_TRACE(::testing::Message()
                         << "dimension=" << dimension << ", sample=" << sample);
            benchmarkMultiply(rows, dimension, sample);
            benchmarkCholesky(rows, dimension, sample);
            benchmarkLu(rows, dimension, sample);
        }
    }

    ASSERT_EQ(rows.size(), kDimensions.size() * samples_per_dimension * 3);
    const double runtime_seconds =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start_time).count();
    writeReport(rows, samples_per_dimension, runtime_seconds);
}

} // namespace
