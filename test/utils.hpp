#pragma once

#include <wite/env/features.hpp>

#include <wite/io/types.hpp>

#include <algorithm>
#include <type_traits>

namespace wite::test {

template<typename Left_T, typename Right_T>
bool ranges_equal(Left_T&& left, Right_T&& right) {
  #if _WITE_HAS_RANGES
  return std::ranges::equal(std::forward<Left_T>(left), std::forward<Right_T>(right));
  #else
  if (left.size() != right.size()) {
    return false;
  }

  return std::equal(left.begin(), left.end(), right.begin());
  #endif
}

template<typename Result_T>
_WITE_NODISCARD Result_T to_integer(wite::io::byte b) {
#if _WITE_FEATURE_USE_STD_BYTE
  return std::to_integer<Result_T>(b);
#else
  return static_cast<Result_T>(b);
#endif
}

}
