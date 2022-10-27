#pragma once

#include <wite/env/environment.hpp>
#include <wite/common/constructor_macros.hpp>

#include <algorithm>
#include <optional>

///////////////////////////////////////////////////////////////////////////////

namespace wite::maths {

///////////////////////////////////////////////////////////////////////////////

enum class range_boundary { closed, open };

///////////////////////////////////////////////////////////////////////////////

template <typename T, range_boundary LBOUND = range_boundary::closed, range_boundary HBOUND = range_boundary::closed>
struct value_range {
  using value_type = T;

  static constexpr auto low_bound  = LBOUND;
  static constexpr auto high_bound = HBOUND;

  _WITE_NODISCARD constexpr bool operator==(const value_range& other) const noexcept {
    return min == other.min and max == other.max;
  }
  _WITE_NODISCARD constexpr bool operator!=(const value_range& other) const noexcept { return not(*this == other); }
  _WITE_NODISCARD constexpr bool operator<(const value_range& other) const noexcept { return max < other.min; }
  _WITE_NODISCARD constexpr bool operator>(const value_range& other) const noexcept { return min > other.max; }

  _WITE_NODISCARD constexpr value_type size() const noexcept { return max - min; }
  _WITE_NODISCARD constexpr bool empty() const noexcept { return min == max; }

  _WITE_NODISCARD constexpr std::optional<value_range> overlap(const value_range& other) const noexcept {
    if (min > other.max or max < other.min) {
      return {};
    }

    return value_range{std::max(min, other.min), std::min(max, other.max)};
  }

  _WITE_NODISCARD constexpr bool contains(value_type val) const noexcept { return not(below_min(val) or above_max(val)); }

  _WITE_NODISCARD constexpr bool below_min(value_type val) const noexcept {
    if constexpr (range_boundary::closed == low_bound) {
      return val < min;
    } else {
      return val <= min;
    }
  }

  _WITE_NODISCARD constexpr bool above_max(value_type val) const noexcept {
    if constexpr (range_boundary::closed == high_bound) {
      return val > max;
    } else {
      return val >= max;
    }
  }

  value_type min;
  value_type max;
};

///////////////////////////////////////////////////////////////////////////////

template<typename Value_T>
struct open_value_range : public value_range<Value_T, range_boundary::open, range_boundary::open> {
  open_value_range(Value_T min, Value_T max) : value_range<Value_T, range_boundary::open, range_boundary::open>{min, max} {}

  WITE_DEFAULT_CONSTRUCTORS(open_value_range);
};

template<typename Value_T>
struct closed_value_range : public value_range<Value_T, range_boundary::closed, range_boundary::closed> {
  closed_value_range(Value_T min, Value_T max) : value_range<Value_T, range_boundary::closed, range_boundary::closed>{min, max} {}

  WITE_DEFAULT_CONSTRUCTORS(closed_value_range);
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::maths

///////////////////////////////////////////////////////////////////////////////
