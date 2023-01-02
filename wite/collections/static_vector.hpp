#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>
#include <wite/core/assert.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
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

#ifdef _WITE_CONFIG_DEBUG
#define _WITE_STATIC_VEC_ITER_DEBUG_ARG(arg) , arg
#else
#define _WITE_STATIC_VEC_ITER_DEBUG_ARG(arg)
#endif  // _WITE_CONFIG_DEBUG

///////////////////////////////////////////////////////////////////////////////

namespace wite::collections {

///////////////////////////////////////////////////////////////////////////////

namespace detail {
  template <typename Vector_T>
  class _static_vector_const_iterator {
   public:
#ifdef _WITE_HAS_CONCEPTS
    using iterator_concept = std::contiguous_iterator_tag;
#endif

    using iterator_category = std::random_access_iterator_tag;

    using value_type      = typename Vector_T::value_type;
    using difference_type = typename Vector_T::difference_type;
    using pointer         = typename Vector_T::const_pointer;
    using reference       = const value_type&;

    using _ptr_t = typename Vector_T::const_pointer;

    constexpr explicit _static_vector_const_iterator(_ptr_t ptr _WITE_STATIC_VEC_ITER_DEBUG_ARG(const Vector_T* parent)) noexcept
      : _ptr{ptr}
      _WITE_STATIC_VEC_ITER_DEBUG_ARG(_parent{parent}) {}

    _WITE_NODISCARD constexpr reference operator*() const { return *_ptr; }
    _WITE_NODISCARD constexpr pointer operator->() const { return _ptr; }

    constexpr _static_vector_const_iterator& operator++() _WITE_RELEASE_NOEXCEPT {
      _WITE_DEBUG_ASSERT(_ptr != (_parent->data() + _parent->size()), "static_vector:operator++: incrementing past end");

      ++_ptr;
      return *this;
    }

    constexpr _static_vector_const_iterator operator++(int) _WITE_RELEASE_NOEXCEPT {  // NOLINT(cert-dcl21-cpp)
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr _static_vector_const_iterator& operator--() _WITE_RELEASE_NOEXCEPT {
      _WITE_DEBUG_ASSERT(_ptr != _parent->data(), "static_vector:operator--: decrementing past beginning");

      --_ptr;
      return *this;
    }

    constexpr _static_vector_const_iterator operator--(int) _WITE_RELEASE_NOEXCEPT {  // NOLINT(cert-dcl21-cpp)
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr _static_vector_const_iterator& operator+=(const difference_type offset) _WITE_RELEASE_NOEXCEPT {
      _WITE_DEBUG_ASSERT(_ptr + offset < (_parent->data() + _parent->size()), "static_vector:operator+=: incrementing past end");
      _ptr += offset;
      return *this;
    }

   private:
    _ptr_t _ptr;

#ifdef _WITE_CONFIG_DEBUG
    const Vector_T* _parent;
#endif
  };
}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, size_t CAPACITY>
class static_vector {
  // TODO: This should be an array of std::optional<Value_T>. To do that transparently will require the implementation of a custom
  // iterator type that does the dereference of the optional.
  using data_type = std::array<Value_T, CAPACITY>;

 public:
  using value_type             = typename data_type::value_type;
  using size_type              = typename data_type::size_type;
  using difference_type        = typename data_type::difference_type;
  using pointer                = typename data_type::pointer;
  using const_pointer          = typename data_type::const_pointer;
  using reference              = typename data_type::reference;
  using const_reference        = typename data_type::const_reference;
  using iterator               = typename data_type::iterator;
  using const_iterator         = typename data_type::const_iterator;
  using reverse_iterator       = typename data_type::reverse_iterator;
  using const_reverse_iterator = typename data_type::const_reverse_iterator;

  WITE_DEFAULT_CONSTRUCTORS(static_vector);

  constexpr explicit static_vector(uint8_t size) : _item_count{size} {}

#ifndef WITE_NO_EXCEPTIONS
  constexpr static_vector(std::initializer_list<value_type> init) {
    if (init.size() > capacity()) {
      throw std::invalid_argument{"Too many initialisation elements"};
    }

    std::copy(init.begin(), init.end(), begin());
    _item_count = init.size();
  }
#endif

  constexpr void swap(static_vector& other) noexcept {
    using std::swap;

    swap(_data, other._data);
    swap(_item_count, other._item_count);
  }

  _WITE_NODISCARD constexpr auto begin() noexcept -> iterator { return _data.begin(); }
  _WITE_NODISCARD constexpr auto begin() const noexcept -> const_iterator { return _data.cbegin(); }
  _WITE_NODISCARD constexpr auto end() noexcept {
#if _WITE_USING_MSVC_ARRAY_ITERATOR
    return iterator(_data.data(), _item_count);
#else
    return iterator(std::next(_data.data(), _item_count));
#endif
  }
  _WITE_NODISCARD constexpr auto end() const noexcept {
#if _WITE_USING_MSVC_ARRAY_ITERATOR
    return const_iterator(_data.data(), _item_count);
#else
    return const_iterator(std::next(_data.data(), _item_count));
#endif
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
  _WITE_NODISCARD constexpr auto operator[](size_type pos) noexcept -> reference { return _data[pos]; }
  _WITE_NODISCARD constexpr auto operator[](size_type pos) const noexcept -> const_reference { return _data[pos]; }

#ifndef WITE_NO_EXCEPTIONS
  _WITE_NODISCARD constexpr auto front() -> reference { return at(0); }
  _WITE_NODISCARD constexpr auto front() const -> const_reference { return at(0); }
  _WITE_NODISCARD constexpr auto back() -> reference { return at(size() - 1); }
  _WITE_NODISCARD constexpr auto back() const -> const_reference { return at(size() - 1); }
#endif

  _WITE_NODISCARD constexpr auto data() noexcept { return _data.data(); }
  _WITE_NODISCARD constexpr auto data() const noexcept { return _data.data(); }

#ifndef WITE_NO_EXCEPTIONS
  _WITE_NODISCARD constexpr auto at(size_type pos) -> reference {
    return const_cast<reference>(const_cast<const static_vector*>(this)->at(pos));
  }

  _WITE_NODISCARD constexpr auto at(size_type pos) const -> const_reference {
    if (pos >= _item_count) {
      throw std::out_of_range{"Static vector access violation"};
    }

    return _data[pos];
  }

  void resize(size_type new_size) {
    if (new_size > capacity()) {
      throw std::bad_array_new_length{};
    }

    _item_count = new_size;
  }

  void push_back(const_reference x) {
    resize(size() + 1);
    back() = x;
  }

  void push_back(value_type&& x) {
    resize(size() + 1);
    back() = std::forward<value_type>(x);
  }

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
  data_type _data{};
  size_type _item_count{0};
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::collections