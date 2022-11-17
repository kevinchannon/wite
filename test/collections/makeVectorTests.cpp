#include <wite/collections/make_Vector.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

TEST_CASE("make_vector tests") {
  SECTION("returns empty vector when called without arguments") {
    REQUIRE(wite::make_vector<double>().empty());
  }
}