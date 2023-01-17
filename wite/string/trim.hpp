/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/env/environment.hpp>

#ifdef WITE_NO_EXCEPTIONS
#error "Exceptions are required if string/trim.hpp is included"
#endif

#include <algorithm>
#include <cwctype>
#include <string>
#include <string_view>
#include <type_traits>

namespace wite::string {

///////////////////////////////////////////////////////////////////////////////

namespace detail {
  template <typename Char_T>
  _WITE_NODISCARD constexpr auto is_not_whitespace(Char_T c) noexcept -> bool {
    if constexpr (std::is_same_v<Char_T, char>) {
      return ! std::isspace(c);
    } else if constexpr (std::is_same_v<Char_T, wchar_t>) {
      return ! std::iswspace(c);
    } else {
      static_assert(std::is_same_v<Char_T, char>, "Invalid character type");
    }
  }
}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename String_T>
_WITE_NODISCARD std::basic_string<typename String_T::value_type> trim_left(String_T&& str) {
  return {std::find_if(str.begin(), str.end(), detail::is_not_whitespace<typename String_T::value_type>), str.end()};
}

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T>
_WITE_NODISCARD std::basic_string<Char_T> trim_left(const Char_T* str) {
  return trim_left(std::basic_string_view<Char_T>{str});
}

///////////////////////////////////////////////////////////////////////////////

template <typename String_T>
_WITE_NODISCARD std::basic_string<typename String_T::value_type> trim_right(String_T&& str) {
  return {str.begin(), std::find_if(str.rbegin(), str.rend(), detail::is_not_whitespace<typename String_T::value_type>).base()};
}

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T>
_WITE_NODISCARD std::basic_string<Char_T> trim_right(const Char_T* str) {
  return trim_right(std::basic_string_view<Char_T>{str});
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::string

///////////////////////////////////////////////////////////////////////////////
