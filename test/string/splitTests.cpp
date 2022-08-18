#include <wite/string/split.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>
#include <algorithm>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Split string on space", "[string]") {
  const auto words = string::split("One small step for man...", ' ');
  const auto expected = std::vector<std::string_view>{"One", "small", "step", "for", "man..."};

  REQUIRE(expected.size() == words.size());
  REQUIRE(std::equal(expected.begin(), expected.end(), words.begin()));
}
