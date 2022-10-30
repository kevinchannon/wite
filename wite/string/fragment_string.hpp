#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/core/assert.hpp>
#include <wite/env/environment.hpp>

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
  _WITE_NODISCARD constexpr size_t string_length(const Char_T* psz) noexcept {
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

      _WITE_NODISCARD constexpr auto operator<=>(const _data_type&) const noexcept = default;
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

    _WITE_NODISCARD constexpr auto operator<=>(const iterator& other) const _WITE_RELEASE_NOEXCEPT {
#ifdef _WITE_CONFIG_DEBUG
      _debug_verify_integrity(other);
#endif

      return _data <=> other._data;
    }

    _WITE_NODISCARD constexpr bool operator==(const iterator& other) const _WITE_RELEASE_NOEXCEPT {
#ifdef _WITE_CONFIG_DEBUG
      _debug_verify_integrity(other);
#endif

      return _data == other._data;
    }


    //TODO: The should assert on the position being in range
    _WITE_NODISCARD constexpr const_reference operator*() const { return *_data.current; }

    iterator& operator++() _WITE_RELEASE_NOEXCEPT {
#if !defined(_WITE_COMPILER_GCC)
      _WITE_DEBUG_ASSERT_FALSE(std::prev(_data.fragment_end) == _data.fragment and _data.fragment->end() == _data.current,
                               "fragment_string::operator++: already at end");
#else
      _WITE_DEBUG_ASSERT_FALSE(_data.fragment_end == _data.fragment,
                               "fragment_string::operator++: already at end");
#endif

      ++_data.current;

      if (_data.fragment->end() == _data.current and std::prev(_data.fragment_end) != _data.fragment) {
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

      const auto fragment_separation = _data.fragment - other._data.fragment;
      if (fragment_separation > 0) {
        return _distance(other._data.fragment, other._data.current, _data.fragment, _data.current);
      } else if (fragment_separation < 0) {
        return -_distance(_data.fragment, _data.current, other._data.fragment, other._data.current);
      }

      return _data.current - other._data.current;
    }

   private:
#ifdef _WITE_CONFIG_DEBUG
    void _debug_verify_integrity(const iterator& other) const {
      _WITE_DEBUG_ASSERT_FALSE(&(*other._data.debug_fragment_range_begin) != &(*_data.debug_fragment_range_begin),
                               "fragment_string: Iterators point to different parent objects");
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
      while (offset > 0) {
        const auto dist_to_fragment_end = static_cast<size_type>(std::distance(_data.current, _data.fragment->end()));
        if (offset < dist_to_fragment_end) {
          _data.current += offset;
          return;
        }

        _WITE_DEBUG_ASSERT((_data.fragment != std::prev(_data.fragment_end)) or
                               (_data.fragment == std::prev(_data.fragment_end) and offset == dist_to_fragment_end),
                           "fragment_string::_seek_forward: trying to seek beyond end of range");
        ++_data.fragment;
        if (_data.fragment == _data.fragment_end) {
          ++_data.current;
        } else {
          _data.current = _data.fragment->begin();
        }

        offset -= dist_to_fragment_end;
      }
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
      _WITE_DEBUG_ASSERT(begin <= end, "fragment_string: invalid arguments (out of order) to _sublength");

      return std::accumulate(begin, end, size_type{}, [](auto&& len, auto&& fragment) { return len += fragment.length(); });
    }

    _WITE_NODISCARD static constexpr difference_type _distance(auto low_frag,
                                                               auto low_current,
                                                               auto high_frag,
                                                               auto high_current) noexcept {
      return _sublength(low_frag, high_frag) - std::distance(low_frag->begin(), low_current) +
             std::distance(high_frag->begin(), high_current);
    }
  };

  using value_type             = Char_T;
  using size_type              = size_t;
  using difference_type        = std::ptrdiff_t;
  using reference              = const value_type&;
  using const_reference        = reference;
  using pointer                = const value_type*;
  using const_pointer          = pointer;
  using const_iterator         = iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator       = std::reverse_iterator<iterator>;

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

  _WITE_NODISCARD std::basic_string<value_type> to_str() const {
    auto out = std::basic_string<value_type>{};
    out.reserve(length());
    out = std::accumulate(_fragments.cbegin(), _fragments.cend(), std::move(out), [](auto&& str, const auto& fragment) {
      return str += fragment.data();
    });
    return out;
  }

  _WITE_NODISCARD constexpr const storage_type& fragments() const noexcept { return _fragments; }

  _WITE_NODISCARD constexpr auto length() const noexcept {
    return std::accumulate(
        _fragments.begin(), _fragments.end(), size_t{}, [](auto&& len, auto&& fragment) { return len += fragment.length(); });
  }

  _WITE_NODISCARD constexpr auto size() const noexcept { return length(); }
  _WITE_NODISCARD constexpr auto max_size() const noexcept { return length(); }
  _WITE_NODISCARD constexpr auto capacity() const noexcept { return length(); }

  _WITE_NODISCARD constexpr auto cbegin() const noexcept {
    return iterator(
        _fragments.begin(), _fragments.end(), _fragments.front().begin() _WITE_FRAG_STR_DEBUG_ARG(_fragments.begin()));
  }
  _WITE_NODISCARD constexpr auto begin() const noexcept { return cbegin(); }
  _WITE_NODISCARD constexpr auto cend() const noexcept {
    return iterator(
        std::prev(_fragments.end()), _fragments.end(), _fragments.back().end() _WITE_FRAG_STR_DEBUG_ARG(_fragments.begin()));
  }
  _WITE_NODISCARD constexpr auto end() const noexcept { return cend(); }
  _WITE_NODISCARD constexpr auto crbegin() const noexcept { return const_reverse_iterator{this->end()}; }
  _WITE_NODISCARD constexpr auto rbegin() const noexcept { return crbegin(); }
  _WITE_NODISCARD constexpr auto crend() const noexcept { return const_reverse_iterator{this->begin()}; }
  _WITE_NODISCARD constexpr auto rend() const noexcept { return crend(); }

  _WITE_NODISCARD constexpr const_reference at(size_type pos) const {
    if (pos >= length()) {
      throw std::out_of_range{"Error: accessing fragment_string beyond end of string"};
    }

    return this->operator[](pos);
  }

  _WITE_NODISCARD constexpr const_reference operator[](size_type pos) const noexcept { return *std::next(begin(), pos); }

  _WITE_NODISCARD constexpr const_reference front() const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(length() > 0, "accessing fragment_string beyond end of string");

    return _fragments.front().front();
  }

  _WITE_NODISCARD constexpr const_reference back() const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(length() > 0, "accessing fragment_string beyond end of string");

    return _fragments.back().back();
  }

  _WITE_NODISCARD constexpr bool empty() const noexcept { return 0 == length(); }

  _WITE_NODISCARD constexpr int compare(const std::string_view& other) const _WITE_RELEASE_NOEXCEPT {
    return _compare(other.begin(), other.end());
  }
  _WITE_NODISCARD constexpr int compare(const std::string& other) const _WITE_RELEASE_NOEXCEPT {
    return _compare(other.begin(), other.end());
  }
  _WITE_NODISCARD constexpr int compare(const char* other) const _WITE_RELEASE_NOEXCEPT {
    return compare(std::string_view{other});
  }

  template <size_t OTHER_FRAG_COUNT>
  _WITE_NODISCARD constexpr int compare(const basic_fragment_string<value_type, OTHER_FRAG_COUNT>& other) const
      _WITE_RELEASE_NOEXCEPT {
    return _compare(other.begin(), other.end());
  }

  _WITE_NODISCARD constexpr bool starts_with(value_type c) const noexcept { return length() > 0 ? front() == c : false; }

  _WITE_NODISCARD constexpr bool starts_with(std::basic_string_view<value_type> sv) const noexcept {
    return _match_substring(this->begin(), this->length(), sv.begin(), sv.end(), sv.length());
  }

  _WITE_NODISCARD constexpr bool starts_with(const Char_T* pszStr) const noexcept {
    return starts_with(std::basic_string_view<value_type>(pszStr));
  }

  template <size_t OTHER_FRAG_COUNT>
  _WITE_NODISCARD constexpr bool starts_with(basic_fragment_string<value_type, OTHER_FRAG_COUNT> other) const noexcept {
    return _match_substring(this->begin(), this->length(), other.begin(), other.end(), other.length());
  }

  _WITE_NODISCARD constexpr bool ends_with(Char_T c) const noexcept { return length() > 0 ? back() == c : false; }

  _WITE_NODISCARD constexpr bool ends_with(std::basic_string_view<value_type> sv) const noexcept {
    return _match_substring(this->rbegin(), this->length(), sv.rbegin(), sv.rend(), sv.length());
  }

  _WITE_NODISCARD constexpr bool ends_with(const Char_T* pszStr) const noexcept {
    return ends_with(std::basic_string_view<value_type>(pszStr));
  }

  template <size_t OTHER_FRAG_COUNT>
  _WITE_NODISCARD constexpr bool ends_with(basic_fragment_string<value_type, OTHER_FRAG_COUNT> other) const noexcept {
    return _match_substring(this->rbegin(), this->length(), other.rbegin(), other.rend(), other.length());
  }

  _WITE_NODISCARD constexpr bool contains(value_type c) const noexcept {
    return std::ranges::any_of(_fragments, [c](const auto& f) { return _fragment_type::npos != f.find(c); });
  }

  _WITE_NODISCARD constexpr bool contains(std::basic_string_view<Char_T> sv) const noexcept {
    return std::basic_string<Char_T>::npos != find(std::move(sv));
  }

  _WITE_NODISCARD constexpr bool contains(const Char_T* pszStr) const noexcept {
    return contains(std::basic_string_view<value_type>(pszStr));
  }

  template <size_t OTHER_FRAG_COUNT>
  _WITE_NODISCARD constexpr bool contains(basic_fragment_string<value_type, OTHER_FRAG_COUNT> other) const noexcept {
    const auto this_len  = this->length();
    const auto other_len = other.length();
    if (other_len > this_len) {
      return false;
    }

    const auto other_begin = other.begin();
    const auto other_end   = other.end();
    const auto this_end    = std::next(this->begin(), this_len - other_len);
    auto effective_len     = this_len;

    for (auto it = this->begin(); it != this_end and effective_len != 0; ++it, --effective_len) {
      if (std::equal(other_begin, other_end, it, std::next(it, other_len))) {
        return true;
      }
    }

    return false;
  }

  _WITE_NODISCARD constexpr std::basic_string<Char_T> substr(size_type pos   = 0,
                                                             size_type count = std::basic_string<Char_T>::npos) const {
    const auto len = length();
    if (pos >= len) {
      throw std::out_of_range{"fragment_string: substring start out of range"};
    }

    const auto it_end = ((count == std::basic_string<Char_T>::npos) or (pos + count >= len))
                            ? this->end()
                            : std::next(this->begin(), pos + count);
    return std::basic_string<Char_T>(std::next(this->begin(), pos), it_end);
  }

  _WITE_NODISCARD constexpr size_type copy(Char_T* dest, size_type count, size_type pos = 0) const {
    const auto len = length();
    if (pos >= len) {
      throw std::out_of_range{"fragment_string: copy start out of range"};
    }

    // TODO: Figure out why this doesnt protect us in GCC.
    count = std::min(count, len - pos);

    std::copy_n(std::next(begin(), pos), count, dest);

    return count;
  }

  _WITE_NODISCARD constexpr size_type find(Char_T ch, size_type pos = 0) const noexcept {
    if (pos >= length()) {
      return std::basic_string<Char_T>::npos;
    }

    auto [fragment, length_of_checked_fragments] = _seek_fragment_containing_position(pos);

    for (; fragment < _fragments.end(); ++fragment) {
      const auto position = fragment->find(ch);
      if (std::basic_string_view<Char_T>::npos == position) {
        length_of_checked_fragments += fragment->length();
      } else {
        return length_of_checked_fragments + position;
      }
    }

    return std::basic_string<Char_T>::npos;
  }

  _WITE_NODISCARD constexpr size_type find(const std::basic_string_view<Char_T> sv, size_type pos = 0) const noexcept {
    const auto this_len = this->length();
    if (pos + sv.size() > this_len) {
      return std::basic_string<Char_T>::npos;
    }

    const auto sv_len = sv.length();
    if (sv_len > this_len) {
      return std::basic_string<Char_T>::npos;
    }

    const auto this_end = this->end();
    auto out            = pos;

    const auto [fragment, length_of_checked_fragments] = _seek_fragment_containing_position(pos);

    auto start = const_iterator(fragment,
                                _fragments.end(),
                                std::next(fragment->begin(), pos - length_of_checked_fragments)
                                    _WITE_FRAG_STR_DEBUG_ARG(_fragments.begin()));

    for (auto it = start, curr_end = std::next(start, sv_len);; ++it, ++curr_end, ++out) {
      if (std::equal(sv.begin(), sv.end(), it, curr_end)) {
        return out;
      }

      if (curr_end == this_end) {
        break;
      }
    }

    return std::basic_string<Char_T>::npos;
  }

 private:
  template <typename ThisIter_T, typename OtherIter_T>
  _WITE_NODISCARD static constexpr bool _match_substring(ThisIter_T this_begin,
                                                         size_type this_length,
                                                         OtherIter_T other_begin,
                                                         OtherIter_T other_end,
                                                         size_type other_length) noexcept {
    if (this_length == 0) {
      return false;
    }

    if (this_length < other_length) {
      return false;
    }

    return std::equal(other_begin, other_end, this_begin, std::next(this_begin, other_length));
  }

  template <typename Iter_T>
  constexpr int _compare(Iter_T begin, Iter_T end) const _WITE_RELEASE_NOEXCEPT {
    auto it_this        = this->begin();
    const auto end_this = this->end();

    for (; it_this != end_this and begin != end; ++it_this, ++begin) {
      if (*it_this < *begin) {
        return -1;
      }

      if (*it_this > *begin) {
        return 1;
      }
    }

    if (begin == end) {
      if (it_this == end_this) {
        return 0;
      } else {
        return 1;
      }
    }

    _WITE_DEBUG_ASSERT(it_this == end_this, "Failed to compare fragment_string");

    return -1;
  }

  std::pair<typename storage_type::const_iterator, size_type> _seek_fragment_containing_position(size_type pos) const noexcept {
    if (pos == 0) {
      return {_fragments.begin(), 0};
    }

    auto length_of_checked_fragments = size_type{0};
    auto fragment                    = _fragments.begin();

    for (; length_of_checked_fragments <= pos; ++fragment) {
      length_of_checked_fragments += fragment->length();
    }

    --fragment;
    length_of_checked_fragments -= fragment->length();

    return {fragment, length_of_checked_fragments};
  }

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

namespace string_literals {
  constexpr basic_fragment_string<char, 1> operator""_fs(const char* str, std::size_t len) {
    return fragment_string{str};
  }

  constexpr basic_fragment_string<wchar_t, 1> operator""_wfs(const wchar_t* str, std::size_t len) {
    return fragment_wstring{str};
  }
}  // namespace string_literals

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////

#include <ostream>

template <typename Char_T, size_t FRAGMENT_COUNT>
std::ostream& operator<<(std::ostream& os, const wite::basic_fragment_string<Char_T, FRAGMENT_COUNT>& fs) {
  for (const auto& f : fs.fragments()) {
    os << f;
  }

  return os;
}

template <typename Char_T, size_t FRAGMENT_COUNT>
std::wostream& operator<<(std::wostream& os, const wite::basic_fragment_string<Char_T, FRAGMENT_COUNT>& fs) {
  for (const auto& f : fs.fragments()) {
    os << f;
  }

  return os;
}

///////////////////////////////////////////////////////////////////////////////

#undef _WITE_FRAG_STR_DEBUG_ARG

///////////////////////////////////////////////////////////////////////////////
