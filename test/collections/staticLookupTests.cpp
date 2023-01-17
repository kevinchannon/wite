/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#include <wite/collections/static_lookup.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>
#include <string_view>
#include <utility>

using namespace wite::collections;
using namespace std::string_view_literals;
using namespace std::string_literals;

TEST_CASE("Static lookup tests") {
  SECTION("constexpr construction from pairs makes the expected lookup") {
    constexpr auto lookup = static_lookup{std::pair{1, "one"sv}, std::pair{2, "two"sv}};
    REQUIRE(2 == lookup.size());
  }

  SECTION("make_static_lookup makes the expected lookup") {
    constexpr auto lookup = make_static_lookup(std::pair{1, "one"sv}, std::pair{2, "two"sv}, std::pair{3, "three"sv});
    REQUIRE(3 == lookup.size());
  }

  SECTION("const construction from pairs makes the expected lookup") {
    const auto lookup = static_lookup{std::pair{1, "one"s},
                                      std::pair{2, "two"s},
                                      std::pair{3, "three"s},
                                      std::pair{4, "four"s},
                                      std::pair{5, "five"s},
                                      std::pair{6, "six"s}};
    REQUIRE(6 == lookup.size());
  }

  SECTION("lookup things up works") {
    constexpr auto translator = static_lookup{
        std::pair{"yes"sv, "oui"sv},
        std::pair{"no"sv, "non"sv},
        std::pair{"please"sv, "s'il vous plait"sv},
        std::pair{"thank you"sv, "merci"sv}
    };

    SECTION("forward lookups") {
      REQUIRE("oui" == translator.at("yes"));
      REQUIRE("non" == translator.at("no"));
      REQUIRE("s'il vous plait" == translator.at("please"));
      REQUIRE("merci" == translator.at("thank you"));
    }

    SECTION("reverse lookups") {
      REQUIRE("yes" == translator.with("oui"));
      REQUIRE("no" == translator.with("non"));
      REQUIRE("please" == translator.with("s'il vous plait"));
      REQUIRE("thank you" == translator.with("merci"));
    }
  }

  SECTION("change values") {
    enum class numero {uno, dos, tres};
    auto lookup = static_lookup{std::pair{numero::uno, "uno"s}, std::pair{numero::dos, "dos"s}, std::pair{numero::tres, "tres"s}};
    lookup.at(numero::dos) = "two"s;

    REQUIRE("two" == lookup.at(numero::dos));
    REQUIRE(numero::dos == lookup.with("two"));
  }
}
