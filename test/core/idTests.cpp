#include <test/utils.hpp>
#include <wite/core/id.hpp>
#include <wite/core/io.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

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

    SECTION("UUID") {
      const auto id = wite::id<TestItem, wite::uuid>{wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDEF"}};
      REQUIRE(wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDEF"} == *id);
    }
  }

  SECTION("string ID works") {
    const auto id = wite::id<TestItem, std::string>{"SN00001"};
    REQUIRE("SN00001" == *id);
  }

  SECTION("IDs are comparable") {
    SECTION("default ID type (UUID)") {
      const auto id = wite::id<TestItem>{wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDEF"}};

      SECTION("equality") {
        REQUIRE(id == wite::id<TestItem>{wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDEF"}});
      }

      SECTION("inequality") {
        REQUIRE(id != wite::id<TestItem>{wite::uuid{"F1234567-89AB-CDEF-0123-456789ABCDEF"}});
      }

      SECTION("less-than") {
        REQUIRE(id < wite::id<TestItem>{wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDFF"}});
      }

      SECTION("greater-than") {
        REQUIRE(id > wite::id<TestItem>{wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDEE"}});
      }

      SECTION("less-than-or-equal-to") {
        REQUIRE(id <= wite::id<TestItem>{wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDFF"}});
        REQUIRE(id <= wite::id<TestItem>{wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDEF"}});
      }

      SECTION("greater-than-or-equal-to") {
        REQUIRE(id >= wite::id<TestItem>{wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDEF"}});
        REQUIRE(id >= wite::id<TestItem>{wite::uuid{"01234567-89AB-CDEF-0123-456789ABCDEE"}});
      }
    }

    SECTION("int ID") {
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
