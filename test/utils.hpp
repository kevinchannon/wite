#pragma once

#include <wite/env/features.hpp>

#include <wite/io/types.hpp>
#include <wite/core/assert.hpp>

#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>

#include <algorithm>
#include <type_traits>

#define WITE_REQ_THROWS(expr, ex_type, msg) \
  REQUIRE_THROWS_MATCHES(expr, ex_type, Catch::Matchers::ExceptionMessageMatcher(msg))

#define WITE_REQUIRE_ASSERTS_WITH(expr, msg) \
  REQUIRE_THROWS_MATCHES(expr, wite::assertion_error, Catch::Matchers::Message("Assertion error: " msg))

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

}
