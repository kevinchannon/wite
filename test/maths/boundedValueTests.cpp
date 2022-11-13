#include <wite/maths/bounded_value.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_approx.hpp>

#include <numbers>

using namespace wite::maths;

TEST_CASE("bounded Value Tests", "[maths]") {
  SECTION("value") {
    SECTION("is expected value") {
      REQUIRE(10 == bounded_value{10}.value());
    }

    SECTION("can be set") {
      auto x = bounded_value{10};

      x.value(20);
      REQUIRE(20 == x.value());
    }
  }

  SECTION("bounds") {
    SECTION("are min/max value by default") {
      REQUIRE(value_range{std::numeric_limits<int>::min(), std::numeric_limits<int>::max()} == bounded_value{10}.bounds());
      REQUIRE(value_range{std::numeric_limits<double>::min(), std::numeric_limits<double>::max()} == bounded_value{100.0}.bounds());
    }

    SECTION("have the expected value on construction") {
      REQUIRE(value_range{0, 10} == bounded_value{5, {0, 10}}.bounds());
      REQUIRE(value_range{-std::numbers::pi, std::numbers::pi} ==
              bounded_value{1.0, {-std::numbers::pi, std::numbers::pi}}.bounds());
    }

    SECTION("set bounds to new value") {
      auto x = bounded_value{0, {-10, 10}};
      x.bounds({-5, 5});

      REQUIRE(value_range{-5, 5} == x.bounds());
    }
  }

  SECTION("is_in_bounds") {
    REQUIRE(bounded_value{0, {0, 10}}.is_in_bounds());
    REQUIRE(bounded_value{10, {0, 10}}.is_in_bounds());
    REQUIRE_FALSE(bounded_value{-1, {0, 10}}.is_in_bounds());
    REQUIRE_FALSE(bounded_value{11, {0, 10}}.is_in_bounds());
  }
}
