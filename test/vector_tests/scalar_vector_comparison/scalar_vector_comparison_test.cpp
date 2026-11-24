#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "assembler/assembler.h"
#include "processor/rv5s/rv5s_processor_h_f.h"
#include "processor/rv5s/rv5s_processor_h_nf.h"
#include "processor/rv5s/rv5s_processor_nh_f.h"
#include "processor/rv5s/rv5s_processor_nh_nf.h"
#include "processor/rvss/rvss_processor.h"

namespace
{

constexpr std::uint64_t INPUT_BASE = 0x1000;
constexpr std::uint64_t VECTOR_OUTPUT_BASE = 0x3000;
constexpr std::uint64_t SCALAR_OUTPUT_BASE = 0x4000;
constexpr std::size_t LANES = 8;

std::uint64_t pseudoValue(std::size_t index)
{
    const std::uint64_t seed =
        0x9e3779b97f4a7c15ULL * (static_cast<std::uint64_t>(index) + 1);
    return (seed ^ (seed >> 30) ^ (seed << 16)) &
           0x0000ffffffffffffULL;
}

struct ComparisonRow
{
    std::string name;
    std::uint32_t rvss_scalar{};
    std::uint32_t rvss_vector{};
    std::uint32_t nhnf_scalar{};
    std::uint32_t nhnf_vector{};
    std::uint32_t nhf_scalar{};
    std::uint32_t nhf_vector{};
    std::uint32_t hnf_scalar{};
    std::uint32_t hnf_vector{};
    std::uint32_t hf_scalar{};
    std::uint32_t hf_vector{};
};

std::filesystem::path csvPath()
{
    return std::filesystem::path(__FILE__).parent_path() /
           "scalar_vector_cycle_comparison.csv";
}

void appendCsvHeaderIfNeeded()
{
    const auto path = csvPath();
    if (std::filesystem::exists(path))
        return;

    std::ofstream out(path, std::ios::out | std::ios::trunc);
    out << "Test Name,"
        << "Single Cycle Scalar,Single Cycle Vector,"
        << "RV5S NH/NF Scalar,RV5S NH/NF Vector,"
        << "RV5S NH/F Scalar,RV5S NH/F Vector,"
        << "RV5S H/NF Scalar,RV5S H/NF Vector,"
        << "RV5S H/F Scalar,RV5S H/F Vector\n";
}

void appendCsvRow(const ComparisonRow& row)
{
    std::ofstream out(csvPath(), std::ios::out | std::ios::app);
    out << row.name << ','
        << row.rvss_scalar << ',' << row.rvss_vector << ','
        << row.nhnf_scalar << ',' << row.nhnf_vector << ','
        << row.nhf_scalar << ',' << row.nhf_vector << ','
        << row.hnf_scalar << ',' << row.hnf_vector << ','
        << row.hf_scalar << ',' << row.hf_vector << '\n';
}

std::string makeScalarSource(std::size_t index)
{
    static const char* operations[] = {"add", "sub", "mul"};
    const std::string operation = operations[index % 3];
    std::ostringstream source;

    for (std::size_t lane = 0; lane < LANES; ++lane)
    {
        const std::string a = std::to_string(lane + 1);
        const std::string b = std::to_string(lane + 9);
        source << "ld x" << a << ", " << lane * 8
               << "(x28)\n";
        source << "ld x" << b << ", " << (lane + LANES) * 8
               << "(x28)\n";
        if (operation == "add")
            source << "add x" << lane + 17 << ", x" << lane + 1
                   << ", x" << lane + 9 << "\n";
        else if (operation == "sub")
            source << "sub x" << lane + 17 << ", x" << lane + 1
                   << ", x" << lane + 9 << "\n";
        else
            source << "mul x" << lane + 17 << ", x" << lane + 1
                   << ", x" << lane + 9 << "\n";
        source << "sd x" << lane + 17 << ", "
               << lane * 8 << "(x29)\n";
    }

    if (index % 5 == 4)
    {
        source << "addi x30, x0, 0\n";
        for (std::size_t lane = 0; lane < LANES; ++lane)
            source << "add x30, x30, x" << lane + 17 << "\n";
        source << "sd x30, 64(x29)\n";
    }
    return source.str();
}

std::string makeVectorSource(std::size_t index)
{
    static const char* operations[] =
        {"vadd.vv", "vsub.vv", "vmul.vv"};
    const std::string operation = operations[index % 3];
    std::ostringstream source;
    source << "vsetvli x0, x27, 24\n";
    source << "vle64.v v1, (x28)\n";
    source << "addi x31, x28, 64\n";
    source << "vle64.v v2, (x31)\n";
    source << operation << " v3, v1, v2\n";
    source << "vse64.v v3, (x29)\n";
    if (index % 5 == 4)
        source << "vdot.vv x30, v3, v2\n";
    return source.str();
}

void initializeScalarVectorMemory(Kites::ProcessorBase& processor)
{
    processor.registers_.WriteGpr(28, INPUT_BASE);
    processor.registers_.WriteGpr(29, SCALAR_OUTPUT_BASE);
    processor.registers_.WriteGpr(27, LANES);
    processor.registers_.SetVectorLength(LANES);

    for (std::size_t lane = 0; lane < LANES * 2; ++lane)
        processor.memory_controller_.writeDoubleWord(
            INPUT_BASE + lane * 8,
            pseudoValue(lane));

    for (std::size_t lane = 0; lane < LANES + 1; ++lane)
        processor.memory_controller_.writeDoubleWord(
            SCALAR_OUTPUT_BASE + lane * 8, 0);
}

template <typename Processor>
std::uint32_t runPipelinedComparison(const std::string& source,
                                     std::size_t laneBytesOffset)
{
    Processor processor;
    initializeScalarVectorMemory(processor);

    if (laneBytesOffset == 0)
    {
        processor.registers_.WriteGpr(29, VECTOR_OUTPUT_BASE);
        for (std::size_t lane = 0; lane < LANES * 2; ++lane)
            processor.registers_.WriteGpr(
                28, INPUT_BASE + lane * 0);
    }

    std::istringstream input(source);
    processor.LoadProgram(Kites::assemble(input));
    processor.DebugRun();
    return processor.cycle_s_;
}

class ScalarVectorComparisonTest
    : public ::testing::TestWithParam<std::size_t>
{
};

TEST_P(ScalarVectorComparisonTest, DescriptiveKernelCycleComparison)
{
    const std::size_t index = GetParam();
    if (index == 0)
    {
        std::filesystem::remove(csvPath());
    }
    static const char* categories[] = {
        "Vector_Elementwise_Add",
        "Vector_Elementwise_Sub",
        "Vector_Elementwise_Multiply",
        "Vector_Safe_Add_Store",
        "Vector_Dot_Reduction"
    };
    const std::string name =
        std::string(categories[index % 5]) + "_seed_" +
        std::to_string(index) + "_lanes_" + std::to_string(LANES);

    const std::string scalarSource = makeScalarSource(index);
    const std::string vectorSource = makeVectorSource(index);

    Kites::RVSSProcessor rvssScalar;
    initializeScalarVectorMemory(rvssScalar);
    std::istringstream scalarInput(scalarSource);
    rvssScalar.LoadProgram(Kites::assemble(scalarInput));
    rvssScalar.DebugRun();

    Kites::RVSSProcessor rvssVector;
    initializeScalarVectorMemory(rvssVector);
    rvssVector.registers_.WriteGpr(29, VECTOR_OUTPUT_BASE);
    std::istringstream vectorInput(vectorSource);
    rvssVector.LoadProgram(Kites::assemble(vectorInput));
    rvssVector.DebugRun();

    ComparisonRow row;
    row.name = name;
    row.rvss_scalar = rvssScalar.cycle_s_;
    row.rvss_vector = rvssVector.cycle_s_;
    row.nhnf_scalar =
        runPipelinedComparison<Kites::RV5StageProcessorNHNF>(
            scalarSource, 1);
    row.nhnf_vector =
        runPipelinedComparison<Kites::RV5StageProcessorNHNF>(
            vectorSource, 0);
    row.nhf_scalar =
        runPipelinedComparison<Kites::RV5StageProcessorNHF>(
            scalarSource, 1);
    row.nhf_vector =
        runPipelinedComparison<Kites::RV5StageProcessorNHF>(
            vectorSource, 0);
    row.hnf_scalar =
        runPipelinedComparison<Kites::RV5StageProcessorHNF>(
            scalarSource, 1);
    row.hnf_vector =
        runPipelinedComparison<Kites::RV5StageProcessorHNF>(
            vectorSource, 0);
    row.hf_scalar =
        runPipelinedComparison<Kites::RV5StageProcessorHF>(
            scalarSource, 1);
    row.hf_vector =
        runPipelinedComparison<Kites::RV5StageProcessorHF>(
            vectorSource, 0);

    appendCsvHeaderIfNeeded();
    appendCsvRow(row);

    EXPECT_GT(row.rvss_scalar, 0u);
    EXPECT_GT(row.rvss_vector, 0u);
    EXPECT_GT(row.nhnf_scalar, 0u);
    EXPECT_GT(row.nhnf_vector, 0u);
    EXPECT_GT(row.hf_scalar, 0u);
    EXPECT_GT(row.hf_vector, 0u);
}

INSTANTIATE_TEST_SUITE_P(
    OneHundredScalarVectorKernels,
    ScalarVectorComparisonTest,
    ::testing::Range<std::size_t>(0, 100),
    [](const ::testing::TestParamInfo<std::size_t>& info)
    {
        return "kernel_" + std::to_string(info.param);
    });

} // namespace
