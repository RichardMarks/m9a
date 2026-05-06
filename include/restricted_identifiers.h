//
// Created by Richard Marks on 5/4/26.
//
#pragma once

namespace m9
{
  struct RestrictedIdentifiers
  {
    // all symbols are stored here as lowercase, and all restricted checks will need to lowercase the input to be tested
    static constexpr const char* SYMBOLS[] = {
      // register names are not able to be used as constant identifiers
      "ra", "rb", "rc", "rd", "re", "rf", "pc", "sp",
      // mnemonics are not able to be used as constant identifiers
      "no", "he", "wf", "so",
      "ld", "ln", "st", "sn", "mv", "cp", "pu", "po", "xr",
      "nd", "or", "eo", "nt", "sl", "sr", "ar",
      "ad", "ai", "sb", "si", "mu", "mi", "dv", "di", "dq", "qi", "ir", "dr", "sx",
      "uj", "zj", "nj", "ls", "lu", "ej",
      "pj", "jp",
      // pseudo-instruction/macro names are not able to be used as constant identifiers
      "mvstbra",
      "mvstwra",
      "mvstdra",
    };
  };
}
