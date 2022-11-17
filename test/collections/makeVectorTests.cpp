#include <wite/collections/make_vector.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

TEST_CASE("make_vector tests") {
  SECTION("returns empty vector when called without arguments") {
    REQUIRE(wite::make_vector<double>().empty());
  }

  SECTION("reserve adapter creates vector with expected capacity") {
    REQUIRE(10 == wite::make_vector<int>(wite::reserve{10}).capacity());
  }
}