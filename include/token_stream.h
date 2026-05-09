//
// Created by Richard Marks on 5/8/26.
//
#pragma once

#include <vector>

#include "token.h"

namespace m9
{
  struct Token;

  struct TokenStream
  {
    const std::vector<Token> &tokens{};
    size_t offset{0};

    explicit TokenStream(const std::vector<Token> &token_stream);

    void Reset();

    [[nodiscard]] auto Consume() -> const Token *;

    [[nodiscard]] auto Peek() const -> const Token *;

    [[nodiscard]] auto GetOffset() const -> size_t;

    [[nodiscard]] auto GetSize() const -> size_t;
  };
}
