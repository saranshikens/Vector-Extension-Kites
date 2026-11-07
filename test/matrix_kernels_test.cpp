#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <memory>

#include "assembler/assembler.h"
#include "processor/rv5s/rv5s_processor_h_f.h"
#include "utils/utils.h"

namespace
{

constexpr uint64_t kMatrixA = 0x2000;
constexpr uint64_t kMatrixBTranspose = 0x2100;
constexpr uint64_t kMatrixL = 0x2200;
constexpr uint64_t kMatrixC = 0x2200;
constexpr uint64_t kMatrixU = 0x2400;
constexpr size_t kMatrixSize = 4;
constexpr size_t kMatrixElements = kMatrixSize * kMatrixSize;

using Matrix = std::array<uint64_t, kMatrixElements>;

const Matrix kMultiplyA = {
    1, 2, 3, 4,
    5, 6, 7, 8,
    9, 10, 11, 12,
    13, 14, 15, 16,
};

const Matrix kMultiplyBTranspose = {
    16, 12, 8, 4,
    15, 11, 7, 3,
    14, 10, 6, 2,
    13, 9, 5, 1,
};

const Matrix kMultiplyExpected = {
    80, 70, 60, 50,
    240, 214, 188, 162,
    400, 358, 316, 274,
    560, 502, 444, 386,
};

const Matrix kCholeskyInput = {
    4, 4, 0, 4,
    4, 13, 9, 4,
    0, 9, 25, 16,
    4, 4, 16, 45,
};

const Matrix kCholeskyExpected = {
    2, 0, 0, 0,
    2, 3, 0, 0,
    0, 3, 4, 0,
    2, 0, 4, 5,
};

const Matrix kLuInput = {
    2, 3, 0, 4,
    4, 9, 3, 8,
    0, 9, 13, 3,
    4, 6, 16, 25,
};

const Matrix kLuExpectedL = {
    1, 0, 0, 0,
    2, 1, 0, 0,
    0, 3, 1, 0,
    2, 0, 4, 1,
};

const Matrix kLuExpectedU = {
    2, 3, 0, 4,
    0, 3, 3, 0,
    0, 0, 4, 3,
    0, 0, 0, 5,
};

void writeMatrix(Kites::MemoryController &memory, uint64_t base, const Matrix &matrix)
{
    for (size_t index = 0; index < matrix.size(); ++index)
        memory.writeDoubleWord_d(base + index * sizeof(uint64_t), matrix[index]);
}

Matrix readMatrix(Kites::MemoryController &memory, uint64_t base)
{
    Matrix matrix{};
    for (size_t index = 0; index < matrix.size(); ++index)
        matrix[index] = memory.readDoubleWord_d(base + index * sizeof(uint64_t));
    return matrix;
}

template <typename Setup>
std::unique_ptr<Kites::RV5StageProcessorHF> runKernel(const char *source, Setup setup)
{
    Kites::setupVmStateDirectory();
    auto vm = std::make_unique<Kites::RV5StageProcessorHF>();
    setup(vm->memory_controller_);
    vm->LoadProgram(Kites::assemble(source));
    vm->DebugRun();
    return vm;
}

TEST(MatrixKernelsTest, VectorMultiplyMatchesScalarAndUsesFewerInstructions)
{
    auto scalar = runKernel("../examples/matrix_multiply_scalar_4x4.s",
                            [](Kites::MemoryController &memory)
                            {
                                writeMatrix(memory, kMatrixA, kMultiplyA);
                                writeMatrix(memory, kMatrixBTranspose, kMultiplyBTranspose);
                            });
    auto vector = runKernel("../examples/matrix_multiply_vector_4x4.s",
                            [](Kites::MemoryController &memory)
                            {
                                writeMatrix(memory, kMatrixA, kMultiplyA);
                                writeMatrix(memory, kMatrixBTranspose, kMultiplyBTranspose);
                            });

    EXPECT_EQ(readMatrix(scalar->memory_controller_, kMatrixC), kMultiplyExpected);
    EXPECT_EQ(readMatrix(vector->memory_controller_, kMatrixC), kMultiplyExpected);
    EXPECT_LT(vector->instructions_retired_, scalar->instructions_retired_);
    EXPECT_LT(vector->cycle_s_, scalar->cycle_s_);
    RecordProperty("scalar_instructions_retired", scalar->instructions_retired_);
    RecordProperty("scalar_cycles", scalar->cycle_s_);
    RecordProperty("vector_instructions_retired", vector->instructions_retired_);
    RecordProperty("vector_cycles", vector->cycle_s_);
}

TEST(MatrixKernelsTest, CholeskyFactorsExactIntegerSpdMatrix)
{
    const auto setup = [](Kites::MemoryController &memory)
    {
        writeMatrix(memory, kMatrixA, kCholeskyInput);
        writeMatrix(memory, kMatrixL, {});
    };
    auto scalar = runKernel("../examples/cholesky_scalar_4x4.s", setup);
    auto vector = runKernel("../examples/cholesky_4x4.s", setup);

    EXPECT_EQ(readMatrix(scalar->memory_controller_, kMatrixL), kCholeskyExpected);
    EXPECT_EQ(readMatrix(vector->memory_controller_, kMatrixL), kCholeskyExpected);
    EXPECT_LT(vector->instructions_retired_, scalar->instructions_retired_);
    EXPECT_LT(vector->cycle_s_, scalar->cycle_s_);
    RecordProperty("cholesky_scalar_instructions_retired", scalar->instructions_retired_);
    RecordProperty("cholesky_scalar_cycles", scalar->cycle_s_);
    RecordProperty("cholesky_vector_instructions_retired", vector->instructions_retired_);
    RecordProperty("cholesky_vector_cycles", vector->cycle_s_);
}

TEST(MatrixKernelsTest, LuFactorsMatrixWithoutPivoting)
{
    const auto setup = [](Kites::MemoryController &memory)
    {
        writeMatrix(memory, kMatrixA, kLuInput);
        writeMatrix(memory, kMatrixL, {});
        writeMatrix(memory, kMatrixU, {});
    };
    auto scalar = runKernel("../examples/lu_scalar_4x4.s", setup);
    auto vector = runKernel("../examples/lu_4x4.s", setup);

    EXPECT_EQ(readMatrix(scalar->memory_controller_, kMatrixL), kLuExpectedL);
    EXPECT_EQ(readMatrix(scalar->memory_controller_, kMatrixU), kLuExpectedU);
    EXPECT_EQ(readMatrix(vector->memory_controller_, kMatrixL), kLuExpectedL);
    EXPECT_EQ(readMatrix(vector->memory_controller_, kMatrixU), kLuExpectedU);
    EXPECT_LT(vector->instructions_retired_, scalar->instructions_retired_);
    EXPECT_LT(vector->cycle_s_, scalar->cycle_s_);
    RecordProperty("lu_scalar_instructions_retired", scalar->instructions_retired_);
    RecordProperty("lu_scalar_cycles", scalar->cycle_s_);
    RecordProperty("lu_vector_instructions_retired", vector->instructions_retired_);
    RecordProperty("lu_vector_cycles", vector->cycle_s_);
}

} // namespace
