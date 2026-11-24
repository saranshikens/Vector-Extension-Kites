#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "assembler/assembler.h"
#include "processor/rv5s/rv5s_processor_h_f.h"
#include "processor/rv5s/rv5s_processor_h_nf.h"
#include "processor/rv5s/rv5s_processor_nh_f.h"
#include "processor/rv5s/rv5s_processor_nh_nf.h"

namespace
{

enum class PipelineMode
{
    NH_NF,
    NH_F,
    H_NF,
    H_F
};

struct PipelineCase
{
    std::size_t index;
    PipelineMode mode;
};

std::uint64_t edgeOrRandom(std::size_t index)
{
    static const std::vector<std::uint64_t> edges = {
        0, 1, 2, 3, 0xffffffffffffffffULL, 0x8000000000000000ULL,
        0x7fffffffffffffffULL, 0x0000000000000001ULL, 0x5555555555555555ULL,
        0xaaaaaaaaaaaaaaaaULL
    };
    if (index < edges.size())
        return edges[index];
    return static_cast<std::uint64_t>(0x9e3779b97f4a7c15ULL * (index + 1)) ^
           static_cast<std::uint64_t>(0xd1b54a32d192ed04ULL * (index >> 3));
}

class PipelinedVectorCorrectnessTest
    : public ::testing::TestWithParam<PipelineCase>
{
};

template <typename Processor>
void runPipelinedCase(const PipelineCase& param)
{
    Processor processor;
    processor.registers_.SetVectorLength(64);

    const std::uint64_t a = edgeOrRandom(param.index);
    const std::uint64_t b = edgeOrRandom(param.index + 37);
    const std::uint64_t c = edgeOrRandom(param.index + 73);
    processor.registers_.WriteVectorRegister(
        1, std::vector<std::uint64_t>(64, a));
    processor.registers_.WriteVectorRegister(
        2, std::vector<std::uint64_t>(64, b));
    processor.registers_.WriteVectorRegister(
        3, std::vector<std::uint64_t>(64, c));
    processor.registers_.WriteGpr(12, 0x3000);
    processor.registers_.WriteGpr(7, 64);
    for (std::size_t lane = 0; lane < 64; ++lane)
        processor.memory_controller_.writeDoubleWord(
            0x3000 + lane * 8,
            edgeOrRandom(param.index + lane * 11));

    std::string source;
    switch (param.index % 8)
    {
    case 0: source = "vadd.vv v4, v1, v2\n"; break;
    case 1: source = "vsub.vv v4, v1, v2\n"; break;
    case 2: source = "vmul.vv v4, v1, v2\n"; break;
    case 3: source = "vdot.vv x5, v1, v2\n"; break;
    case 4: source = "vle64.v v4, (x12)\n"; break;
    case 5: source = "vse64.v v3, (x12)\n"; break;
    case 6: source = "vsetvli x6, x7, 24\n"; break;
    case 7:
    {
        static const std::uint16_t vtypes[] = {
            0x18, 0x58, 0x98, 0xd8, 0x08, 0x10, 0x00, 0x03
        };
        source = "vsetvli x6, x7, " +
                 std::to_string(vtypes[param.index % 8]) + "\n";
        break;
    }
    }

    std::istringstream input(source);
    processor.LoadProgram(Kites::assemble(input));
    processor.DebugRun();

    if (param.index % 8 == 0)
    {
        const auto result = processor.registers_.ReadVectorRegister(4);
        for (std::size_t lane = 0; lane < 64; ++lane)
            EXPECT_EQ(result[lane], a + b);
    }
    else if (param.index % 8 == 1)
    {
        const auto result = processor.registers_.ReadVectorRegister(4);
        for (std::size_t lane = 0; lane < 64; ++lane)
            EXPECT_EQ(result[lane], a - b);
    }
    else if (param.index % 8 == 2)
    {
        const auto result = processor.registers_.ReadVectorRegister(4);
        for (std::size_t lane = 0; lane < 64; ++lane)
            EXPECT_EQ(result[lane], a * b);
    }
    else if (param.index % 8 == 3)
    {
        std::uint64_t expected = 0;
        for (std::size_t lane = 0; lane < 64; ++lane)
            expected += a * b;
        EXPECT_EQ(processor.registers_.ReadGpr(5), expected);
    }
    else if (param.index % 8 == 4)
    {
        const auto result = processor.registers_.ReadVectorRegister(4);
        for (std::size_t lane = 0; lane < 64; ++lane)
            EXPECT_EQ(result[lane],
                      processor.memory_controller_.readDoubleWord(
                          0x3000 + lane * 8));
    }
    else if (param.index % 8 == 5)
    {
        for (std::size_t lane = 0; lane < 64; ++lane)
            EXPECT_EQ(processor.memory_controller_.readDoubleWord(
                          0x3000 + lane * 8),
                      c);
    }
    else if (param.index % 8 == 6)
    {
        EXPECT_EQ(processor.registers_.GetVectorLength(), 64u);
        EXPECT_EQ(processor.registers_.ReadGpr(6), 64u);
    }
    else
    {
        static const std::uint16_t vtypes[] = {
            0x18, 0x58, 0x98, 0xd8, 0x08, 0x10, 0x00, 0x03
        };
        const auto config =
            Kites::vector_isa::decodeVType(vtypes[param.index % 8]);
        EXPECT_EQ(processor.registers_.GetVectorConfig().sew,
                  config.sew);
        EXPECT_EQ(processor.registers_.GetVectorConfig().lmul,
                  config.lmul);
        EXPECT_EQ(processor.registers_.GetVectorConfig().tail_agnostic,
                  config.tail_agnostic);
        EXPECT_EQ(processor.registers_.GetVectorConfig().mask_agnostic,
                  config.mask_agnostic);
    }
}

TEST_P(PipelinedVectorCorrectnessTest, IntegerVectorInstructionMatchesAllModes)
{
    const PipelineCase param = GetParam();
    switch (param.mode)
    {
    case PipelineMode::NH_NF:
        runPipelinedCase<Kites::RV5StageProcessorNHNF>(param);
        break;
    case PipelineMode::NH_F:
        runPipelinedCase<Kites::RV5StageProcessorNHF>(param);
        break;
    case PipelineMode::H_NF:
        runPipelinedCase<Kites::RV5StageProcessorHNF>(param);
        break;
    case PipelineMode::H_F:
        runPipelinedCase<Kites::RV5StageProcessorHF>(param);
        break;
    }
}

std::vector<PipelineCase> makePipelinedCases()
{
    std::vector<PipelineCase> cases;
    cases.reserve(4000);
    for (std::size_t index = 0; index < 1000; ++index)
    {
        cases.push_back({index, PipelineMode::NH_NF});
        cases.push_back({index, PipelineMode::NH_F});
        cases.push_back({index, PipelineMode::H_NF});
        cases.push_back({index, PipelineMode::H_F});
    }
    return cases;
}

INSTANTIATE_TEST_SUITE_P(
    PipelinedVectorEdgeAndRandomCases,
    PipelinedVectorCorrectnessTest,
    ::testing::ValuesIn(makePipelinedCases()),
    [](const ::testing::TestParamInfo<PipelineCase>& info)
    {
        const char* mode = "";
        switch (info.param.mode)
        {
        case PipelineMode::NH_NF: mode = "NH_NF"; break;
        case PipelineMode::NH_F: mode = "NH_F"; break;
        case PipelineMode::H_NF: mode = "H_NF"; break;
        case PipelineMode::H_F: mode = "H_F"; break;
        }
        return std::string(mode) + "_case_" +
               std::to_string(info.param.index);
    });

} // namespace
