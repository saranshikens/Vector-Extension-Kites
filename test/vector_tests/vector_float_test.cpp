
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <sstream>
#include <vector>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"

namespace
{
uint64_t ToBits(double value)
{
    uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

double FromBits(uint64_t bits)
{
    double value = 0.0;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

class RVSSVectorFloatTest : public ::testing::Test
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

    void SetUp() override
    {
        processor.registers_.SetVectorLength(4);
    }

    std::vector<double> ReadVector(size_t reg)
    {
        const auto bits =
            processor.registers_.ReadVectorRegister(reg);

        std::vector<double> values;

        for (uint64_t value : bits)
        {
            values.push_back(FromBits(value));
        }

        return values;
    }
};

TEST_F(RVSSVectorFloatTest, VectorDivideByScalar)
{
    processor.registers_.WriteVectorRegister(
        1,
        {ToBits(10.0), ToBits(20.0),
         ToBits(30.0), ToBits(40.0)}
    );

    processor.registers_.WriteFpr(2, ToBits(10.0));

    Execute("vfdiv.vf v3, v1, f2\n");

    EXPECT_EQ(
        ReadVector(3),
        (std::vector<double>{1.0, 2.0, 3.0, 4.0})
    );
}

TEST_F(RVSSVectorFloatTest, VectorNegativeMultiplySubtractAccumulate)
{
    processor.registers_.WriteVectorRegister(
        1,
        {ToBits(1.0), ToBits(2.0),
         ToBits(3.0), ToBits(4.0)}
    );

    processor.registers_.WriteVectorRegister(
        3,
        {ToBits(100.0), ToBits(100.0),
         ToBits(100.0), ToBits(100.0)}
    );

    processor.registers_.WriteFpr(2, ToBits(10.0));

    Execute("vfnmsac.vf v3, f2, v1\n");

    EXPECT_EQ(
        ReadVector(3),
        (std::vector<double>{90.0, 80.0, 70.0, 60.0})
    );
}

TEST_F(RVSSVectorFloatTest, OperationsRespectActiveVectorLength)
{
    // Initialize four lanes while VL is 4.
    processor.registers_.WriteVectorRegister(
        1,
        {ToBits(10.0), ToBits(20.0),
         ToBits(30.0), ToBits(40.0)}
    );

    processor.registers_.WriteFpr(2, ToBits(10.0));

    // Execute with only two active lanes.
    processor.registers_.SetVectorLength(2);

    Execute("vfdiv.vf v3, v1, f2\n");

    // Read only the two active lanes.
    EXPECT_EQ(
        ReadVector(3),
        (std::vector<double>{1.0, 2.0})
    );
}

} // namespace
