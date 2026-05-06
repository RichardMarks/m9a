//
// Created by Richard Marks on 5/4/26.
//
#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace m9
{
  struct IntermediateAssemblyFile
  {
    std::unique_ptr<std::ofstream> fp{nullptr};
    std::unique_ptr<std::stringstream> ss{nullptr};
    std::vector<std::string> lines{};

    explicit IntermediateAssemblyFile(const std::string &filename);

    ~IntermediateAssemblyFile();

    void Write(const std::string &text);

    [[nodiscard]] std::string GetContentsAsString() const;

    [[nodiscard]] std::vector<std::string> GetContentsAsLines() const;
  };
}
