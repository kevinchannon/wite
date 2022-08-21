#include <wite/string/join.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <list>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Join strings", "[string]")
{
  
  SECTION("Empty input") {
    REQUIRE(""s == string::join(std::vector<std::string>{}));
  }
  
  SECTION("Single item") {
    REQUIRE("lonely"s == string::join(std::initializer_list{"lonely"s}));
  }
  
  SECTION("Join on default char") {
    REQUIRE("strings to join"s == string::join(std::array{"strings"s, "to"s, "join"s}));
  }
  
  SECTION("Join on non-default char") {
    REQUIRE("1.618,2.718,3.142"s == string::join(std::list{"1.618"s, "2.718"s, "3.142"s}, ','));
  }
}
