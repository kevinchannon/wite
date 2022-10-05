#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <numeric>
#include <ranges>
#include <string>

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

namespace detail {

  ///////////////////////////////////////////////////////////////////////////////

  template <typename Char_T>
  [[nodiscard]] constexpr size_t string_length(const Char_T* psz) noexcept {
    if constexpr (std::is_same_v<Char_T, char>) {
      return std::strlen(psz);
    } else if constexpr (std::is_same_v<Char_T, wchar_t>) {
      return std::wcslen(psz);
    } else {
      static_assert(std::bool_constant<std::is_same_v<Char_T, char>>::value, "Invalid char type");
    }
  }

  template<typename Char_T>
  struct _fragment {
    size_t length{0};
    const Char_T* data{nullptr};

    WITE_DEFAULT_CONSTRUCTORS(_fragment);

    _fragment(size_t len, const Char_T* data) : length{len}, data{data} {}
  };

}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T, size_t FRAGMENT_COUNT = 1>
class basic_fragment_string {

 public:
  using storage_type = std::array<detail::_fragment<Char_T>, FRAGMENT_COUNT>;

  class iterator {
   public:
    using value_type      = Char_T;
    using size_type       = size_t;
    using difference_type = std::ptrdiff_t;
    using reference       = const value_type&;
    using const_reference = reference;
    using pointer         = const value_type*;
    using const_pointer   = pointer;

    iterator(basic_fragment_string::storage_type::const_iterator begin_fragment,
             basic_fragment_string::storage_type::const_iterator end_fragment,
             pointer current)
        : _fragment{begin_fragment}, _fragment_end{end_fragment}, _current{current} {}

    [[nodiscard]] constexpr auto operator<=>(const iterator&) const = default;

    [[nodiscard]] constexpr const_reference operator*() const { return *_current; }

    iterator& operator++() _WITE_RELEASE_NOEXCEPT {
      ++_current;
      if (0 == *_current and _fragment != std::prev(_fragment_end)) {
        ++_fragment;
        _current = _fragment->data;
      }

      return *this;
    }

    iterator& operator--() _WITE_RELEASE_NOEXCEPT {
      if (_current == _fragment->data) {
        --_fragment;
        _current = _fragment->data + _fragment->length;
      }

      --_current;
      return *this;
    }

   private:
    void _seek(size_type offset) {
      auto fragment_start_offset = size_t{0};

      _fragment = std::find_if(_fragment, _fragment_end, [offset, &fragment_start_offset](const auto& f) {
        if (fragment_start_offset + f.length >= offset) {
          return true;
        }

        fragment_start_offset += f.length;
      });

      _current = _fragment->data + (offset - fragment_start_offset);
    }

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

  constexpr basic_fragment_string(pointer str) : _fragments{{{detail::string_length(str), str}}} {}

  template <size_t STR_LEN>
  constexpr basic_fragment_string(value_type psz[STR_LEN]) noexcept : _fragments{{{detail::string_length(psz), psz}}} {}

  template <size_t LEFT_FRAG_COUNT, size_t RIGHT_FRAG_COUNT>
  constexpr basic_fragment_string(const basic_fragment_string<value_type, LEFT_FRAG_COUNT>& left,
                                  const basic_fragment_string<value_type, RIGHT_FRAG_COUNT>& right) noexcept {
    const auto it = std::copy(left.fragments().begin(), left.fragments().end(), _fragments.begin());
    std::copy(right.fragments().begin(), right.fragments().end(), it);
  }

  [[nodiscard]] std::basic_string<value_type> to_str() const {
    auto out = std::basic_string<value_type>{};
    out.reserve(length());
    out = std::accumulate(_fragments.cbegin(), _fragments.cend(), std::move(out), [](auto&& str, const auto& fragment) {
      return str += fragment.data;
    });
    return out;
  }

  [[nodiscard]] constexpr const storage_type& fragments() const noexcept { return _fragments; }

  [[nodiscard]] constexpr auto length() const noexcept {
    return std::accumulate(
        _fragments.begin(), _fragments.end(), size_t{}, [](auto&& len, auto&& fragment) { return len += fragment.length; });
  }

  [[nodiscard]] constexpr auto size() const noexcept { return length(); }

  [[nodiscard]] constexpr auto begin() const noexcept {
    return iterator(_fragments.begin(), _fragments.end(), _fragments.front().data);
  }
  [[nodiscard]] constexpr auto end() const noexcept {
    return iterator(std::prev(_fragments.end()), _fragments.end(), std::next(_fragments.back().data, _fragments.back().length));
  }

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
