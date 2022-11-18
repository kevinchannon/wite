#include <wite/collections/make_vector.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

TEST_CASE("make_vector tests") {
  SECTION("returns empty vector when called without arguments") {
    REQUIRE(wite::make_vector<double>().empty());
  }

  SECTION("reserve adapter creates vector with expected capacity") {
    REQUIRE(10 == wite::make_vector<int>(wite::arg::reserve{10}).capacity());
  }

  SECTION("size adapter creates a vector with the expected size") {
    const auto v = wite::make_vector<float>(wite::arg::size{10, 1.0f});
    REQUIRE(10 == v.size());
    REQUIRE(std::vector<float>{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f} == v);
  }
}