#pragma once

#include <wite/env/environment.hpp>

#include <cmath>
#include <limits>

namespace wite::maths {

template<typename T>
_WITE_NODISCARD T next_value(T value) noexcept {
  return std::nexttoward(value, std::numeric_limits<T>::max());
}

}
