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
  requires(DIMENSION_COUNT > 0)
class point {
 protected:
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

  [[nodiscard]] constexpr const value_type& operator[](size_type dim) const noexcept { return _value[dim]; }

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

#define _WITE_DEFN_POINT_ND(N)                                                                              \
  template <typename Value_T = WITE_DEFAULT_POINT_TYPE>                                                     \
  class point_##N##d : public point<N, Value_T> {                                                           \
    using _base_type = point<N, Value_T>;                                                                   \
                                                                                                            \
   public:                                                                                                  \
    WITE_DEFAULT_CONSTRUCTORS(point_##N##d);                                                                \
                                                                                                            \
    constexpr point_##N##d(typename _base_type::_storage_type val) noexcept : _base_type{std::move(val)} {} \
                                                                                                            \
    constexpr point_##N##d(std::initializer_list<Value_T> init) noexcept : _base_type{std::move(init)} {}   \
                                                                                                            \
    template <typename... Value_Ts>                                                                         \
      requires(N == sizeof...(Value_Ts))                                                                    \
    constexpr point_##N##d(Value_Ts... vals) noexcept : _base_type{std::forward<Value_Ts>(vals)...} {}      \
  }

_WITE_DEFN_POINT_ND(2);
_WITE_DEFN_POINT_ND(3);
_WITE_DEFN_POINT_ND(4);

#undef _WITE_DEFN_POINT_ND

}  // namespace wite::geometry
