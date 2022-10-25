#include <wite/maths/bounds.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace wite::maths;

TEST_CASE("bounds tests", "[maths]") {
  SECTION("construct bounds") {
    SECTION("int-bounds") {
      const auto b = bounds{0, 10};
      REQUIRE(0 == b.min);
      REQUIRE(10 == b.max);
    }

    SECTION("double-bounds") {
      const auto b = bounds{0.0, 10.0};
      REQUIRE(0.0 == b.min);
      REQUIRE(10.0 == b.max);
    }
  }

  SECTION("comparison") {
    SECTION("bounds are equal") {
      REQUIRE(bounds{123, 456} == bounds{123, 456});
      REQUIRE_FALSE(bounds{123, 456} == bounds{123, 457});
      REQUIRE_FALSE(bounds{123, 456} == bounds{124, 456});
    }

    SECTION("bounds are not equal") {
      REQUIRE_FALSE(bounds{123, 456} != bounds{123, 456});
      REQUIRE(bounds{123, 456} != bounds{123, 457});
      REQUIRE(bounds{123, 456} != bounds{124, 456});
    }

    SECTION("bounds are 'less' than another bounds") {
      REQUIRE(bounds{0, 1} < bounds{2, 3});
      REQUIRE_FALSE(bounds{0, 1} < bounds{1, 2});
      REQUIRE_FALSE(bounds{0, 1} < bounds{0, 1});
      REQUIRE_FALSE(bounds{0, 1} < bounds{-1, 2});
      REQUIRE_FALSE(bounds{0, 1} < bounds{-2, -1});
    }
  }
}
