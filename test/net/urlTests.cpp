#include <wite/net/url.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("IP Address tests", "[net]") {
  SECTION("construct"){
    SECTION("from string") {
      REQUIRE("0.0.0.0" == wite::net::ip{"0.0.0.0"}.str());
    }
  }
}
