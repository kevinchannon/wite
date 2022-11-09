#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/core/assert.hpp>
#include <wite/env/environment.hpp>
#include <wite/maths/numeric.hpp>
#include <wite/common/concepts.hpp>

#include <algorithm>
#include <optional>

///////////////////////////////////////////////////////////////////////////////

namespace wite::maths {

///////////////////////////////////////////////////////////////////////////////

template <typename T, typename = void>
struct is_value_range : std::false_type {};

template <typename T>
struct is_value_range<T,
                      ::std::void_t<decltype(std::declval<T>().min()),   // has a minimum
                                    decltype(std::declval<T>().max())>>  // ...and a maximum
    : std::true_type {};

template <typename T>
constexpr bool is_value_range_v = is_value_range<T>::value;

#ifdef _WITE_HAS_CONCEPTS
template<typename T>
concept value_range_type = requires(T& vr) {
                             vr.min();
                             vr.max();
                           };
#endif

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

  _WITE_NODISCARD constexpr value_type min() const noexcept { return _min; }
  void min(value_type x) _WITE_RELEASE_NOEXCEPT {
    if constexpr (low_bound == range_boundary::closed) {
      _WITE_DEBUG_ASSERT(not above_max(x), "value_range setting min > max");
    } else {
      _WITE_DEBUG_ASSERT(not above_max(maths::next_value(x)), "value_range setting min > max");
    }
    _min = x;
  }

  _WITE_NODISCARD constexpr value_type max() const noexcept { return _max; }
  void max(value_type x) _WITE_RELEASE_NOEXCEPT {
    if constexpr (high_bound == range_boundary::closed) {
      _WITE_DEBUG_ASSERT(not below_min(x), "value_range setting max < min");
    } else {
      _WITE_DEBUG_ASSERT(not below_min(maths::prev_value(x)), "value_range setting max < min");
    }
    _max = x;
  }

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

template <value_range_type RangeValue_T, value_range_type... ValueRange_Ts>
_WITE_NODISCARD typename RangeValue_T::value_type min(RangeValue_T left, ValueRange_Ts... other_values) noexcept {
  if constexpr (sizeof...(ValueRange_Ts) == 1) {
    const auto val_range_min = [](const auto& vr) { return vr.min(); };
    return std::min(left.min(), val_range_min(other_values...));
  } else {
    return std::min(left.min(), min(other_values...));
  }
}

///////////////////////////////////////////////////////////////////////////////

template <value_range_type RangeValue_T, value_range_type... ValueRange_Ts>
_WITE_NODISCARD typename RangeValue_T::value_type max(RangeValue_T left, ValueRange_Ts... other_values) noexcept {
  if constexpr (sizeof...(ValueRange_Ts) == 1) {
    const auto val_range_max = [](const auto& vr) { return vr.max(); };
    return std::max(left.max(), val_range_max(other_values...));
  } else {
    return std::max(left.max(), max(other_values...));
  }
}

///////////////////////////////////////////////////////////////////////////////

template<typename... Value_Ts>
  requires(sizeof...(Value_Ts) >= 2 and not is_value_range_v<common::common_type_t<Value_Ts...>>)
_WITE_NODISCARD value_range<common::common_type_t<Value_Ts...>> envelope(Value_Ts... values) noexcept {
  const auto [min, max] = maths::min_max(values...);
  return {min, max};
}

template <typename Range_T>
_WITE_NODISCARD value_range<typename Range_T::value_type> envelope(Range_T&& values) noexcept {
  const auto range = std::ranges::minmax(std::forward<Range_T>(values));
  return {range.min, range.max};
}

template <value_range_type... ValueRange_Ts>
  requires(sizeof...(ValueRange_Ts) >= 2)
_WITE_NODISCARD common::common_type_t<ValueRange_Ts...> envelope(ValueRange_Ts... values) noexcept {
  return {min(values...), max(values...)};
}

template <std::ranges::range Range_T>
_WITE_NODISCARD value_range<typename Range_T::value_type> envelope(Range_T&& values) noexcept {
  const auto min_max = std::ranges::minmax(std::forward<Range_T>(values));
  return {min_max.min, min_max.max};
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::maths

///////////////////////////////////////////////////////////////////////////////
