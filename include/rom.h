//
// Created by Richard Marks on 5/8/26.
//
#pragma once

#include <cstdint>

namespace m9
{
  struct Rom
  {
    struct Header
    {
      uint8_t signature[8]{0x4d, 0x39, 0x52, 0x4f, 0x4d, 0x31, 0x2e, 0x30};
      uint16_t start_address{0x19E4};
      uint16_t data_length{0};
    } header{};
    // 13x 4k memory banks is the largest program+data possible to load without
    // corrupting the system, thus we only allow writing this much data to a rom
    static constexpr auto MAX_DATA_SIZE_IN_BYTES = 0xD000;
    uint8_t data[MAX_DATA_SIZE_IN_BYTES]{0};
  };
}