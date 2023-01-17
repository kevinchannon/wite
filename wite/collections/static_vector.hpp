/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/collections/dereferencing_iterator.hpp>
#include <wite/common/constructor_macros.hpp>
#include <wite/core/assert.hpp>
#include <wite/env/environment.hpp>

#include <algorithm>
#include <array>
#include <compare>
#include <cstdint>
#include <optional>
#include <stdexcept>

#ifdef _WITE_COMPILER_MSVC
#define _WITE_USING_MSVC_ARRAY_ITERATOR 1
#else
#ifdef _WITE_COMPILER_GCC
#define _WITE_USING_MSVC_ARRAY_ITERATOR 0
#else
#ifdef _WITE_COMPILER_CLANG
#ifdef _WITE_PLATFORM_OS_WINDOWS
#define _WITE_USING_MSVC_ARRAY_ITERATOR 1
#else
#define _WITE_USING_MSVC_ARRAY_ITERATOR 0
#endif  // _WITE_PLATFORM_OS_WINDOWS
#endif  // _WITE_COMPILER_CLANG
#endif  // _WITE_COMPILER_GCC
#endif  // _WITE_COMPILER_MSVC

///////////////////////////////////////////////////////////////////////////////

namespace wite::collections {

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, size_t CAPACITY>
class static_vector {
 public:
  using raw_value_type = std::optional<Value_T>;

 private:
  using data_type = std::array<std::optional<Value_T>, CAPACITY>;
  using _this_t   = static_vector<Value_T, CAPACITY>;

 public:
  using value_type             = Value_T;
  using size_type              = typename data_type::size_type;
  using difference_type        = typename data_type::difference_type;
  using pointer                = value_type*;
  using const_pointer          = const value_type*;
  using reference              = value_type&;
  using const_reference        = const value_type&;
  using iterator               = dereferencing_iterator<_this_t>;
  using const_iterator         = dereferencing_const_iterator<_this_t>;
  using reverse_iterator       = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  WITE_DEFAULT_CONSTRUCTORS(static_vector);

  constexpr explicit static_vector(uint8_t size) : _item_count{size} {}

#ifndef WITE_NO_EXCEPTIONS
  constexpr static_vector(std::initializer_list<value_type> init) {
    if (init.size() > capacity()) {
      throw std::invalid_argument{"Too many initialisation elements"};
    }

    _item_count = init.size();
    std::transform(init.begin(), init.end(), _data.begin(), [](auto&& x) { return std::optional<value_type>{x}; });
  }
#endif

  constexpr void swap(static_vector& other) noexcept {
    using std::swap;

    swap(_data, other._data);
    swap(_item_count, other._item_count);
  }

  _WITE_NODISCARD constexpr auto begin() noexcept -> iterator {
    return iterator(_data.data() _WITE_DEREF_ITER_DEBUG_ARG(this) _WITE_DEREF_ITER_DEBUG_ARG(_data.data())
                        _WITE_DEREF_ITER_DEBUG_ARG(_data.data() + _item_count));
  }
  _WITE_NODISCARD constexpr auto begin() const noexcept -> const_iterator {
    return const_iterator(_data.data() _WITE_DEREF_ITER_DEBUG_ARG(this) _WITE_DEREF_ITER_DEBUG_ARG(_data.data())
                              _WITE_DEREF_ITER_DEBUG_ARG(_data.data() + _item_count));
  }
  _WITE_NODISCARD constexpr auto end() noexcept {
    return iterator(std::next(_data.data(), _item_count) _WITE_DEREF_ITER_DEBUG_ARG(this) _WITE_DEREF_ITER_DEBUG_ARG(_data.data())
                        _WITE_DEREF_ITER_DEBUG_ARG(_data.data() + _item_count));
  }
  _WITE_NODISCARD constexpr auto end() const noexcept {
    return const_iterator(std::next(_data.data(), _item_count) _WITE_DEREF_ITER_DEBUG_ARG(this)
                              _WITE_DEREF_ITER_DEBUG_ARG(_data.data()) _WITE_DEREF_ITER_DEBUG_ARG(_data.data() + _item_count));
  }
  _WITE_NODISCARD constexpr auto rbegin() noexcept { return reverse_iterator(end()); }
  _WITE_NODISCARD constexpr auto rbegin() const noexcept { return const_reverse_iterator(end()); }
  _WITE_NODISCARD constexpr auto rend() noexcept { return reverse_iterator(begin()); }
  _WITE_NODISCARD constexpr auto rend() const noexcept { return const_reverse_iterator(begin()); }
  _WITE_NODISCARD constexpr auto cbegin() const noexcept { return begin(); }
  _WITE_NODISCARD constexpr auto cend() const noexcept { return end(); }
  _WITE_NODISCARD constexpr auto crbegin() const noexcept { return rbegin(); }
  _WITE_NODISCARD constexpr auto crend() const noexcept { return rend(); }
  _WITE_NODISCARD constexpr auto size() const noexcept { return _item_count; }
  _WITE_NODISCARD constexpr auto capacity() const noexcept -> size_type { return _data.max_size(); }
  _WITE_NODISCARD constexpr auto empty() const noexcept { return _item_count == 0; }
  _WITE_NODISCARD constexpr auto operator[](size_type pos) noexcept -> reference { return *_data[pos]; }
  _WITE_NODISCARD constexpr auto operator[](size_type pos) const noexcept -> const_reference { return *_data[pos]; }

#ifndef WITE_NO_EXCEPTIONS
  _WITE_NODISCARD constexpr auto front() -> reference { return at(0); }
  _WITE_NODISCARD constexpr auto front() const -> const_reference { return at(0); }
  _WITE_NODISCARD constexpr auto back() -> reference { return at(size() - 1); }
  _WITE_NODISCARD constexpr auto back() const -> const_reference { return at(size() - 1); }
#endif

  _WITE_NODISCARD constexpr auto ptr() noexcept { return _data.data(); }
  _WITE_NODISCARD constexpr auto ptr() const noexcept { return _data.data(); }

#ifndef WITE_NO_EXCEPTIONS
  _WITE_NODISCARD constexpr auto at(size_type pos) -> reference {
    return const_cast<reference>(const_cast<const static_vector*>(this)->at(pos));
  }

  _WITE_NODISCARD constexpr auto at(size_type pos) const -> const_reference {
    if (pos >= _item_count) {
      throw std::out_of_range{"Static vector access violation"};
    }

    return operator[](pos);
  }

  constexpr void resize(size_type new_size) {
    if (new_size > capacity()) {
      throw std::length_error{"static_vector: new size exceeds max size"};
    }

    _unsafe_resize(new_size);
  }

  constexpr void resize(size_type new_size, value_type value) {
    if (new_size > capacity()) {
      throw std::length_error{"static_vector: new size exceeds max size"};
    }

    _unsafe_resize(new_size, std::move(value));
  }

  void push_back(const_reference x) { resize(size() + 1, x); }

  void push_back(value_type&& x) { resize(size() + 1, std::forward<value_type>(x)); }

#endif

  void clear() noexcept { _item_count = 0; }

  _WITE_NODISCARD constexpr bool operator==(const static_vector& other) const noexcept {
    if (_item_count != other._item_count) {
      return false;
    }

    return std::equal(begin(), end(), other.begin());
  }

  _WITE_NODISCARD constexpr bool operator!=(const static_vector& other) const noexcept { return !(*this == other); }

 private:
  constexpr void _alloc(size_type n, value_type v) noexcept(noexcept(value_type{v})) {
    std::generate_n(std::next(_data.begin(), _item_count), n, [&v]() { return std::optional<value_type>{v}; });
  }

  void _destroy_last_n(size_type n) noexcept {
    const auto erase_start = std::reverse_iterator{std::next(_data.begin(), _item_count)};
    std::for_each(erase_start, std::next(erase_start, n), [](auto&& val) { val.reset(); });
  }

  constexpr void _unsafe_resize(size_type n, value_type v = value_type{}) {
    if (n > size()) {
      _alloc(n - size(), std::move(v));
    } else if (n < size()) {
      _destroy_last_n(size() - n);
    }

    _item_count = n;
  }

  data_type _data{};
  size_type _item_count{0};
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::collections
