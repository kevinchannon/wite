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
      SECTION("const operations") {
        const auto r = TestResult_t{123};
        REQUIRE(123 == *r);
      }

      SECTION("mutating operations"){
        auto r = TestResult_t{123};
        *r = 321;
        REQUIRE(321 == *r);
      }
    }

    SECTION("operator->"){
      struct A {
        int x;
      };

      SECTION("const operations") {
        const auto r = wite::result<A, int>{A{10}};
        REQUIRE(10 == r->x);
      }

      SECTION("mutating operations") {
        auto r = wite::result<A, int>{A{10}};
        r->x = 20;

        REQUIRE(20 == r->x);
      }
    }
  }

  SECTION("implicit cast to bool"){
    SECTION("valid result case"){
      const auto r = TestResult_t{111};
      REQUIRE(r);
    }

    SECTION("error result case") {
      const auto r = TestResult_t{ETestError::error_3};
      REQUIRE(not r);
    }
  }

  SECTION("has_value"){
    SECTION("valid result case"){
      const auto r = TestResult_t{111};
      REQUIRE(r.has_value());
    }

    SECTION("error result case") {
      const auto r = TestResult_t{ETestError::error_3};
      REQUIRE(not r.has_value());
    }
  }

  SECTION("value_or"){
    SECTION("returns the value if the result is valid") {
      const auto r = TestResult_t{3214};
      REQUIRE(3214 == r.value_or(20));
    }

    SECTION("returns the default value if the result is an error") {
      const auto r = TestResult_t{ETestError::error_2};
      REQUIRE(20 == r.value_or(20));
    }
  }

  SECTION("and_then"){
    SECTION("applies the function if the  result is valid"){
      const auto r = TestResult_t{1};
      REQUIRE(2 == r.and_then([](auto&& x) { return 2*x;}));
    }
  }
}
