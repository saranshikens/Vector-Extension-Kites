#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"

namespace
{

class SingleCycleVectorCorrectnessTest
    : public ::testing::TestWithParam<std::size_t>
{
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

TEST_P(SingleCycleVectorCorrectnessTest, IntegerVectorInstructionMatchesAluSemantics)
{
    const std::size_t index = GetParam();
    Kites::RVSSProcessor processor;
    processor.registers_.SetVectorLength(64);

    const std::uint64_t a = edgeOrRandom(index);
    const std::uint64_t b = edgeOrRandom(index + 37);
    const std::uint64_t c = edgeOrRandom(index + 73);
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
            edgeOrRandom(index + lane * 11));

    std::string source;
    switch (index % 8)
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
                 std::to_string(vtypes[index % 8]) + "\n";
        break;
    }
    }

    std::istringstream input(source);
    processor.LoadProgram(Kites::assemble(input));
    processor.DebugRun();

    if (index % 8 == 0)
    {
        const auto result = processor.registers_.ReadVectorRegister(4);
        for (std::size_t lane = 0; lane < 64; ++lane)
            EXPECT_EQ(result[lane], a + b);
    }
    else if (index % 8 == 1)
    {
        const auto result = processor.registers_.ReadVectorRegister(4);
        for (std::size_t lane = 0; lane < 64; ++lane)
            EXPECT_EQ(result[lane], a - b);
    }
    else if (index % 8 == 2)
    {
        const auto result = processor.registers_.ReadVectorRegister(4);
        for (std::size_t lane = 0; lane < 64; ++lane)
            EXPECT_EQ(result[lane], a * b);
    }
    else if (index % 8 == 3)
    {
        std::uint64_t expected = 0;
        for (std::size_t lane = 0; lane < 64; ++lane)
            expected += a * b;
        EXPECT_EQ(processor.registers_.ReadGpr(5), expected);
    }
    else if (index % 8 == 4)
    {
        const auto result = processor.registers_.ReadVectorRegister(4);
        for (std::size_t lane = 0; lane < 64; ++lane)
            EXPECT_EQ(result[lane],
                      processor.memory_controller_.readDoubleWord(
                          0x3000 + lane * 8));
    }
    else if (index % 8 == 5)
    {
        for (std::size_t lane = 0; lane < 64; ++lane)
            EXPECT_EQ(processor.memory_controller_.readDoubleWord(
                          0x3000 + lane * 8),
                      c);
    }
    else if (index % 8 == 6)
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
            Kites::vector_isa::decodeVType(vtypes[index % 8]);
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

INSTANTIATE_TEST_SUITE_P(
    IntegerVectorEdgeAndRandomCases,
    SingleCycleVectorCorrectnessTest,
    ::testing::Range<std::size_t>(0, 1000),
    [](const ::testing::TestParamInfo<std::size_t>& info)
    {
        return "case_" + std::to_string(info.param);
    });

} // namespace
