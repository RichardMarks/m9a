//
// Created by Richard Marks on 4/27/26.
//

#include <filesystem>
#include <exception>
#include <format>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <iostream>

#include "read_source.h"
#include "expression_parser.h"
#include "utils.h"


auto ExtractFilePathDirectiveArgument(const std::string_view line)
{
  const auto start_pos = line.find_first_of("\"'");

  if (start_pos == std::string_view::npos)
  {
    throw std::runtime_error("Error: Filenames must be enclosed in matching quotes. No quotes found.");
  }

  const auto quote_type = line[start_pos];
  const auto end_pos = line.find(quote_type, start_pos + 1);

  if (end_pos == std::string_view::npos)
  {
    if (const auto wrong_quote = line.find_first_of("\"'", start_pos + 1); wrong_quote != std::string_view::npos)
    {
      throw std::runtime_error("Error: Filenames must be enclosed in matching quotes. Mismatched quotes found.");
    }
    throw std::runtime_error("Error: Filenames must be enclosed in matching quotes. Closing quote missing.");
  }

  return std::string{line.substr(start_pos + 1, end_pos - start_pos - 1)};
}

namespace m9
{
  struct ConditionalAssemblyParserState
  {
    static constexpr auto BEGIN_CONDITIONAL_ASSEMBLY = "\x25\x69\x66"; // %if
    static constexpr auto SWAP_CONDITIONAL_ASSEMBLY = "\x25\x65\x6c\x73\x65"; // %else
    static constexpr auto END_CONDITIONAL_ASSEMBLY = "\x25\x65\x6e\x64\x69\x66"; // %endif

    enum class State : uint8_t { EXCLUDE, INCLUDE };

    std::vector<State> active_stack = {State::INCLUDE};

    const std::unordered_map<std::string, std::string> &symbols;

    explicit ConditionalAssemblyParserState(const std::unordered_map<std::string, std::string> &syms) : symbols(syms)
    {
    }

    auto HandleBeginConditionalAssembly(const auto &line)
    {
      if (const auto parent_status = active_stack.back(); parent_status == State::EXCLUDE)
      {
        active_stack.push_back(State::EXCLUDE);
        return;
      }

      try
      {
        ExpressionParser expression_parser(line.substr(4), symbols);
        const bool result = expression_parser.Evaluate();
        active_stack.push_back(result ? State::INCLUDE : State::EXCLUDE);
      } catch (const std::runtime_error &e)
      {
        throw std::runtime_error(std::format("{} in text \"{}\"", e.what(), line));
      }
    }

    auto HandleConditionalAssemblyInversion(const auto &line)
    {
      if (active_stack.size() <= 1)
      {
        throw std::runtime_error(std::format("Mismatched {}", SWAP_CONDITIONAL_ASSEMBLY));
      }

      const auto parents_parent_index = active_stack.size() - 2;
      const auto parent_status = active_stack.at(parents_parent_index);
      const auto opposite_status = active_stack.back() == State::INCLUDE ? State::EXCLUDE : State::INCLUDE;
      const auto next_status = parent_status == State::INCLUDE && opposite_status == State::INCLUDE
                                 ? State::INCLUDE
                                 : State::EXCLUDE;
      active_stack.back() = next_status;
    }

    auto HandleEndConditionalAssembly(const auto &line)
    {
      if (active_stack.size() <= 1)
      {
        throw std::runtime_error(std::format("Mismatched {}", END_CONDITIONAL_ASSEMBLY));
      }

      active_stack.pop_back();
    }

    // returns true when the line should be included in assembly
    auto HandleLine(const auto &line) -> bool
    {
      if (line.starts_with(BEGIN_CONDITIONAL_ASSEMBLY))
      {
        HandleBeginConditionalAssembly(line);
        return false;
      }

      if (line.starts_with(SWAP_CONDITIONAL_ASSEMBLY))
      {
        HandleConditionalAssemblyInversion(line);
        return false;
      }

      if (line.starts_with(END_CONDITIONAL_ASSEMBLY))
      {
        HandleEndConditionalAssembly(line);
        return false;
      }

      const auto should_include = active_stack.back();
      return should_include == State::INCLUDE;
    }
  };

  enum class FileIncludeType
  {
    TEXT,
    BINARY,
  };


  struct FileReader
  {
    static std::vector<uint8_t> ReadAllBytes(const std::filesystem::path &path)
    {
      const std::uintmax_t file_size = std::filesystem::file_size(path);
      std::vector<uint8_t> buffer;
      buffer.resize(file_size);
      std::ifstream file(path, std::ios::binary);

      if (!file)
      {
        throw std::system_error(std::error_code(errno, std::generic_category()),
                                "Failed to open file: " + path.string());
      }

      if (file_size > 0)
      {
        file.read(reinterpret_cast<char *>(buffer.data()),
                  static_cast<std::streamsize>(file_size));

        if (!file)
        {
          throw std::system_error(std::error_code(errno, std::generic_category()),
                                  "Failed to read complete file: " + path.string());
        }
      }

      return buffer;
    }
  };


  struct SourceFileProcessor
  {
    size_t absolute_line_count{1LLu};
    size_t relative_line_count{1LLu};
    std::unordered_set<std::string> seen{};
    std::unordered_map<std::string, std::string> dependencies{};

    std::unique_ptr<ConditionalAssemblyParserState> conditional_assembly_parser_state{nullptr};

    const std::unordered_map<std::string, std::string> &symbols;

    std::string current_file;
    std::string root_file;

    explicit SourceFileProcessor(const std::unordered_map<std::string, std::string> &syms) : symbols(syms)
    {
      conditional_assembly_parser_state = std::make_unique<ConditionalAssemblyParserState>(symbols);
    }

    struct DataLine
    {
      std::string line{};
      size_t start_offset{};
      size_t end_offset{};
    };

    void IncludeFileAsDataBytesDirectives(std::string_view source_file, Source & src)
    {
      if (!std::filesystem::exists(source_file))
      {
        throw std::runtime_error(FormatErrorMessage(std::format("File not found: '{}'", source_file)));
      }

      std::vector<DataLine> data_lines{};

      try
      {
        const auto file_bytes = FileReader::ReadAllBytes(source_file);
        constexpr auto CHUNK_SIZE = 16;
        const auto total = file_bytes.size();

        for (auto i = 0ull; i < total; i += CHUNK_SIZE)
        {
          const auto diff = total - i;
          const auto count = CHUNK_SIZE < diff ? CHUNK_SIZE : diff;
          std::stringstream ss;
          ss << ".BYTE";
          for (auto j = 0; j < count; ++j)
          {
            const auto byte = file_bytes[i + j];
            ss << " ";
            ss << std::format("0x{:02X}", byte);
          }
          data_lines.push_back(DataLine{
            .line = ss.str(),
            .start_offset = i,
            .end_offset = i + count - 1,
          });
        }
      } catch (const std::exception& ex)
      {
        throw std::runtime_error(FormatErrorMessage(ex.what()));
      }

      const auto generated_bytes = data_lines.back().end_offset;

      const auto FormatDataLineSegment = [&generated_bytes](const auto start_offset, const auto end_offset)
      {
        if (generated_bytes < 256)
        {
          return std::format("{:02X}-{:02X}", start_offset, end_offset);
        }
        if (generated_bytes < 65536)
        {
          return std::format("{:04X}-{:04X}", start_offset, end_offset);
        }
        return std::format("{:08X}-{:08X}", start_offset, end_offset);
      };

      for (const auto &[data_line, start_offset, end_offset] : data_lines)
      {
        src.lines.emplace_back(SourceLine{
            .source_file = std::format("BIN[{}]:{}", source_file.data(), FormatDataLineSegment(start_offset, end_offset)),
            .source_line = data_line,
            .relative_line_number = relative_line_count,
            .absolute_line_number = absolute_line_count,
          });

        // Note: relative does not get updated, as this is a binary file inclusion
        // relative_line_count++;
        absolute_line_count++;
      }
    }

    void IncludeFile(const std::string_view source_file, Source &src,
                     const FileIncludeType file_include_type = FileIncludeType::TEXT)
    {
      if (file_include_type == FileIncludeType::BINARY)
      {
        // any file can be included as binary multiple times - does not recurse
        // every binary is included as multiple .bytes directive lines followed by up to 16 byte values
        IncludeFileAsDataBytesDirectives(source_file, src);
        return;
      }

      if (!(source_file.ends_with(".asm") || source_file.ends_with(".m9s")))
      {
        throw std::runtime_error(FormatErrorMessage("Unsupported file type"));
      }

      if (seen.contains(source_file.data()))
      {
        if (source_file == root_file)
        {
          throw std::runtime_error(
            FormatErrorMessage(std::format("{} is the root and cannot be a dependency", source_file)));
        }
        const auto &dep = dependencies.at(source_file.data());
        throw std::runtime_error(FormatErrorMessage(std::format("{} is already a dependency of {}", source_file, dep)));
      }
      const auto next_file = std::string{source_file.data()};
      ProcessFile(next_file, src);
    }

    auto FormatErrorMessage(const std::string &message) -> std::string
    {
      return std::format("Error in {}:{} ({} abs). {}", current_file, relative_line_count, absolute_line_count,
                         message);
    }

    void ProcessFile(const std::string &source_file, Source &src)
    {
      if (!seen.contains(source_file))
      {
        if (seen.empty())
        {
          root_file = source_file;
        }
        seen.insert(source_file);
        dependencies.try_emplace(source_file, current_file);
      }

      if (!std::filesystem::exists(source_file))
      {
        throw std::runtime_error(FormatErrorMessage(std::format("File not found: '{}'", source_file)));
      }

      current_file = source_file;

      std::ifstream file(source_file);
      std::string contents((std::istreambuf_iterator(file)), std::istreambuf_iterator<char>());
      std::istringstream content_stream(contents);
      std::string line;
      std::string stripped_line;

      relative_line_count = 1LLu;

      while (std::getline(content_stream, line))
      {
        std::cerr << std::format("Processing {}:{}: '{}'\n", current_file, relative_line_count, line);
        if (line.empty())
        {
          relative_line_count++;
          absolute_line_count++;
          continue;
        }

        stripped_line = Util::Trim(Util::StripComments(Util::Trim(line)));

        if (stripped_line.empty())
        {
          relative_line_count++;
          absolute_line_count++;
          continue;
        }

        if (stripped_line.starts_with(".inc"))
        {
          if (conditional_assembly_parser_state->active_stack.back() == ConditionalAssemblyParserState::State::INCLUDE)
          {
            const auto filename_to_include = ExtractFilePathDirectiveArgument(stripped_line);
            if (filename_to_include == source_file)
            {
              throw std::runtime_error(FormatErrorMessage(std::format("'{}' cannot include itself.", source_file)));
            }
            std::cerr << std::format("Including Dependency {} in {}:{} ({} abs).\n", filename_to_include, current_file,
                                     relative_line_count, absolute_line_count);
            const auto last_relative = relative_line_count;
            IncludeFile(filename_to_include, src);
            current_file = source_file;
            relative_line_count = last_relative;
          }
          continue;
        }

        if (stripped_line.starts_with(".bin"))
        {
          if (conditional_assembly_parser_state->active_stack.back() == ConditionalAssemblyParserState::State::INCLUDE)
          {
            const auto filename_to_include = ExtractFilePathDirectiveArgument(stripped_line);
            if (filename_to_include == source_file)
            {
              throw std::runtime_error(FormatErrorMessage(std::format("'{}' cannot include itself.", source_file)));
            }
            std::cerr << std::format("Including Binary Dependency {} in {}:{} ({} abs).\n", filename_to_include, current_file,
                                     relative_line_count, absolute_line_count);
            IncludeFile(filename_to_include, src, FileIncludeType::BINARY);
            current_file = source_file;
          }
          continue;
        }

        try
        {
          if (!conditional_assembly_parser_state->HandleLine(stripped_line))
          {
            relative_line_count++;
            absolute_line_count++;
            continue;
          }
        } catch (const std::runtime_error &e)
        {
          throw std::runtime_error(FormatErrorMessage(e.what()));
        }

        src.lines.emplace_back(SourceLine{
          .source_file = source_file,
          .source_line = stripped_line,
          .relative_line_number = relative_line_count,
          .absolute_line_number = absolute_line_count,
        });

        relative_line_count++;
        absolute_line_count++;
      }

      if (conditional_assembly_parser_state->active_stack.back() == ConditionalAssemblyParserState::State::EXCLUDE)
      {
        throw std::runtime_error(FormatErrorMessage(
          std::format("{} has not been closed", ConditionalAssemblyParserState::BEGIN_CONDITIONAL_ASSEMBLY)));
      }
    }
  };
}

auto m9::ReadSource::ToSource(const std::string &filename,
                              const std::unordered_map<std::string, std::string> &syms) -> std::unique_ptr<Source>
{
  const auto source_processor = std::make_unique<SourceFileProcessor>(syms);
  auto src = std::make_unique<Source>();
  source_processor->ProcessFile(filename, *src);
  src->filename = filename;
  src->dependencies = source_processor->dependencies;
  return std::move(src);
}
