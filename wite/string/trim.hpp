#pragma once

#include <algorithm>
#include <cwctype>
#include <string>
#include <string_view>
#include <type_traits>

namespace wite::string {

///////////////////////////////////////////////////////////////////////////////

namespace detail {
  template <typename Char_T>
  [[nodiscard]] constexpr auto is_not_whitespace(Char_T c) noexcept -> bool {
    if constexpr (std::is_same_v<Char_T, char>) {
      return not std::isspace(c);
    } else if constexpr (std::is_same_v<Char_T, wchar_t>) {
      return not std::iswspace(c);
    } else {
      static_assert(std::is_same_v<Char_T, char>, "Invalid character type");
    }
  }
}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename String_T>
[[nodiscard]] std::basic_string<typename String_T::value_type> trim_left(const String_T& str) {
  return {std::find_if(str.begin(), str.end(), detail::is_not_whitespace<typename String_T::value_type>), str.end()};
}

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T>
[[nodiscard]] std::basic_string<Char_T> trim_left(const Char_T* str) {
  return trim_left(std::basic_string_view<Char_T>{str});
}

///////////////////////////////////////////////////////////////////////////////

template <typename String_T>
[[nodiscard]] std::basic_string<typename String_T::value_type> trim_right(const String_T& str) {
  return {str.begin(), std::find_if(str.rbegin(), str.rend(), detail::is_not_whitespace<typename String_T::value_type>).base()};
}

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T>
[[nodiscard]] std::basic_string<Char_T> trim_right(const Char_T* str) {
  return trim_right(std::basic_string_view<Char_T>{str});
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::string

///////////////////////////////////////////////////////////////////////////////