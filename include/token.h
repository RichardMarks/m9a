//
// Created by Richard Marks on 5/8/26.
//
#pragma once

#include <string>
#include <optional>

namespace m9
{
  enum class TokenType
  {
    UNKNOWN,
    LABEL,
    MNEMONIC,
    OPERAND,
    DIRECTIVE,

    REGISTER,
    ADDRESS,
    IMMEDIATE,
  };

  struct Token
  {
    TokenType token_type{TokenType::UNKNOWN};
    TokenType token_context_type{TokenType::UNKNOWN};
    std::string value{};
    std::optional<uint32_t> num_value{};
  };

  static auto TokenTypeStr(const TokenType &token_type) -> std::string
  {
    switch (token_type)
    {
    case TokenType::DIRECTIVE:
      return "Directive";
    case TokenType::UNKNOWN:
      return "Unknown";
    case TokenType::LABEL:
      return "Label";
    case TokenType::MNEMONIC:
      return "Mnemonic";
    case TokenType::OPERAND:
      return "Operand";
    case TokenType::ADDRESS:
      return "Address";
    case TokenType::REGISTER:
      return "Register";
    case TokenType::IMMEDIATE:
      return "Immediate";
    default: break;
    }
    return "";
  }

}