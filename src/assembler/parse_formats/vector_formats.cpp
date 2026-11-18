
/**
 * @file vector_formats.cpp
 * @brief Parser formats for the supported vector instruction subset.
 */

#include "assembler/parser.h"
#include "common/instructions.h"
#include "utils/utils.h"

namespace Kites
{

bool Parser::parse_O_GPR_C_GPR_C_VTYPE()
{
    if (peekToken(1).line_number == currentToken().line_number &&
        peekToken(1).type == TokenType::GP_REGISTER &&
        peekToken(2).line_number == currentToken().line_number &&
        peekToken(2).type == TokenType::COMMA &&
        peekToken(3).line_number == currentToken().line_number &&
        peekToken(3).type == TokenType::GP_REGISTER &&
        peekToken(4).line_number == currentToken().line_number &&
        peekToken(4).type == TokenType::COMMA &&
        peekToken(5).line_number == currentToken().line_number &&
        peekToken(5).type == TokenType::NUM &&
        (peekToken(6).type == TokenType::EOF_ ||
         peekToken(6).line_number != currentToken().line_number))
    {
        ICUnit block;

        block.setOpcode(currentToken().value);
        block.setLineNumber(currentToken().line_number);
        block.setInstructionIndex(instruction_index_);

        block.setRd(reg_alias_to_name.at(peekToken(1).value));
        block.setRs1(reg_alias_to_name.at(peekToken(3).value));
        block.setImm(peekToken(5).value);

        skipCurrentLine();

        intermediate_code_.emplace_back(block, true);

        instruction_number_line_number_mapping_[instruction_index_] =
            block.getLineNumber();

        instruction_index_++;

        return true;
    }

    return false;
}

bool Parser::parse_O_VREG_C_LP_GPR_RP()
{
    if (peekToken(1).line_number == currentToken().line_number &&
        peekToken(1).type == TokenType::VEC_REGISTER &&
        peekToken(2).line_number == currentToken().line_number &&
        peekToken(2).type == TokenType::COMMA &&
        peekToken(3).line_number == currentToken().line_number &&
        peekToken(3).type == TokenType::LPAREN &&
        peekToken(4).line_number == currentToken().line_number &&
        peekToken(4).type == TokenType::GP_REGISTER &&
        peekToken(5).line_number == currentToken().line_number &&
        peekToken(5).type == TokenType::RPAREN &&
        (peekToken(6).type == TokenType::EOF_ ||
         peekToken(6).line_number != currentToken().line_number))
    {
        ICUnit block;

        block.setOpcode(currentToken().value);
        block.setLineNumber(currentToken().line_number);
        block.setInstructionIndex(instruction_index_);

        block.setRd(peekToken(1).value);
        block.setRs1(reg_alias_to_name.at(peekToken(4).value));

        skipCurrentLine();

        intermediate_code_.emplace_back(block, true);

        instruction_number_line_number_mapping_[instruction_index_] =
            block.getLineNumber();

        instruction_index_++;

        return true;
    }

    return false;
}

bool Parser::parse_O_VREG_C_VREG_C_VREG()
{
    if (peekToken(1).line_number == currentToken().line_number &&
        peekToken(1).type == TokenType::VEC_REGISTER &&
        peekToken(2).line_number == currentToken().line_number &&
        peekToken(2).type == TokenType::COMMA &&
        peekToken(3).line_number == currentToken().line_number &&
        peekToken(3).type == TokenType::VEC_REGISTER &&
        peekToken(4).line_number == currentToken().line_number &&
        peekToken(4).type == TokenType::COMMA &&
        peekToken(5).line_number == currentToken().line_number &&
        peekToken(5).type == TokenType::VEC_REGISTER &&
        (peekToken(6).type == TokenType::EOF_ ||
         peekToken(6).line_number != currentToken().line_number))
    {
        ICUnit block;

        block.setOpcode(currentToken().value);
        block.setLineNumber(currentToken().line_number);
        block.setInstructionIndex(instruction_index_);

        block.setRd(peekToken(1).value);
        block.setRs1(peekToken(3).value);
        block.setRs2(peekToken(5).value);

        skipCurrentLine();

        intermediate_code_.emplace_back(block, true);

        instruction_number_line_number_mapping_[instruction_index_] =
            block.getLineNumber();

        instruction_index_++;

        return true;
    }

    return false;
}

bool Parser::parse_O_GPR_C_VREG_C_VREG()
{
    if (peekToken(1).line_number == currentToken().line_number &&
        peekToken(1).type == TokenType::GP_REGISTER &&
        peekToken(2).line_number == currentToken().line_number &&
        peekToken(2).type == TokenType::COMMA &&
        peekToken(3).line_number == currentToken().line_number &&
        peekToken(3).type == TokenType::VEC_REGISTER &&
        peekToken(4).line_number == currentToken().line_number &&
        peekToken(4).type == TokenType::COMMA &&
        peekToken(5).line_number == currentToken().line_number &&
        peekToken(5).type == TokenType::VEC_REGISTER &&
        (peekToken(6).type == TokenType::EOF_ ||
         peekToken(6).line_number != currentToken().line_number))
    {
        ICUnit block;

        block.setOpcode(currentToken().value);
        block.setLineNumber(currentToken().line_number);
        block.setInstructionIndex(instruction_index_);

        block.setRd(reg_alias_to_name.at(peekToken(1).value));
        block.setRs1(peekToken(3).value);
        block.setRs2(peekToken(5).value);

        skipCurrentLine();

        intermediate_code_.emplace_back(block, true);

        instruction_number_line_number_mapping_[instruction_index_] =
            block.getLineNumber();

        instruction_index_++;

        return true;
    }

    return false;
}

} // namespace Kites
