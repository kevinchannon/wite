#include <wite/string/split.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <string>
#include <vector>
#include <algorithm>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Split string tests", "[string]") {
  struct TestParams {
    std::string name;
    std::string toSplit;
    char delimiter;
    std::vector<std::string_view> expected;
  };

  const auto testParams = GENERATE(
    TestParams{"Split string on space", "One small step for man...", ' ', {"One", "small", "step", "for", "man..."}},
    TestParams{"Split string on non-space", "1,2,3,4,5", ',', {"1", "2", "3", "4", "5"}},
    TestParams{"Split string without any delimiters in it", "1 2 3 4 5", ',', {"1 2 3 4 5"}},
    TestParams{"Split string containing only delimiters works", "  ", ' ', {"", "", ""}},
    TestParams{"Split string starting with a delimiter works", " 1", ' ', {"", "1"}},
    TestParams{"Split string ending with a delimiter works", "1 ", ' ', {"1", ""}}
  );

  const auto words = string::split(testParams.toSplit, testParams.delimiter);
  
  SECTION(testParams.name){
      REQUIRE(testParams.expected.size() == words.size());
      REQUIRE(std::equal(testParams.expected.begin(), testParams.expected.end(), words.begin()));
  }
}
