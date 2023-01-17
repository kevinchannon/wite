/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/env/environment.hpp>

#ifdef WITE_NO_EXCEPTIONS
#error "Exceptions are required if string/join.hpp is included"
#endif

#include <algorithm>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>

///////////////////////////////////////////////////////////////////////////////

namespace wite::string {

///////////////////////////////////////////////////////////////////////////////

namespace detail::join {
  template <typename Char_T>
  _WITE_NODISCARD _WITE_CONSTEVAL std::enable_if_t<std::is_same_v<char, Char_T>, char> space_character() noexcept {
      return ' ';
  }

  template <typename Char_T>
  _WITE_NODISCARD _WITE_CONSTEVAL std::enable_if_t<std::is_same_v<wchar_t, Char_T>, wchar_t> space_character() noexcept {
      return L' ';
  }
}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename Range_T>
_WITE_NODISCARD std::basic_string<typename Range_T::value_type::value_type> join(
    const Range_T& strings,
    typename Range_T::value_type::value_type delimiter = detail::join::space_character<typename Range_T::value_type::value_type>()) {
  using Char_t = typename Range_T::value_type::value_type;

  switch (strings.size()) {
    case 0:
      return std::basic_string<Char_t>{};
    case 1:
      return *strings.begin();
    default:;  // Fall through to main body of function.
  }

  const auto size =
      std::accumulate(
          strings.begin(), strings.end(), size_t(0), [](const auto& curr, const auto& str) { return curr + str.length(); }) +
      strings.size() - 1;

  std::basic_string<Char_t> result(size, Char_t{});
  
  auto pos = std::copy(strings.begin()->begin(), strings.begin()->end(), result.begin());
  std::for_each(std::next(strings.begin()), strings.end(), [&pos, delimiter](const auto& s) {
    *pos++ = delimiter;
    pos    = std::copy(s.begin(), s.end(), pos);
  });

  return result;
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::string