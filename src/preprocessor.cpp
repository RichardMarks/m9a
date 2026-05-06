//
// Created by Richard Marks on 5/3/26.
//

#include <format>
#include <iostream>
#include <algorithm>
#include <regex>

#include "read_source.h"

#include "preprocessor.h"


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
}

auto m9::Preprocessor::ExecuteConstantSubstitutionPass(
  const std::vector<std::string> &lines) -> std::vector<std::string>
{
  std::unordered_map<std::string, std::string> constants;
  std::vector<std::string> result;

  // lets auto replace all register names with the correct values by using the substitution pass
  constants.try_emplace("ra", "0x00");
  constants.try_emplace("rb", "0x01");
  constants.try_emplace("rc", "0x02");
  constants.try_emplace("rd", "0x03");
  constants.try_emplace("re", "0x04");
  constants.try_emplace("rf", "0x05");
  constants.try_emplace("sp", "0x06");
  constants.try_emplace("pc", "0x07");

  // pass 1. find all constant definitions (all .def directive lines get excluded from output here)
  // pass 2. substitute all constants
  // [lines] -> [P1] -> [P2] -> [lines with all constants replaced]

  // All lines with .def directives
  for (const auto &line: lines)
  {
    if (!line.starts_with(CONSTANT_DIRECTIVE))
    {
      result.push_back(line);
      continue;
    }
    try
    {
      // extract constant definition identifier and substitution value strings
      // and insert them into the constants map
      const auto res = ConstantsParser::Parse(line);
      // identifiers are case-insensitive so added as lowercase
      constants.try_emplace(Util::ToLower(res->identifier), res->substitution);
    } catch (ConstantsParser::ParseException &e)
    {
      std::cerr << e.what() << std::endl;
    }
  }

  for (auto &line: result)
  {
    // do not replace label lines
    if (line.ends_with(":"))
    {
      continue;
    }

    const auto tokens = Util::StrTok(line, " \t,");

    if (tokens.empty())
    {
      continue;
    }

    std::string replaced_line = tokens.at(0);
    for (auto i = 1; i < tokens.size(); ++i)
    {
      const auto &token = tokens.at(i);
      if (const auto case_insensitive_token = Util::ToLower(token); constants.contains(case_insensitive_token))
      {
        replaced_line += " " + constants.at(case_insensitive_token);
        continue;
      }
      replaced_line += " " + token;
    }
    line = std::move(replaced_line);
  }

  return result;
}
