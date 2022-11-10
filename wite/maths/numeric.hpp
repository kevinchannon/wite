#pragma once

#include <wite/common/concepts.hpp>
#include <wite/env/environment.hpp>

#include <cmath>
#include <limits>
#include <utility>

#ifdef _WITE_HAS_CONCEPTS
#include <concepts>
#endif

namespace wite::maths {

template <typename T>
_WITE_NODISCARD T next_value(T value) noexcept {
  if constexpr (std::is_floating_point_v<T>) {
    return std::nexttoward(value, std::numeric_limits<T>::max());
  } else {
    return value + 1;
  }
}

template <typename T>
_WITE_NODISCARD T prev_value(T value) noexcept {
  if constexpr (std::is_floating_point_v<T>) {
    return std::nexttoward(value, std::numeric_limits<T>::min());
  } else {
    return value - 1;
  }
}

#ifdef _WITE_HAS_CONCEPTS

template <std::totally_ordered FirstValue_T, std::totally_ordered... OtherValue_Ts>
  requires(sizeof...(OtherValue_Ts) > 0 and common::is_pod_like<std::remove_reference_t<FirstValue_T>>)
_WITE_NODISCARD FirstValue_T min(FirstValue_T first_value, OtherValue_Ts... other_values) noexcept {
  static_assert(common::all_types_are_the_same_v<FirstValue_T, OtherValue_Ts...>,
                "All arguments to wite::common::min should have the same type");

  if constexpr (sizeof...(OtherValue_Ts) == 1) {
    return std::min(first_value, other_values...);
  } else {
    return std::min(first_value, min(other_values...));
  }
}

template <std::totally_ordered FirstValue_T, std::totally_ordered... OtherValue_Ts>
  requires(sizeof...(OtherValue_Ts) > 0 and not common::is_pod_like<std::remove_reference_t<FirstValue_T>>)
_WITE_NODISCARD FirstValue_T min(FirstValue_T&& first_value, OtherValue_Ts&&... other_values) noexcept {
  static_assert(common::all_types_are_the_same_v<FirstValue_T, OtherValue_Ts...>,
                "All arguments to wite::common::min should have the same type");

  if constexpr (sizeof...(OtherValue_Ts) == 1) {
    return std::min(std::forward<FirstValue_T>(first_value), std::forward<OtherValue_Ts>(other_values)...);
  } else {
    return std::min(std::forward<FirstValue_T>(first_value), min(std::forward<OtherValue_Ts>(other_values)...));
  }
}

template <std::totally_ordered FirstValue_T, std::totally_ordered... OtherValue_Ts>
  requires(sizeof...(OtherValue_Ts) > 0 and not common::is_pod_like<std::remove_reference_t<FirstValue_T>>)
_WITE_NODISCARD FirstValue_T max(FirstValue_T&& first_value, OtherValue_Ts&&... other_values) noexcept {
  static_assert(common::all_types_are_the_same_v<FirstValue_T, OtherValue_Ts...>,
                "All arguments to wite::common::max should have the same type");

  if constexpr (sizeof...(OtherValue_Ts) == 1) {
    return std::max(std::forward<FirstValue_T>(first_value), std::forward<OtherValue_Ts>(other_values)...);
  } else {
    return std::max(std::forward<FirstValue_T>(first_value), max(std::forward<OtherValue_Ts>(other_values)...));
  }
} 

template <std::totally_ordered FirstValue_T, std::totally_ordered... OtherValue_Ts>
  requires(sizeof...(OtherValue_Ts) > 0 and common::is_pod_like<std::remove_reference_t<FirstValue_T>>)
_WITE_NODISCARD FirstValue_T max(FirstValue_T first_value, OtherValue_Ts... other_values) noexcept {
  static_assert(common::all_types_are_the_same_v<FirstValue_T, OtherValue_Ts...>,
                "All arguments to wite::common::max should have the same type");

  if constexpr (sizeof...(OtherValue_Ts) == 1) {
    return std::max(first_value, other_values...);
  } else {
    return std::max(first_value, max(other_values...));
  }
}

template <std::totally_ordered Value_T>
  requires(not common::is_pod_like<Value_T>)
_WITE_NODISCARD std::pair<const Value_T&, const Value_T&> minmax(Value_T&& left, Value_T&& right) noexcept {
  if (left <= right) {
    return {std::forward<Value_T>(left), std::forward<Value_T>(right)};
  } else {
    return {std::forward<Value_T>(right), std::forward<Value_T>(left)};
  }
}

template <std::totally_ordered... Value_Ts>
  requires(sizeof...(Value_Ts) > 1 and common::is_pod_like<std::remove_reference_t<common::common_type_t<Value_Ts...>>>)
_WITE_NODISCARD std::pair<typename common::common_type_t<Value_Ts...>, typename common::common_type_t<Value_Ts...>> minmax(
    Value_Ts... values) noexcept {
  static_assert(common::all_types_are_the_same_v<Value_Ts...>,
                "All arguments to wite::common::minmax should have the same type");

  return {min(values...), max(values...)};
}

template <std::totally_ordered... Value_Ts>
  requires(sizeof...(Value_Ts) > 1 and not common::is_pod_like<std::remove_reference_t<common::common_type_t<Value_Ts...>>>)
_WITE_NODISCARD std::pair<const typename common::common_type_t<Value_Ts...>&, const typename common::common_type_t<Value_Ts...>&> minmax(
    const Value_Ts&... values) noexcept {
  static_assert(common::all_types_are_the_same_v<Value_Ts...>,
                "All arguments to wite::common::minmax should have the same type");

  return {min(values...), max(values...)};
}

#endif  // _WITE_HAS_CONCEPTS

}  // namespace wite::maths
