//
// Created by Richard Marks on 5/3/26.
//
#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "token.h"

namespace m9
{
  struct Preprocessor
  {
    static auto ExecuteConditionalAssemblyAndDependencyPass(const std::string& file_name, const std::unordered_map<std::string, std::string> &syms) -> std::vector<std::string>;

    static auto ExecuteConstantSubstitutionPass(const std::vector<std::string> & lines) -> std::vector<Token>;
  };
}