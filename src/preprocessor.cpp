//
// Created by Richard Marks on 5/3/26.
//

#include <format>
#include <iostream>

#include "read_source.h"

#include "preprocessor.h"

auto m9::Preprocessor::ExecuteConditionalAssemblyAndDependencyPass(const std::string &file_name,
                                      const std::unordered_map<std::string, std::string> &syms) -> std::vector<
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
  }
  catch (const std::exception &e)
  {
    throw std::runtime_error(std::format("Preprocessor Failed on File: {} because: {}", file_name, e.what()));
  }
}
