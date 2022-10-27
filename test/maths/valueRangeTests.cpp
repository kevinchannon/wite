#include <wite/maths/value_range.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_approx.hpp>

#include <limits>

using namespace wite::maths;

TEST_CASE("value_range tests", "[maths]") {
  SECTION("construct value range") {
    SECTION("int value range") {
      const auto b = value_range{0, 10};
      REQUIRE(0 == b.min);
      REQUIRE(10 == b.max);
    }

    SECTION("double value range") {
      const auto b = value_range{0.0, 10.0};
      REQUIRE(0.0 == b.min);
      REQUIRE(10.0 == b.max);
    }
  }

  SECTION("comparison") {
    SECTION("value ranges are equal") {
      REQUIRE(value_range{123, 456} == value_range{123, 456});
      REQUIRE_FALSE(value_range{123, 456} == value_range{123, 457});
      REQUIRE_FALSE(value_range{123, 456} == value_range{124, 456});
    }

    SECTION("value ranges are not equal") {
      REQUIRE_FALSE(value_range{123, 456} != value_range{123, 456});
      REQUIRE(value_range{123, 456} != value_range{123, 457});
      REQUIRE(value_range{123, 456} != value_range{124, 456});
    }

    SECTION("value ranges are 'less-than' than another value_range") {
      REQUIRE(value_range{0, 1} < value_range{2, 3});
      REQUIRE_FALSE(value_range{0, 1} < value_range{ 1, 2});
      REQUIRE_FALSE(value_range{0, 1} < value_range{ 0, 1});
      REQUIRE_FALSE(value_range{0, 1} < value_range{-1, 2});
      REQUIRE_FALSE(value_range{0, 1} < value_range{-2,-1});
    }

    SECTION("value ranges are 'greater-than' than another value_range") {
      REQUIRE(value_range{2, 3} > value_range{0, 1});
      REQUIRE_FALSE(value_range{ 1, 2} > value_range{0, 1});
      REQUIRE_FALSE(value_range{ 0, 1} > value_range{0, 1});
      REQUIRE_FALSE(value_range{-1, 2} > value_range{0, 1});
      REQUIRE_FALSE(value_range{-2,-1} > value_range{0, 1});
    }
  }

  SECTION("size()") {
    REQUIRE(10 == value_range{10, 20}.size());
    REQUIRE(10.0 == Catch::Approx(value_range{100.0, 110.0}.size()).epsilon(0.0));
  }

  SECTION("overlap()") {
    SECTION("low-side no overlap") {
      REQUIRE(std::nullopt == value_range{2, 3}.overlap(value_range{0, 1}));
    }

    SECTION("Low-side overlap") {
      REQUIRE(value_range{0, 3} == value_range{0, 10}.overlap(value_range{-3, 3}));
    }

    SECTION("sub-range") {
      REQUIRE(value_range{0, 3} == value_range{-10, 10}.overlap(value_range{0, 3}));
    }

    SECTION("super-range") {
      REQUIRE(value_range{1.0, 5.0} == value_range{1.0, 5.0}.overlap(value_range{-100.0, 100.0}));
    }

    SECTION("high-side overlap") {
      REQUIRE(value_range{6, 10} == value_range{0, 10}.overlap(value_range{6, 1000}));
    }

    SECTION("high-side, no overlap") {
      REQUIRE(std::nullopt == value_range{0, 10}.overlap(value_range{11, 20}));
    }
  }

  SECTION("empty()") {
    SECTION("default-constructed range is empty") {
      REQUIRE(value_range<double>{}.empty());
    }

    SECTION("when min and max are equal") {
      REQUIRE(value_range{7.0, 7.0}.empty());
    }
  }

  SECTION("below_min()") {
    SECTION("open range") {
      REQUIRE(open_value_range{0.0, 1.0}.below_min(0.0));
      REQUIRE_FALSE(open_value_range{0.0, 1.0}.below_min(0.0 + std::numeric_limits<double>::epsilon()));
    }

    SECTION("closed range") {
      REQUIRE_FALSE(closed_value_range{0.0, 1.0}.below_min(0.0));
      REQUIRE(closed_value_range{0.0, 1.0}.below_min(0.0 - std::numeric_limits<double>::epsilon()));
    }
  }

  SECTION("above_max()") {
    SECTION("open range") {
      REQUIRE(open_value_range{0.0, 1.0}.above_max(1.0));
      REQUIRE_FALSE(open_value_range{0.0, 1.0}.above_max(1.0 - std::numeric_limits<double>::epsilon()));
    }

    SECTION("closed range") {
      REQUIRE_FALSE(closed_value_range{0.0, 1.0}.above_max(1.0));
      REQUIRE(closed_value_range{0.0, 1.0}.above_max(1.0 + std::numeric_limits<double>::epsilon()));
    }
  }

  SECTION("contains()") {
    SECTION("open range") {
      REQUIRE_FALSE(open_value_range{0.0, 1.0}.contains(0.0));
      REQUIRE(open_value_range{0.0, 1.0}.contains(0.0 + std::numeric_limits<double>::epsilon()));
      REQUIRE(open_value_range{0.0, 1.0}.contains(1.0 - std::numeric_limits<double>::epsilon()));
      REQUIRE_FALSE(open_value_range{0.0, 1.0}.contains(1.0));
    }

    SECTION("closed range") {
      REQUIRE(closed_value_range{0.0, 1.0}.contains(0.0));
      REQUIRE_FALSE(closed_value_range{0.0, 1.0}.contains(0.0 - std::numeric_limits<double>::epsilon()));
      REQUIRE_FALSE(closed_value_range{0.0, 1.0}.contains(1.0 + std::numeric_limits<double>::epsilon()));
      REQUIRE(closed_value_range{0.0, 1.0}.contains(1.0));
    }
  }
}
