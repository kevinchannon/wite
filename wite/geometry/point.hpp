#pragma once

#include <wite/common/constructor_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <type_traits>
#include <initializer_list>

#ifndef WITE_DEFAULT_POINT_TYPE
#define WITE_DEFAULT_POINT_TYPE double
#endif

namespace wite::geometry {

enum class dim { x = 0, y = 1, z = 2, t = 3 };

template <size_t DIMENSION_COUNT, typename Value_T = WITE_DEFAULT_POINT_TYPE>
class point {
  using _storage_type = std::array<Value_T, DIMENSION_COUNT>;

 public:
  using value_type = Value_T;
  using size_type  = typename _storage_type::size_type;

  static_assert(std::is_arithmetic_v<value_type>, "points should have numeric value types");

  WITE_DEFAULT_CONSTRUCTORS(point);

  constexpr point(_storage_type val) : _value{std::move(val)} {}

  constexpr point(std::initializer_list<value_type> init) { std::ranges::copy(init, _value.begin()); }

  [[nodiscard]] constexpr size_type dimensions() const noexcept { return _value.size(); }

  template <size_t DIM>
  [[nodiscard]] constexpr const value_type& get() const noexcept {
    return _value[DIM];
  }

  template <dim DIM>
  [[nodiscard]] constexpr const value_type& get() const noexcept {
    return _value[static_cast<uint32_t>(DIM)];
  }

 private:
  _storage_type _value;
};

template<size_t DIM, typename Value_T>
point(std::initializer_list<Value_T>) -> point<DIM, Value_T>;

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_2d = point<2, Value_T>;

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_3d = point<3, Value_T>;

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_4d = point<4, Value_T>;

}  // namespace wite::geometry
