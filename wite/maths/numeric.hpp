#pragma once

#include <wite/env/environment.hpp>
#include <wite/common/concepts.hpp>

#include <cmath>
#include <limits>

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


template <typename FirstValue_T, typename... OtherValue_Ts>
  requires(sizeof...(OtherValue_Ts) > 0)
_WITE_NODISCARD FirstValue_T min(FirstValue_T first_value, OtherValue_Ts... other_values) noexcept {
  static_assert(common::all_types_are_the_same_v<FirstValue_T, OtherValue_Ts...>, "All arguments to wite::common::min should have the same type");

  if constexpr (sizeof...(OtherValue_Ts) == 1) {
    return std::min(first_value, other_values...);
  } else {
    return std::min(first_value, min(other_values...));
  }
}

}  // namespace wite::maths
