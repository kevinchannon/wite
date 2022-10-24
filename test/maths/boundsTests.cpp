#include <wite/maths/bounds.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace wite::maths;

TEST_CASE("bounds tests", "[maths]") {
  SECTION("construct bounds") {
    const auto b = bounds{0, 10};
    REQUIRE( 0 == b.min);
    REQUIRE(10 == b.max);
  }
}
