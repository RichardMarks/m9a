//
// Created by Richard Marks on 5/4/26.
//

#include <format>

#include "intermediate_assembly.h"

m9::IntermediateAssemblyFile::IntermediateAssemblyFile(const std::string &filename)
{
  fp = std::make_unique<std::ofstream>(filename);

  if (!fp)
  {
    throw std::runtime_error(std::format("Could not allocate for {}", filename));
  }

  if (!fp->is_open())
  {
    throw std::runtime_error(std::format("Could not open {}", filename));
  }

  ss = std::make_unique<std::stringstream>();
}

m9::IntermediateAssemblyFile::~IntermediateAssemblyFile()
{
  lines.clear();
  if (fp)
  {
    if (fp->is_open())
    {
      fp->close();
    }
  }
}

void m9::IntermediateAssemblyFile::Write(const std::string &text)
{
  *ss << text << std::endl;
  *fp << text << std::endl;
  lines.push_back(text);
}

std::string m9::IntermediateAssemblyFile::GetContentsAsString() const
{
  return ss->str();
}

std::vector<std::string> m9::IntermediateAssemblyFile::GetContentsAsLines() const
{
  return lines;
}

void m9::M9IFile::WriteM9I2TokenStreamDiagnosticsFile(const std::string &name, const std::vector<Token> &token_stream)
{
  IntermediateAssemblyFile m9i2(std::format("{}.m9i2", name));

  auto token_index = 1;
  for (const auto &[token_type, token_context_type, value, num_value]: token_stream)
  {
    const auto tts = TokenTypeStr(token_type);
    const auto tct = TokenTypeStr(token_context_type);
    const auto num = num_value.has_value()
                       ? std::format("(Number: {:8d} (0x{:X})", num_value.value(), num_value.value())
                       : "";
    const auto fmt = std::format("{:>14d}:{:>10}[{:>8}]: {:16} {}", token_index, tts, tct, value, num);
    m9i2.Write(fmt);
    token_index++;
  }
}
