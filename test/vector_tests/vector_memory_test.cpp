
#include <gtest/gtest.h>

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"

namespace
{

class RVSSVectorMemoryTest : public ::testing::Test
{
protected:
    Kites::RVSSProcessor processor;

    void SetUp() override
    {
        processor.registers_.SetVectorLength(4);
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

TEST_F(RVSSVectorMemoryTest, VectorLoadAndStoreRoundTrip)
{
    Execute(
        // Source and destination addresses
        "addi x10, x0, 256\n"
        "addi x11, x0, 512\n"

        // Initialize source memory using scalar stores
        "addi x12, x0, 11\n"
        "sd x12, 0(x10)\n"

        "addi x12, x0, 22\n"
        "sd x12, 8(x10)\n"

        "addi x12, x0, 33\n"
        "sd x12, 16(x10)\n"

        "addi x12, x0, 44\n"
        "sd x12, 24(x10)\n"

        // Load four 64-bit elements into v1
        "vle64.v v1, (x10)\n"

        // Store the vector into destination memory
        "vse64.v v1, (x11)\n"

        // Read destination memory using scalar loads
        "ld x13, 0(x11)\n"
        "ld x14, 8(x11)\n"
        "ld x15, 16(x11)\n"
        "ld x16, 24(x11)\n"
    );

    EXPECT_EQ(
        processor.registers_.ReadVectorRegister(1),
        (std::vector<uint64_t>{11, 22, 33, 44}));

    EXPECT_EQ(processor.registers_.ReadGpr(13), 11u);
    EXPECT_EQ(processor.registers_.ReadGpr(14), 22u);
    EXPECT_EQ(processor.registers_.ReadGpr(15), 33u);
    EXPECT_EQ(processor.registers_.ReadGpr(16), 44u);
}

TEST_F(RVSSVectorMemoryTest, VectorLoadIntoDifferentRegister)
{
    Execute(
        "addi x10, x0, 256\n"

        // Initialize memory
        "addi x12, x0, 5\n"
        "sd x12, 0(x10)\n"

        "addi x12, x0, 15\n"
        "sd x12, 8(x10)\n"

        "addi x12, x0, 25\n"
        "sd x12, 16(x10)\n"

        "addi x12, x0, 35\n"
        "sd x12, 24(x10)\n"

        // Load into v5 instead of v1
        "vle64.v v5, (x10)\n"
    );

    EXPECT_EQ(
        processor.registers_.ReadVectorRegister(5),
        (std::vector<uint64_t>{5, 15, 25, 35}));
}

} // namespace
