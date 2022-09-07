#pragma once

#include <wite/configure/features.hpp>

#include <algorithm>

namespace wite::test {

template<typename Left_T, typename Right_T>
bool ranges_equal(Left_T&& left, Right_T&& right) {
  #if _WITE_HAS_RANGES
  return std::ranges::equal(std::forward<Left_T>(left), std::forward<Right>(right))
  #else
  if (left.size() != right.size()) {
    return false;
  }

  return std::equal(left.begin(), left.end(), right.begin());
  #endif
}

}
