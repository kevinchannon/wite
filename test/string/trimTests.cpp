#include <wite/string/trim.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Trim strings", "[string]") {
  SECTION("Trim left") {
    SECTION("Narrow strings") {
      SECTION("Nothing to trim") {
        REQUIRE("It's the same!"s == string::trim_left("It's the same!"));
      }

      SECTION("Only white space") {
        REQUIRE(""s == string::trim_left(" \t\r\n\v"));
      }

      SECTION("Trim whitespace from left") {
        REQUIRE("<-- There's the whitespace"s == string::trim_left("\v  \n \t\t \t\r <-- There's the whitespace"));
      }

      SECTION("Leave whitespace on the right") {
        REQUIRE("There's the whitespace -->  \n \t\t \t\r "s == string::trim_left("There's the whitespace -->  \n \t\t \t\r "));
      }
    }

    SECTION("Wide strings") {
      SECTION("Nothing to trim") {
        REQUIRE(L"It's the same!"s == string::trim_left(L"It's the same!"));
      }

      SECTION("Only white space") {
        REQUIRE(L""s == string::trim_left(L" \v\t\r\n"));
      }

      SECTION("Trim whitespace from left") {
        REQUIRE(L"<-- There's the whitespace"s == string::trim_left(L"  \n \v\t\t \t\r <-- There's the whitespace"));
      }

      SECTION("Leave whitespace on the right") {
        REQUIRE(L"There's the whitespace -->  \n \t\t \t\r "s == string::trim_left(L"There's the whitespace -->  \n \t\t \t\r "));
      }
    }
  }

  SECTION("Trim right") {
    SECTION("Narrow strings") {
      SECTION("Nothing to trim") {
        REQUIRE("It's the same!"s == string::trim_right("It's the same!"));
      }

      SECTION("Only white space") {
        REQUIRE(""s == string::trim_right("\v \t\r\n"));
      }

      SECTION("Trim whitespace from right") {
        REQUIRE("There's the whitespace -->"s == string::trim_right("There's the whitespace -->  \n \t\t \t\r\v "));
      }

      SECTION("Leave whitespace on the left") {
        REQUIRE("  \n \t\t \t\r <-- There's the whitespace"s == string::trim_right("  \n \t\t \t\r <-- There's the whitespace"));
      }
    }

    SECTION("Wide strings") {
      SECTION("Nothing to trim") {
        REQUIRE(L"It's the same!"s == string::trim_right(L"It's the same!"));
      }

      SECTION("Only white space") {
        REQUIRE(L""s == string::trim_right(L" \t\r\n\v"));
      }

      SECTION("Trim whitespace from right") {
        REQUIRE(L"There's the whitespace -->"s == string::trim_right(L"There's the whitespace --> \v \n \t\t \t\r "));
      }

      SECTION("Leave whitespace on the left") {
        REQUIRE(L"  \n \t\t \t\r <-- There's the whitespace"s ==
                string::trim_right(L"  \n \t\t \t\r <-- There's the whitespace"));
      }
    }
  }
}
