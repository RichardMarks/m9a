//
// Created by Richard Marks on 4/27/26.
//
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace m9
{
  class ExpressionParser
  {
    const std::unordered_map<std::string, std::string> &symbols;
    std::vector<std::string> tokens{};
    size_t pos{0};

    auto GetSymbolValue(const std::string &s) const -> int;

    std::string Peek() const;

    std::string Consume();

    auto ParseLogicalOR() -> bool;

    auto ParseLogicalAND() -> bool;

    auto ParseComparison() -> bool;

    auto ParseMathAdditive() -> int;

    auto ParseMathMultiplicative() -> int;

    auto ParseUnary() -> int;

    auto ParsePrimary() -> int;

  public:
    ExpressionParser(std::string_view expr, const std::unordered_map<std::string, std::string> &syms);

    [[nodiscard]] auto Evaluate() -> bool;
  };
}
