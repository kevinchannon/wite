#include <wite/collections/stack_vector.hpp>

#include "../utils.hpp"

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

TEST_CASE("Stack vector intializer-list assignment operator works", "[stack_vector]") {
  collections::stack_vector<double, 10> v = {1.1, 2.2, 3.3};
  REQUIRE(3 == v.size());
  REQUIRE(test::ranges_equal(std::vector<double>{1.1, 2.2, 3.3}, v));
}

TEST_CASE("Stack vector swaps", "[stack_vector]") {
  auto v_1 = collections::stack_vector<int, 5>{1, 2, 3, 4, 5};
  auto v_2 = collections::stack_vector<int, 5>{6, 7, 8, 9, 10};

  std::swap(v_1, v_2);

  const auto expected_1 = {6, 7, 8, 9, 10};
  const auto expected_2 = {1, 2, 3, 4, 5};

  REQUIRE(test::ranges_equal(expected_1, v_1));
  REQUIRE(test::ranges_equal(expected_2, v_2));
}

TEST_CASE("Stack vector comparison works") {
  auto v_1 = collections::stack_vector<int, 5>{1, 2, 3, 4, 5};
  auto v_2 = collections::stack_vector<int, 5>{1, 2, 3, 4, 5};

  REQUIRE(v_1 == v_2);

  auto v_3 = collections::stack_vector<int, 5>{6, 7, 8, 9, 10};
  REQUIRE(v_1 != v_3);
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

TEST_CASE("Stack vector front() returns the first element", "[stack_vector]") {
  REQUIRE(2 == collections::stack_vector<uint32_t, 5>{2, 3, 4}.front());

  SECTION("And throws if the vector is empty") {
    REQUIRE_THROWS_AS(collections::stack_vector<uint32_t COMMA 5>{}.front(), std::out_of_range);
  }
}

TEST_CASE("Stack vector back() returns the last element", "[stack_vector]") {
  REQUIRE(4 == collections::stack_vector<uint32_t, 5>{2, 3, 4}.back());

  SECTION("And throws if the vector is empty") {
    REQUIRE_THROWS_AS(collections::stack_vector<uint32_t COMMA 5>{}.back(), std::out_of_range);
  }
}

TEST_CASE("Stack vector data() returns address of first element", "[stack_vector]") {
  struct A { int x; };
  const auto v = collections::stack_vector<A, 10>{{1}, {2}, {3}};

  REQUIRE(&v[0] == v.data());
}

TEST_CASE("Stack vector resize() changes the size of the vector as expected", "[stack_vector]") {
  auto v = collections::stack_vector<int, 5>{1, 2};
  v.resize(4);
  
  REQUIRE(4 == v.size());

  SECTION("And the new values are default constructed") {
    REQUIRE(int{} == v[2]);
    REQUIRE(int{} == v[3]);
  }

  SECTION("And throws if the new size is bigger than the capacity") {
    REQUIRE_THROWS_AS(v.resize(6), std::bad_array_new_length);
  }
}

TEST_CASE("Stack vector push_back() pushes data onto the vector", "[stack_vector]") {
  auto v = collections::stack_vector<int, 2>{};
  
  v.push_back(10);
  REQUIRE(1 == v.size());

  v.push_back(20);
  REQUIRE(2 == v.size());

  SECTION("And throws if the capacity is exceeded") {
    REQUIRE_THROWS_AS(v.push_back(30), std::bad_array_new_length);
  }
}

TEST_CASE("Stack vector push_back() moves r-values into itself", "[stack_vector]") {
  struct Moveable {
    bool moved{false};

    Moveable() {}
    Moveable(const Moveable& other) {}
    Moveable(Moveable&& other) noexcept { other.moved = true; }

    Moveable& operator=(const Moveable& other)
    {
      moved = other.moved;
      return *this;
    }

    Moveable& operator=(Moveable&& other) noexcept {
      other.moved = true;
      moved       = true;

      return *this;
    }
  };

  auto v = collections::stack_vector<Moveable, 5>{};

  auto m = Moveable{};
  v.push_back(std::move(m));

  REQUIRE(m.moved);
}

TEST_CASE("Stack vector clear() empties the  vector", "[stack_vector]") {
  auto v = collections::stack_vector<int, 10>{1, 2, 3};
  v.clear();

  REQUIRE(v.empty());
}
