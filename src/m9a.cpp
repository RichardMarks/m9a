//
// Created by Richard Marks on 4/27/26.
//


#include <iostream>
#include <exception>
#include <filesystem>
#include <format>
#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>

#include "m9a.h"
#include "cmd_line.h"
#include "preprocessor.h"

static void PrintBanner()
{
  auto out = std::stringstream{};
  out << std::format("{} v{}.{}.{} (c) {}, Richard Marks", m9::APP_NAME, m9::VERSION_MAJOR, m9::VERSION_MINOR,
                     m9::VERSION_PATCH, m9::COPYRIGHT_YEAR);
  std::cerr << out.str() << std::endl;
}

static void PrintUsage()
{
  auto out = std::stringstream{};
  out << "Usage: m9a [-h] <input.m9s> [-o output.m9b]\n\n";
  std::cerr << out.str() << std::endl;
}

struct IntermediateAssemblyFile
{
  std::unique_ptr<std::ofstream> fp{nullptr};
  std::unique_ptr<std::stringstream> ss{nullptr};

  explicit IntermediateAssemblyFile(const std::string &filename)
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

  ~IntermediateAssemblyFile()
  {
    if (fp)
    {
      if (fp->is_open())
      {
        fp->close();
      }
    }
  }

  void Write(const std::string &text) const
  {
    *ss << text << std::endl;
    *fp << text << std::endl;
  }

  [[nodiscard]] std::string GetContentsAsString() const
  {
    return ss->str();
  }
};

static void Execute(const std::vector<std::string> &args)
{
  if (!args.empty())
  {
    std::cerr << "Executing with args: ";
    for (const auto &arg: args)
    {
      std::cerr << arg << " ";
    }
    std::cerr << std::endl;
  }

  const cmd_line::CmdLine command_line(args);

  command_line.PrintFlags();
  command_line.PrintDefines();
  command_line.PrintInputs();
  command_line.PrintOutputs();

  const auto &input_filenames = command_line.GetInputs();
  const auto &output_filename = command_line.GetOutput();

  const auto &symbols = command_line.GetDefinedValues();

  const IntermediateAssemblyFile m9i("assembly.m9i");

  for (const auto &input_filename: input_filenames)
  {
    for (const auto preprocessed_lines =
           m9::Preprocessor::ExecuteConditionalAssemblyAndDependencyPass(input_filename, symbols); const auto &line:
         preprocessed_lines)
    {
      m9i.Write(line);
    }
  }

  const auto contents = m9i.GetContentsAsString();
  std::cerr << contents << std::endl;

  std::cerr << "Done" << std::endl;
}

int main(const int argc, char *argv[])
{
  try
  {
    PrintBanner();
    if (argc < 2)
    {
      PrintUsage();
      return EXIT_SUCCESS;
    }
    const std::vector<std::string> args(argv + 1, argv + argc);
    Execute(args);
  } catch (std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
