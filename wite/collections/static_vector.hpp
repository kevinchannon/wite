#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/core/assert.hpp>
#include <wite/env/environment.hpp>

#include <algorithm>
#include <array>
#include <compare>
#include <cstdint>
#include <stdexcept>
#include <optional>

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
    using pointer         = const value_type*;
    using reference       = const value_type&;

    using _ptr_t  = const typename std::optional<value_type>*;
    using _this_t = _static_vector_const_iterator;

    constexpr explicit _static_vector_const_iterator(_ptr_t ptr _WITE_STATIC_VEC_ITER_DEBUG_ARG(const Vector_T* parent)) noexcept
        : _ptr{ptr} _WITE_STATIC_VEC_ITER_DEBUG_ARG(_parent{parent}) {}

    WITE_DEFAULT_CONSTRUCTORS(_static_vector_const_iterator);

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
    const Vector_T* _parent;
#endif
  };

  template <typename Vector_T>
  class _static_vector_iterator : public _static_vector_const_iterator<Vector_T> {
    using _base_t = _static_vector_const_iterator<Vector_T>;
    using _this_t = _static_vector_iterator;
   public:
#ifdef _WITE_HAS_CONCEPTS
    using iterator_concept = std::contiguous_iterator_tag;
#endif

    using iterator_category = std::random_access_iterator_tag;

    using value_type      = typename Vector_T::value_type;
    using difference_type = typename Vector_T::difference_type;
    using pointer         = value_type*;
    using reference       = value_type&;

    constexpr explicit _static_vector_iterator(typename std::remove_const<typename _base_t::_ptr_t>::type ptr
                                                   _WITE_STATIC_VEC_ITER_DEBUG_ARG(const Vector_T* parent)) noexcept
        : _base_t{ptr _WITE_STATIC_VEC_ITER_DEBUG_ARG(parent)} {}

    WITE_DEFAULT_CONSTRUCTORS(_static_vector_iterator);

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
      _WITE_DEBUG_ASSERT(this->_ptr + offset < (this->_parent->ptr() + this->_parent->size()), "static_vector::operator+: incrementing past end");

      auto out = *this;
      out += offset;
      return out;
    }

    _WITE_NODISCARD _WITE_RELEASE_CONSTEXPR _this_t operator-(const difference_type offset) const _WITE_RELEASE_NOEXCEPT {
      _WITE_DEBUG_ASSERT(this->_ptr - offset >= this->_parent->ptr(), "static_vector::operator-: decrementing past beginning");
      _WITE_DEBUG_ASSERT(this->_ptr - offset < (this->_parent->ptr() + this->_parent->size()), "static_vector::operator-: incrementing past end");

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
}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, size_t CAPACITY>
class static_vector {
  using data_type = std::array<std::optional<Value_T>, CAPACITY>;
  using _this_t = static_vector<Value_T, CAPACITY>;

 public:
  using value_type             = Value_T;
  using size_type              = typename data_type::size_type;
  using difference_type        = typename data_type::difference_type;
  using pointer                = value_type*;
  using const_pointer          = const value_type*;
  using reference              = value_type&;
  using const_reference        = const value_type&;
  using iterator               = detail::_static_vector_iterator<_this_t>;
  using const_iterator         = detail::_static_vector_const_iterator<_this_t>;
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
    std::transform(init.begin(), init.end(), _data.begin(), [](auto&& x){return std::optional<value_type>{x};});
  }
#endif

  constexpr void swap(static_vector& other) noexcept {
    using std::swap;

    swap(_data, other._data);
    swap(_item_count, other._item_count);
  }

  _WITE_NODISCARD constexpr auto begin() noexcept -> iterator { return iterator(_data.data() _WITE_STATIC_VEC_ITER_DEBUG_ARG(this)); }
  _WITE_NODISCARD constexpr auto begin() const noexcept -> const_iterator { return const_iterator(_data.data() _WITE_STATIC_VEC_ITER_DEBUG_ARG(this)); }
  _WITE_NODISCARD constexpr auto end() noexcept {
    return iterator(std::next(_data.data(), _item_count) _WITE_STATIC_VEC_ITER_DEBUG_ARG(this));
  }
  _WITE_NODISCARD constexpr auto end() const noexcept {
    return const_iterator(std::next(_data.data(), _item_count) _WITE_STATIC_VEC_ITER_DEBUG_ARG(this));
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

  void push_back(const_reference x) {
    resize(size() + 1, x);
  }

  void push_back(value_type&& x) {
    resize(size() + 1, std::forward<value_type>(x));
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

  constexpr void _alloc(size_type n, value_type v) noexcept(noexcept(value_type{v})) {
    std::generate_n(std::next(_data.begin(), _item_count), n, [&v](){ return std::optional<value_type>{v};});
  }

  void _destroy_last_n(size_type n) noexcept {
    const auto erase_start = std::reverse_iterator{std::next(_data.begin(), _item_count)};
    std::for_each(erase_start, std::next(erase_start, n), [](auto&& val) { val.reset();});
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
