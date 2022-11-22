#include <wite/core/index.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

TEST_CASE("Index tests", "[core]") {
  struct TestCollection {};

  SECTION("value returns the value that the index was constructed with") {

    REQUIRE(10 == wite::index<TestCollection>{10}.value());
  }
}
