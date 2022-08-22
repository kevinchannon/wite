#include <wite/string/split.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>
#include <vector>
#include <algorithm>

using namespace std::string_literals;
using namespace wite;

template<typename Char_T>
struct TestParams {
  
  std::string name;
  std::basic_string<Char_T> toSplit;
  Char_T delimiter;
  string::split_behaviour behaviour;
  std::vector<const Char_T*> expected;
};

TEST_CASE("Split string tests", "[string]") {
  SECTION("Narrow strings") {
    SECTION("Defaults to using space as a delimiter and drops empty items", "[string]") {
      const auto words = string::split("one two   three");
      const auto expected = std::vector<std::string>{"one", "two", "three"};

      REQUIRE(expected.size() == words.size());
      REQUIRE(expected == words);
    }
    
    SECTION("NULL string literal returns empty") {
      const auto words = string::split((const char*)nullptr);
      const auto expected = std::vector<std::string>{};

      REQUIRE(expected.size() == words.size());
      REQUIRE(expected == words);
    }
    
    SECTION("NULL string literal returns empty when split to") {
      const auto words = string::split_to<std::vector<std::string>>((const char*)nullptr);
      const auto expected = std::vector<std::string>{};

      REQUIRE(expected.size() == words.size());
      REQUIRE(expected == words);
    }
    
    SECTION("Default constructed string_view returns empty") {
      const auto words = string::split(std::string_view{});
      const auto expected = std::vector<std::string>{};

      REQUIRE(expected.size() == words.size());
      REQUIRE(expected == words);
    }

    const auto testParams = GENERATE(
      TestParams<char>{"Split to string view on space",
          "One small step for man...",' ', string::split_behaviour::drop_empty, {"One", "small", "step", "for", "man..."}},
      TestParams<char>{"Split string on non-space",
          "1,2,3,4,5", ',', string::split_behaviour::drop_empty, {"1", "2", "3", "4", "5"}},
      TestParams<char>{"Split string on space, keeping empty items",
          "1,2,,3,4,5", ',', string::split_behaviour::keep_empty, {"1", "2", "", "3", "4", "5"}},
      TestParams<char>{"Split string on space, dropping empty items",
          "1,2,,3,4,5", ',', string::split_behaviour::drop_empty, {"1", "2", "3", "4", "5"}},
      TestParams<char>{"Split string without any delimiters in it",
          "1 2 3 4 5", ',', string::split_behaviour::drop_empty, {"1 2 3 4 5"}},
      TestParams<char>{"Split string containing only delimiters works, keeping empty items",
          "  ", ' ', string::split_behaviour::keep_empty, {"", "", ""}},
      TestParams<char>{"Split string containing only delimiters works, dropping empty items",
          "       ", ' ', string::split_behaviour::drop_empty, {}},
      TestParams<char>{"Split string starting with a delimiter works, keeping empty items",
          " 1", ' ', string::split_behaviour::keep_empty, {"", "1"}},
      TestParams<char>{"Split string starting with a delimiter works, dropping empty items",
          " 1", ' ', string::split_behaviour::drop_empty, {"1"}},
      TestParams<char>{"Split string ending with a delimiter works, keeping empty items",
          "1 ", ' ', string::split_behaviour::keep_empty, {"1", ""}},
      TestParams<char>{"Split string starting with a delimiter works, dropping empty items",
          " 1", ' ', string::split_behaviour::drop_empty, {"1"}}
    );

    SECTION(testParams.name){
      
      SECTION("To strings") {
        const auto words = string::split_to<std::vector<std::string>>(testParams.toSplit, testParams.delimiter, testParams.behaviour);

        REQUIRE(testParams.expected.size() == words.size());
        REQUIRE(std::equal(testParams.expected.begin(), testParams.expected.end(), words.begin()));
      }
      
      SECTION("To string_views") {
        const auto words = string::split_to<std::vector<std::string_view>>(testParams.toSplit, testParams.delimiter, testParams.behaviour);

        REQUIRE(testParams.expected.size() == words.size());
        REQUIRE(std::equal(testParams.expected.begin(), testParams.expected.end(), words.begin()));
      }
    }
  }
  
  SECTION("Wide strings") {
    SECTION("Defaults to using space as a delimiter and drops empty items", "[string]") {
      const auto words = string::split(L"one two   three");
      const auto expected = std::vector<std::wstring>{L"one", L"two", L"three"};

      REQUIRE(expected.size() == words.size());
      REQUIRE(expected == words);
    }
    
    SECTION("NULL string literal returns empty") {
      const auto words = string::split((const wchar_t*)nullptr);
      const auto expected = std::vector<std::wstring>{};

      REQUIRE(expected.size() == words.size());
      REQUIRE(expected == words);
    }
    
    SECTION("NULL string literal returns empty when split to") {
      const auto words = string::split_to<std::vector<std::wstring>>((const wchar_t*)nullptr);
      const auto expected = std::vector<std::wstring>{};

      REQUIRE(expected.size() == words.size());
      REQUIRE(expected == words);
    }
    
    SECTION("Default constructed string_view returns empty") {
      const auto words = string::split(std::wstring_view{});
      const auto expected = std::vector<std::wstring>{};

      REQUIRE(expected.size() == words.size());
      REQUIRE(expected == words);
    }

    const auto testParams = GENERATE(
      TestParams<wchar_t>{"Split to string view on space",
          L"One small step for man...", L' ', string::split_behaviour::drop_empty, {L"One", L"small", L"step", L"for", L"man..."}},
      TestParams<wchar_t>{"Split string on non-space",
          L"1,2,3,4,5", L',', string::split_behaviour::drop_empty, {L"1", L"2", L"3", L"4", L"5"}},
      TestParams<wchar_t>{"Split string on space, keeping empty items",
          L"1,2,,3,4,5", L',', string::split_behaviour::keep_empty, {L"1", L"2", L"", L"3", L"4", L"5"}},
      TestParams<wchar_t>{"Split string on space, dropping empty items",
          L"1,2,,3,4,5", L',', string::split_behaviour::drop_empty, {L"1", L"2", L"3", L"4", L"5"}},
      TestParams<wchar_t>{"Split string without any delimiters in it",
          L"1 2 3 4 5", L',', string::split_behaviour::drop_empty, {L"1 2 3 4 5"}},
      TestParams<wchar_t>{"Split string containing only delimiters works, keeping empty items",
          L"  ", L' ', string::split_behaviour::keep_empty, {L"", L"", L""}},
      TestParams<wchar_t>{"Split string containing only delimiters works, dropping empty items",
          L"       ", L' ', string::split_behaviour::drop_empty, {}},
      TestParams<wchar_t>{"Split string starting with a delimiter works, keeping empty items",
          L" 1", L' ', string::split_behaviour::keep_empty, {L"", L"1"}},
      TestParams<wchar_t>{"Split string starting with a delimiter works, dropping empty items",
          L" 1", L' ', string::split_behaviour::drop_empty, {L"1"}},
      TestParams<wchar_t>{"Split string ending with a delimiter works, keeping empty items",
          L"1 ", L' ', string::split_behaviour::keep_empty, {L"1", L""}},
      TestParams<wchar_t>{"Split string starting with a delimiter works, dropping empty items",
          L" 1", L' ', string::split_behaviour::drop_empty, {L"1"}}
    );

    SECTION(testParams.name){
      
      SECTION("To strings") {
        const auto words = string::split_to<std::vector<std::wstring>>(testParams.toSplit, testParams.delimiter, testParams.behaviour);

        REQUIRE(testParams.expected.size() == words.size());
        REQUIRE(std::equal(testParams.expected.begin(), testParams.expected.end(), words.begin()));
      }
      
      SECTION("To string_views") {
        const auto words = string::split_to<std::vector<std::wstring_view>>(testParams.toSplit, testParams.delimiter, testParams.behaviour);

        REQUIRE(testParams.expected.size() == words.size());
        REQUIRE(std::equal(testParams.expected.begin(), testParams.expected.end(), words.begin()));
      }
    }
  }
}
