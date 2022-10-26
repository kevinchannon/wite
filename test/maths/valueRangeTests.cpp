#include <wite/maths/value_range.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_approx.hpp>

using namespace wite::maths;

TEST_CASE("value_range tests", "[maths]") {
  SECTION("construct value range") {
    SECTION("int value range") {
      const auto b = value_range{0, 10};
      REQUIRE(0 == b.min);
      REQUIRE(10 == b.max);
    }

    SECTION("double value range") {
      const auto b = value_range{0.0, 10.0};
      REQUIRE(0.0 == b.min);
      REQUIRE(10.0 == b.max);
    }
  }

  SECTION("comparison") {
    SECTION("value ranges are equal") {
      REQUIRE(value_range{123, 456} == value_range{123, 456});
      REQUIRE_FALSE(value_range{123, 456} == value_range{123, 457});
      REQUIRE_FALSE(value_range{123, 456} == value_range{124, 456});
    }

    SECTION("value ranges are not equal") {
      REQUIRE_FALSE(value_range{123, 456} != value_range{123, 456});
      REQUIRE(value_range{123, 456} != value_range{123, 457});
      REQUIRE(value_range{123, 456} != value_range{124, 456});
    }

    SECTION("value ranges are 'less-than' than another value_range") {
      REQUIRE(value_range{0, 1} < value_range{2, 3});
      REQUIRE_FALSE(value_range{0, 1} < value_range{ 1, 2});
      REQUIRE_FALSE(value_range{0, 1} < value_range{ 0, 1});
      REQUIRE_FALSE(value_range{0, 1} < value_range{-1, 2});
      REQUIRE_FALSE(value_range{0, 1} < value_range{-2,-1});
    }

    SECTION("value ranges are 'greater-than' than another value_range") {
      REQUIRE(value_range{2, 3} > value_range{0, 1});
      REQUIRE_FALSE(value_range{ 1, 2} > value_range{0, 1});
      REQUIRE_FALSE(value_range{ 0, 1} > value_range{0, 1});
      REQUIRE_FALSE(value_range{-1, 2} > value_range{0, 1});
      REQUIRE_FALSE(value_range{-2,-1} > value_range{0, 1});
    }
  }

  SECTION("size()") {
    REQUIRE(10 == value_range{10, 20}.size());
    REQUIRE(10.0 == Catch::Approx(value_range{100.0, 110.0}.size()).epsilon(0.0));
  }
}
