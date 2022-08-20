#include <wite/string/split.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>
#include <vector>
#include <algorithm>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Split string tests", "[string]") {
  SECTION("Defaults to using space as a delimiter", "[string]") {
    const auto words = string::split("one two three");
    const auto expected = std::vector<std::string_view>{"one", "two", "three"};

    REQUIRE(expected.size() == words.size());
    REQUIRE(std::equal(expected.begin(), expected.end(), words.begin()));
  }

  struct TestParams {
    std::string name;
    std::string toSplit;
    char delimiter;
    string::split_behaviour behaviour;
    std::vector<std::string_view> expected;
  };

  const auto testParams = GENERATE(
    TestParams{"Split string on space",
        "One small step for man...",' ', string::split_behaviour::drop_empty, {"One", "small", "step", "for", "man..."}},
    TestParams{"Split string on non-space",
        "1,2,3,4,5", ',', string::split_behaviour::drop_empty, {"1", "2", "3", "4", "5"}},
    TestParams{"Split string on space, keeping empty items",
        "1,2,,3,4,5", ',', string::split_behaviour::keep_empty, {"1", "2", "", "3", "4", "5"}},
    TestParams{"Split string on space, dropping empty items",
        "1,2,,3,4,5", ',', string::split_behaviour::drop_empty, {"1", "2", "3", "4", "5"}},
    TestParams{"Split string without any delimiters in it",
        "1 2 3 4 5", ',', string::split_behaviour::drop_empty, {"1 2 3 4 5"}},
    TestParams{"Split string containing only delimiters works, keeping empty items",
        "  ", ' ', string::split_behaviour::keep_empty, {"", "", ""}},
    TestParams{"Split string containing only delimiters works, dropping empty items",
        "       ", ' ', string::split_behaviour::drop_empty, {}},
    TestParams{"Split string starting with a delimiter works, keeping empty items",
        " 1", ' ', string::split_behaviour::keep_empty, {"", "1"}},
    TestParams{"Split string starting with a delimiter works, dropping empty items",
        " 1", ' ', string::split_behaviour::drop_empty, {"1"}},
    TestParams{"Split string ending with a delimiter works, keeping empty items",
        "1 ", ' ', string::split_behaviour::keep_empty, {"1", ""}},
    TestParams{"Split string starting with a delimiter works, dropping empty items",
        " 1", ' ', string::split_behaviour::drop_empty, {"1"}}
  );

  SECTION(testParams.name){
    const auto words = string::split(testParams.toSplit, testParams.delimiter, testParams.behaviour);

    REQUIRE(testParams.expected.size() == words.size());
    REQUIRE(std::equal(testParams.expected.begin(), testParams.expected.end(), words.begin()));
  }
}
