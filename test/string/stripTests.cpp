#include <wite/string/strip.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Strip strings", "[string]"){
  SECTION("Narrow strings") {
    SECTION("C-string with nothing to strip is not changed") {
//      REQUIRE("Nothing to strip"s == string::strip("Nothing to strip"));
    }
    
    SECTION("String with nothing to strip is not changed") {
      REQUIRE("Nothing to strip"s == string::strip("Nothing to strip"s));
    }
    
    SECTION("String view with nothing to strip is not changed") {
      REQUIRE("Nothing to strip"s == string::strip(std::string_view{"Nothing to strip"}));
    }
  }
}
