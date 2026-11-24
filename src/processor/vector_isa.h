
#pragma once

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace Kites
{
namespace vector_isa
{

constexpr uint8_t OPCODE_VECTOR       = 0x57;
constexpr uint8_t OPCODE_VECTOR_LOAD  = 0x07;
constexpr uint8_t OPCODE_VECTOR_STORE = 0x27;

constexpr size_t MAX_VECTOR_LANES = 64;

enum class VectorSew
{
    E8,
    E16,
    E32,
    E64
};

enum class VectorLmul
{
    MF8,
    MF4,
    MF2,
    M1,
    M2,
    M4,
    M8
};

struct VectorConfig
{
    VectorSew sew {VectorSew::E64};
    VectorLmul lmul {VectorLmul::M1};
    bool tail_agnostic {false};
    bool mask_agnostic {false};
};

inline uint8_t sewBits(uint16_t vtype)
{
    return static_cast<uint8_t>((vtype >> 3) & 0x7);
}

inline uint8_t lmulBits(uint16_t vtype)
{
    return static_cast<uint8_t>(vtype & 0x7);
}

inline bool tailAgnostic(uint16_t vtype)
{
    return ((vtype >> 6) & 0x1) != 0;
}

inline bool maskAgnostic(uint16_t vtype)
{
    return ((vtype >> 7) & 0x1) != 0;
}

inline bool isMaskedInstruction(uint32_t instruction)
{
    return ((instruction >> 25) & 0x1) == 0;
}

inline bool isMaskSet(const std::vector<uint64_t>& mask, size_t lane)
{
    if (mask.empty())
        return true;
    return ((mask[lane / 64] >> (lane % 64)) & 1) != 0;
}

inline size_t vlmax(const VectorConfig& config)
{
    size_t bits_per_lane = 64;
    switch (config.sew)
    {
    case VectorSew::E8: bits_per_lane = 8; break;
    case VectorSew::E16: bits_per_lane = 16; break;
    case VectorSew::E32: bits_per_lane = 32; break;
    case VectorSew::E64: bits_per_lane = 64; break;
    }

    size_t lmul_numerator = 1;
    size_t lmul_denominator = 1;
    switch (config.lmul)
    {
    case VectorLmul::MF8: lmul_denominator = 8; break;
    case VectorLmul::MF4: lmul_denominator = 4; break;
    case VectorLmul::MF2: lmul_denominator = 2; break;
    case VectorLmul::M1: break;
    case VectorLmul::M2: lmul_numerator = 2; break;
    case VectorLmul::M4: lmul_numerator = 4; break;
    case VectorLmul::M8: lmul_numerator = 8; break;
    }

    const size_t base_lanes =
        (MAX_VECTOR_LANES * 64) / bits_per_lane;
    return std::min(MAX_VECTOR_LANES,
                    base_lanes * lmul_numerator / lmul_denominator);
}

inline VectorConfig decodeVType(uint16_t vtype)
{
    VectorConfig config;

    switch (sewBits(vtype))
    {
    case 0b000: config.sew = VectorSew::E8; break;
    case 0b001: config.sew = VectorSew::E16; break;
    case 0b010: config.sew = VectorSew::E32; break;
    case 0b011: config.sew = VectorSew::E64; break;
    default:
        throw std::invalid_argument("Unsupported vsew encoding");
    }

    switch (lmulBits(vtype))
    {
    case 0b101: config.lmul = VectorLmul::MF8; break;
    case 0b110: config.lmul = VectorLmul::MF4; break;
    case 0b111: config.lmul = VectorLmul::MF2; break;
    case 0b000: config.lmul = VectorLmul::M1; break;
    case 0b001: config.lmul = VectorLmul::M2; break;
    case 0b010: config.lmul = VectorLmul::M4; break;
    case 0b011: config.lmul = VectorLmul::M8; break;
    default:
        throw std::invalid_argument("Reserved vlmul encoding");
    }

    config.tail_agnostic = tailAgnostic(vtype);
    config.mask_agnostic = maskAgnostic(vtype);
    return config;
}

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
    const uint16_t vtype = vtypeImmediate(instruction);
    try
    {
        (void)decodeVType(vtype);
        return true;
    }
    catch (const std::invalid_argument&)
    {
        return false;
    }
}

inline bool isVectorLoad(uint32_t instruction)
{
    return opcode(instruction) == OPCODE_VECTOR_LOAD &&
           funct3(instruction) == 0b011 &&
           ((instruction >> 20) & 0xFFF) == 0x020;
}

inline bool isVectorStore(uint32_t instruction)
{
    return opcode(instruction) == OPCODE_VECTOR_STORE &&
           funct3(instruction) == 0b011 &&
           ((instruction >> 20) & 0xFFF) == 0x020;
}

constexpr uint8_t FUNCT6_VADD = 0b000000;
constexpr uint8_t FUNCT6_VSUB = 0b000010;
constexpr uint8_t FUNCT6_VMUL = 0b100101;
constexpr uint8_t FUNCT6_VDOT = 0b010101;
constexpr uint8_t FUNCT3_VDOT = 0b010;

// RVV floating-point operations with a scalar FPR operand (OPFVF).
constexpr uint8_t FUNCT3_VF = 0b101;
constexpr uint8_t FUNCT6_VFDIV = 0b100000;
constexpr uint8_t FUNCT6_VFNMSAC = 0b101111;


// Unsigned 64-bit vector dot product.
// Preserves modulo-2^64 arithmetic semantics.
inline uint64_t dotProduct(
    const uint64_t *source1,
    const uint64_t *source2,
    size_t vl)
{
    uint64_t accumulator = 0;

    for (size_t lane = 0; lane < vl; ++lane)
    {
        accumulator += source1[lane] * source2[lane];
    }

    return accumulator;
}


enum class VectorOperation
{
    INVALID,
    ADD,
    SUB,
    MUL
};

enum class VectorFloatOperation
{
    INVALID,
    DIV,
    FNMSAC
};

inline bool isVectorFloatArithmetic(uint32_t instruction)
{
    return opcode(instruction) == OPCODE_VECTOR &&
           funct3(instruction) == FUNCT3_VF &&
           (funct6(instruction) == FUNCT6_VFDIV ||
            funct6(instruction) == FUNCT6_VFNMSAC);
}

inline VectorFloatOperation decodeVectorFloat(uint32_t instruction)
{
    if (!isVectorFloatArithmetic(instruction))
        return VectorFloatOperation::INVALID;

    switch (funct6(instruction))
    {
    case FUNCT6_VFDIV:
        return VectorFloatOperation::DIV;
    case FUNCT6_VFNMSAC:
        return VectorFloatOperation::FNMSAC;
    default:
        return VectorFloatOperation::INVALID;
    }
}

inline bool isVectorDot(uint32_t instruction)
{
    return opcode(instruction) == OPCODE_VECTOR &&
           funct3(instruction) == FUNCT3_VDOT &&
           funct6(instruction) == FUNCT6_VDOT;
}

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
