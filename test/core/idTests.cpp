#include <test/utils.hpp>
#include <wite/core/id.hpp>
#include <wite/core/io.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <sstream>
#include <string>

TEST_CASE("ID tests", "[core]") {
  struct TestItem {};

  SECTION("constexpr IDs build") {
    SECTION("int") {
      constexpr auto id = wite::id<TestItem, int>{1};
      REQUIRE(1 == *id);
    }

    SECTION("char") {
      constexpr auto id = wite::id<TestItem, char>{'2'};
      REQUIRE('2' == *id);
    }

    SECTION("size_t") {
      constexpr auto id = wite::id<TestItem, size_t>{3};
      REQUIRE(3 == *id);
    }
  }

  SECTION("string ID works") {
    const auto id = wite::id<TestItem, std::string>{"SN00001"};
    REQUIRE("SN00001" == *id);
  }

  SECTION("IDs are comparable") {
    constexpr auto id = wite::id<TestItem, size_t>{100};

    SECTION("equality") {
      REQUIRE(id == wite::id<TestItem, size_t>{100});
    }

    SECTION("inequality") {
      REQUIRE(id != wite::id<TestItem, size_t>{101});
    }

    SECTION("less-than") {
      REQUIRE(id < wite::id<TestItem, size_t>{101});
    }

    SECTION("greater-than") {
      REQUIRE(id > wite::id<TestItem, size_t>{99});
    }

    SECTION("less-than-or-equal-to") {
      REQUIRE(id <= wite::id<TestItem, size_t>{101});
      REQUIRE(id <= wite::id<TestItem, size_t>{100});
    }

    SECTION("greater-than-or-equal-to") {
      REQUIRE(id >= wite::id<TestItem, size_t>{100});
      REQUIRE(id >= wite::id<TestItem, size_t>{99});
    }
  }
}

TEST_CASE("ID IO tests", "[core]") {
  struct TestItem {};

  SECTION("stream insertion works") {
    const auto id = wite::id<TestItem, int>{123};

    std::stringstream ss;
    ss << id;

    REQUIRE("123" == ss.str());
  }
}
