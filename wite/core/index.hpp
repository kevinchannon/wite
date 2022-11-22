#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>

#include <compare>
#include <type_traits>

namespace wite {

template <typename Obj_T>
class index {
 public:
  using value_type      = size_t;
  using collection_type = Obj_T;

  constexpr explicit index(value_type idx) noexcept : _idx{idx} {}

  WITE_DEFAULT_CONSTRUCTORS(index);

  _WITE_NODISCARD constexpr auto operator<=>(const index&) const noexcept = default;

  index& operator++() noexcept {
    ++_idx;
    return *this;
  }

  index operator++(int) noexcept {
    auto temp = *this;
    ++_idx;
    return temp;
  }

  index& operator--() noexcept {
    --_idx;
    return *this;
  }

  index operator--(int) noexcept {
    auto temp = *this;
    --_idx;
    return temp;
  }

  index& operator+=(std::make_signed_t<value_type> offset) noexcept {
    _idx += offset;
    return *this;
  }

  index& operator-=(std::make_signed_t<value_type> offset) noexcept {
    _idx -= offset;
    return *this;
  }

  _WITE_NODISCARD index operator+(std::make_signed_t<value_type> offset) const noexcept { return index{_idx + offset}; }

  _WITE_NODISCARD constexpr value_type value() const noexcept { return _idx; }

 private:
  value_type _idx;
};

}  // namespace wite
