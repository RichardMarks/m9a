//
// Created by Richard Marks on 5/8/26.
//
#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include "opcode.h"
#include "token_stream.h"

namespace m9
{
  // FIXME: this probably should be something that is passed into the assembler as a cmdline def
  constexpr auto MM_MEMORY_BANK_1 = 0x19E4;

  struct TokenStreamAssembler
  {
    enum class SignExtendSourceBpp
    {
      SX8 = 1,
      SX16 = 2
    };

    enum class Pass
    {
      Pass1,
      Pass2,
    };

    Pass pass{Pass::Pass1};

    uint16_t start_address{MM_MEMORY_BANK_1};
    uint16_t program_counter{0x0000};

    std::vector<uint8_t> bytes{};
    std::unique_ptr<TokenStream> token_stream{nullptr};
    std::unordered_map<std::string, uint16_t> label_addresses{};

    size_t instruction_count{0};

    explicit TokenStreamAssembler(const std::vector<Token> &raw_token_stream);

    void Emit8(uint8_t value);

    void Emit16(uint16_t value);

    void Emit32(uint32_t value);

    void EmitTypeO(OpCode op_code);

    void EmitTypeOSRA(OpCode op_code, uint8_t sz, uint8_t reg, uint16_t address);

    void EmitTypeOSAR(OpCode op_code, uint8_t sz, uint16_t address, uint8_t reg);

    void EmitTypeOSRI1(OpCode op_code, uint8_t reg, uint8_t imm);

    void EmitTypeOSRI2(OpCode op_code, uint8_t reg, uint16_t imm);

    void EmitTypeOSRI4(OpCode op_code, uint8_t reg, uint32_t imm);

    void EmitTypeOSRR(OpCode op_code, uint8_t sz, uint8_t reg_a, uint8_t reg_b);

    void EmitTypeOSR(OpCode op_code, uint8_t sz, uint8_t reg);

    void EmitTypeORR(OpCode op_code, uint8_t reg_a, uint8_t reg_b);

    void EmitTypeOA(OpCode op_code, uint16_t address);

    void EmitTypeOAR(OpCode op_code, uint16_t address, uint8_t reg);

    void EmitTypeOARR(OpCode op_code, uint16_t address, uint8_t reg_a, uint8_t reg_b);

    void EmitTypeORRR(OpCode op_code, uint8_t reg_a, uint8_t reg_b, uint8_t reg_c);

    void EmitTypeORC(OpCode op_code, uint8_t reg, uint8_t count);

    void EmitTypeOR(OpCode op_code, uint8_t reg);

    void EmitTypeORD(OpCode op_code, uint8_t reg, SignExtendSourceBpp sx_bpp);

    static void ValidateRegister(uint8_t reg);

    static void ExpectTokenType(TokenType expected_type, const Token &token);

    void ExpectAnotherToken() const;

    [[nodiscard]] uint8_t GetRegisterOperand() const;

    [[nodiscard]] uint32_t GetImmediateOperand() const;

    [[nodiscard]] uint8_t GetIndirectRegisterOperand() const;

    [[nodiscard]] uint16_t GetAddressOperand() const;

    void HandleDirective(const Token &current_token);

    void HandleMnemonic(const Token &current_token);

    void HandleToken(const Token &current_token);

    void ExecutePass();
  };
}
