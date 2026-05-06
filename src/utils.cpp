//
// Created by Richard Marks on 4/28/26.
//
#include <algorithm>
#include "utils.h"

auto m9::Util::LTrim(const std::string &s) -> std::string
{
  const auto start_pos = s.find_first_not_of(" \t");
  if (std::string::npos == start_pos)
  {
    return "";
  }
  return s.substr(start_pos);
}

auto m9::Util::RTrim(const std::string &s) -> std::string
{
  const auto end_pos = s.find_last_not_of(" \t");
  if (std::string::npos == end_pos)
  {
    return "";
  }
  return s.substr(0, end_pos + 1);
}

auto m9::Util::Trim(const std::string &s) -> std::string
{
  return LTrim(RTrim(s));
}

auto m9::Util::StripComments(const std::string &s) -> std::string
{
  auto in_quotes = false;
  auto index = 0;
  for (const auto c: s)
  {
    if (c == '"' || c == '\'')
    {
      in_quotes = !in_quotes;
    }
    if (c == ';' && !in_quotes)
    {
      return s.substr(0, index);
    }
    index++;
  }
  return s;
}

auto m9::Util::ToLower(const std::string &s) -> std::string
{
  std::string s_copy{s};
  std::ranges::transform(s_copy, s_copy.begin(), [](const unsigned char c) { return std::tolower(c); });
  return s_copy;
}

auto m9::Util::StrTok(const std::string &input, const std::string &delimiters) -> std::vector<std::string>
{
  auto tokens = std::vector<std::string>{};
  std::string current_token;

  for (const auto c: input)
  {
    if (delimiters.contains(c))
    {
      if (!current_token.empty())
      {
        tokens.push_back(current_token);
        current_token = "";
      }
    } else
    {
      current_token.push_back(c);
    }
  }

  if (!current_token.empty())
  {
    tokens.push_back(current_token);
  }

  return tokens;
}
