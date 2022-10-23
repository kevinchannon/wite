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

  SECTION("construct from initializer list") {
    const auto p = point_4d{1.1, 2.2, 3.3, 4.4}; 

    REQUIRE(1.1 == p.get<dim::x>());
    REQUIRE(2.2 == p.get<dim::y>());
    REQUIRE(3.3 == p.get<dim::z>());
    REQUIRE(4.4 == p.get<dim::t>());
  }

  SECTION("get()") {
    SECTION("values are default-constructed") {
      REQUIRE(WITE_DEFAULT_POINT_TYPE{} == point_3d{}.get<dim::x>());
      REQUIRE(WITE_DEFAULT_POINT_TYPE{} == point_3d{}.get<dim::y>());
      REQUIRE(WITE_DEFAULT_POINT_TYPE{} == point_3d{}.get<dim::z>());
    }

    SECTION("returns the expected value for each dimension") {
      REQUIRE(1.23 == point_3d{1.23, 3.14, 2.72}.get<dim::x>());
      REQUIRE(3.14 == point_3d{1.23, 3.14, 2.72}.get<dim::y>());
      REQUIRE(2.72 == point_3d{1.23, 3.14, 2.72}.get<dim::z>());
    }
  }
}
