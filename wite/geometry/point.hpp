#pragma once

#include <wite/common/constructor_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>

#ifndef WITE_DEFAULT_POINT_TYPE
#define WITE_DEFAULT_POINT_TYPE double
#endif

namespace wite::geometry {

template <size_t DIMENSION_COUNT, typename Value_T = WITE_DEFAULT_POINT_TYPE>
class point {
  using _storage_type = std::array<Value_T, DIMENSION_COUNT>;

 public:
  using value_type = Value_T;
  using size_type  = typename _storage_type::size_type;

  WITE_DEFAULT_CONSTRUCTORS(point);

  constexpr point(_storage_type val) : _value{std::move(val)} {}

  constexpr point(std::initializer_list<value_type> init) { std::ranges::copy(init, _value.begin()); }

  [[nodiscard]] constexpr size_type dimensions() const noexcept { return _value.size(); }

  template <size_t DIM>
  [[nodiscard]] constexpr const value_type& get() const noexcept {
    return _value[DIM];
  }

 private:
  _storage_type _value;
};

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_2d = point<2, Value_T>;

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_3d = point<3, Value_T>;

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_4d = point<4, Value_T>;

}  // namespace wite::geometry
