//
// Created by Richard Marks on 4/27/26.
//
#include "expression_parser.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_set>

#define VERBOSE_DEBUGGING 1
#undef VERBOSE_DEBUGGING

constexpr auto OPEN_PARENTHESIS = "(";
constexpr auto CLOSE_PARENTHESIS = ")";

static auto IsEqualityComparisonOperator(const auto &op) -> bool
{
  return op == "eq" || op == "==" || op == "equals";
}

static auto IsLessComparisonOperator(const auto &op) -> bool
{
  return op == "lt" || op == "<";
}

static auto IsGreaterComparisonOperator(const auto &op) -> bool
{
  return op == "gt" || op == ">";
}

static auto IsComparisonOperator(const auto &op) -> bool
{
  return IsEqualityComparisonOperator(op) || IsLessComparisonOperator(op) || IsGreaterComparisonOperator(op);
}

static auto IsMathAddOperator(const auto &op) -> bool
{
  return op == "plus" || op == "+" || op == "add";
}

static auto IsMathSubtractOperator(const auto &op) -> bool
{
  return op == "minus" || op == "-" || op == "sub";
}

static auto IsMathMultiplyOperator(const auto &op) -> bool
{
  return op == "times" || op == "*" || op == "mul";
}

static auto IsMathDivideOperator(const auto &op) -> bool
{
  return op == "divided_by" || op == "/" || op == "div";
}

static auto IsMathModulusOperator(const auto &op) -> bool
{
  return op == "%%" || op == "mod" || op == "modulo";
}

static auto IsMathNegationOperator(const auto &op) -> bool
{
  return op == "-";
}

static auto IsAdditiveMathOperator(const auto &op) -> bool
{
  return IsMathAddOperator(op) || IsMathSubtractOperator(op);
}

static auto IsMultiplicativeMathOperator(const auto &op) -> bool
{
  return IsMathMultiplyOperator(op) || IsMathDivideOperator(op) || IsMathModulusOperator(op);
}

static auto IsLogicalOROperator(const auto &op) -> bool
{
  return op == "or" || op == "||";
}

static auto IsLogicalANDOperator(const auto &op) -> bool
{
  return op == "and" || op == "&&";
}

static auto IsFalseString(const auto &sym) -> bool
{
  return sym == "false" || sym == "FALSE" || sym == "off" || sym == "OFF";
}

static auto IsTrueString(const auto &sym) -> bool
{
  return sym == "true" || sym == "TRUE" || sym == "on" || sym == "ON";
}

auto m9::ExpressionParser::GetSymbolValue(const std::string &s) const -> int
{
  if (s.empty())
  {
    throw std::runtime_error("Expected value");
  }

  if (symbols.contains(s))
  {
    if (IsFalseString(symbols.at(s)))
    {
      return 0;
    }

    if (IsTrueString(symbols.at(s)))
    {
      return 1;
    }

    try
    {
      const auto value = std::stoi(symbols.at(s), nullptr, 0);
      return value;
    } catch (...)
    {
      return 0;
    }
  }

  try
  {
    const auto value = std::stoi(s, nullptr, 0);
    return value;
  } catch (...)
  {
    return 0;
  }
}

std::string m9::ExpressionParser::Peek() const
{
  const auto peek_token = pos < tokens.size() ? tokens[pos] : "";
#ifdef VERBOSE_DEBUGGING
  std::cerr << "   Peek: \"" << peek_token << "\" at token #" << (pos + 1) << "/" << tokens.size() << std::endl;
#endif
  return peek_token;
}

std::string m9::ExpressionParser::Consume()
{
  const auto peek_token = pos < tokens.size() ? tokens[pos] : "";
#ifdef VERBOSE_DEBUGGING
  std::cerr << "Consume: \"" << peek_token << "\" at token #" << (pos + 1) << "/" << tokens.size() << std::endl;
#endif
  pos++;
  return peek_token;
}

auto m9::ExpressionParser::ParseLogicalOR() -> bool
{
  bool left = ParseLogicalAND();
  while (IsLogicalOROperator(Peek()))
  {
    Consume();
    const auto right = ParseLogicalAND();
    left = left || right;
  }
  return left;
}

auto m9::ExpressionParser::ParseLogicalAND() -> bool
{
  bool left = ParseComparison();
  while (IsLogicalANDOperator(Peek()))
  {
    Consume();
    const auto right = ParseComparison();
    left = left && right;
  }
  return left;
}

auto m9::ExpressionParser::ParseComparison() -> bool
{
  // Relational (eq, lt, gt)
  const int left = ParseMathAdditive();
  if (const std::string op = Peek(); IsComparisonOperator(op))
  {
    Consume();
    const int right = ParseMathAdditive();
    if (IsEqualityComparisonOperator(op))
    {
      return left == right;
    }
    if (IsLessComparisonOperator(op))
    {
      return left < right;
    }
    if (IsGreaterComparisonOperator(op))
    {
      return left > right;
    }
  }
  // Fallback for single values: %if (A)
  return left != 0;
}

auto m9::ExpressionParser::ParseMathAdditive() -> int
{
  // Additive operators (+, -)
  int left = ParseMathMultiplicative();
  while (IsAdditiveMathOperator(Peek()))
  {
    std::string op = Consume();
    const int right = ParseMathMultiplicative();
    if (IsMathAddOperator(op))
    {
      left += right;
    } else
    {
      left -= right;
    }
  }
  return left;
}

auto m9::ExpressionParser::ParseMathMultiplicative() -> int
{
  // Multiplicative Operators (*, /, %) (Term)
  int left = ParseUnary();
  while (IsMultiplicativeMathOperator(Peek()))
  {
    std::string op = Consume();
    const int right = ParseUnary();
    if (IsMathMultiplyOperator(op))
    {
      left *= right;
    } else if (IsMathDivideOperator(op))
    {
      if (right == 0)
      {
        throw std::runtime_error("Division by zero");
      }
      left /= right;
    } else
    {
      if (right == 0)
      {
        throw std::runtime_error("Modulo by zero");
      }
      left %= right;
    }
  }
  return left;
}

auto m9::ExpressionParser::ParseUnary() -> int
{
  // Unary Negation (-)
  if (IsMathNegationOperator(Peek()))
  {
    Consume();
    return -ParsePrimary();
  }
  return ParsePrimary();
}

auto m9::ExpressionParser::ParsePrimary() -> int
{
  // Primary (Parens, Symbols, Literals)
  if (Peek() == OPEN_PARENTHESIS)
  {
    Consume(); // '('
    // IMPORTANT: Jump back to TOP of precedence for sub-expressions
    const int result = ParseLogicalOR();
    if (Consume() != CLOSE_PARENTHESIS)
    {
      throw std::runtime_error("Mismatched parentheses");
    }
    return result;
  }
  const auto value = GetSymbolValue(Consume());
  return value;
}

m9::ExpressionParser::ExpressionParser(std::string_view expr, const std::unordered_map<std::string, std::string> &syms)
  : symbols(syms)
{
  const auto operators = std::unordered_set{'(', ')', '+', '-', '*', '/', '%'};
  std::string sanitized;
  for (char c: expr)
  {
    if (operators.contains(c))
    {
      sanitized += ' ';
      sanitized += c;
      sanitized += ' ';
    } else
    {
      sanitized += c;
    }
  }
  std::stringstream ss(sanitized);
  std::string t;
  while (ss >> t)
  {
    tokens.push_back(t);
  }

#ifdef VERBOSE_DEBUGGING
  std::cerr << "Parsing Expression: \"" << expr << "\"" << std::endl;
  std::cerr << "Expression Tokens: " << tokens.size() << "\n";
  auto index = 1;
  for (const auto& token : tokens)
  {
    std::cerr << std::setw(4) << index << ": \"" << token << "\"" << std::endl;
    index++;
  }
#endif
}

auto m9::ExpressionParser::Evaluate() -> bool
{
  if (tokens.empty())
  {
    return false;
  }
  const bool result = ParseLogicalOR();
#ifdef VERBOSE_DEBUGGING
  std::cerr << "Evaluation Result is " << result << std::endl;
#endif
  if (pos < tokens.size())
  {
    throw std::runtime_error(std::format("Unexpected token: '{}' at token #{}/{}", tokens[pos], pos + 1, tokens.size()));
  }
  return result;
}
