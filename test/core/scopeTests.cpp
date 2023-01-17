/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#include <wite/core/scope.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <stdexcept>

TEST_CASE("Scope exit tests", "[core]") {
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

#if _WITE_HAS_UNCAUGHT_EXCEPTION

TEST_CASE("Scope success tests") {
  SECTION("calls exit function on exit if no exception is in progress") {
    auto fn_called = false;
    {
      auto _ = wite::scope_success{[&fn_called]() { fn_called = true; }};
    }

    REQUIRE(fn_called);
  }

  SECTION("doesn't call exit function on exit if an exception is in progress") {
    auto fn_called = false;
    try {
      auto _ = wite::scope_success{[&fn_called]() { fn_called = true; }};
      throw std::runtime_error{""};
    } catch (const std::runtime_error&) {
    }

    REQUIRE_FALSE(fn_called);
  }

  SECTION("exit not called if it is released") {
    auto fn_called = false;
    {
      auto exit_janitor = wite::scope_success{[&fn_called]() { fn_called = true; }};
      exit_janitor.release();
    }

    REQUIRE_FALSE(fn_called);
  }
}

TEST_CASE("Scope fail tests") {
  SECTION("doesn't call exit function on exit if no exception is in progress") {
    auto fn_called = false;
    {
      auto _ = wite::scope_fail{[&fn_called]() { fn_called = true; }};
    }

    REQUIRE_FALSE(fn_called);
  }

  SECTION("calls exit function on exit if an exception is in progress") {
    auto fn_called = false;
    try {
      auto _ = wite::scope_fail{[&fn_called]() { fn_called = true; }};
      throw std::runtime_error{""};
    } catch (const std::runtime_error&) {
    }

    REQUIRE(fn_called);
  }

  SECTION("exit not called if it is released") {
    auto fn_called = false;
    try {
      auto exit_janitor = wite::scope_fail{[&fn_called]() { fn_called = true; }};
      exit_janitor.release();

      throw std::runtime_error{""};
    } catch (const std::runtime_error&) {
    }

    REQUIRE_FALSE(fn_called);
  }
}

#endif
