#pragma once

#include <wite/string/trim.hpp>

#include <string>
#include <string_view>
#include <type_traits>

namespace wite::string {

///////////////////////////////////////////////////////////////////////////////

template <typename String_T>
[[nodiscard]] std::basic_string<typename String_T::value_type> strip(const String_T& str) {
  return trim_left(trim_right(str));
}

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T>
[[nodiscard]] std::basic_string<Char_T> strip(const Char_T* str) {
  return trim_left(trim_right(std::basic_string_view<Char_T>{str}));
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::string

///////////////////////////////////////////////////////////////////////////////
