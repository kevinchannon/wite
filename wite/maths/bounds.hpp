#pragma once

#include <wite/env/environment.hpp>

namespace wite::maths {

template <typename T>
struct bounds {
  using value_type = T;

  _WITE_NODISCARD constexpr bool operator==(const bounds& other) const noexcept { return min == other.min and max == other.max; }
  _WITE_NODISCARD constexpr bool operator!=(const bounds& other) const noexcept { return not (*this == other); }

  value_type min;
  value_type max;
};

}  // namespace wite::maths
