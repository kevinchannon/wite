/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#include <wite/maths/value_range.hpp>
#include <wite/maths/io.hpp>

#include <test/utils.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_approx.hpp>

#include <limits>
#include <type_traits>

using namespace wite::maths;

TEST_CASE("value_range tests", "[maths]") {
  SECTION("construct value range") {
    SECTION("int value range") {
      const auto b = value_range{0, 10};
      static_assert(std::is_same_v<decltype(b)::value_type, int>, "value_range type is incorrectly inferred");

      REQUIRE(0 == b.min());
      REQUIRE(10 == b.max());

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if max < min") {
        WITE_REQUIRE_ASSERTS_WITH(value_range(1, 0), "value_range min should be <= max");
      }
#endif
    }

    SECTION("double value range") {
      const auto b = value_range{0.0, 10.0};
      static_assert(std::is_same_v<decltype(b)::value_type, double>, "value_range type is incorrectly inferred");

      REQUIRE(0.0 == b.min());
      REQUIRE(10.0 == b.max());

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if max < min") {
        WITE_REQUIRE_ASSERTS_WITH(value_range(10.0, 10.0 * (1.0 - std::numeric_limits<double>::epsilon())), "value_range min should be <= max");
      }
#endif
    }
  }

  SECTION("comparison") {
    SECTION("value ranges are equal") {
      REQUIRE(closed_value_range{123, 456} == closed_value_range{123, 456});
      REQUIRE_FALSE(closed_value_range{123, 456} == closed_value_range{123, 457});
      REQUIRE_FALSE(closed_value_range{123, 456} == closed_value_range{124, 456});
    }

    SECTION("value ranges are not equal") {
      REQUIRE_FALSE(closed_value_range{123, 456} != closed_value_range{123, 456});
      REQUIRE(closed_value_range{123, 456} != closed_value_range{123, 457});
      REQUIRE(closed_value_range{123, 456} != closed_value_range{124, 456});
    }
  }

  SECTION("size()") {
    REQUIRE(10 == value_range{10, 20}.size());
    REQUIRE(10.0 == Catch::Approx(value_range{100.0, 110.0}.size()).epsilon(0.0));

    SECTION("width and size are equal") {
      constexpr auto r = value_range {-100.0, 100.0};
      REQUIRE(r.size() == r.width());
    }
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

  SECTION("mid()") {
    REQUIRE(0.5 == value_range{0.0, 1.0}.mid());
    REQUIRE(10.0 == value_range{10.0, 10.0}.mid());

    REQUIRE(2 == value_range{0, 3}.mid());
    REQUIRE(2 == value_range{0, 4}.mid());
  }

  SECTION("set min") {
    auto r = value_range{10, 20};
    REQUIRE(10 == r.min());

    SECTION("new value is less than max") {
      r.min(12);
      REQUIRE(12 == r.min());
      REQUIRE(20 == r.max());
    }

    SECTION("new value is equal to max") {

      SECTION("closed range is OK"){
        r.min(20);
        REQUIRE(20 == r.min());
        REQUIRE(20 == r.max());
      }

#ifdef _WITE_CONFIG_DEBUG
      SECTION("open range asserts in debug") {
        auto open_min_rng = value_range<int, range_boundary::open, range_boundary::closed>{10, 20};
        WITE_REQUIRE_ASSERTS_WITH(open_min_rng.min(20), "value_range setting min > max");
      }
#endif
    }

#ifdef _WITE_CONFIG_DEBUG
    SECTION("asserts in debug if min > max") {
      WITE_REQUIRE_ASSERTS_WITH(r.min(21), "value_range setting min > max");
    }
#endif
  }

  SECTION("set max") {
    auto r = value_range{10, 20};
    REQUIRE(10 == r.min());

    SECTION("new value is greater than min") {
      r.max(11);
      REQUIRE(10 == r.min());
      REQUIRE(11 == r.max());
    }

    SECTION("new value is equal to min") {
      SECTION("closed range is OK") {
        r.max(10);
        REQUIRE(10 == r.min());
        REQUIRE(10 == r.max());
      }

#ifdef _WITE_CONFIG_DEBUG
      SECTION("open range asserts in debug") {
        auto open_max_rng = value_range<int, range_boundary::closed, range_boundary::open>{10, 20};
        WITE_REQUIRE_ASSERTS_WITH(open_max_rng.max(10), "value_range setting max < min");
      }
#endif
    }

#ifdef _WITE_CONFIG_DEBUG
    SECTION("asserts in debug if max < min") {
      WITE_REQUIRE_ASSERTS_WITH(r.max(9), "value_range setting max < min");
    }
#endif
  }

  SECTION("clamp") {
    SECTION("in range returns input value") {
      REQUIRE(1.0 == value_range{0.9, 1.5}.clamp(1.0));
    }

    SECTION("below range returns min") {
      REQUIRE(0.9 == value_range{0.9, 1.5}.clamp(0.8));
    }

    SECTION("above range returns max") {
      REQUIRE(1.5 == value_range{0.9, 1.5}.clamp(1.6));
    }

    SECTION("empty range returns min/max") {
      REQUIRE(1.5 == value_range{1.5, 1.5}.clamp(1.6));
    }
  }

  SECTION("interpolate") {
    SECTION("zero returns min") {
      REQUIRE(-1.0 == value_range{-1.0, 1.0}.interpolate(0.0));
    }

    SECTION("one returns min") {
      REQUIRE(1.0 == value_range{-1.0, 1.0}.interpolate(1.0));
    }

    SECTION("intermediate value returns correct fraction") {
      REQUIRE(Catch::Approx(-0.2).epsilon(1e-10) == value_range{-1.0, 1.0}.interpolate(0.4));
    }
  }

  SECTION("fraction") {
    SECTION("min returns zero") {
      REQUIRE(0.0 == value_range(0, 100).fraction(0));
    }

    SECTION("max returns zero") {
      REQUIRE(1.0 == value_range(0, 100).fraction(100));
    }

    SECTION("intermediate value is correct") {
      REQUIRE(Catch::Approx(0.35).epsilon(1e-10) == value_range(0, 100).fraction(35));
    }

    SECTION("empty range returns NaN") {
      REQUIRE(std::isnan(value_range{0.0, 0.0}.fraction(0.0)));
    }
  }
}

TEST_CASE("value_range from envelope tests", "[maths]") {
  SECTION("some integer values") {
      REQUIRE(value_range{-4, 12} == envelope(1, 2, 5, 10, -4, 12));
  }

  SECTION("some floating-point values") {
      REQUIRE(value_range{-4.1, 5.0} == envelope(1.0, 0.9, 5.0, 1.0, -4.1, 1.2));
  }

  SECTION("a collection of values") {
      REQUIRE(value_range{1.0, 5.0} == envelope(std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0}));
  }

  SECTION("some value_ranges") {
      REQUIRE(value_range{-1.0, 1.0} == envelope(value_range{0.0, 1.0}, value_range{-1.0, 0.0}, value_range{-0.5, 0.5}));
  }
}

TEST_CASE("value_range min tests", "[maths]") {
  SECTION("floating-point value ranges") {
      REQUIRE(-1.0 == min(value_range{0.0, 1.0}, value_range{-1.0, 0.0}, value_range{-0.5, 0.5}));
  }
}

TEST_CASE("value_range max tests", "[maths]") {
  SECTION("floating-point value ranges") {
      REQUIRE(1.0 == max(value_range{0.0, 1.0}, value_range{-1.0, 0.0}, value_range{-0.5, 0.5}));
  }
}
