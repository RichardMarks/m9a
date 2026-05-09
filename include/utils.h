//
// Created by Richard Marks on 4/28/26.
//
#pragma once

#include <string>
#include <vector>

namespace m9
{
  struct Util
  {
    static auto LTrim(const std::string &s) -> std::string;

    static auto RTrim(const std::string &s) -> std::string;

    static auto Trim(const std::string &s) -> std::string;

    static auto StripComments(const std::string &s) -> std::string;

    static auto ToLower(const std::string &s) -> std::string;

    static auto StrTok(const std::string& input, const std::string& delimiters) -> std::vector<std::string>;

    static void PrintHexDump(const uint8_t *memory, const uint32_t start, const uint32_t end);
  };
}
