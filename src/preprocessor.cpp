//
// Created by Richard Marks on 5/3/26.
//

#include <format>
#include <iostream>
#include <algorithm>
#include <regex>

#include "read_source.h"

#include "preprocessor.h"

#include <unordered_set>


#include "restricted_identifiers.h"
#include "utils.h"

constexpr auto CONSTANT_DIRECTIVE = R"(%const)";

auto m9::Preprocessor::ExecuteConditionalAssemblyAndDependencyPass(const std::string &file_name,
                                                                   const std::unordered_map<std::string, std::string> &
                                                                   syms) -> std::vector<
  std::string>
{
  try
  {
    std::vector<std::string> result;
    for (const auto source = ReadSource::ToSource(file_name, syms); const auto &[source_file, source_line,
           relative_line_number, absolute_line_number]: source->lines)
    {
      if (source_file.starts_with("BIN["))
      {
        result.push_back(std::format("{:90} ; {}", source_line, source_file));
        continue;
      }
      result.push_back(std::format("{:90} ; {}:{}", source_line, source_file, relative_line_number));
    }
    return result;
  } catch (const std::exception &e)
  {
    throw std::runtime_error(std::format("Preprocessor Failed on File: {} because: {}", file_name, e.what()));
  }
}

namespace m9
{
  struct ConstantsParser
  {
    static constexpr auto MAX_IDENTIFIER_LENGTH = 256;

    class ParseException : public std::runtime_error
    {
    public:
      explicit ParseException(const std::string &message)
        : std::runtime_error("Constant Parsing Error: " + message)
      {
      }
    };

    static bool IsRestricted(const std::string &identifier)
    {
      const auto id = Util::ToLower(identifier);
      const auto Predicate = [&id](const char *symbol)
      {
        return symbol == id;
      };
      const auto result = std::ranges::any_of(RestrictedIdentifiers::SYMBOLS, Predicate);
      return result;
    }

    static void ValidateIdentifier(const std::string &identifier)
    {
      if (identifier.empty())
      {
        throw ParseException("Identifier is empty.");
      }

      if (identifier.length() > MAX_IDENTIFIER_LENGTH)
      {
        throw ParseException(std::format("Identifier exceeds maximum length of {} characters.", MAX_IDENTIFIER_LENGTH));
      }

      if (!std::isalpha(identifier[0]) && identifier[0] != '_')
      {
        throw ParseException(std::format("Identifier '{}' must start with a letter or underscore.", identifier));
      }

      for (char const &c: identifier)
      {
        if (std::isalnum(c) || c == '_')
        {
          continue;
        }
        throw ParseException(std::format("Identifier contains at invalid character: {}", std::string(1, c)));
      }

      if (IsRestricted(identifier))
      {
        throw ParseException(std::format("Identifier '{}' is a restricted/reserved symbol.", identifier));
      }
    }

    static void ValidateSubstitution(const std::string &substitution)
    {
      if (substitution.empty())
      {
        throw ParseException("Substitution value is missing.");
      }

      if (constexpr auto QUOTE_CHARACTER = '"'; substitution.front() == QUOTE_CHARACTER && substitution.back() ==
                                                QUOTE_CHARACTER)
      {
        if (substitution.length() < 2)
        {
          throw ParseException("Malformed empty quoted string.");
        }

        for (size_t i = 1; i < substitution.length() - 1; ++i)
        {
          if (substitution[i] == QUOTE_CHARACTER && substitution[i - 1] != '\\')
          {
            throw ParseException(std::format("Unescaped {} inside string literal.", QUOTE_CHARACTER));
          }
        }
      } else
      {
        if (substitution.find_first_of(" \t") != std::string::npos)
        {
          throw ParseException("Unquoted substitution contains spaces.");
        }
      }
    }

    struct Result
    {
      std::string identifier{};
      std::string substitution{};
    };

    static std::unique_ptr<Result> Parse(const std::string &input)
    {
      std::string work(input);
      auto result = std::make_unique<Result>();

      work = Util::Trim(Util::Trim(Util::StripComments(work)).substr(work.find_first_of(" \t")));

      const auto id_end = work.find_first_of(" \t");
      const auto identifier = work.substr(0, id_end);

      ValidateIdentifier(identifier);

      if (id_end == std::string::npos)
      {
        throw ParseException("Missing substitution value.");
      }

      auto substitution = work.substr(id_end);
      const auto start = substitution.find_first_not_of(" \t");
      if (start == std::string::npos)
      {
        throw ParseException("Missing substitution value.");
      }
      substitution = substitution.substr(start);

      ValidateSubstitution(substitution);

      result->identifier = identifier;
      result->substitution = substitution;

      return result;
    }
  };

  struct ConstantsTable
  {
    std::unordered_map<std::string, std::string> constants{};

    [[nodiscard]] auto Has(const std::string &key) const -> bool
    {
      return constants.contains(key);
    }

    [[nodiscard]] auto Get(const std::string &key) const -> std::string
    {
      return constants.at(key);
    }

    void Add(const std::string &key, const std::string &value)
    {
      constants.try_emplace(key, value);
    }
  };

  static auto IsMnemonic(const std::string &identifier) -> bool
  {
    const std::unordered_set<std::string> mnemonics{
      "no", "he", "wf", "so",
      "ld", "ln", "st", "sn", "mv", "cp", "pu", "po", "xr",
      "nd", "or", "eo", "nt", "sl", "sr", "ar",
      "ad", "ai", "sb", "si", "mu", "mi", "dv", "di", "dq", "qi", "ir", "dr", "sx",
      "uj", "zj", "nj", "ls", "lu", "ej",
      "pj", "jp",
    };
    return mnemonics.contains(identifier);
  }

  struct TokenStreamBuilder
  {
    static auto PrepareLineForTokenizer(const std::string &line) -> std::string
    {
      const auto result = Util::Trim(Util::StripComments(Util::Trim(line)));
      return result;
    }

    static void ParseNumber(const std::string &s, Token& result)
    {
      std::cerr << std::format("ParseNumber '{}'", s) << std::endl;

      std::unordered_map<std::string, uint8_t> reg_map{
        {"ra", 0x00 + 128},
        {"rb", 0x01 + 128},
        {"rc", 0x02 + 128},
        {"rd", 0x03 + 128},
        {"re", 0x04 + 128},
        {"rf", 0x05 + 128},
        {"sp", 0x06 + 128},
        {"pc", 0x07 + 128}
      };

      if (reg_map.contains(s))
      {
        result.value = std::format("@{:d}", reg_map.at(s) - 128);
        result.num_value = reg_map.at(s);
        result.token_type = TokenType::OPERAND;
        return;
      }

      if (s.starts_with("0x"))
      {
        // likely a hexadecimal number
        // lets check
        auto is_num = false;
        try
        {
          const auto converted = std::stol(s, nullptr, 16);
          result.value = std::format("{:d}", converted);
          result.num_value = converted;
          is_num = true;
        } catch (...)
        {
          is_num = false;
        }
        if (is_num)
        {
          result.token_type = TokenType::OPERAND;
        }
        return;
      }

      if (std::isdigit(s[0]))
      {
        // likely is a decimal number
        // lets check
        auto is_num = false;
        try
        {
          const auto converted = std::stol(s, nullptr, 10);
          result.value = std::format("{:d}", converted);
          result.num_value = converted;
          is_num = true;
        } catch (...)
        {
          is_num = false;
        }
        if (is_num)
        {
          result.token_type = TokenType::OPERAND;
        }
      }
    }

    static auto StrToToken(const std::string &s) -> Token
    {
      Token result;
      result.token_type = TokenType::UNKNOWN;
      result.value = s;
      result.num_value = std::nullopt;
      if (s.ends_with(':'))
      {
        result.token_type = TokenType::LABEL;
        result.value = s.substr(0, s.size() - 1);
        return result;
      }
      if (s.starts_with('.'))
      {
        result.token_type = TokenType::DIRECTIVE;
        result.value = s.substr(1);
        return result;
      }
      if (IsMnemonic(s))
      {
        result.token_type = TokenType::MNEMONIC;
        return result;
      }

      if (s.starts_with('[') && s.ends_with(']') && s.size() > 2)
      {
        // address reference
        const auto ref = s.substr(1, s.size() - 2);
        result.token_context_type = TokenType::ADDRESS;
        ParseNumber(ref, result);
        return result;
      }

      if (s.ends_with(".b"))
      {
        if (const auto ref = s.substr(0, s.size() - 2); IsMnemonic(ref))
        {
          result.token_type = TokenType::MNEMONIC;
        }
      }

      ParseNumber(s, result);
      return result;
    }

    static auto Build(const std::vector<std::string> &lines, const ConstantsTable &ct) -> std::vector<Token>
    {
      std::vector<Token> result;

      for (const auto &line: lines)
      {
        const auto prepared_line = PrepareLineForTokenizer(line);
        std::cerr << std::format("Input Line: {}\nPrepared: {}", line, prepared_line) << std::endl;
        const auto str_tokens = Util::StrTok(prepared_line, " \t,");
        auto token_index = 0;
        for (const auto &str_token: str_tokens)
        {
          std::cerr << std::format("  Token {}: {}", token_index, str_token) << std::endl;
          token_index++;
        }
        token_index = 0;
        for (const auto &str_token: str_tokens)
        {
          const auto lower_token = Util::ToLower(str_token);
          if (ct.Has(lower_token))
          {
            const auto replacement = ct.Get(lower_token);
            std::cerr << std::format("  Token Has Constant Replacement {}: {} -> {}", token_index, lower_token,
                                     replacement) << std::endl;
            result.emplace_back(StrToToken(replacement));
            token_index++;
            continue;
          }
          result.emplace_back(StrToToken(lower_token));
          token_index++;
        }
      }

      return result;
    }
  };
}

auto m9::Preprocessor::ExecuteConstantSubstitutionPass(
  const std::vector<std::string> &lines) -> std::vector<Token>
{
  ConstantsTable ct;
  std::vector<Token> result;

  // lets auto replace all register names with the correct values by using the substitution pass
  ct.Add("ra", "0x00");
  ct.Add("rb", "0x01");
  ct.Add("rc", "0x02");
  ct.Add("rd", "0x03");
  ct.Add("re", "0x04");
  ct.Add("rf", "0x05");
  ct.Add("sp", "0x06");
  ct.Add("pc", "0x07");

  // pass 1. find all constant definitions (all .def directive lines get excluded from output here)
  // pass 2. substitute all constants
  // [lines] -> [P1] -> [P2] -> [lines with all constants replaced]

  // All lines with %const directives
  std::vector<std::string> to_process;
  for (const auto &line: lines)
  {
    if (!line.starts_with(CONSTANT_DIRECTIVE))
    {
      to_process.push_back(line);
      continue;
    }
    try
    {
      // extract constant definition identifier and substitution value strings
      // and insert them into the constants map
      const auto res = ConstantsParser::Parse(line);
      // identifiers are case-insensitive so added as lowercase
      ct.Add(Util::ToLower(res->identifier), res->substitution);
    } catch (ConstantsParser::ParseException &e)
    {
      std::cerr << e.what() << std::endl;
    }
  }

  const auto token_stream = TokenStreamBuilder::Build(to_process, ct);

  result.reserve(token_stream.size());
  for (const auto &token: token_stream)
  {
    result.emplace_back(token);
  }
  return result;
}
