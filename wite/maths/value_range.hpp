#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/core/assert.hpp>
#include <wite/env/environment.hpp>

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

  value_range(value_type min, value_type max) _WITE_RELEASE_NOEXCEPT : _min{min}, _max{max} {
    _WITE_DEBUG_ASSERT(_min <= _max, "value_range min should be <= max");
  }

  WITE_DEFAULT_CONSTRUCTORS(value_range);

  static constexpr auto low_bound  = LBOUND;
  static constexpr auto high_bound = HBOUND;

  _WITE_NODISCARD constexpr bool operator==(const value_range& other) const noexcept {
    return _min == other._min and _max == other._max;
  }
  _WITE_NODISCARD constexpr bool operator!=(const value_range& other) const noexcept { return not(*this == other); }
  _WITE_NODISCARD constexpr bool operator<(const value_range& other) const noexcept { return _max < other._min; }
  _WITE_NODISCARD constexpr bool operator>(const value_range& other) const noexcept { return _min > other._max; }

  _WITE_NODISCARD constexpr value_type min() const noexcept { return _min; }
  void min(value_type x) noexcept { _min = x; }

  _WITE_NODISCARD constexpr value_type max() const noexcept { return _max; }

  _WITE_NODISCARD constexpr value_type size() const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_min <= _max, "value_range min should be <= max");
    return _max - _min; 
  }

  _WITE_NODISCARD constexpr bool empty() const noexcept { return _min == _max; }

  _WITE_NODISCARD constexpr std::optional<value_range> overlap(const value_range& other) const _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_min <= _max, "value_range min should be <= max");
    _WITE_DEBUG_ASSERT(other._min <= other._max, "value_range min should be <= max");

    if (_min > other._max or _max < other._min) {
      return {};
    }

    return value_range{std::max(_min, other._min), std::min(_max, other._max)};
  }

  _WITE_NODISCARD constexpr bool contains(value_type val) const noexcept { return not(below_min(val) or above_max(val)); }

  _WITE_NODISCARD constexpr bool below_min(value_type val) const noexcept {
    if constexpr (range_boundary::closed == low_bound) {
      return val < _min;
    } else {
      return val <= _min;
    }
  }

  _WITE_NODISCARD constexpr bool above_max(value_type val) const noexcept {
    if constexpr (range_boundary::closed == high_bound) {
      return val > _max;
    } else {
      return val >= _max;
    }
  }

  _WITE_NODISCARD constexpr value_type mid() const noexcept { return _max - size() / 2; }

 private:
  value_type _min;
  value_type _max;
};

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
class open_value_range : public value_range<Value_T, range_boundary::open, range_boundary::open> {
 public:
  open_value_range(Value_T min, Value_T max) : value_range<Value_T, range_boundary::open, range_boundary::open>{min, max} {}

  WITE_DEFAULT_CONSTRUCTORS(open_value_range);
};

template <typename Value_T>
class closed_value_range : public value_range<Value_T, range_boundary::closed, range_boundary::closed> {
 public:
  closed_value_range(Value_T min, Value_T max) : value_range<Value_T, range_boundary::closed, range_boundary::closed>{min, max} {}

  WITE_DEFAULT_CONSTRUCTORS(closed_value_range);
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::maths

///////////////////////////////////////////////////////////////////////////////
