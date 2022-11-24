#include <wite/core/index.hpp>
#include <wite/core/io.hpp>
#include <test/utils.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <sstream>

TEST_CASE("Index tests", "[core]") {
  struct TestCollection {};

  SECTION("value returns the value that the index was constructed with") {
    REQUIRE(10 == wite::index<TestCollection>{10}.value());
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
    SECTION("Asserts in debug if index undeflows") {
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
    SECTION("Asserts in debug if index undeflows") {
      idx = wite::index<TestCollection>{0};
      WITE_REQUIRE_ASSERTS_WITH(idx--, "Index underflow");
    }
#endif
  }

  SECTION("increment-assignment") {
    auto idx = wite::index<TestCollection>{100};
    REQUIRE(110 == (idx += 10).value());
    REQUIRE(100 == (idx += -10).value());
  }

  SECTION("decrement-assignment") {
    auto idx = wite::index<TestCollection>{100};
    REQUIRE(90 == (idx -= 10).value());
    REQUIRE(100 == (idx -= -10).value());
  }

  SECTION("addition of int") {
    REQUIRE(15 == (wite::index<TestCollection>{10} + 5).value());
    REQUIRE(5 == (wite::index<TestCollection>{10} + -5).value());
  }

  SECTION("subrtaction of int") {
    REQUIRE(5 == (wite::index<TestCollection>{10} - 5).value());
    REQUIRE(15 == (wite::index<TestCollection>{10} - -5).value());
  }
}

TEST_CASE("Index IO tests", "[core]") {
  std::stringstream ss;

  ss << wite::index<std::vector<int>>{100};

  REQUIRE("100" == ss.str());
}
