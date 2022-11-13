#include <wite/maths/bounded_value.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_approx.hpp>

using namespace wite::maths;

TEST_CASE("bounded Value Tests", "[maths]") {
  SECTION("value") {
    SECTION("is expected value") {
      const auto x = bounded_value{10};
      REQUIRE(10 == x.value());
    }
  }
}
