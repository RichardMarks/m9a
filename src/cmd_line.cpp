//
// Created by Richard Marks on 4/27/26.
//
#include "cmd_line.h"

void cmd_line::CmdLine::ParseArgs(const std::vector<std::string> &args)
{
  auto next_is_output = false;
  for (const auto& arg : args)
  {
    if (arg.starts_with("-d"))
    {
      auto define_key = arg.substr(2);
      const auto eq_pos = arg.find('=');
      std::string define_value = "true";
      if (eq_pos != std::string::npos)
      {
        define_key = arg.substr(2, eq_pos - 2);
        define_value = arg.substr(eq_pos + 1);
      }
      defines[define_key] = define_value;
      continue;
    }

    if (arg == "-o" && !next_is_output)
    {
      next_is_output = true;
      continue;
    }
    if (arg.starts_with('-') && arg.size() == 2)
    {
      flags.insert(arg.at(1));
      continue;
    }

    if (next_is_output)
    {
      output_name = arg;
      next_is_output = false;
      continue;
    }

    if (arg.ends_with(".asm") || arg.ends_with(".m9s"))
    {
      input_names.push_back(arg);
    } else
    {
      throw std::runtime_error("Unknown argument '" + arg + "'");
    }
  }

  if (input_names.empty())
  {
    AddInput(DEFAULT_INPUT_FILENAME);
  }
}

cmd_line::CmdLine::CmdLine(const int argc, char *argv[])
{
  const std::vector<std::string> args(argv + 1, argv + argc);
  ParseArgs(args);
}

cmd_line::CmdLine::CmdLine(const std::vector<std::string> &args)
{
  ParseArgs(args);
}

void cmd_line::CmdLine::UseDefaults()
{
  ClearDefines();
  ClearFlags();
  ClearInputs();
  AddInput(DEFAULT_INPUT_FILENAME);
  SetOutputName(DEFAULT_OUTPUT_FILENAME);
}

void cmd_line::CmdLine::ClearInputs()
{
  input_names.clear();
}

void cmd_line::CmdLine::ClearFlags()
{
  flags.clear();
}

void cmd_line::CmdLine::ClearDefines()
{
  defines.clear();
}

void cmd_line::CmdLine::AddInput(const std::string &input)
{
  input_names.push_back(input);
}

void cmd_line::CmdLine::SetOutputName(const std::string &name)
{
  output_name = name;
}

void cmd_line::CmdLine::SetFlag(const char flag)
{
  flags.insert(flag);
}

void cmd_line::CmdLine::SetDefine(const std::string &define_key, const std::string &define_value)
{
  defines.try_emplace(define_key, define_value);
}

auto cmd_line::CmdLine::IsFlagSet(const char flag) const -> bool
{
  return flags.contains(flag);
}

auto cmd_line::CmdLine::IsDefined(const std::string &define_key) const -> bool
{
  if (defines.contains(define_key))
  {
    // zero and false are false
    if (const auto &val = defines.at(define_key); val == "false" || val == "0")
    {
      return false;
    }
    // every other value is true
    return true;
  }

  // not found in the defines - is always false
  return false;
}

auto cmd_line::CmdLine::GetDefinedValue(const std::string &define_key) const -> std::string
{
  if (defines.contains(define_key))
  {
    const auto val = defines.at(define_key);
    return val;
  }
  return "";
}

auto cmd_line::CmdLine::GetDefinedValues() const -> std::unordered_map<std::string, std::string>
{
  std::unordered_map<std::string, std::string> defined_values;
  for (const auto &[k, v] : defines)
  {
    defined_values[k] = v;
  }
  return defined_values;
}

auto cmd_line::CmdLine::GetInputs() const -> std::vector<std::string>
{
  return std::vector(input_names.begin(), input_names.end());
}

auto cmd_line::CmdLine::GetOutput() const -> std::string
{
  return output_name;
}

auto cmd_line::CmdLine::HasFlagsSet() const -> bool
{
  return !flags.empty();
}

auto cmd_line::CmdLine::HasDefinedValues() const -> bool
{
  return !defines.empty();
}

auto cmd_line::CmdLine::HasInputs() const -> bool
{
  return !input_names.empty();
}

void cmd_line::CmdLine::PrintFlags() const
{
  if (HasFlagsSet())
  {
    std::cerr << "  Flags: ";

    for (const auto& flag : flags)
    {
      std::cerr << flag << " ";
    }
    std::cerr << std::endl;
  }
}

void cmd_line::CmdLine::PrintDefines() const
{
  if (HasDefinedValues())
  {
    std::cerr << "Defines: ";
    for (const auto& [k, v] : defines)
    {
      std::cerr << "(" << k << "=" << v << ") ";
    }
    std::cerr << std::endl;
  }
}

void cmd_line::CmdLine::PrintInputs() const
{
  if (HasInputs())
  {
    std::cerr << " Inputs: ";
    for (const auto& filename : input_names)
    {
      std::cerr << filename << " ";
    }
    std::cerr << std::endl;
  }
}

void cmd_line::CmdLine::PrintOutputs() const
{
  std::cerr << " Output: " << output_name << std::endl;
}
