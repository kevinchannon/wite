#include <wite/core/result.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

enum class ETestError {
  error_1,
  error_2,
  error_3
};

using TestResult_t = wite::result<uint32_t, ETestError>;

TEST_CASE("Result tests", "[core]") {
  SECTION("result::ok returns the expected value", "[core]") {
    REQUIRE(TestResult_t{10}.ok());
    REQUIRE_FALSE(TestResult_t{ETestError::error_1}.ok());
  }

  SECTION("result::is_error returns the expected value") {
    REQUIRE_FALSE(TestResult_t{10}.is_error());
    REQUIRE(TestResult_t{ETestError::error_1}.is_error());
  }

  SECTION("result::value returns the value") {
    REQUIRE(10 == TestResult_t{10}.value());
  }

  SECTION("result::error returns the error value") {
    REQUIRE(ETestError::error_2 == TestResult_t{ETestError::error_2}.error());
  }

  SECTION("dereference operators") {
    SECTION("operator*") {
      const auto r = TestResult_t{123};
      REQUIRE(123 == *r);
    }

    SECTION("operator->"){
      struct A {
        int x;
      };

      const auto r = wite::result<A, int>{A{10}};
      REQUIRE(10 == r->x);
    }
  }
}
