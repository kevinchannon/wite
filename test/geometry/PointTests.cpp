#include <wite/geometry/point.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace wite::geometry;

TEST_CASE("Point Tests", "[geometry]") {
  SECTION("dimensions() returns the correct value") {
    SECTION("2D") {
      REQUIRE(size_t{2} == point_2d<double>{}.dimensions());
      REQUIRE(size_t{2} == point_2d{}.dimensions());
    }
    
    SECTION("3D") {
      REQUIRE(size_t{3} == point_3d<int>{}.dimensions());
      REQUIRE(size_t{3} == point_3d{}.dimensions());
    }
    
    SECTION("4D") {
      REQUIRE(size_t{4} == point_4d<float>{}.dimensions());
      REQUIRE(size_t{4} == point_4d{}.dimensions());
    }
  }
}
