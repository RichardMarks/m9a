//
// Created by Richard Marks on 5/8/26.
//
#include <iostream>
#include <format>
#include <functional>

#include "token_stream_assembler.h"

m9::TokenStreamAssembler::TokenStreamAssembler(const std::vector<Token> &raw_token_stream)
{
  token_stream = std::make_unique<TokenStream>(raw_token_stream);

  // first pass to handle directives and labels
  pass = Pass::Pass1;
  token_stream->Reset();
  ExecutePass();

  // second pass
  pass = Pass::Pass2;
  token_stream->Reset();
  ExecutePass();
}

void m9::TokenStreamAssembler::Emit8(const uint8_t value)
{
  if (pass == Pass::Pass2)
  {
    std::cerr << std::format("Emit8: 0x{:02X}", value) << std::endl;
    bytes.push_back(value);
  }
  program_counter += 1;
}

void m9::TokenStreamAssembler::Emit16(const uint16_t value)
{
  if (pass == Pass::Pass2)
  {
    std::cerr << std::format("Emit16: 0x{:02X}", value) << std::endl;
    std::array<uint8_t, 2> t_bytes{};
    std::memcpy(&t_bytes[0], &value, sizeof(value));
    for (const auto byte: t_bytes)
    {
      std::cerr << std::format("  -> Emit8: 0x{:02X}", byte) << std::endl;
      bytes.push_back(byte);
    }
  }
  program_counter += 2;
}

void m9::TokenStreamAssembler::Emit32(const uint32_t value)
{
  if (pass == Pass::Pass2)
  {
    std::cerr << std::format("Emit32: 0x{:04X}", value) << std::endl;
    std::array<uint8_t, 4> t_bytes{};
    std::memcpy(&t_bytes[0], &value, sizeof(value));
    for (const auto byte: t_bytes)
    {
      std::cerr << std::format("  -> Emit8: 0x{:02X}", byte) << std::endl;
      bytes.push_back(byte);
    }
  }
  program_counter += 4;
}

void m9::TokenStreamAssembler::EmitTypeO(const OpCode op_code)
{
  Emit8(static_cast<uint8_t>(op_code));
}

void m9::TokenStreamAssembler::EmitTypeOSRA(const OpCode op_code, const uint8_t sz, const uint8_t reg,
                                            const uint16_t address)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(sz);
  Emit8(reg);
  Emit16(address);
}

void m9::TokenStreamAssembler::EmitTypeOSAR(const OpCode op_code, const uint8_t sz, const uint16_t address,
                                            const uint8_t reg)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(sz);
  Emit16(address);
  Emit8(reg);
}

void m9::TokenStreamAssembler::EmitTypeOSRI1(const OpCode op_code, const uint8_t reg, const uint8_t imm)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(1);
  Emit8(reg);
  Emit8(imm);
}

void m9::TokenStreamAssembler::EmitTypeOSRI2(const OpCode op_code, const uint8_t reg, const uint16_t imm)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(2);
  Emit8(reg);
  Emit16(imm);
}

void m9::TokenStreamAssembler::EmitTypeOSRI4(const OpCode op_code, const uint8_t reg, const uint32_t imm)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(4);
  Emit8(reg);
  Emit32(imm);
}

void m9::TokenStreamAssembler::EmitTypeOSRR(const OpCode op_code, const uint8_t sz, const uint8_t reg_a,
                                            const uint8_t reg_b)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(sz);
  Emit8(reg_a);
  Emit8(reg_b);
}

void m9::TokenStreamAssembler::EmitTypeOSR(const OpCode op_code, const uint8_t sz, const uint8_t reg)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(sz);
  Emit8(reg);
}

void m9::TokenStreamAssembler::EmitTypeORR(const OpCode op_code, const uint8_t reg_a, const uint8_t reg_b)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(reg_a);
  Emit8(reg_b);
}

void m9::TokenStreamAssembler::EmitTypeOA(const OpCode op_code, const uint16_t address)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit16(address);
}

void m9::TokenStreamAssembler::EmitTypeOAR(const OpCode op_code, const uint16_t address, const uint8_t reg)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit16(address);
  Emit8(reg);
}

void m9::TokenStreamAssembler::EmitTypeOARR(const OpCode op_code, const uint16_t address, const uint8_t reg_a,
                                            const uint8_t reg_b)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit16(address);
  Emit8(reg_a);
  Emit8(reg_b);
}

void m9::TokenStreamAssembler::EmitTypeORRR(const OpCode op_code, const uint8_t reg_a, const uint8_t reg_b,
                                            const uint8_t reg_c)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(reg_a);
  Emit8(reg_b);
  Emit8(reg_c);
}

void m9::TokenStreamAssembler::EmitTypeORC(const OpCode op_code, const uint8_t reg, const uint8_t count)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(reg);
  Emit8(count);
}

void m9::TokenStreamAssembler::EmitTypeOR(const OpCode op_code, const uint8_t reg)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(reg);
}

void m9::TokenStreamAssembler::EmitTypeORD(const OpCode op_code, const uint8_t reg, const SignExtendSourceBpp sx_bpp)
{
  Emit8(static_cast<uint8_t>(op_code));
  Emit8(reg);
  Emit8(static_cast<uint8_t>(sx_bpp));
}

void m9::TokenStreamAssembler::ValidateRegister(const uint8_t reg)
{
  if (reg > 0x07)
  {
    throw std::runtime_error("Invalid register: " + std::to_string(reg));
  }
}

void m9::TokenStreamAssembler::ExpectTokenType(const TokenType expected_type, const Token &token)
{
  if (token.token_type != expected_type)
  {
    throw std::runtime_error(
      "ExpectTokenType: Unexpected token type: " + TokenTypeStr(expected_type) + " with value: " + token.value);
  }
}

void m9::TokenStreamAssembler::ExpectAnotherToken() const
{
  if (const auto peek = token_stream->Peek(); !peek)
  {
    throw std::runtime_error("Unexpected end of token stream");
  }
}

uint8_t m9::TokenStreamAssembler::GetRegisterOperand() const
{
  ExpectAnotherToken();
  const auto op_reg = token_stream->Consume();
  ExpectTokenType(TokenType::OPERAND, *op_reg);
  const auto reg = op_reg->num_value.value();
  ValidateRegister(reg);
  return reg;
}

uint32_t m9::TokenStreamAssembler::GetImmediateOperand() const
{
  ExpectAnotherToken();
  const auto op_imm = token_stream->Consume();
  ExpectTokenType(TokenType::OPERAND, *op_imm);
  const auto imm = op_imm->num_value.value();
  return imm;
}

uint8_t m9::TokenStreamAssembler::GetIndirectRegisterOperand() const
{
  ExpectAnotherToken();
  const auto op_reg = token_stream->Consume();
  // ExpectTokenType(TokenType::UNKNOWN, *op_reg);
  if (op_reg->token_context_type != TokenType::ADDRESS)
  {
    throw std::runtime_error(
      "GetIndirectRegisterOperand: Unexpected token context type: " + TokenTypeStr(op_reg->token_context_type));
  }
  auto reg = op_reg->num_value.value_or(0xFF);
  if (reg > 0x07 && reg != 0xFF)
  {
    reg -= 128;
  }
  ValidateRegister(reg);
  return reg;
}

uint16_t m9::TokenStreamAssembler::GetAddressOperand() const
{
  ExpectAnotherToken();

  const auto op_addr = token_stream->Consume();
  auto address = 0ul;
  if (op_addr->token_type == TokenType::UNKNOWN)
  {
    if (pass == Pass::Pass2)
    {
      std::cerr << "Unknown Token - checking for known label address for: " << op_addr->value << std::endl;
      if (label_addresses.contains(op_addr->value))
      {
        address = label_addresses.at(op_addr->value);
        std::cerr << std::format("Label address: 0x{:04X}", address) << std::endl;
      }
    }
  } else if (op_addr->token_type == TokenType::OPERAND)
  {
    if (op_addr->token_context_type == TokenType::ADDRESS)
    {
      address = op_addr->num_value.value_or(0);
      std::cerr << std::format("Immediate address: 0x{:04X}", address) << std::endl;
    } else
    {
      throw std::runtime_error("GetAddressOperand: Unexpected token type: " + TokenTypeStr(op_addr->token_type));
    }
  } else
  {
    throw std::runtime_error("GetAddressOperand: Unexpected token type: " + TokenTypeStr(op_addr->token_type));
  }
  return address;
}

void m9::TokenStreamAssembler::HandleDirective(const Token &current_token)
{
  const auto directive = current_token.value;

  const std::unordered_map<std::string, std::function<void()> > handlers{
    {
      "start", [&]()
      {
        const auto address = GetAddressOperand();
        start_address = address;
      }
    },
    {
      "byte", [&]()
      {
        // consume tokens while type is Operand and there are still tokens
        while (true)
        {
          if (!token_stream->Peek() || token_stream->Peek()->token_type != TokenType::OPERAND)
          {
            break;
          }
          const auto op_byte = token_stream->Consume();
          const auto byte = op_byte->num_value.value_or(0);
          Emit8(byte);
        }
      }
    }
  };

  const auto Handler = handlers.contains(directive) ? handlers.at(directive) : nullptr;
  if (!Handler)
  {
    throw std::runtime_error("Unknown directive: " + directive);
  }
  Handler();
}

void m9::TokenStreamAssembler::HandleMnemonic(const Token &current_token)
{
  const auto mnemonic = current_token.value;

  const std::unordered_map<std::string, std::function<void()> > handlers{
    {
      "no", [&]()
      {
        EmitTypeO(OpCode::NO);
      }
    },
    {
      "he",
      [&]()
      {
        EmitTypeO(OpCode::HE);
      }
    },
    {
      "wf",
      [&]()
      {
        EmitTypeO(OpCode::WF);
      }
    },
    {
      "so",
      [&]()
      {
        EmitTypeO(OpCode::SO);
      }
    },
    {
      "ld", [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto address = GetAddressOperand();
        auto sz = 4;
        if (mnemonic.ends_with(".b"))
        {
          sz = 1;
        } else if (mnemonic.ends_with(".w"))
        {
          sz = 2;
        }
        EmitTypeOSRA(OpCode::LD, sz, reg, address);
      }
    },
    {
      "ln", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetIndirectRegisterOperand();
        auto sz = 4;
        if (mnemonic.ends_with(".b"))
        {
          sz = 1;
        } else if (mnemonic.ends_with(".w"))
        {
          sz = 2;
        }
        EmitTypeOSRR(OpCode::LN, sz, reg_a, reg_b);
      }
    },
    {
      "st", [&]()
      {
        const auto address = GetAddressOperand();
        const auto reg = GetRegisterOperand();
        auto sz = 4;
        if (mnemonic.ends_with(".b"))
        {
          sz = 1;
        } else if (mnemonic.ends_with(".w"))
        {
          sz = 2;
        }
        EmitTypeOSAR(OpCode::ST, sz, address, reg);
      }
    },
    {
      "sn", [&]()
      {
        const auto reg_a = GetIndirectRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        auto sz = 4;
        if (mnemonic.ends_with(".b"))
        {
          sz = 1;
        } else if (mnemonic.ends_with(".w"))
        {
          sz = 2;
        }
        EmitTypeOSRR(OpCode::SN, sz, reg_a, reg_b);
      }
    },
    {
      "mv",
      [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto imm = GetImmediateOperand();

        if (mnemonic.ends_with(".b"))
        {
          // TYPE_OSRI1
          EmitTypeOSRI1(OpCode::MV, reg, static_cast<uint8_t>(imm));
        } else if (mnemonic.ends_with(".w"))
        {
          // TYPE_OSRI2
          EmitTypeOSRI2(OpCode::MV, reg, static_cast<uint16_t>(imm));
        } else
        {
          // TYPE_OSRI4 by default
          EmitTypeOSRI4(OpCode::MV, reg, imm);
        }
      }
    },
    {
      "cp", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        auto sz = 4;
        if (mnemonic.ends_with(".b"))
        {
          sz = 1;
        } else if (mnemonic.ends_with(".w"))
        {
          sz = 2;
        }
        EmitTypeOSRR(OpCode::CP, sz, reg_a, reg_b);
      }
    },
    {
      "pu", [&]()
      {
        const auto reg = GetRegisterOperand();
        auto sz = 4;
        if (mnemonic.ends_with(".b"))
        {
          sz = 1;
        } else if (mnemonic.ends_with(".w"))
        {
          sz = 2;
        }
        EmitTypeOSR(OpCode::PU, sz, reg);
      }
    },
    {
      "po", [&]()
      {
        const auto reg = GetRegisterOperand();
        auto sz = 4;
        if (mnemonic.ends_with(".b"))
        {
          sz = 1;
        } else if (mnemonic.ends_with(".w"))
        {
          sz = 2;
        }
        EmitTypeOSR(OpCode::PO, sz, reg);
      }
    },
    {
      "xr", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        auto sz = 4;
        if (mnemonic.ends_with(".b"))
        {
          sz = 1;
        } else if (mnemonic.ends_with(".w"))
        {
          sz = 2;
        }
        EmitTypeOSRR(OpCode::XR, sz, reg_a, reg_b);
      }
    },
    //
    // MATH & LOGIC OPS
    //
    {
      "nd", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        const auto reg_c = GetRegisterOperand();
        EmitTypeORRR(OpCode::ND, reg_a, reg_b, reg_c);
      }
    },
    {
      "or", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        const auto reg_c = GetRegisterOperand();
        EmitTypeORRR(OpCode::OR, reg_a, reg_b, reg_c);
      }
    },
    {
      "eo", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        const auto reg_c = GetRegisterOperand();
        EmitTypeORRR(OpCode::EO, reg_a, reg_b, reg_c);
      }
    },
    {
      "nt", [&]()
      {
        const auto reg = GetRegisterOperand();
        EmitTypeOR(OpCode::NT, reg);
      }
    },
    {
      "sl", [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto count = GetImmediateOperand();
        EmitTypeORC(OpCode::SL, reg, static_cast<uint8_t>(count));
      }
    },
    {
      "sr", [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto count = GetImmediateOperand();
        EmitTypeORC(OpCode::SR, reg, static_cast<uint8_t>(count));
      }
    },
    {
      "ar", [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto count = GetImmediateOperand();
        EmitTypeORC(OpCode::AR, reg, static_cast<uint8_t>(count));
      }
    },
    {
      "ad", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        const auto reg_c = GetRegisterOperand();
        EmitTypeORRR(OpCode::AD, reg_a, reg_b, reg_c);
      }
    },
    {
      "ai", [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto imm = GetImmediateOperand();
        if (mnemonic.ends_with(".b"))
        {
          EmitTypeOSRI1(OpCode::AI, reg, static_cast<uint8_t>(imm));
        } else if (mnemonic.ends_with(".w"))
        {
          EmitTypeOSRI2(OpCode::AI, reg, static_cast<uint16_t>(imm));
        } else
        {
          EmitTypeOSRI4(OpCode::AI, reg, imm);
        }
      }
    },
    {
      "sb", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        const auto reg_c = GetRegisterOperand();
        EmitTypeORRR(OpCode::SB, reg_a, reg_b, reg_c);
      }
    },
    {
      "si", [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto imm = GetImmediateOperand();
        if (mnemonic.ends_with(".b"))
        {
          EmitTypeOSRI1(OpCode::SI, reg, static_cast<uint8_t>(imm));
        } else if (mnemonic.ends_with(".w"))
        {
          EmitTypeOSRI2(OpCode::SI, reg, static_cast<uint16_t>(imm));
        } else
        {
          EmitTypeOSRI4(OpCode::SI, reg, imm);
        }
      }
    },
    {
      "mu", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        const auto reg_c = GetRegisterOperand();
        EmitTypeORRR(OpCode::MU, reg_a, reg_b, reg_c);
      }
    },
    {
      "mi", [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto imm = GetImmediateOperand();
        if (mnemonic.ends_with(".b"))
        {
          EmitTypeOSRI1(OpCode::MI, reg, static_cast<uint8_t>(imm));
        } else if (mnemonic.ends_with(".w"))
        {
          EmitTypeOSRI2(OpCode::MI, reg, static_cast<uint16_t>(imm));
        } else
        {
          EmitTypeOSRI4(OpCode::MI, reg, imm);
        }
      }
    },
    {
      "dv", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        const auto reg_c = GetRegisterOperand();
        EmitTypeORRR(OpCode::DV, reg_a, reg_b, reg_c);
      }
    },
    {
      "di", [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto imm = GetImmediateOperand();
        if (mnemonic.ends_with(".b"))
        {
          EmitTypeOSRI1(OpCode::DI, reg, static_cast<uint8_t>(imm));
        } else if (mnemonic.ends_with(".w"))
        {
          EmitTypeOSRI2(OpCode::DI, reg, static_cast<uint16_t>(imm));
        } else
        {
          EmitTypeOSRI4(OpCode::DI, reg, imm);
        }
      }
    },
    {
      "dq", [&]()
      {
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        const auto reg_c = GetRegisterOperand();
        EmitTypeORRR(OpCode::DQ, reg_a, reg_b, reg_c);
      }
    },
    {
      "qi", [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto imm = GetImmediateOperand();
        if (mnemonic.ends_with(".b"))
        {
          EmitTypeOSRI1(OpCode::QI, reg, static_cast<uint8_t>(imm));
        } else if (mnemonic.ends_with(".w"))
        {
          EmitTypeOSRI2(OpCode::QI, reg, static_cast<uint16_t>(imm));
        } else
        {
          EmitTypeOSRI4(OpCode::QI, reg, imm);
        }
      }
    },
    {
      "ir", [&]()
      {
        const auto reg = GetRegisterOperand();
        EmitTypeOR(OpCode::IR, reg);
      }
    },
    {
      "dr", [&]()
      {
        const auto reg = GetRegisterOperand();
        EmitTypeOR(OpCode::DR, reg);
      }
    },
    {
      "sx", [&]()
      {
        const auto reg = GetRegisterOperand();
        const auto source_size = GetImmediateOperand();
        SignExtendSourceBpp sx_bpp;
        if (source_size == static_cast<uint8_t>(SignExtendSourceBpp::SX8))
        {
          sx_bpp = SignExtendSourceBpp::SX8;
        } else if (source_size == static_cast<uint8_t>(SignExtendSourceBpp::SX16))
        {
          sx_bpp = SignExtendSourceBpp::SX16;
        } else
        {
          throw std::runtime_error(std::format("Invalid Source Size: {} != 1 or 2", source_size));
        }
        EmitTypeORD(OpCode::SX, reg, sx_bpp);
      }
    },
    //
    // CONTROL FLOW OPS
    //
    {
      "uj",
      [&]()
      {
        const auto address = GetAddressOperand();
        EmitTypeOA(OpCode::UJ, address);
      }
    },
    {
      "zj", [&]()
      {
        const auto address = GetAddressOperand();
        const auto reg = GetRegisterOperand();
        EmitTypeOAR(OpCode::ZJ, address, reg);
      }
    },
    {
      "nj", [&]()
      {
        const auto address = GetAddressOperand();
        const auto reg = GetRegisterOperand();
        EmitTypeOAR(OpCode::NJ, address, reg);
      }
    },
    {
      "ls", [&]()
      {
        const auto address = GetAddressOperand();
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        EmitTypeOARR(OpCode::LS, address, reg_a, reg_b);
      }
    },
    {
      "lu", [&]()
      {
        const auto address = GetAddressOperand();
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        EmitTypeOARR(OpCode::LU, address, reg_a, reg_b);
      }
    },
    {
      "ej", [&]()
      {
        const auto address = GetAddressOperand();
        const auto reg_a = GetRegisterOperand();
        const auto reg_b = GetRegisterOperand();
        EmitTypeOARR(OpCode::EJ, address, reg_a, reg_b);
      }
    },
    {
      "pj", [&]()
      {
        const auto address = GetAddressOperand();
        EmitTypeOA(OpCode::PJ, address);
      }
    },
    {
      "jp", [&]()
      {
        EmitTypeO(OpCode::JP);
      }
    },
  };

  const auto Handler = handlers.contains(mnemonic) ? handlers.at(mnemonic) : nullptr;
  if (!Handler)
  {
    // possibly a suffixed mnemonic
    const auto without_suffix = mnemonic.substr(0, mnemonic.size() - 2);
    if (const auto Handler2 = handlers.contains(without_suffix) ? handlers.at(without_suffix) : nullptr)
    {
      Handler2();
      return;
    }
    throw std::runtime_error("Unknown mnemonic: " + mnemonic);
  }
  Handler();
}

void m9::TokenStreamAssembler::HandleToken(const Token &current_token)
{
  switch (const auto token_type = current_token.token_type)
  {
  case TokenType::DIRECTIVE: HandleDirective(current_token);
    break;
  case TokenType::MNEMONIC: HandleMnemonic(current_token);
    break;
  case TokenType::LABEL:
  {
    const auto &label = current_token.value;
    if (pass == Pass::Pass1)
    {
      if (label_addresses.contains(label))
      {
        throw std::runtime_error("Duplicate label name: " + label);
      }
      label_addresses.try_emplace(label, program_counter);
    }
  }
  break;
  default:
  {
    throw std::runtime_error("HandleToken: Unexpected token type: " + TokenTypeStr(token_type));
  }
  }
}

void m9::TokenStreamAssembler::ExecutePass()
{
  program_counter = start_address;
  const auto stream_size = token_stream->GetSize();
  for (auto i = 0ull; i < stream_size; i++)
  {
    if (token_stream->GetOffset() >= token_stream->GetSize())
    {
      break;
    }
    const auto next_token = token_stream->Consume();
    if (!next_token)
    {
      throw std::runtime_error("Unexpected null token");
    }

    if (pass == Pass::Pass2)
    {
      const auto token_type = next_token->token_type;
      const auto tts = TokenTypeStr(token_type);
      std::cerr << std::format("{:08X}: {:>10}: {:64} @0x{:04X}", i, tts, next_token->value, program_counter) <<
        std::endl;
    }

    try
    {
      HandleToken(*next_token);
    } catch ([[maybe_unused]] std::exception &e)
    {
      std::cerr << std::format("ExecutePass Error at token {:d}/{:d}", (1 + token_stream->GetOffset()),
                               token_stream->GetSize()) << std::endl;
      throw;
    }
    i = token_stream->GetOffset();
  }
}
