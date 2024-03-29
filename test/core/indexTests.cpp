/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#include <test/utils.hpp>
#include <wite/core/index.hpp>
#include <wite/core/io.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <sstream>
#include <tuple>

TEST_CASE("Index tests", "[core]") {
  struct TestCollection {};

  SECTION("value returns the value that the index was constructed with") {
    REQUIRE(10 == wite::index<TestCollection>{10}.value());
  }

  SECTION("value() and operator*() return the same thing") {
    const auto idx = wite::index<TestCollection>{123};
    REQUIRE(idx.value() == *idx);
  }

  SECTION("indices are comparable") {
    const auto idx = wite::index<TestCollection>{100};

    SECTION("equality") {
      REQUIRE(idx == wite::index<TestCollection>{100});
    }

    SECTION("inequality") {
      REQUIRE(idx != wite::index<TestCollection>{101});
    }

    SECTION("less-than") {
      REQUIRE(idx < wite::index<TestCollection>{101});
    }

    SECTION("greater-than") {
      REQUIRE(idx > wite::index<TestCollection>{99});
    }

    SECTION("less-than-or-equal-to") {
      REQUIRE(idx <= wite::index<TestCollection>{101});
      REQUIRE(idx <= wite::index<TestCollection>{100});
    }

    SECTION("greater-than-or-equal-to") {
      REQUIRE(idx >= wite::index<TestCollection>{100});
      REQUIRE(idx >= wite::index<TestCollection>{99});
    }
  }

  SECTION("pre-increment operator") {
    auto idx = wite::index<TestCollection>{1};
    REQUIRE(2 == (++idx).value());

#ifdef _WITE_CONFIG_DEBUG
    SECTION("Asserts in debug if index rolls over") {
      idx = wite::index<TestCollection>{std::numeric_limits<size_t>::max()};
      WITE_REQUIRE_ASSERTS_WITH(++idx, "Index overflow");
    }
#endif
  }

  SECTION("post-increment operator") {
    auto idx = wite::index<TestCollection>{1000};
    REQUIRE(1000 == (idx++).value());
    REQUIRE(1001 == idx.value());

#ifdef _WITE_CONFIG_DEBUG
    SECTION("Asserts in debug if index rolls over") {
      idx = wite::index<TestCollection>{std::numeric_limits<size_t>::max()};
      WITE_REQUIRE_ASSERTS_WITH(idx++, "Index overflow");
    }
#endif
  }

  SECTION("pre-decrement operator") {
    auto idx = wite::index<TestCollection>{3};
    REQUIRE(2 == (--idx).value());

#ifdef _WITE_CONFIG_DEBUG
    SECTION("Asserts in debug if index underflows") {
      idx = wite::index<TestCollection>{0};
      WITE_REQUIRE_ASSERTS_WITH(--idx, "Index underflow");
    }
#endif
  }

  SECTION("post-decrement operator") {
    auto idx = wite::index<TestCollection>{1001};
    REQUIRE(1001 == (idx--).value());
    REQUIRE(1000 == idx.value());

#ifdef _WITE_CONFIG_DEBUG
    SECTION("Asserts in debug if index underflows") {
      idx = wite::index<TestCollection>{0};
      WITE_REQUIRE_ASSERTS_WITH(idx--, "Index underflow");
    }
#endif
  }

  SECTION("increment-assignment") {
    auto idx = wite::index<TestCollection>{100};
    REQUIRE(110 == (idx += 10).value());
    REQUIRE(100 == (idx += -10).value());

#ifdef _WITE_CONFIG_DEBUG
    SECTION("Asserts in debug if index overflows") {
      idx = wite::index<TestCollection>{std::numeric_limits<size_t>::max() - 5};
      WITE_REQUIRE_ASSERTS_WITH(idx += 6, "Index overflow");
    }

    SECTION("Asserts in debug if index underflows") {
      idx = wite::index<TestCollection>{5};
      WITE_REQUIRE_ASSERTS_WITH(idx += -6, "Index underflow");
    }
#endif
  }

  SECTION("decrement-assignment") {
    auto idx = wite::index<TestCollection>{100};
    REQUIRE(90 == (idx -= 10).value());
    REQUIRE(100 == (idx -= -10).value());

#ifdef _WITE_CONFIG_DEBUG
    SECTION("Asserts in debug if index underflows") {
      idx = wite::index<TestCollection>{5};
      WITE_REQUIRE_ASSERTS_WITH(idx -= 6, "Index underflow");
    }

    SECTION("Asserts in debug if index overflows") {
      idx = wite::index<TestCollection>{std::numeric_limits<size_t>::max() - 5};
      WITE_REQUIRE_ASSERTS_WITH(idx -= -6, "Index overflow");
    }
#endif
  }

  SECTION("addition of int") {
    REQUIRE(15 == (wite::index<TestCollection>{10} + 5).value());
    REQUIRE(5 == (wite::index<TestCollection>{10} + -5).value());

#ifdef _WITE_CONFIG_DEBUG
    SECTION("Asserts in debug if index underflows") {
      WITE_REQUIRE_ASSERTS_WITH(std::ignore = wite::index<TestCollection>{5} + -6, "Index underflow");
    }

    SECTION("Asserts in debug if index overflows") {
      WITE_REQUIRE_ASSERTS_WITH(std::ignore = wite::index<TestCollection>{std::numeric_limits<size_t>::max() - 5} + 6,
                                "Index overflow");
    }
#endif
  }

  SECTION("subrtaction of int") {
    REQUIRE(5 == (wite::index<TestCollection>{10} - 5).value());
    REQUIRE(15 == (wite::index<TestCollection>{10} - -5).value());

#ifdef _WITE_CONFIG_DEBUG
    SECTION("Asserts in debug if index underflows") {
      WITE_REQUIRE_ASSERTS_WITH(std::ignore = wite::index<TestCollection>{5} - 6, "Index underflow");
    }

    SECTION("Asserts in debug if index overflows") {
      WITE_REQUIRE_ASSERTS_WITH(std::ignore = wite::index<TestCollection>{std::numeric_limits<size_t>::max() - 5} - -6,
                                "Index overflow");
    }
#endif
  }
}

TEST_CASE("Index IO tests", "[core]") {
  std::stringstream ss;

  ss << wite::index<std::vector<int>>{100};

  REQUIRE("100" == ss.str());
}
