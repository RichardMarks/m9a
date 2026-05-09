//
// Created by Richard Marks on 5/8/26.
//
#include <fstream>
#include <format>

#include "rom.h"
#include "rom_file.h"

void m9::RomFile::Write(const std::string &filename, const Rom &rom)
{
  std::ofstream fp(filename, std::ios::binary);
  if (!fp.is_open())
  {
    throw std::runtime_error(std::format("RomFile::Write - Could not open {}", filename));
  }
  fp.write(reinterpret_cast<const std::ostream::char_type *>(&rom.header.signature), sizeof(rom.header.signature));
  fp.write(reinterpret_cast<const std::ostream::char_type *>(&rom.header.data_length), sizeof(rom.header.data_length));
  fp.write(reinterpret_cast<const std::ostream::char_type *>(&rom.header.start_address), sizeof(rom.header.start_address));
  fp.write(reinterpret_cast<const std::ostream::char_type *>(&rom.data), rom.header.data_length);
  fp.close();
}

void m9::RomFile::Read(const std::string &filename, Rom &rom)
{
  std::ifstream fp(filename, std::ios::binary);
  if (!fp.is_open())
  {
    throw std::runtime_error(std::format("RomFile::Read - Could not open {}", filename));
  }
  fp.read(reinterpret_cast<std::istream::char_type *>(&rom.header.signature), sizeof(rom.header.signature));
  fp.read(reinterpret_cast<std::istream::char_type *>(&rom.header.data_length), sizeof(rom.header.data_length));
  fp.read(reinterpret_cast<std::istream::char_type *>(&rom.header.start_address), sizeof(rom.header.start_address));
  fp.read(reinterpret_cast<std::istream::char_type *>(&rom.data), rom.header.data_length);
  fp.close();
}
