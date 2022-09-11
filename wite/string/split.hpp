#pragma once

#include <wite/env/environment.hpp>
#include <wite/common/concepts.hpp>

#include <cstdint>
#include <string_view>
#include <tuple>
#include <vector>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////

namespace wite::string {

///////////////////////////////////////////////////////////////////////////////

enum class split_behaviour : uint8_t { keep_empty, drop_empty };

///////////////////////////////////////////////////////////////////////////////

namespace detail {

  template <typename Iter_T, typename Char_T>
  _WITE_NODISCARD Iter_T advance_past_delimiter(Iter_T begin,
                                              const Iter_T end,
                                              Char_T delimiter,
                                              split_behaviour behaviour) noexcept {
    auto out = (begin == end ? begin : std::next(begin));
    if (behaviour == split_behaviour::drop_empty) {
      out = std::find_if(begin, end, [delimiter](auto c) { return c != delimiter; });
    }

    return out;
  }

  template <typename Char_T>
  _WITE_NODISCARD constexpr auto space_character() noexcept -> Char_T {
    if constexpr (std::is_same_v<Char_T, char>) {
      return ' ';
    } else if constexpr (std::is_same_v<Char_T, wchar_t>) {
      return L' ';
    } else {
      static_assert(std::is_same_v<Char_T, char>, "Invalid character type");
    }
  }

  template <typename Result_T>
  _WITE_NODISCARD std::tuple<Result_T, std::basic_string_view<typename Result_T::value_type>, bool> first_token(
      std::basic_string_view<typename Result_T::value_type> str,
      typename Result_T::value_type delimiter,
      split_behaviour behaviour) noexcept {
    const auto token_end = std::find(str.begin(), str.end(), delimiter);
    const auto start_next      = advance_past_delimiter(token_end, str.end(), delimiter, behaviour);

#if _WITE_HAS_CONCEPTS
    return {{str.begin(), token_end}, {start_next, str.end()}, token_end != str.end()};
#else
    // In C++17, string_view doesn't have an iterator-pair constructor, so we need to express things in terms
    // of a pointer and size.
    const auto token_end_offset = std::distance(str.begin(), token_end);
    const auto token            = Result_T(str.data(), token_end_offset);

    const auto start_next_offset = std::distance(str.begin(), start_next);
    const auto remainder =
        std::basic_string_view<typename Result_T::value_type>(str.data() + start_next_offset, str.length() - start_next_offset);

    return std::tuple<Result_T, std::basic_string_view<typename Result_T::value_type>, bool>(
        token, remainder, token_end != str.end());
#endif
  }

}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename Result_T>
_WITE_NODISCARD Result_T split_to(
    std::basic_string_view<typename Result_T::value_type::value_type> str,
    typename Result_T::value_type::value_type delimiter = detail::space_character<typename Result_T::value_type::value_type>(),
    split_behaviour behaviour                           = split_behaviour::drop_empty) noexcept {
  auto out = Result_T{};

  while (true) {
    auto [token, remainder, performed_a_split] = detail::first_token<typename Result_T::value_type>(str, delimiter, behaviour);

    if (split_behaviour::keep_empty == behaviour || (split_behaviour::drop_empty == behaviour && (! token.empty()))) {
      out.push_back(std::move(token));
    }

    if (! performed_a_split) {
      break;
    }

    str = std::move(remainder);
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Result_T, typename Char_T>
#if _WITE_HAS_CONCEPTS
requires common::is_pod_like<Char_T> _WITE_NODISCARD Result_T split_to(
#else
    _WITE_NODISCARD std::enable_if_t<std::is_pod_v<Char_T> , Result_T> split_to(
#endif
    const Char_T* str,
    Char_T delimiter          = detail::space_character<Char_T>(),
    split_behaviour behaviour = split_behaviour::drop_empty) noexcept {
  if (nullptr == str) {
    return {};
  }

  return split_to<Result_T>(std::basic_string_view<Char_T>(str), delimiter, behaviour);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T>
_WITE_NODISCARD std::vector<std::basic_string<Char_T>> split(std::basic_string_view<Char_T> str,
                                                           Char_T delimiter          = detail::space_character<Char_T>(),
                                                           split_behaviour behaviour = split_behaviour::drop_empty) noexcept {
  return split_to<std::vector<std::basic_string<Char_T>>>(str, delimiter, behaviour);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T>
#if _WITE_HAS_CONCEPTS
requires std::is_standard_layout_v<Char_T> && std::is_trivial_v<Char_T>
_WITE_NODISCARD std::vector<std::basic_string<Char_T>> split(const Char_T* str) noexcept {
#else
_WITE_NODISCARD std::enable_if_t<std::is_pod_v<Char_T>, std::vector<std::basic_string<Char_T>>> split(const Char_T* str) noexcept {
#endif

  if (nullptr == str) {
    return {};
  }

  return split(std::basic_string_view<Char_T>(str));
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::string

///////////////////////////////////////////////////////////////////////////////
