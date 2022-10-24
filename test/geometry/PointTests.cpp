#include <wite/geometry/point.hpp>

#include <catch2/catch_test_macros.hpp>

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

  SECTION("assign from initializer list") {
    auto p = point_4d<>{};
    p      = {1.1, 2.2, 3.3, 4.4};

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
      REQUIRE(3.14 == point_3d{1.23, 3.14, 2.72}.get<dim::y>());
      REQUIRE(1.23 == point_3d{1.23, 3.14, 2.72}.get<dim::x>());
      REQUIRE(2.72 == point_3d{1.23, 3.14, 2.72}.get<dim::z>());
    }
  }

  SECTION("set()") {
    auto p = point_3d<int>{};

    SECTION("using enum dims") {
      p.set<dim::x>(1);
      p.set<dim::y>(2);
      p.set<dim::z>(3);

      REQUIRE(1 == p.get<dim::x>());
      REQUIRE(2 == p.get<dim::y>());
      REQUIRE(3 == p.get<dim::z>());
    }

    SECTION("using int dims") {
      p.set<0>(1);
      p.set<1>(2);
      p.set<2>(3);

      REQUIRE(1 == p.get<0>());
      REQUIRE(2 == p.get<1>());
      REQUIRE(3 == p.get<2>());
    }
  }

  SECTION("operator[]") {
    auto p = point_4d{0.2, 1.2, 2.2, 3.2};

    SECTION("non-mutating") {
      REQUIRE(0.2 == p[0]);
      REQUIRE(1.2 == p[1]);
      REQUIRE(2.2 == p[2]);
      REQUIRE(3.2 == p[3]);
    }

    SECTION("mutating") {
      p[0] = 1000.0;
      p[1] = 1001.0;
      p[2] = 1002.0;
      p[3] = 1003.0;

      REQUIRE(1000.0 == p[0]);
      REQUIRE(1001.0 == p[1]);
      REQUIRE(1002.0 == p[2]);
      REQUIRE(1003.0 == p[3]);
    }
  }

  SECTION("comparison") {
    SECTION("equal") {
      REQUIRE(point_2d{1.2, 3.4} == point_2d{1.2, 3.4});
    }

    SECTION("not equal") {
      REQUIRE(point_2d{1.2, 3.4} != point_2d{1.2, 3.5});
      REQUIRE(point_2d{1.2, 3.4} != point_2d{1.3, 3.4});
    }

    SECTION("less-than") {
      REQUIRE(point_2d{1.2, 3.4} < point_2d{1.2, 3.5});
      REQUIRE(point_2d{1.2, 3.4} < point_2d{1.3, 3.4});
    }

    SECTION("less-than-or-equal-to") {
      REQUIRE(point_2d{1.2, 3.4} <= point_2d{1.2, 3.4});
      REQUIRE(point_2d{1.2, 3.4} <= point_2d{1.2, 3.5});
      REQUIRE(point_2d{1.2, 3.4} <= point_2d{1.3, 3.4});
    }

    SECTION("greater-than") {
      REQUIRE(point_2d{1.2, 3.5} > point_2d{1.2, 3.4});
      REQUIRE(point_2d{1.3, 3.4} > point_2d{1.2, 3.4});
    }

    SECTION("greater-than-or-equal-to") {
      REQUIRE(point_2d{1.2, 3.4} >= point_2d{1.2, 3.4});
      REQUIRE(point_2d{1.2, 3.5} >= point_2d{1.2, 3.4});
      REQUIRE(point_2d{1.3, 3.4} >= point_2d{1.2, 3.4});
    }
  }
}
