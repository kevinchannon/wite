#include <wite/string/split.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>
#include <algorithm>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Split string on space", "[string]") {
  const auto words = string::split("One small step for man...");
  const auto expected = std::vector<std::string_view>{"One", "small", "step", "for", "man..."};

  REQUIRE(expected.size() == words.size());
  REQUIRE(std::equal(expected.begin(), expected.end(), words.begin()));
}

TEST_CASE("Split string on non-space", "[string]") {
  const auto words = string::split("1,2,3,4,5", ',');
  const auto expected = std::vector<std::string_view>{"1", "2", "3", "4", "5"};

  REQUIRE(expected.size() == words.size());
  REQUIRE(std::equal(expected.begin(), expected.end(), words.begin()));
}

TEST_CASE("Split string without any delimiters in it", "[string]") {
  const auto words = string::split("1 2 3 4 5", ',');
  const auto expected = std::vector<std::string_view>{"1 2 3 4 5"};

  REQUIRE(expected.size() == words.size());
  REQUIRE(std::equal(expected.begin(), expected.end(), words.begin()));
}

TEST_CASE("Split string containing only delimiters works", "[string]") {
  const auto words = string::split("  ");
  const auto expected = std::vector<std::string_view>{"", "", ""};

  REQUIRE(expected.size() == words.size());
  REQUIRE(std::equal(expected.begin(), expected.end(), words.begin()));
}

TEST_CASE("Split string starting with a delimiter works", "[string]") {
  const auto words = string::split(" 1");
  const auto expected = std::vector<std::string_view>{"", "1"};

  REQUIRE(expected.size() == words.size());
  REQUIRE(std::equal(expected.begin(), expected.end(), words.begin()));
}

TEST_CASE("Split string ending with a delimiter works", "[string]") {
  const auto words = string::split("1 ");
  const auto expected = std::vector<std::string_view>{"1", ""};

  REQUIRE(expected.size() == words.size());
  REQUIRE(std::equal(expected.begin(), expected.end(), words.begin()));
}

