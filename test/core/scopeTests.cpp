#include <wite/core/scope.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

TEST_CASE("Scope exit tests") {
  SECTION("calls exit function on exit") {
    auto fn_called = false;
    {
      auto _ = wite::scope_exit{[&fn_called]() { fn_called = true; }};
    }

    REQUIRE(fn_called);
  }

  SECTION("exit not called if it is released") {
    auto fn_called = false;
    {
      auto exit_janitor = wite::scope_exit{[&fn_called]() { fn_called = true; }};
      exit_janitor.release();
    }

    REQUIRE_FALSE(fn_called);
  }
}
