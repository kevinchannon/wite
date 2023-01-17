/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/env/environment.hpp>
#include <wite/string/trim.hpp>

#ifdef WITE_NO_EXCEPTIONS
#error "Exceptions are required if string/split.hpp is included"
#endif

#include <string>
#include <string_view>
#include <type_traits>

namespace wite::string {

///////////////////////////////////////////////////////////////////////////////

template <typename String_T>
_WITE_NODISCARD std::basic_string<typename String_T::value_type> strip(String_T&& str) {
  return trim_left(trim_right(std::forward<String_T>(str)));
}

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T>
_WITE_NODISCARD std::basic_string<Char_T> strip(const Char_T* str) {
  return trim_left(trim_right(std::basic_string_view<Char_T>{str}));
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::string

///////////////////////////////////////////////////////////////////////////////
