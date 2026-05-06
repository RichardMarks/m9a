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
