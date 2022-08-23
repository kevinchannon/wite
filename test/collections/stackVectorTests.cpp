#include <wite/collections/stack_vector.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Stack vector has correct capacity on construction", "[stack_vector]") {
  REQUIRE(10 == collections::stack_vector<int, 10>{}.capacity());
}

TEST_CASE("Stack vector has correct size on construction", "[stack_vector]") {
  SECTION("When default constructed") {
    REQUIRE(0 == collections::stack_vector<int, 10>{}.size());
  }

  SECTION("When constructed with a size") {
    REQUIRE(5 == collections::stack_vector<int, 10>(5).size());
  }

  SECTION("When constructed with an initializer list") {
    REQUIRE(3 == collections::stack_vector<double, 15>{1.618, 2.718, 3.142}.size());
  }
}

// This is necessary to allow the Catch2 REQUIRE_THROWS_AS macro to work with things that have commas in them.
#define COMMA ,

TEST_CASE("Stack vector constructor throws if initializer list is too long", "[stack vector]") {
  REQUIRE_THROWS_AS(collections::stack_vector<int COMMA 3>{1 COMMA 2 COMMA 3 COMMA 4}, std::invalid_argument);
}

TEST_CASE("Stack vector swaps", "[stack_vector]") {
  auto v_1 = collections::stack_vector<int, 5>{1, 2, 3, 4, 5};
  auto v_2 = collections::stack_vector<int, 5>{6, 7, 8, 9, 10};

  std::swap(v_1, v_2);

  const auto expected_1 = {6, 7, 8, 9, 10};
  const auto expected_2 = {1, 2, 3, 4, 5};

  REQUIRE(std::ranges::equal(expected_1, v_1));
  REQUIRE(std::ranges::equal(expected_2, v_2));
}

TEST_CASE("Stack vector works with std::distance", "[stack_vector]") {
  const auto v = collections::stack_vector<char, 10>{'a', 'b', 'c'};
  REQUIRE(3 == std::distance(v.begin(), v.end()));
  REQUIRE(3 == std::distance(v.rbegin(), v.rend()));
}

TEST_CASE("Stack vector empty reports the correct value", "[stack_vector]") {
  SECTION("When the vector is empty") {
    REQUIRE(collections::stack_vector<wchar_t, 4>{}.empty());
  }

  SECTION("When the vector is not empty") {
    REQUIRE_FALSE(collections::stack_vector<wchar_t, 4>{L'x'}.empty());
  }
}

TEST_CASE("Stack vector operator[] returns the value of the expected element", "[stack_vector]") {
  const auto v = collections::stack_vector<uint64_t, 16>{3, 2, 1, 0};
  REQUIRE(3 == v[0]);
  REQUIRE(2 == v[1]);
  REQUIRE(1 == v[2]);
  REQUIRE(0 == v[3]);
}

TEST_CASE("Stack vector at() returns the same as operator[]", "[stack_vector]") {
  const auto v = collections::stack_vector<uint64_t, 16>{3, 2, 1, 0};
  REQUIRE(v[0] == v.at(0));
  REQUIRE(v[1] == v.at(1));
  REQUIRE(v[2] == v.at(2));
  REQUIRE(v[3] == v.at(3));

  SECTION("And throws if the index is out of range") {
    REQUIRE_THROWS_AS(v.at(4), std::out_of_range);
  }
}