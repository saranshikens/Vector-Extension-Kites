#pragma once

#include <cstdint>
#include <cstddef>

namespace Kites
{
namespace vector_isa
{

// Vector instruction opcodes
constexpr uint8_t OPCODE_VECTOR       = 0x57;
constexpr uint8_t OPCODE_VECTOR_LOAD  = 0x07;
constexpr uint8_t OPCODE_VECTOR_STORE = 0x27;
constexpr uint8_t VTYPE_E64_M1 = 0x18;
constexpr size_t MAX_VECTOR_LANES = 64;

// Instruction field extraction
inline uint8_t opcode(uint32_t instruction)
{
    return instruction & 0x7F;
}

inline uint8_t vd(uint32_t instruction)
{
    return (instruction >> 7) & 0x1F;
}

inline uint8_t funct3(uint32_t instruction)
{
    return (instruction >> 12) & 0x07;
}

inline uint8_t vs1(uint32_t instruction)
{
    return (instruction >> 15) & 0x1F;
}

inline uint8_t vs2(uint32_t instruction)
{
    return (instruction >> 20) & 0x1F;
}

inline uint8_t funct6(uint32_t instruction)
{
    return (instruction >> 26) & 0x3F;
}

// Instruction classification
inline bool isVectorArithmetic(uint32_t instruction)
{
    return opcode(instruction) == OPCODE_VECTOR &&
           funct3(instruction) == 0b000;
}

inline bool isVectorConfiguration(uint32_t instruction)
{
    return opcode(instruction) == OPCODE_VECTOR &&
           funct3(instruction) == 0b111;
}

inline uint16_t vtypeImmediate(uint32_t instruction)
{
    return static_cast<uint16_t>((instruction >> 20) & 0x7FF);
}

inline bool isSupportedVType(uint32_t instruction)
{
    return vtypeImmediate(instruction) == VTYPE_E64_M1;
}

inline bool isVectorLoad(uint32_t instruction)
{
    return opcode(instruction) == OPCODE_VECTOR_LOAD &&
           funct3(instruction) == 0b011;
}

inline bool isVectorStore(uint32_t instruction)
{
    return opcode(instruction) == OPCODE_VECTOR_STORE &&
           funct3(instruction) == 0b011;
}

// Supported arithmetic operations
constexpr uint8_t FUNCT6_VADD = 0b000000;
constexpr uint8_t FUNCT6_VSUB = 0b000010;
constexpr uint8_t FUNCT6_VMUL = 0b100101;

enum class VectorOperation
{
    INVALID,
    ADD,
    SUB,
    MUL
};

inline VectorOperation decodeArithmetic(uint32_t instruction)
{
    if (!isVectorArithmetic(instruction))
        return VectorOperation::INVALID;

    switch (funct6(instruction))
    {
    case FUNCT6_VADD:
        return VectorOperation::ADD;

    case FUNCT6_VSUB:
        return VectorOperation::SUB;

    case FUNCT6_VMUL:
        return VectorOperation::MUL;

    default:
        return VectorOperation::INVALID;
    }
}

} // namespace vector_isa
} // namespace Kites