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

template<typename Iter_T>  
Iter_T advance_past_delimiter(Iter_T begin, const Iter_T end, char delimiter, split_behaviour behaviour)
{
  auto out = (begin == end ? begin : std::next(begin));
  if (behaviour == split_behaviour::drop_empty) {
    out = std::find_if(begin, end, [delimiter](auto c) { return c != delimiter; });
  }
  
  return out;
}
  
template<typename Result_T>
std::tuple<Result_T, std::string_view, bool> first_token(
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

template<typename Result_T>
std::vector<Result_T> split_to(
  std::string_view str,
  char delimiter=' ',
  split_behaviour behaviour=split_behaviour::drop_empty)
{
  auto out = std::vector<Result_T>{};

  while (true) {
    auto [token, remainder, performed_a_split] = detail::first_token<Result_T>(str, delimiter, behaviour);
    
    if (split_behaviour::keep_empty == behaviour or (split_behaviour::drop_empty == behaviour and (not token.empty()))) {
      out.push_back(std::move(token));
    }

    if (not performed_a_split) {
      break;
    }

    str = std::move(remainder);
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

inline std::vector<std::string_view> split(std::string_view str, char delimiter = ' ', split_behaviour behaviour=split_behaviour::drop_empty) {
  return split_to<std::string_view>(str, delimiter, behaviour);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::string

///////////////////////////////////////////////////////////////////////////////
