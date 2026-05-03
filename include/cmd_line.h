//
// Created by Richard Marks on 4/27/26.
//
#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <iostream>

namespace cmd_line
{
  constexpr auto DEFAULT_INPUT_FILENAME = "main.m9s";
  constexpr auto DEFAULT_OUTPUT_FILENAME = "output.m9b";

  class CmdLine
  {
    std::unordered_set<char> flags{};
    std::unordered_map<std::string, std::string> defines{};
    std::vector<std::string> input_names{};
    std::string output_name{DEFAULT_OUTPUT_FILENAME};

    void ParseArgs(const std::vector<std::string> &args);

  public:
    CmdLine(int argc, char *argv[]);

    explicit CmdLine(const std::vector<std::string> &args);

    ~CmdLine() = default;

    CmdLine(const CmdLine &) = delete;

    CmdLine &operator=(const CmdLine &) = delete;

    CmdLine(CmdLine &&) = delete;

    CmdLine &operator=(CmdLine &&) = delete;

    void UseDefaults();

    void ClearInputs();

    void ClearFlags();

    void ClearDefines();

    void AddInput(const std::string &input);

    void SetOutputName(const std::string &name);

    void SetFlag(char flag);

    void SetDefine(const std::string &define_key, const std::string &define_value);

    [[nodiscard]] auto IsFlagSet(char flag) const -> bool;

    [[nodiscard]] auto IsDefined(const std::string &define_key) const -> bool;

    [[nodiscard]] auto GetDefinedValue(const std::string &define_key) const -> std::string;

    [[nodiscard]] auto GetDefinedValues() const -> std::unordered_map<std::string, std::string>;

    [[nodiscard]] auto GetInputs() const -> std::vector<std::string>;

    [[nodiscard]] auto GetOutput() const -> std::string;

    [[nodiscard]] auto HasFlagsSet() const -> bool;

    [[nodiscard]] auto HasDefinedValues() const -> bool;

    [[nodiscard]] auto HasInputs() const -> bool;

    void PrintFlags() const;

    void PrintDefines() const;

    void PrintInputs() const;

    void PrintOutputs() const;
  };
}
