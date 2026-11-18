
#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"
#include "utils/utils.h"

namespace
{
    constexpr uint64_t MATRIX_A_BASE = 0x100000;

    struct MatrixCase
    {
        size_t dimension;
        uint64_t a_base;
        uint64_t b_base;
        uint64_t c_base;

        std::vector<uint64_t> a;
        std::vector<uint64_t> b_transpose;
        std::vector<uint64_t> expected;
    };

    struct KernelResult
    {
        std::vector<uint64_t> output;
        uint64_t instructions = 0;
        uint64_t cycles = 0;
    };

    struct BenchmarkResult
    {
        size_t dimension;
        KernelResult scalar;
        KernelResult vector;

        double instruction_reduction;
        double cycle_speedup;
        double cycle_reduction;
    };

    MatrixCase createMatrixCase(size_t n)
    {
        MatrixCase test_case;

        test_case.dimension = n;

        const uint64_t matrix_bytes =
            static_cast<uint64_t>(n * n * sizeof(uint64_t));

        test_case.a_base = MATRIX_A_BASE;
        test_case.b_base = test_case.a_base + matrix_bytes + 0x100;
        test_case.c_base = test_case.b_base + matrix_bytes + 0x100;

        test_case.a.resize(n * n);
        test_case.b_transpose.resize(n * n);
        test_case.expected.resize(n * n);

        // Generate deterministic matrix A.
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t k = 0; k < n; ++k)
            {
                test_case.a[i * n + k] =
                    ((i + 1) * (k + 2)) % 7 + 1;
            }
        }

        // Generate B in transposed row-major layout.
        //
        // b_transpose[j][k] = B[k][j]
        //
        // This makes each column of B contiguous in memory.
        for (size_t j = 0; j < n; ++j)
        {
            for (size_t k = 0; k < n; ++k)
            {
                test_case.b_transpose[j * n + k] =
                    ((j + 2) * (k + 1)) % 5 + 1;
            }
        }

        // Calculate the expected matrix C = A * B.
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = 0; j < n; ++j)
            {
                uint64_t sum = 0;

                for (size_t k = 0; k < n; ++k)
                {
                    sum +=
                        test_case.a[i * n + k] *
                        test_case.b_transpose[j * n + k];
                }

                test_case.expected[i * n + j] = sum;
            }
        }

        return test_case;
    }

    std::string generateScalarAssembly(const MatrixCase& test_case)
    {
        const size_t n = test_case.dimension;

        std::ostringstream assembly;

        assembly << ".text\n";

        assembly << "li s0, " << test_case.a_base << "\n";
        assembly << "li s1, " << test_case.b_base << "\n";
        assembly << "li s2, " << test_case.c_base << "\n";

        // Each output element is computed independently.
        //
        // C[i][j] = sum(A[i][k] * BT[j][k])
        //
        // The scalar implementation performs one multiply and
        // one accumulation for every k.

        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = 0; j < n; ++j)
            {
                assembly << "\n# Calculate C[" << i << "][" << j << "]\n";

                assembly << "li t2, 0\n";

                for (size_t k = 0; k < n; ++k)
                {
                    const size_t a_index = i * n + k;
                    const size_t b_index = j * n + k;

                    const uint64_t a_offset =
                        a_index * sizeof(uint64_t);

                    const uint64_t b_offset =
                        b_index * sizeof(uint64_t);

                    assembly << "ld t3, " << a_offset << "(s0)\n";
                    assembly << "ld t4, " << b_offset << "(s1)\n";

                    assembly << "mul t5, t3, t4\n";
                    assembly << "add t2, t2, t5\n";
                }

                const uint64_t c_offset =
                    (i * n + j) * sizeof(uint64_t);

                assembly << "sd t2, "
                         << c_offset
                         << "(s2)\n";
            }
        }

        return assembly.str();
    }

    std::string generateVectorAssembly(const MatrixCase& test_case)
    {
        const size_t n = test_case.dimension;

        std::ostringstream assembly;

        assembly << ".text\n";

        // Configure the vector length.
        assembly << "li t0, " << n << "\n";
        assembly << "vsetvli x0, t0, 24\n";

        // Each output element is calculated using one vector dot.
        //
        // v1 = A[i][:]
        // v2 = BT[j][:]
        // t2 = dot(v1, v2)
        //
        // Only two vector registers are needed.

        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = 0; j < n; ++j)
            {
                const uint64_t a_address =
                    test_case.a_base +
                    i * n * sizeof(uint64_t);

                const uint64_t b_address =
                    test_case.b_base +
                    j * n * sizeof(uint64_t);

                const uint64_t c_address =
                    test_case.c_base +
                    (i * n + j) * sizeof(uint64_t);

                assembly << "\n# Calculate C[" << i << "][" << j << "]\n";

                assembly << "li t1, " << a_address << "\n";
                assembly << "vle64.v v1, (t1)\n";

                assembly << "li t1, " << b_address << "\n";
                assembly << "vle64.v v2, (t1)\n";

                assembly << "vdot.vv t2, v1, v2\n";

                assembly << "li t1, " << c_address << "\n";
                assembly << "sd t2, 0(t1)\n";
            }
        }

        return assembly.str();
    }

    KernelResult runKernel(
        const MatrixCase& test_case,
        const std::string& assembly_source)
    {
        Kites::setupVmStateDirectory();

        auto processor =
            std::make_unique<Kites::RVSSProcessor>();

        // Initialize input matrices before loading the program.
        for (size_t i = 0; i < test_case.a.size(); ++i)
        {
            processor->memory_controller_.writeDoubleWord_d(
                test_case.a_base + i * sizeof(uint64_t),
                test_case.a[i]);

            processor->memory_controller_.writeDoubleWord_d(
                test_case.b_base + i * sizeof(uint64_t),
                test_case.b_transpose[i]);
        }

        std::istringstream input(assembly_source);

        auto program = Kites::assemble(input);

        processor->LoadProgram(program);

        // Suppress the per-instruction debug output.
        std::ostringstream suppressed_output;
        auto* previous_buffer =
            std::cout.rdbuf(suppressed_output.rdbuf());

        try
        {
            // Run the entire program, including every instruction.
            processor->DebugRun();
        }
        catch (...)
        {
            std::cout.rdbuf(previous_buffer);
            throw;
        }

        std::cout.rdbuf(previous_buffer);

        KernelResult result;

        result.instructions = processor->instructions_retired_;
        result.cycles = processor->cycle_s_;

        result.output.resize(test_case.dimension *
                             test_case.dimension);

        for (size_t i = 0; i < result.output.size(); ++i)
        {
            result.output[i] =
                processor->memory_controller_.readDoubleWord_d(
                    test_case.c_base + i * sizeof(uint64_t));
        }

        return result;
    }

    BenchmarkResult benchmarkDimension(size_t n)
    {
        MatrixCase test_case = createMatrixCase(n);

        const std::string scalar_assembly =
            generateScalarAssembly(test_case);

        const std::string vector_assembly =
            generateVectorAssembly(test_case);

        BenchmarkResult result;

        result.dimension = n;

        result.scalar =
            runKernel(test_case, scalar_assembly);

        result.vector =
            runKernel(test_case, vector_assembly);

        // Check both implementations against the same reference.
        EXPECT_EQ(result.scalar.output, test_case.expected)
            << "Scalar result incorrect for "
            << n << "x" << n;

        EXPECT_EQ(result.vector.output, test_case.expected)
            << "Vector result incorrect for "
            << n << "x" << n;

        if (result.scalar.instructions == 0 ||
            result.scalar.cycles == 0 ||
            result.vector.instructions == 0 ||
            result.vector.cycles == 0)
        {
            throw std::runtime_error(
                "Invalid instruction or cycle count");
        }

        result.instruction_reduction =
            100.0 *
            (static_cast<double>(result.scalar.instructions) -
             static_cast<double>(result.vector.instructions)) /
            static_cast<double>(result.scalar.instructions);

        result.cycle_speedup =
            static_cast<double>(result.scalar.cycles) /
            static_cast<double>(result.vector.cycles);

        result.cycle_reduction =
            100.0 *
            (static_cast<double>(result.scalar.cycles) -
             static_cast<double>(result.vector.cycles)) /
            static_cast<double>(result.scalar.cycles);

        return result;
    }

    void writeCsv(const std::vector<BenchmarkResult>& results)
    {
        const std::filesystem::path csv_path =
            std::filesystem::path(__FILE__).parent_path() /
            "matrix_multiply_speedup.csv";

        std::ofstream csv(csv_path);

        if (!csv.is_open())
        {
            throw std::runtime_error(
                "Could not create matrix_multiply_speedup.csv");
        }

        csv << "Dimension,"
            << "Scalar Instructions,"
            << "Vector Instructions,"
            << "Scalar Cycles,"
            << "Vector Cycles,"
            << "Instruction Reduction (%),"
            << "Cycle Reduction (%),"
            << "Cycle Speedup (x)\n";

        csv << std::fixed << std::setprecision(2);

        for (const auto& result : results)
        {
            csv << result.dimension << "x" << result.dimension << ","
                << result.scalar.instructions << ","
                << result.vector.instructions << ","
                << result.scalar.cycles << ","
                << result.vector.cycles << ","
                << result.instruction_reduction << ","
                << result.cycle_reduction << ","
                << result.cycle_speedup << "\n";
        }

        std::cout << "\nCSV report written to:\n"
                  << csv_path.string() << "\n";
    }
}

TEST(RVSSMatrixMultiplyScalingTest, BenchmarkMultipleDimensions)
{
    // All dimensions are <= 64, matching the supported maximum
    // vector length. The selected sizes also keep test execution
    // practical while showing scaling behaviour.
    const std::vector<size_t> dimensions = {
        2, 4, 8, 12
    };

    std::vector<BenchmarkResult> results;

    for (const size_t n : dimensions)
    {
        std::cout << "\nBenchmarking "
                  << n << "x" << n << "...\n";

        results.push_back(benchmarkDimension(n));
    }

    std::cout << "\n";
    std::cout << "============================================================\n";
    std::cout << "          RVSS MATRIX MULTIPLICATION SCALING\n";
    std::cout << "============================================================\n";

    std::cout << std::left
              << std::setw(12) << "Dimension"
              << std::right
              << std::setw(12) << "Scalar"
              << std::setw(12) << "Vector"
              << std::setw(12) << "Speedup"
              << std::setw(14) << "Reduction"
              << "\n";

    std::cout << std::string(62, '-') << "\n";

    for (const auto& result : results)
    {
        const std::string dimension =
            std::to_string(result.dimension) + "x" +
            std::to_string(result.dimension);

        std::cout << std::left
                  << std::setw(12) << dimension
                  << std::right
                  << std::setw(12) << result.scalar.cycles
                  << std::setw(12) << result.vector.cycles
                  << std::setw(11) << std::fixed
                  << std::setprecision(2)
                  << result.cycle_speedup << "x"
                  << std::setw(12)
                  << result.cycle_reduction << "%"
                  << "\n";
    }

    std::cout << std::string(62, '-') << "\n";

    std::cout << "\nDetailed instruction statistics:\n";

    for (const auto& result : results)
    {
        std::cout << result.dimension << "x"
                  << result.dimension
                  << ": scalar = " << result.scalar.instructions
                  << ", vector = " << result.vector.instructions
                  << ", instruction reduction = "
                  << std::fixed << std::setprecision(2)
                  << result.instruction_reduction << "%\n";
    }

    std::cout << "============================================================\n";

    writeCsv(results);

    // The vector implementation should use fewer instructions and
    // cycles for each tested dimension.
    for (const auto& result : results)
    {
        EXPECT_LT(
            result.vector.instructions,
            result.scalar.instructions)
            << "Instruction count did not improve for "
            << result.dimension << "x" << result.dimension;

        EXPECT_LT(
            result.vector.cycles,
            result.scalar.cycles)
            << "Cycle count did not improve for "
            << result.dimension << "x" << result.dimension;
    }
}
