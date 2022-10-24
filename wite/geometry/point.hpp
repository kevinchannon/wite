#pragma once

#include <wite/common/constructor_macros.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <type_traits>

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

  static constexpr size_type dimensions_v = DIMENSION_COUNT;

  static_assert(std::is_arithmetic_v<value_type>, "points should have numeric value types");

  WITE_DEFAULT_CONSTRUCTORS(point);

  constexpr point(_storage_type val) noexcept : _value{std::move(val)} {}

  constexpr point(std::initializer_list<value_type> init) noexcept { std::ranges::copy(init, _value.begin()); }

  template <typename... Value_Ts>
    requires(DIMENSION_COUNT == sizeof...(Value_Ts))
  constexpr point(Value_Ts... vals) noexcept {
    _init<0>(vals...);
  }

  [[nodiscard]] constexpr auto operator<=>(const point&) const noexcept = default;

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
  template <size_t IDX, typename V, typename... OtherValue_Ts>
    requires std::is_same_v<V, value_type>
  constexpr void _init(V val, OtherValue_Ts... other_vals) noexcept {
    _value[IDX] = val;

    if constexpr (0 != sizeof...(OtherValue_Ts)) {
      _init<IDX + 1>(other_vals...);
    }
  }

  _storage_type _value;
};

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_2d = point<2, Value_T>;

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_3d = point<3, Value_T>;

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_4d = point<4, Value_T>;

}  // namespace wite::geometry
