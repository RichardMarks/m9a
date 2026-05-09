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
#include <vector>
#include <functional>

#include "cmd_line.h"
#include "preprocessor.h"
#include "intermediate_assembly.h"
#include "rom.h"
#include "rom_file.h"
#include "utils.h"
#include "token_stream_assembler.h"
#include "m9a.h"

namespace m9
{
  struct Assembler
  {
    static void Assemble(const std::vector<Token> &token_stream, Rom &rom)
    {
      const TokenStreamAssembler tsa{token_stream};
      if (tsa.bytes.size() >= Rom::MAX_DATA_SIZE_IN_BYTES)
      {
        const auto lost = Rom::MAX_DATA_SIZE_IN_BYTES - tsa.bytes.size();
        const auto msg = std::format("Assembled binary will not fit in Rom data. '{}' bytes will be lost.", lost);
        throw std::runtime_error(msg);
      }
      std::cerr << std::format("Assembled Binary is {} Bytes", tsa.bytes.size()) << std::endl;
      auto offset = 0;
      for (const auto &byte: tsa.bytes)
      {
        rom.data[offset++] = byte;
      }
      rom.header.data_length = offset;
      rom.header.start_address = tsa.start_address;
      std::cerr << std::format("Assembled Rom Data Length: {}", rom.header.data_length) << std::endl;
      std::cerr << std::format("Assembled Rom Start Address: 0x{:04X}", rom.header.start_address) << std::endl;
      Util::PrintHexDump(rom.data, 0, rom.header.data_length);
    }
  };
}

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
  out << "Usage: m9a [-h -dDEF] <input.m9s> [-o output.m9b]\n\n";
  std::cerr << out.str() << std::endl;
}

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

  m9::IntermediateAssemblyFile m9i("assembly.m9i");

  for (const auto &input_filename: input_filenames)
  {
    for (const auto preprocessed_lines =
           m9::Preprocessor::ExecuteConditionalAssemblyAndDependencyPass(input_filename, symbols); const auto &line:
         preprocessed_lines)
    {
      m9i.Write(line);
    }
  }

  const auto contents = m9i.GetContentsAsLines();

  std::cerr << "Preprocessed Line Count: " << contents.size() << std::endl;

  const auto token_stream = m9::Preprocessor::ExecuteConstantSubstitutionPass(contents);

  std::cerr << "Token Stream Token Count: " << token_stream.size() << std::endl;

  m9::M9IFile::WriteM9I2TokenStreamDiagnosticsFile("assembly", token_stream);

  const auto rom = std::make_unique<m9::Rom>();
  m9::Assembler::Assemble(token_stream, *rom);
  m9::RomFile::Write(output_filename, *rom);
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
