#include <wite/core/index.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

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
  }
}
