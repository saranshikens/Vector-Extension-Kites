
#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <vector>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"

namespace
{

class RVSSVectorConfigTest : public ::testing::Test
{
protected:
    Kites::RVSSProcessor processor;

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

TEST_F(RVSSVectorConfigTest, DefaultVectorLength)
{
    EXPECT_EQ(processor.registers_.GetVectorLength(), 4u);
}

TEST_F(RVSSVectorConfigTest, SetVectorLengthToTwo)
{
    processor.registers_.WriteGpr(10, 2);

    Execute("vsetvli x5, x10, 24\n");

    EXPECT_EQ(processor.registers_.GetVectorLength(), 2u);
    EXPECT_EQ(processor.registers_.ReadGpr(5), 2u);
}

TEST_F(RVSSVectorConfigTest, SetVectorLengthToFour)
{
    processor.registers_.WriteGpr(10, 4);

    Execute("vsetvli x5, x10, 24\n");

    EXPECT_EQ(processor.registers_.GetVectorLength(), 4u);
    EXPECT_EQ(processor.registers_.ReadGpr(5), 4u);
}

TEST_F(RVSSVectorConfigTest, SetVectorLengthToOne)
{
    processor.registers_.WriteGpr(10, 1);

    Execute("vsetvli x5, x10, 24\n");

    EXPECT_EQ(processor.registers_.GetVectorLength(), 1u);
    EXPECT_EQ(processor.registers_.ReadGpr(5), 1u);
}

TEST_F(RVSSVectorConfigTest, ZeroRequestedVectorLength)
{
    processor.registers_.WriteGpr(10, 0);

    Execute("vsetvli x5, x10, 24\n");

    EXPECT_EQ(processor.registers_.GetVectorLength(), 0u);
    EXPECT_EQ(processor.registers_.ReadGpr(5), 0u);
}

TEST_F(RVSSVectorConfigTest, ZeroRegisterRequestsMaximumVectorLength)
{
    Execute("vsetvli x5, x0, 24\n");

    EXPECT_EQ(processor.registers_.GetVectorLength(), 64u);
    EXPECT_EQ(processor.registers_.ReadGpr(5), 64u);
}

TEST_F(RVSSVectorConfigTest, ZeroDestinationAndSourceKeepVectorLength)
{
    processor.registers_.SetVectorLength(2);

    Execute("vsetvli x0, x0, 24\n");

    EXPECT_EQ(processor.registers_.GetVectorLength(), 2u);
}

TEST_F(RVSSVectorConfigTest, VectorLengthDoesNotExceedCapacity)
{
    processor.registers_.WriteGpr(10, 100);

    Execute("vsetvli x5, x10, 24\n");

    EXPECT_LE(processor.registers_.GetVectorLength(), 64u);
}

} // namespace
