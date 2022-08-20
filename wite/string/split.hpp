#pragma once

#include <string_view>
#include <vector>
#include <tuple>
#include <cstdint>

///////////////////////////////////////////////////////////////////////////////

namespace wite::string {

///////////////////////////////////////////////////////////////////////////////

enum class split_behaviour : uint8_t{
  keep_empty,
  drop_empty
};

///////////////////////////////////////////////////////////////////////////////

namespace detail {
  
inline std::string_view::const_iterator advance_past_delimiter(
  std::string_view::const_iterator begin,
  const std::string_view::const_iterator end,
  char delimiter,
  split_behaviour behaviour)
{
  auto out = (begin == end ? begin : std::next(begin));
  if (behaviour == split_behaviour::drop_empty) {
    out = std::find_if(begin, end, [delimiter](auto c) { return c != delimiter; });
  }
  
  return out;
}
  
inline std::tuple<std::string_view, std::string_view, bool> first_token(
  std::string_view str,
  char delimiter,
  split_behaviour behaviour)
{
  const auto token_end = std::find(str.begin(), str.end(), delimiter);
  auto start_next = advance_past_delimiter(token_end, str.end(), delimiter, behaviour);
  
  return {
    {str.begin(), token_end},
    {start_next, str.end()},
    token_end != str.end()
  };
}

}

///////////////////////////////////////////////////////////////////////////////

inline std::vector<std::string_view> split(std::string_view str, char delimiter = ' ', split_behaviour behaviour=split_behaviour::drop_empty) {
  auto result = std::vector<std::string_view>{};

  while (true) {
    auto [token, remainder, performed_a_split] = detail::first_token(str, delimiter, behaviour);
    
    if (split_behaviour::keep_empty == behaviour or (split_behaviour::drop_empty == behaviour and (not token.empty()))) {
      result.push_back(std::move(token));
    }

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
