
#include <gtest/gtest.h>

#include <array>
#include <bit>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "assembler/assembler.h"
#include "processor/rvss/rvss_processor.h"
#include "utils/utils.h"

namespace {

constexpr std::uint64_t A_BASE = 0x2000;
constexpr std::uint64_t L_BASE = 0x2200;
constexpr int N = 8;

using Matrix = std::array<double, N * N>;

// Input matrix A, stored in column-major order.
const Matrix INPUT_MATRIX = {
    16,  8,  8,  8,  8,  8,  8,  8,
     8, 29, -1, -1, -1, -1, -1, -1,
     8, -1, 14,  8,  8,  8,  8,  8,
     8, -1,  8, 10, 10, 10, 10, 10,
     8, -1,  8, 10, 19,  7,  7,  7,
     8, -1,  8, 10,  7, 27, 15, 15,
     8, -1,  8, 10,  7, 15, 16, 16,
     8, -1,  8, 10,  7, 15, 16, 25
};

// Expected lower triangular factor L, column-major.
const Matrix EXPECTED_L = {
    4,  2,  2,  2,  2,  2,  2,  2,
    0,  5, -1, -1, -1, -1, -1, -1,
    0,  0,  3,  1,  1,  1,  1,  1,
    0,  0,  0,  2,  2,  2,  2,  2,
    0,  0,  0,  0,  3, -1, -1, -1,
    0,  0,  0,  0,  0,  4,  1,  1,
    0,  0,  0,  0,  0,  0,  2,  2,
    0,  0,  0,  0,  0,  0,  0,  3
};

std::filesystem::path locateProgram(const std::string& filename) {
    const auto current = std::filesystem::current_path();

    const std::array<std::filesystem::path, 5> candidates = {
        current / "test/vector_tests/programs" / filename,
        current / "vector_tests/programs" / filename,
        current / "../test/vector_tests/programs" / filename,
        current / "../../test/vector_tests/programs" / filename,
        std::filesystem::path(__FILE__).parent_path() / "programs" / filename
    };

    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) {
            return std::filesystem::absolute(path);
        }
    }

    throw std::runtime_error(
        "Assembly file not found: " + filename);
}

void initializeMatrix(Kites::RVSSProcessor& processor) {
    for (int col = 0; col < N; ++col) {
        for (int row = 0; row < N; ++row) {
            const auto index = col * N + row;
            const auto address = A_BASE +
                static_cast<std::uint64_t>(index * sizeof(double));

            processor.memory_controller_.writeDoubleWord_d(
                address,
                std::bit_cast<std::uint64_t>(INPUT_MATRIX[index]));
        }
    }

    // Clear the output matrix.
    for (int i = 0; i < N * N; ++i) {
        processor.memory_controller_.writeDoubleWord_d(
            L_BASE + static_cast<std::uint64_t>(i * sizeof(double)),
            std::bit_cast<std::uint64_t>(0.0));
    }
}

struct KernelResult {
    Matrix factor{};
};

KernelResult runKernel(const std::string& filename) {
    Kites::setupVmStateDirectory();

    auto processor = std::make_unique<Kites::RVSSProcessor>();

    initializeMatrix(*processor);

    const auto programPath = locateProgram(filename);

    std::ifstream source(programPath);
    if (!source.is_open()) {
        throw std::runtime_error(
            "Unable to open assembly file: " + programPath.string());
    }

    // assemble() accepts an input stream.
    auto machineCode = Kites::assemble(source);

    processor->LoadProgram(machineCode);
    processor->DebugRun();

    KernelResult result;

    for (int col = 0; col < N; ++col) {
        for (int row = 0; row < N; ++row) {
            const auto index = col * N + row;
            const auto address = L_BASE +
                static_cast<std::uint64_t>(index * sizeof(double));

            const auto bits =
                processor->memory_controller_.readDoubleWord(address);

            result.factor[index] = std::bit_cast<double>(bits);
        }
    }

    return result;
}

void expectCorrectFactor(const Matrix& actual) {
    constexpr double tolerance = 1e-9;

    for (int col = 0; col < N; ++col) {
        for (int row = 0; row < N; ++row) {
            const int index = col * N + row;

            EXPECT_NEAR(
                actual[index],
                EXPECTED_L[index],
                tolerance
            ) << "Incorrect L[" << row << "][" << col << "]";
        }
    }
}

} // namespace

TEST(RVSSCholeskyTest, ScalarKernelProducesExpectedFactor) {
    const auto result = runKernel("cholesky_scalar_rvss.s");

    expectCorrectFactor(result.factor);

    std::cout << "\n8x8 scalar Cholesky test passed.\n";
}

TEST(RVSSCholeskyTest, VectorKernelProducesExpectedFactor) {
    const auto result = runKernel("cholesky_vector_rvss.s");

    expectCorrectFactor(result.factor);

    std::cout << "\n8x8 vector Cholesky test passed.\n";
}

TEST(RVSSCholeskyTest, ScalarAndVectorProduceSameFactor) {
    const auto scalar = runKernel("cholesky_scalar_rvss.s");
    const auto vector = runKernel("cholesky_vector_rvss.s");

    for (int i = 0; i < N * N; ++i) {
        EXPECT_NEAR(
            scalar.factor[i],
            vector.factor[i],
            1e-9
        ) << "Scalar/vector mismatch at element " << i;
    }

    std::cout << "\n8x8 scalar/vector comparison passed.\n";
}
