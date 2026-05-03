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

#include "m9a.h"
#include "cmd_line.h"
#include "read_source.h"

static void PrintBanner()
{
  auto out = std::stringstream{};
  out << std::format("{} v{}.{}.{} (c) {}, Richard Marks", m9::APP_NAME, m9::VERSION_MAJOR, m9::VERSION_MINOR, m9::VERSION_PATCH, m9::COPYRIGHT_YEAR);
  std::cerr << out.str() << std::endl;
}

static void PrintUsage()
{
  auto out = std::stringstream{};
  out << "Usage: m9a [-h] <input.m9s> [-o output.m9b]\n\n";
  std::cerr << out.str() << std::endl;
}

static void Execute(const std::vector<std::string>& args)
{
  if (!args.empty())
  {
    std::cerr << "Executing with args: ";
    for (const auto& arg : args)
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

  const auto& input_filenames = command_line.GetInputs();
  const auto& output_filename = command_line.GetOutput();

  const auto& symbols = command_line.GetDefinedValues();

  std::vector<std::unique_ptr<m9::Source>> sources;

  for (const auto& input_filename : input_filenames)
  {
    if (!std::filesystem::exists(input_filename))
    {
      continue;
    }
    auto source = m9::ReadSource::ToSource(input_filename, symbols);
    sources.emplace_back(std::move(source));
  }

  for (const auto& source : sources)
  {
    const auto line_count = source->lines.size();
    std::cerr << "Sourced Line Count: " << line_count << std::endl;
    for (const auto&[source_file, source_line, relative_line_number, absolute_line_number] : source->lines)
    {
      if (source_file.starts_with("BIN["))
      {
        std::cerr << source_file << " (" << absolute_line_number << " abs) : " << source_line << std::endl;
        continue;
      }
      std::cerr << source_file << ":" << relative_line_number << " (" << absolute_line_number << " abs) : " << source_line << std::endl;
    }
  }

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
