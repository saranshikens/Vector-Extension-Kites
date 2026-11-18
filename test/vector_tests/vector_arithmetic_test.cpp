
#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <vector>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"

namespace
{

class RVSSVectorArithmeticTest : public ::testing::Test
{
protected:
    Kites::RVSSProcessor processor;

    void Execute(const std::string& source)
    {
        std::istringstream input(source);
        auto program = Kites::assemble(input);

        processor.LoadProgram(program);

        // Execute exactly the number of assembled instructions.
        for (size_t i = 0; i < program.text_buffer.size(); ++i)
        {
            processor.Step();
        }
    }

    void SetUp() override
    {
        processor.registers_.SetVectorLength(4);

        processor.registers_.WriteVectorRegister(
            1, {1, 2, 3, 4});

        processor.registers_.WriteVectorRegister(
            2, {10, 20, 30, 40});
    }

    std::vector<uint64_t> ReadVector(size_t reg)
    {
        return processor.registers_.ReadVectorRegister(reg);
    }
};

TEST_F(RVSSVectorArithmeticTest, VectorAddition)
{
    Execute("vadd.vv v3, v1, v2\n");

    EXPECT_EQ(ReadVector(3),
              (std::vector<uint64_t>{11, 22, 33, 44}));
}

TEST_F(RVSSVectorArithmeticTest, VectorSubtraction)
{
    Execute("vsub.vv v3, v1, v2\n");

    EXPECT_EQ(ReadVector(3),
              (std::vector<uint64_t>{
                  static_cast<uint64_t>(-9LL),
                  static_cast<uint64_t>(-18LL),
                  static_cast<uint64_t>(-27LL),
                  static_cast<uint64_t>(-36LL)
              }));
}

TEST_F(RVSSVectorArithmeticTest, VectorMultiplication)
{
    Execute("vmul.vv v3, v1, v2\n");

    EXPECT_EQ(ReadVector(3),
              (std::vector<uint64_t>{10, 40, 90, 160}));
}

TEST_F(RVSSVectorArithmeticTest, DestinationDoesNotModifySources)
{
    Execute("vadd.vv v3, v1, v2\n");

    EXPECT_EQ(ReadVector(1),
              (std::vector<uint64_t>{1, 2, 3, 4}));

    EXPECT_EQ(ReadVector(2),
              (std::vector<uint64_t>{10, 20, 30, 40}));
}

TEST_F(RVSSVectorArithmeticTest, DifferentDestinationRegister)
{
    Execute("vadd.vv v7, v1, v2\n");

    EXPECT_EQ(ReadVector(7),
              (std::vector<uint64_t>{11, 22, 33, 44}));
}

TEST_F(RVSSVectorArithmeticTest, AdditionWithZeroVector)
{
    processor.registers_.WriteVectorRegister(
        2, {0, 0, 0, 0});

    Execute("vadd.vv v3, v1, v2\n");

    EXPECT_EQ(ReadVector(3),
              (std::vector<uint64_t>{1, 2, 3, 4}));
}

TEST_F(RVSSVectorArithmeticTest, MultiplicationWithZeroVector)
{
    processor.registers_.WriteVectorRegister(
        2, {0, 0, 0, 0});

    Execute("vmul.vv v3, v1, v2\n");

    EXPECT_EQ(ReadVector(3),
              (std::vector<uint64_t>{0, 0, 0, 0}));
}

} // namespace

