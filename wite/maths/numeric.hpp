#pragma once

#include <wite/env/environment.hpp>

#include <cmath>
#include <limits>

namespace wite::maths {

template<typename T>
_WITE_NODISCARD T next_value(T value) noexcept {
  if constexpr (std::is_floating_point_v<T>) {
    return std::nexttoward(value, std::numeric_limits<T>::max());
  } else {
    return value + 1;
  }
}

}
