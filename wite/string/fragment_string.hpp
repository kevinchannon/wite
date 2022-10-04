#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <numeric>
#include <string>

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

namespace detail {

  ///////////////////////////////////////////////////////////////////////////////

  template <typename Char_T>
  [[nodiscard]] size_t string_length(const Char_T* psz) {
    if constexpr (std::is_same_v<Char_T, char>) {
      return std::strlen(psz);
    } else if constexpr (std::is_same_v<Char_T, wchar_t>) {
      return std::wcslen(psz);
    } else {
      static_assert(std::bool_constant<std::is_same_v<Char_T, char>>::value, "Invalid char type");
    }
  }
}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T, size_t FRAGMENT_COUNT = 1>
class basic_fragment_string {
 public:
  using storage_type = std::array<const Char_T*, FRAGMENT_COUNT>;

  class iterator {
   public:
    using value_type      = Char_T;
    using size_type       = size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = const value_type&;
    using const_reference = reference;
    using pointer         = const value_type*;
    using const_pointer   = pointer;

    iterator(const basic_fragment_string& parent)
        : _fragment{parent.fragments().begin()}, _fragment_end{parent.fragments().end()}, _current{*_fragment} {}

    [[nodiscard]] constexpr const_reference operator*() const { return *_current; }

    iterator& operator++() _WITE_RELEASE_NOEXCEPT {
      ++_current;
      if (0 == *_current and _fragment != std::prev(_fragment_end)) {
        ++_fragment;
        _current = *_fragment;
      }

      return *this;
    }

   private:
    basic_fragment_string::storage_type::const_iterator _fragment;
    basic_fragment_string::storage_type::const_iterator _fragment_end;
    pointer _current;
  };

  using value_type      = Char_T;
  using size_type       = size_t;
  using difference_type = std::ptrdiff_t;
  using reference       = const value_type&;
  using const_reference = reference;
  using pointer         = const value_type*;
  using const_pointer   = pointer;
  using const_iterator  = iterator;

  WITE_DEFAULT_CONSTRUCTORS(basic_fragment_string);

  constexpr basic_fragment_string(pointer str) : _fragments{{str}} {}

  template <size_t STR_LEN>
  constexpr basic_fragment_string(value_type psz[STR_LEN]) noexcept : _fragments{psz} {}

  template <size_t LEFT_FRAG_COUNT, size_t RIGHT_FRAG_COUNT>
  constexpr basic_fragment_string(const basic_fragment_string<value_type, LEFT_FRAG_COUNT>& left,
                                  const basic_fragment_string<value_type, RIGHT_FRAG_COUNT>& right) noexcept {
    const auto it = std::copy(left.fragments().cbegin(), left.fragments().cend(), _fragments.begin());
    std::copy(right.fragments().cbegin(), right.fragments().cend(), it);
  }

  [[nodiscard]] std::basic_string<value_type> to_str() const {
    auto out = std::basic_string<value_type>{};
    out.reserve(length());
    out = std::accumulate(
        _fragments.cbegin(), _fragments.cend(), std::move(out), [](auto&& curr, auto&& next) { return curr += next; });
    return out;
  }

  [[nodiscard]] constexpr const storage_type& fragments() const noexcept { return _fragments; }

  [[nodiscard]] constexpr auto length() const noexcept {
    return std::accumulate(_fragments.begin(), _fragments.end(), size_t{}, [](auto&& len, auto&& fragment) {
      return len += detail::string_length(fragment);
    });
  }

  [[nodiscard]] constexpr auto size() const noexcept { return length(); }

  [[nodiscard]] auto begin() const noexcept { return iterator(*this); }

 private:
  storage_type _fragments;
};

///////////////////////////////////////////////////////////////////////////////

using fragment_string  = basic_fragment_string<char>;
using fragment_wstring = basic_fragment_string<wchar_t>;

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T, size_t LEFT_FRAG_COUNT, size_t STR_LEN>
basic_fragment_string<Char_T, LEFT_FRAG_COUNT + 1> operator+(const basic_fragment_string<Char_T, LEFT_FRAG_COUNT>& left,
                                                             const Char_T right[STR_LEN]) {
  return basic_fragment_string<Char_T, LEFT_FRAG_COUNT + 1>(left, basic_fragment_string<Char_T>(right));
}

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T, size_t LEFT_FRAG_COUNT>
basic_fragment_string<Char_T, LEFT_FRAG_COUNT + 1> operator+(const basic_fragment_string<Char_T, LEFT_FRAG_COUNT>& left,
                                                             const Char_T* right) {
  return basic_fragment_string<Char_T, LEFT_FRAG_COUNT + 1>(left, basic_fragment_string<Char_T>(right));
}

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T, size_t LEFT_FRAG_COUNT, size_t RIGHT_FRAG_COUNT>
basic_fragment_string<Char_T, LEFT_FRAG_COUNT + RIGHT_FRAG_COUNT> operator+(
    const basic_fragment_string<Char_T, LEFT_FRAG_COUNT>& left,
    const basic_fragment_string<Char_T, RIGHT_FRAG_COUNT>& right) {
  return basic_fragment_string<Char_T, LEFT_FRAG_COUNT + RIGHT_FRAG_COUNT>(left, right);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
