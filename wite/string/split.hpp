#pragma once

#include <string_view>
#include <vector>
#include <utility>

///////////////////////////////////////////////////////////////////////////////

namespace wite::string {

///////////////////////////////////////////////////////////////////////////////

std::pair<std::string_view, std::string_view> first_token(std::string_view str, char delimiter) {
  const auto pos = std::find(str.begin(), str.end(), delimiter);
  return {{str.begin(), pos}, {pos == str.end() ? pos : std::next(pos), str.end()}};
}

///////////////////////////////////////////////////////////////////////////////

std::vector<std::string_view> split(std::string_view str, char delimiter = ' ') {
  auto result = std::vector<std::string_view>{};

  while (true) {
    auto [token, remainder] = first_token(str, delimiter);
    result.push_back(std::move(token));

    if (remainder.empty()) {
      break;
    }

    str = std::move(remainder);
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::string

///////////////////////////////////////////////////////////////////////////////
