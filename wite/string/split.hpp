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

template<typename Iter_T, typename Char_T>  
[[nodiscard]] Iter_T advance_past_delimiter(Iter_T begin, const Iter_T end, Char_T delimiter, split_behaviour behaviour) noexcept
{
  auto out = (begin == end ? begin : std::next(begin));
  if (behaviour == split_behaviour::drop_empty) {
    out = std::find_if(begin, end, [delimiter](auto c) { return c != delimiter; });
  }
  
  return out;
}

template<typename Char_T>
[[nodiscard]] constexpr auto space_character() noexcept -> Char_T
{
  if constexpr (std::is_same_v<Char_T, char>) {
    return ' ';
  } else if constexpr (std::is_same_v<Char_T, wchar_t>) {
    return L' ';
  } else {
    static_assert(std::is_same_v<Char_T, char>, "Invalid character type");
  }
}
  
template<typename Result_T>
[[nodiscard]] std::tuple<Result_T, std::basic_string_view<typename Result_T::value_type>, bool> first_token(
  std::basic_string_view<typename Result_T::value_type> str,
  typename Result_T::value_type delimiter,
  split_behaviour behaviour) noexcept
{
  const auto token_end = std::find(str.begin(), str.end(), delimiter);
  auto start_next = advance_past_delimiter(token_end, str.end(), delimiter, behaviour);
  
  return {
    {str.begin(), token_end},
    {start_next, str.end()},
    token_end != str.end()
  };
}

} // namespace detail

///////////////////////////////////////////////////////////////////////////////

template<typename Result_T>
[[nodiscard]] Result_T split_to(
  std::basic_string_view<typename Result_T::value_type::value_type> str,
  typename Result_T::value_type::value_type delimiter = detail::space_character<typename Result_T::value_type::value_type>(),
  split_behaviour behaviour = split_behaviour::drop_empty) noexcept
{
  auto out = Result_T{};

  while (true) {
    auto [token, remainder, performed_a_split] = detail::first_token<typename Result_T::value_type>(str, delimiter, behaviour);
    
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

template<typename Char_T>
requires std::is_pod_v<Char_T>
[[nodiscard]] std::vector<std::basic_string<Char_T>> split_to(
  const Char_T* str,
  Char_T delimiter=detail::space_character<Char_T>(),
  split_behaviour behaviour=split_behaviour::drop_empty) noexcept
{
  return split_to(std::basic_string_view<Char_T>(str), delimiter, behaviour);
}

///////////////////////////////////////////////////////////////////////////////

template<typename Char_T>
[[nodiscard]] std::vector<std::basic_string<Char_T>> split(
  std::basic_string_view<Char_T> str,
  Char_T delimiter = detail::space_character<Char_T>(), 
  split_behaviour behaviour=split_behaviour::drop_empty) noexcept
{
  return split_to<std::vector<std::basic_string<Char_T>>>(str, delimiter, behaviour);
}

///////////////////////////////////////////////////////////////////////////////

template<typename Char_T>
requires std::is_pod_v<Char_T>
[[nodiscard]] std::vector<std::basic_string<Char_T>> split(const Char_T* str) noexcept {
  return split(std::basic_string_view<Char_T>(str));
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::string

///////////////////////////////////////////////////////////////////////////////
