#include <wite/string/trim.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Trim strings", "[string]")
{
  SECTION("Trim left") {
    SECTION("Nothing to trim") {
      REQUIRE("It's the same!"s == string::trim_left("It's the same!"));
    }
  }
  
  SECTION("Trim right") {
    SECTION("Nothing to trim") {
      REQUIRE("It's the same!"s == string::trim_right("It's the same!"));
    }
  }
}
