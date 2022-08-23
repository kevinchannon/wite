#include <wite/string/strip.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Strip strings", "[string]") {
  SECTION("Narrow strings") {
    SECTION("C-string with nothing to strip is not changed") {
      REQUIRE("Nothing to strip"s == string::strip("Nothing to strip"));
    }

    SECTION("String with nothing to strip is not changed") {
      REQUIRE("Nothing to strip"s == string::strip("Nothing to strip"s));
    }

    SECTION("String view with nothing to strip is not changed") {
      REQUIRE("Nothing to strip"s == string::strip(std::string_view{"Nothing to strip"}));
    }

    SECTION("Empty string") {
      REQUIRE(""s == string::strip(""));
    }

    SECTION("Only whitespace") {
      const auto whitespace_str = GENERATE("   ", "\t\t\t", "\n\n\n", "\r\r\r", "\v\v\v");
      REQUIRE(""s == string::strip(whitespace_str));
      REQUIRE(""s == string::strip(std::string(whitespace_str)));
      REQUIRE(""s == string::strip(std::string_view(whitespace_str)));
    }

    SECTION("Leading and trailing white-space is removed") {
      const auto [str, expected] = GENERATE(table<const char*, std::string>({{" space ", "space"s},
                                                                             {"\ttab\t", "tab"s},
                                                                             {"\rcarriage return\r", "carriage return"s},
                                                                             {"\vvertical tab\v", "vertical tab"s}}));

      REQUIRE(expected == string::strip(str));
    }
  }

  SECTION("Wide strings") {
    SECTION("C-string with nothing to strip is not changed") {
      REQUIRE(L"Nothing to strip"s == string::strip(L"Nothing to strip"));
    }

    SECTION("String with nothing to strip is not changed") {
      REQUIRE(L"Nothing to strip"s == string::strip(L"Nothing to strip"s));
    }

    SECTION("String view with nothing to strip is not changed") {
      REQUIRE(L"Nothing to strip"s == string::strip(std::wstring_view{L"Nothing to strip"}));
    }

    SECTION("Empty string") {
      REQUIRE(L""s == string::strip(L""));
    }

    SECTION("Only whitespace") {
      const auto whitespace_str = GENERATE(L"   ", L"\t\t\t", L"\n\n\n", L"\r\r\r", L"\v\v\v");

      REQUIRE(L""s == string::strip(whitespace_str));
      REQUIRE(L""s == string::strip(std::wstring(whitespace_str)));
      REQUIRE(L""s == string::strip(std::wstring_view(whitespace_str)));
    }

    SECTION("Leading and trailing white-space is removed") {
      const auto [str, expected] = GENERATE(table<const wchar_t*, std::wstring>({{L" space ", L"space"s},
                                                                                 {L"\ttab\t", L"tab"s},
                                                                                 {L"\rcarriage return\r", L"carriage return"s},
                                                                                 {L"\vvertical tab\v", L"vertical tab"s}}));

      REQUIRE(expected == string::strip(str));
    }
  }
}
