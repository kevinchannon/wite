#include <wite/maths/bounded_value.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_approx.hpp>

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
  }
}
