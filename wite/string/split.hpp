#pragma once

#include <string_view>
#include <vector>
#include <tuple>

///////////////////////////////////////////////////////////////////////////////

namespace wite::string {

///////////////////////////////////////////////////////////////////////////////

std::tuple<std::string_view, std::string_view, bool> first_token(std::string_view str, char delimiter) {
  const auto pos = std::find(str.begin(), str.end(), delimiter);
  return {
    {str.begin(), pos},
    {pos == str.end() ? pos : std::next(pos), str.end()},
    pos != str.end()
  };
}

///////////////////////////////////////////////////////////////////////////////

std::vector<std::string_view> split(std::string_view str, char delimiter = ' ') {
  auto result = std::vector<std::string_view>{};

  while (true) {
    auto [token, remainder, performed_a_split] = first_token(str, delimiter);
    result.push_back(std::move(token));

    if (not performed_a_split) {
      break;
    }

    str = std::move(remainder);
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::string

///////////////////////////////////////////////////////////////////////////////
