
#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <vector>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"

namespace
{

class RVSSVectorSequenceTest : public ::testing::Test
{
protected:
    Kites::RVSSProcessor processor;

    void SetUp() override
    {
        processor.registers_.SetVectorLength(4);

        processor.registers_.WriteVectorRegister(
            1, {1, 2, 3, 4});

        processor.registers_.WriteVectorRegister(
            2, {10, 20, 30, 40});
    }

    void Execute(const std::string& source)
    {
        std::istringstream input(source);
        auto program = Kites::assemble(input);

        processor.LoadProgram(program);

        for (size_t i = 0; i < program.text_buffer.size(); ++i)
        {
            processor.Step();
        }
    }
};

TEST_F(RVSSVectorSequenceTest, MultipleVectorInstructions)
{
    Execute(
        "vadd.vv v3, v1, v2\n"
        "vmul.vv v4, v3, v2\n"
        "vsub.vv v5, v4, v1\n"
    );

    EXPECT_EQ(
        processor.registers_.ReadVectorRegister(3),
        (std::vector<uint64_t>{11, 22, 33, 44}));

    EXPECT_EQ(
        processor.registers_.ReadVectorRegister(4),
        (std::vector<uint64_t>{110, 440, 990, 1760}));

    EXPECT_EQ(
        processor.registers_.ReadVectorRegister(5),
        (std::vector<uint64_t>{109, 438, 987, 1756}));
}

TEST_F(RVSSVectorSequenceTest, DotProductUsingVectorMultiply)
{
    // Element-wise multiplication:
    // [1,2,3,4] * [10,20,30,40]
    // = [10,40,90,160]

    Execute("vmul.vv v3, v1, v2\n");

    EXPECT_EQ(
        processor.registers_.ReadVectorRegister(3),
        (std::vector<uint64_t>{10, 40, 90, 160}));

    // Accumulate the lanes using scalar addition.
    uint64_t sum = 0;

    for (const auto value :
         processor.registers_.ReadVectorRegister(3))
    {
        sum += value;
    }

    EXPECT_EQ(sum, 300u);
}

TEST_F(RVSSVectorSequenceTest, VectorRegisterIndependence)
{
    processor.registers_.WriteVectorRegister(
        6, {100, 200, 300, 400});

    Execute("vadd.vv v3, v1, v2\n");

    EXPECT_EQ(
        processor.registers_.ReadVectorRegister(6),
        (std::vector<uint64_t>{100, 200, 300, 400}));
}

} // namespace
