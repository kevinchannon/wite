#pragma once

#include <wite/common/constructor_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>

namespace wite::collections {

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, size_t CAPACITY>
class stack_vector {
  // TODO: This should be an array of std::optional<Value_T>. To do that transparently will require the implementation of a custom
  // iterator type that does the derefernce of the optional.
  using data_type = std::array<Value_T, CAPACITY>;

 public:
  using value_type             = data_type::value_type;
  using size_type              = data_type::size_type;
  using difference_type        = data_type::difference_type;
  using pointer                = data_type::pointer;
  using const_pointer          = data_type::const_pointer;
  using reference              = data_type::reference;
  using const_reference        = data_type::const_reference;
  using iterator               = data_type::iterator;
  using const_iterator         = data_type::const_iterator;
  using reverse_iterator       = data_type::reverse_iterator;
  using const_reverse_iterator = data_type::const_reverse_iterator;

  DEFAULT_CONSTRUCTORS(stack_vector);

  constexpr explicit stack_vector(uint8_t size) : _item_count{size} {}

  constexpr stack_vector(std::initializer_list<value_type> init) {
    if (init.size() > capacity()) {
      throw std::invalid_argument{"Too many initialisation elements"};
    }

    std::copy(init.begin(), init.end(), begin());
    _item_count = init.size();
  }

  constexpr void swap(stack_vector& other) noexcept {
    using std::swap;

    swap(_data, other._data);
    swap(_item_count, other._item_count);
  }

  [[nodiscard]] constexpr auto begin() noexcept -> iterator { return _data.begin(); }
  [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator { return _data.cbegin(); }
  [[nodiscard]] constexpr auto end() noexcept { return iterator(_data.data(), _item_count); }
  [[nodiscard]] constexpr auto end() const noexcept { return const_iterator(_data.data(), _item_count); }
  [[nodiscard]] constexpr auto rbegin() noexcept { return reverse_iterator(end()); }
  [[nodiscard]] constexpr auto rbegin() const noexcept { return const_reverse_iterator(end()); }
  [[nodiscard]] constexpr auto rend() noexcept { return reverse_iterator(begin()); }
  [[nodiscard]] constexpr auto rend() const noexcept { return const_reverse_iterator(begin()); }
  [[nodiscard]] constexpr auto cbegin() const noexcept { return begin(); }
  [[nodiscard]] constexpr auto cend() const noexcept { return end(); }
  [[nodiscard]] constexpr auto crbegin() const noexcept { return rbegin(); }
  [[nodiscard]] constexpr auto crend() const noexcept { return rend(); }
  [[nodiscard]] constexpr auto size() const noexcept { return _item_count; }
  [[nodiscard]] constexpr auto capacity() const noexcept -> size_type { return _data.max_size(); }
  [[nodiscard]] constexpr auto empty() const noexcept { return _item_count == 0; }
  [[nodiscard]] constexpr auto operator[](size_type pos) noexcept -> reference { return _data[pos]; }
  [[nodiscard]] constexpr auto operator[](size_type pos) const noexcept -> const_reference { return _data[pos]; }
  [[nodiscard]] constexpr auto front() -> reference { return at(0); }
  [[nodiscard]] constexpr auto front() const -> const_reference { return at(0); }
  [[nodiscard]] constexpr auto back() -> reference { return at(size() - 1); }
  [[nodiscard]] constexpr auto back() const -> const_reference { return at(size() - 1); }
  [[nodiscard]] constexpr auto data() noexcept { return _data.data(); }
  [[nodiscard]] constexpr auto data() const noexcept { return _data.data(); }

  [[nodiscard]] constexpr auto at(size_type pos) -> reference {
    return const_cast<reference>(const_cast<const stack_vector*>(this)->at(pos));
  }

  [[nodiscard]] constexpr auto at(size_type pos) const -> const_reference {
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

  void clear() noexcept { _item_count = 0; }

  [[nodiscard]] constexpr bool operator==(const stack_vector& other) const noexcept {
    if (_item_count != other._item_count) {
      return false;
    }

    return std::equal(begin(), end(), other.begin());
  }

  [[nodiscard]] constexpr bool operator!=(const stack_vector& other) const noexcept { return not(*this == other); }

 private:
  data_type _data{};
  size_type _item_count{0};
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::collections