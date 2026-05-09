//
// Created by Richard Marks on 5/6/26.
//
#pragma once

#include <string>

namespace m9
{
  struct Rom;
  struct RomFile
  {
    static void Write(const std::string& filename, const Rom & rom);
    static void Read(const std::string& filename, Rom & rom);
  };

}
