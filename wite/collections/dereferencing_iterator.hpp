#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/core/assert.hpp>
#include <wite/env/environment.hpp>

#include <algorithm>
#include <array>
#include <compare>
#include <cstdint>
#include <stdexcept>

///////////////////////////////////////////////////////////////////////////////

#ifdef _WITE_CONFIG_DEBUG
#define _WITE_DEREF_ITER_DEBUG_ARG(arg) , arg
#else
#define _WITE_DEREF_ITER_DEBUG_ARG(arg)
#endif  // _WITE_CONFIG_DEBUG

namespace wite::collections {

template <typename Container_T,
          typename IterConcept_T  = std::contiguous_iterator_tag,
          typename IterCategory_T = std::random_access_iterator_tag>
class dereferencing_const_iterator {
 public:
#ifdef _WITE_HAS_CONCEPTS
  using iterator_concept = IterConcept_T;
#endif

  using iterator_category = IterCategory_T;

  using value_type      = typename Container_T::value_type;
  using difference_type = typename Container_T::difference_type;
  using pointer         = const value_type*;
  using reference       = const value_type&;

  using _ptr_t  = const typename Container_T::raw_value_type*;
  using _this_t = dereferencing_const_iterator;

  constexpr explicit dereferencing_const_iterator(_ptr_t ptr _WITE_DEREF_ITER_DEBUG_ARG(const Container_T* parent)) noexcept
      : _ptr{ptr} _WITE_DEREF_ITER_DEBUG_ARG(_parent{parent}) {}

  WITE_DEFAULT_CONSTRUCTORS(dereferencing_const_iterator);

  _WITE_NODISCARD constexpr reference operator*() const noexcept { return **_ptr; }
  _WITE_NODISCARD constexpr pointer operator->() const noexcept { return &**_ptr; }

  _WITE_RELEASE_CONSTEXPR _this_t& operator++() _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_ptr != (_parent->ptr() + _parent->size()), "static_vector::operator++: incrementing past end");

    ++_ptr;
    return *this;
  }

  _WITE_RELEASE_CONSTEXPR _this_t operator++(int) _WITE_RELEASE_NOEXCEPT {  // NOLINT(cert-dcl21-cpp)
    auto out = *this;
    ++*this;
    return out;
  }

  _WITE_RELEASE_CONSTEXPR _this_t& operator--() _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_ptr != _parent->ptr(), "static_vector::operator--: decrementing past beginning");

    --_ptr;
    return *this;
  }

  _WITE_RELEASE_CONSTEXPR _this_t operator--(int) _WITE_RELEASE_NOEXCEPT {  // NOLINT(cert-dcl21-cpp)
    auto out = *this;
    --*this;
    return out;
  }

  _WITE_RELEASE_CONSTEXPR _this_t& operator+=(const difference_type offset) _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_ptr + offset >= _parent->ptr(), "static_vector::operator+=: decrementing past beginning");
    _WITE_DEBUG_ASSERT(_ptr + offset < (_parent->ptr() + _parent->size()), "static_vector::operator+=: incrementing past end");
    _ptr += offset;
    return *this;
  }

  _WITE_RELEASE_CONSTEXPR _this_t& operator-=(const difference_type offset) _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_ptr - offset >= _parent->ptr(), "static_vector::operator-=: decrementing past beginning");
    _WITE_DEBUG_ASSERT(_ptr - offset < (_parent->ptr() + _parent->size()), "static_vector::operator-=: incrementing past end");
    _ptr -= offset;
    return *this;
  }

  _WITE_NODISCARD _WITE_RELEASE_CONSTEXPR _this_t operator+(const difference_type offset) const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_ptr + offset >= _parent->ptr(), "static_vector::operator+: decrementing past beginning");
    _WITE_DEBUG_ASSERT(_ptr + offset < (_parent->ptr() + _parent->size()), "static_vector::operator+: incrementing past end");

    auto out = *this;
    out += offset;
    return out;
  }

  _WITE_NODISCARD _WITE_RELEASE_CONSTEXPR _this_t operator-(const difference_type offset) const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_ptr - offset >= _parent->ptr(), "static_vector::operator-: decrementing past beginning");
    _WITE_DEBUG_ASSERT(_ptr - offset < (_parent->ptr() + _parent->size()), "static_vector::operator-: incrementing past end");

    auto out = *this;
    out -= offset;
    return out;
  }

  _WITE_NODISCARD _WITE_RELEASE_CONSTEXPR difference_type operator-(const _this_t& other) const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_parent == other._parent,
                       "static_vector::operator-: distance comparison between two iterators with different parent containers");
    return _ptr - other._ptr;
  }

  _WITE_NODISCARD _WITE_RELEASE_CONSTEXPR reference operator[](const difference_type offset) const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(offset >= 0, "static_vector::operator[]: negative indices are invalid");
    _WITE_DEBUG_ASSERT(_ptr + offset < _parent->ptr() + _parent->size(), "static_vector::operator[]: index out of range");
    return _ptr[offset].value();
  }

  _WITE_NODISCARD constexpr auto operator==(const _this_t& other) const noexcept { return _ptr == other._ptr; }

  _WITE_NODISCARD constexpr auto operator!=(const _this_t& other) const noexcept {
    return not(*this == other);  // NOLINT
  }

  _WITE_NODISCARD _WITE_RELEASE_CONSTEXPR auto operator<=>(const _this_t& other) const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_parent == other._parent,
                       "static_vector::operator<=>: comparison between two iterators with different parent containers");
    return _ptr <=> other._ptr;
  }

 protected:
  _ptr_t _ptr;

#ifdef _WITE_CONFIG_DEBUG
  const Container_T* _parent;
#endif
};

template <typename Container_T>
class dereferencing_iterator : public dereferencing_const_iterator<Container_T> {
  using _base_t = dereferencing_const_iterator<Container_T>;
  using _this_t = dereferencing_iterator;

 public:
#ifdef _WITE_HAS_CONCEPTS
  using iterator_concept = std::contiguous_iterator_tag;
#endif

  using iterator_category = std::random_access_iterator_tag;

  using value_type      = typename Container_T::value_type;
  using difference_type = typename Container_T::difference_type;
  using pointer         = value_type*;
  using reference       = value_type&;

  constexpr explicit dereferencing_iterator(typename std::remove_const<typename _base_t::_ptr_t>::type ptr
                                                _WITE_DEREF_ITER_DEBUG_ARG(const Container_T* parent)) noexcept
      : _base_t{ptr _WITE_DEREF_ITER_DEBUG_ARG(parent)} {}

  WITE_DEFAULT_CONSTRUCTORS(dereferencing_iterator);

  _WITE_NODISCARD constexpr reference operator*() const noexcept { return const_cast<reference>(_base_t::operator*()); }
  _WITE_NODISCARD constexpr pointer operator->() const noexcept { return const_cast<pointer>(_base_t::operator->()); }

  _WITE_RELEASE_CONSTEXPR _this_t& operator++() _WITE_RELEASE_NOEXCEPT {
    _base_t::operator++();
    return *this;
  }

  _WITE_RELEASE_CONSTEXPR _this_t operator++(int) _WITE_RELEASE_NOEXCEPT {  // NOLINT(cert-dcl21-cpp)
    auto out = *this;
    ++*this;
    return out;
  }

  _WITE_RELEASE_CONSTEXPR _this_t& operator--() _WITE_RELEASE_NOEXCEPT {
    _base_t::operator--();
    return *this;
  }

  _WITE_RELEASE_CONSTEXPR _this_t operator--(int) _WITE_RELEASE_NOEXCEPT {  // NOLINT(cert-dcl21-cpp)
    auto out = *this;
    --*this;
    return out;
  }

  _WITE_RELEASE_CONSTEXPR _this_t& operator+=(const difference_type offset) _WITE_RELEASE_NOEXCEPT {
    _base_t::operator+=(offset);
    return *this;
  }

  _WITE_RELEASE_CONSTEXPR _this_t& operator-=(const difference_type offset) _WITE_RELEASE_NOEXCEPT {
    _base_t::operator-=(offset);
    return *this;
  }

  _WITE_NODISCARD _WITE_RELEASE_CONSTEXPR _this_t operator+(const difference_type offset) const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(this->_ptr + offset >= this->_parent->ptr(), "static_vector::operator+: decrementing past beginning");
    _WITE_DEBUG_ASSERT(this->_ptr + offset < (this->_parent->ptr() + this->_parent->size()),
                       "static_vector::operator+: incrementing past end");

    auto out = *this;
    out += offset;
    return out;
  }

  _WITE_NODISCARD _WITE_RELEASE_CONSTEXPR _this_t operator-(const difference_type offset) const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(this->_ptr - offset >= this->_parent->ptr(), "static_vector::operator-: decrementing past beginning");
    _WITE_DEBUG_ASSERT(this->_ptr - offset < (this->_parent->ptr() + this->_parent->size()),
                       "static_vector::operator-: incrementing past end");

    auto out = *this;
    out -= offset;
    return out;
  }

  _WITE_NODISCARD _WITE_RELEASE_CONSTEXPR difference_type operator-(const _this_t& other) const _WITE_RELEASE_NOEXCEPT {
    return _base_t::operator-(other);
  }

  _WITE_NODISCARD _WITE_RELEASE_CONSTEXPR reference operator[](const difference_type offset) const _WITE_RELEASE_NOEXCEPT {
    return const_cast<reference>(_base_t::operator[](offset));
  }
};

}  // namespace wite::collections
