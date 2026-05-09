//
// Created by Richard Marks on 5/8/26.
//

#include "token_stream.h"

m9::TokenStream::TokenStream(const std::vector<Token> &token_stream) : tokens{token_stream}
{
}

void m9::TokenStream::Reset()
{
  offset = 0;
}

auto m9::TokenStream::Consume() -> const Token *
{
  if (tokens.empty() || offset >= tokens.size())
  {
    return nullptr;
  }
  const Token *token = &tokens.at(offset++);
  return token;
}

auto m9::TokenStream::Peek() const -> const Token *
{
  if (tokens.empty() || offset >= tokens.size())
  {
    return nullptr;
  }
  const auto token = &tokens.at(offset);
  return token;
}

auto m9::TokenStream::GetOffset() const -> size_t
{
  return offset;
}

auto m9::TokenStream::GetSize() const -> size_t
{
  return tokens.size();
}
