//
// Created by Richard Marks on 4/28/26.
//
#pragma once

#include <string>

namespace m9
{
  struct Util
  {
    static auto LTrim(const std::string &s) -> std::string;

    static auto RTrim(const std::string &s) -> std::string;

    static auto Trim(const std::string &s) -> std::string;

    static auto StripComments(const std::string &s) -> std::string;

    static auto ToLower(std::string &s) -> std::string;
  };
}
