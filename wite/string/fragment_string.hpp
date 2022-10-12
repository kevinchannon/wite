#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>
#include <wite/core/assert.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>

///////////////////////////////////////////////////////////////////////////////

#ifdef _WITE_CONFIG_DEBUG
#define _WITE_FRAG_STR_DEBUG_ARG(arg) , arg
#else
#define _WITE_FRAG_STR_DEBUG_ARG(arg)
#endif  // _WITE_CONFIG_DEBUG

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
}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T, size_t FRAGMENT_COUNT = 1>
class basic_fragment_string {
  using _fragment_type = std::basic_string_view<Char_T>;

 public:
  using storage_type = std::array<_fragment_type, FRAGMENT_COUNT>;

  class iterator {
    using _fragment_iterator = typename basic_fragment_string::storage_type::const_iterator;

    struct _data_type {
      _fragment_iterator fragment;
      const _fragment_iterator fragment_end;
      typename basic_fragment_string::storage_type::value_type::const_iterator current;

#ifdef _WITE_CONFIG_DEBUG
      const typename basic_fragment_string::storage_type::const_iterator debug_fragment_range_begin;
#endif

      [[nodiscard]] constexpr auto operator<=>(const _data_type&) const noexcept = default;
    } _data;

   public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = Char_T;
    using size_type         = size_t;
    using difference_type   = std::ptrdiff_t;
    using reference         = const value_type&;
    using const_reference   = reference;
    using pointer           = const value_type*;
    using const_pointer     = pointer;

    iterator(typename basic_fragment_string::storage_type::const_iterator begin_fragment,
             typename basic_fragment_string::storage_type::const_iterator end_fragment,
             typename basic_fragment_string::storage_type::value_type::const_iterator current
                 _WITE_FRAG_STR_DEBUG_ARG(typename basic_fragment_string::storage_type::const_iterator fragment_range_begin))
        : _data{begin_fragment, end_fragment, current _WITE_FRAG_STR_DEBUG_ARG(fragment_range_begin)} {}

    [[nodiscard]] constexpr auto operator<=>(const iterator& other) const _WITE_RELEASE_NOEXCEPT {
#ifdef _WITE_CONFIG_DEBUG
      _debug_verify_integrity(other);
#endif

      return _data <=> other._data;
    }

    [[nodiscard]] constexpr bool operator==(const iterator& other) const _WITE_RELEASE_NOEXCEPT {
#ifdef _WITE_CONFIG_DEBUG
      _debug_verify_integrity(other);
#endif

      return _data == other._data;
    }

    [[nodiscard]] constexpr const_reference operator*() const { return *_data.current; }

    iterator& operator++() _WITE_RELEASE_NOEXCEPT {
      _WITE_DEBUG_ASSERT_FALSE(
          _data.fragment == std::prev(_data.fragment_end) and _data.current == std::prev(_data.fragment_end)->end(),
          "fragment_string::operator++: already at end");

      ++_data.current;
      if (_data.fragment->end() == _data.current and _data.fragment != std::prev(_data.fragment_end)) {
        ++_data.fragment;
        _data.current = _data.fragment->begin();
      }

      return *this;
    }

    iterator& operator--() _WITE_RELEASE_NOEXCEPT {
      _WITE_DEBUG_ASSERT_FALSE(_data.fragment == _data.debug_fragment_range_begin and _data.current == _data.fragment->begin(),
                               "fragment_string::operator--: already at beginning");

      if (_data.current == _data.fragment->begin()) {
        --_data.fragment;
        _data.current = _data.fragment->end();
      }

      --_data.current;
      return *this;
    }

    iterator& operator+=(difference_type offset) _WITE_RELEASE_NOEXCEPT {
      _seek(offset);
      return *this;
    }

    iterator& operator-=(difference_type offset) _WITE_RELEASE_NOEXCEPT {
      _seek(-offset);
      return *this;
    }

    iterator operator+(difference_type offset) const _WITE_RELEASE_NOEXCEPT {
      auto out = *this;
      out += offset;
      return out;
    }

    iterator operator-(difference_type offset) const _WITE_RELEASE_NOEXCEPT {
      auto out = *this;
      out -= offset;
      return out;
    }

    difference_type operator-(const iterator& other) const _WITE_RELEASE_NOEXCEPT {
#ifdef _WITE_CONFIG_DEBUG
      _debug_verify_integrity(other);
#endif

      auto distance = difference_type{0};

      const auto fragment_separation = _data.fragment - other._data.fragment;
      if (fragment_separation > 0) {
        return _sublength(other._data.fragment, _data.fragment) + std::distance(_data.fragment->begin(), _data.current) -
               std::distance(other._data.fragment->begin(), other._data.current);
      } else if (fragment_separation < 0) {
        return std::distance(other._data.fragment->begin(), other._data.current) -
               std::distance(_data.fragment->begin(), _data.current) - _sublength(_data.fragment, other._data.fragment);
      }

      return _data.current - other._data.current;
    }

   private:
#ifdef _WITE_CONFIG_DEBUG
    void _debug_verify_integrity(const iterator& other) const {
      _WITE_DEBUG_ASSERT_FALSE(&(*other._data.debug_fragment_range_begin) != &(*_data.debug_fragment_range_begin),
                               "ERROR: Iterators point to different parent objects");
    }
#endif

    void _seek(difference_type offset) {
      if (offset > 0) {
        _seek_forward(static_cast<size_type>(offset));
      } else {
        _seek_backward(static_cast<size_type>(-offset));
      }
    }

    void _seek_forward(size_type offset) {
      _data.fragment = std::find_if(_data.fragment, _data.fragment_end, [&offset](const auto& f) {
        const auto fragment_len = f.length();
        if (fragment_len > offset) {
          return true;
        }

        offset -= fragment_len;
        return false;
      });

      _WITE_DEBUG_ASSERT(_data.fragment != _data.fragment_end,
                         "fragment_string::_seek_forward: trying to seek beyond end of range");

      _data.current = std::next(_data.fragment->begin(), offset);
    }

    void _seek_backward(size_type offset) {
      while (offset > 0) {
        const auto idx = static_cast<size_type>(std::distance(_data.fragment->begin(), _data.current));
        if (offset <= idx) {
          _data.current -= offset;
          return;
        }

        _WITE_DEBUG_ASSERT(_data.fragment != _data.debug_fragment_range_begin,
                           "fragment_string::_seek_backward: trying to seek to before start of range");
        --_data.fragment;
        _data.current = _data.fragment->end();
        offset -= idx;
      }
    }

    static size_type _sublength(_fragment_iterator begin, _fragment_iterator end) _WITE_RELEASE_NOEXCEPT {
      return std::accumulate(begin, end, size_type{}, [](auto&& len, auto&& fragment) { return len += fragment.length(); });
    }
  };

  using value_type      = Char_T;
  using size_type       = size_t;
  using difference_type = std::ptrdiff_t;
  using reference       = const value_type&;
  using const_reference = reference;
  using pointer         = const value_type*;
  using const_pointer   = pointer;
  using const_iterator  = iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = std::reverse_iterator<iterator>;

  WITE_DEFAULT_CONSTRUCTORS(basic_fragment_string);

  constexpr basic_fragment_string(pointer str) : _fragments{{{str}}} {}

  template <size_t STR_LEN>
  constexpr basic_fragment_string(value_type psz[STR_LEN]) noexcept : _fragments{{{psz}}} {}

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
      return str += fragment.data();
    });
    return out;
  }

  [[nodiscard]] constexpr const storage_type& fragments() const noexcept { return _fragments; }

  [[nodiscard]] constexpr auto length() const noexcept {
    return std::accumulate(
        _fragments.begin(), _fragments.end(), size_t{}, [](auto&& len, auto&& fragment) { return len += fragment.length(); });
  }

  [[nodiscard]] constexpr auto size() const noexcept { return length(); }

  [[nodiscard]] constexpr auto begin() const noexcept {
    return iterator(
        _fragments.begin(), _fragments.end(), _fragments.front().begin() _WITE_FRAG_STR_DEBUG_ARG(_fragments.begin()));
  }
  [[nodiscard]] constexpr auto end() const noexcept {
    return iterator(
        std::prev(_fragments.end()), _fragments.end(), _fragments.back().end() _WITE_FRAG_STR_DEBUG_ARG(_fragments.begin()));
  }
  [[nodiscard]] constexpr auto rbegin() const noexcept {
    return const_reverse_iterator{this->end()};
  }
  [[nodiscard]] constexpr auto rend() const noexcept {
    return const_reverse_iterator{this->begin()};
  }

  [[nodiscard]] constexpr const_reference at(size_type pos) const {
    if (pos >= length()) {
      throw std::out_of_range{"Error: accessing fragment_string beyond end of string"};
    }

    return this->operator[](pos);
  }

  [[nodiscard]] constexpr const_reference operator[](size_type pos) const noexcept { return *std::next(begin(), pos); }

  [[nodiscard]] constexpr const_reference front() const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(length() > 0, "accessing fragment_string beyond end of string");

    return _fragments.front().front();
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
