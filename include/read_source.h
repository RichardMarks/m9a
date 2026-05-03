//
// Created by Richard Marks on 4/27/26.
//
#pragma once

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace m9
{
  struct SourceLine
  {
    std::string source_file{};
    std::string source_line{};
    size_t relative_line_number{};
    size_t absolute_line_number{};
  };

  struct Source
  {
    std::vector<SourceLine> lines{};
  };

  struct ReadSource
  {
    static auto ToSource(const std::string& filename, const std::unordered_map<std::string, std::string> &syms) -> std::unique_ptr<Source>;
  };
}
