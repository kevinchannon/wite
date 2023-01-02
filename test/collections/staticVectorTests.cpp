#include <wite/collections/static_vector.hpp>

#include "../utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <vector>

using namespace std::string_literals;
using namespace wite;

TEST_CASE("Static vector tests", "[collections]") {
  SECTION("has correct capacity on construction", "[static_vector]") {
    REQUIRE(10 == collections::static_vector<int, 10>{}.capacity());
  }

  SECTION("has correct size on construction", "[static_vector]") {
    SECTION("When default constructed") {
      REQUIRE(0 == collections::static_vector<int, 10>{}.size());
    }

    SECTION("When constructed with a size") {
      REQUIRE(5 == collections::static_vector<int, 10>(5).size());
    }

    SECTION("When constructed with an initializer list") {
      REQUIRE(3 == collections::static_vector<double, 15>{1.618, 2.718, 3.142}.size());
    }
  }

// This is necessary to allow the Catch2 REQUIRE_THROWS_AS macro to work with things that have commas in them.
#define COMMA ,

  SECTION("constructor throws if initializer list is too long", "[static vector]") {
    REQUIRE_THROWS_AS((collections::static_vector<int, 3>{1, 2, 3, 4}), std::invalid_argument);
  }

  SECTION("intializer-list assignment operator works", "[static_vector]") {
    collections::static_vector<double, 10> v = {1.1, 2.2, 3.3};
    REQUIRE(3 == v.size());
    REQUIRE(test::ranges_equal(std::vector<double>{1.1, 2.2, 3.3}, v));
  }

  SECTION("swaps", "[static_vector]") {
    auto v_1 = collections::static_vector<int, 5>{1, 2, 3, 4, 5};
    auto v_2 = collections::static_vector<int, 5>{6, 7, 8, 9, 10};

    std::swap(v_1, v_2);

    const auto expected_1 = {6, 7, 8, 9, 10};
    const auto expected_2 = {1, 2, 3, 4, 5};

    REQUIRE(test::ranges_equal(expected_1, v_1));
    REQUIRE(test::ranges_equal(expected_2, v_2));
  }

  SECTION("comparison works") {
    auto v_1 = collections::static_vector<int, 5>{1, 2, 3, 4, 5};
    auto v_2 = collections::static_vector<int, 5>{1, 2, 3, 4, 5};

    REQUIRE(v_1 == v_2);

    auto v_3 = collections::static_vector<int, 5>{6, 7, 8, 9, 10};
    REQUIRE(v_1 != v_3);
  }

  SECTION("works with std::distance", "[static_vector]") {
    const auto v = collections::static_vector<char, 10>{'a', 'b', 'c'};
    REQUIRE(3 == std::distance(v.begin(), v.end()));
    REQUIRE(3 == std::distance(v.rbegin(), v.rend()));
  }

  SECTION("empty reports the correct value", "[static_vector]") {
    SECTION("When the vector is empty") {
      REQUIRE(collections::static_vector<wchar_t, 4>{}.empty());
    }

    SECTION("When the vector is not empty") {
      REQUIRE_FALSE(collections::static_vector<wchar_t, 4>{L'x'}.empty());
    }
  }

  SECTION("operator[] returns the value of the expected element", "[static_vector]") {
    const auto v = collections::static_vector<uint64_t, 16>{3, 2, 1, 0};
    REQUIRE(3 == v[0]);
    REQUIRE(2 == v[1]);
    REQUIRE(1 == v[2]);
    REQUIRE(0 == v[3]);
  }

  SECTION("at() returns the same as operator[]", "[static_vector]") {
    const auto v = collections::static_vector<uint64_t, 16>{3, 2, 1, 0};
    REQUIRE(v[0] == v.at(0));
    REQUIRE(v[1] == v.at(1));
    REQUIRE(v[2] == v.at(2));
    REQUIRE(v[3] == v.at(3));

    SECTION("And throws if the index is out of range") {
      REQUIRE_THROWS_AS(v.at(4), std::out_of_range);
    }
  }

  SECTION("front() returns the first element", "[static_vector]") {
    REQUIRE(2 == collections::static_vector<uint32_t, 5>{2, 3, 4}.front());

    SECTION("And throws if the vector is empty") {
      REQUIRE_THROWS_AS((collections::static_vector<uint32_t, 5>{}.front()), std::out_of_range);
    }
  }

  SECTION("back() returns the last element", "[static_vector]") {
    REQUIRE(4 == collections::static_vector<uint32_t, 5>{2, 3, 4}.back());

    SECTION("And throws if the vector is empty") {
      REQUIRE_THROWS_AS((collections::static_vector<uint32_t, 5>{}.back()), std::out_of_range);
    }
  }

  SECTION("data() returns address of first element", "[static_vector]") {
    struct A {
      int x;
    };
    const auto v = collections::static_vector<A, 10>{{1}, {2}, {3}};

    REQUIRE(&v[0] == v.data());
  }

  SECTION("resize() changes the size of the vector as expected", "[static_vector]") {
    auto v = collections::static_vector<int, 5>{1, 2};
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

  SECTION("push_back() pushes data onto the vector", "[static_vector]") {
    auto v = collections::static_vector<int, 2>{};

    v.push_back(10);
    REQUIRE(1 == v.size());

    v.push_back(20);
    REQUIRE(2 == v.size());

    SECTION("And throws if the capacity is exceeded") {
      REQUIRE_THROWS_AS(v.push_back(30), std::bad_array_new_length);
    }
  }

  SECTION("push_back() moves r-values into itself", "[static_vector]") {
    struct Moveable {
      bool moved{false};

      Moveable() {}
      Moveable(const Moveable&) {}
      Moveable(Moveable&& other) noexcept { other.moved = true; }

      Moveable& operator=(const Moveable& other) {
        moved = other.moved;
        return *this;
      }

      Moveable& operator=(Moveable&& other) noexcept {
        other.moved = true;
        moved       = true;

        return *this;
      }
    };

    auto v = collections::static_vector<Moveable, 5>{};

    auto m = Moveable{};
    v.push_back(std::move(m));

    REQUIRE(m.moved);
  }

  SECTION("clear() empties the  vector", "[static_vector]") {
    auto v = collections::static_vector<int, 10>{1, 2, 3};
    v.clear();

    REQUIRE(v.empty());
  }
}

TEST_CASE("Static vector const iterator tests", "[collections]"){
  auto v = collections::static_vector<int, 20>{1, 2, 3, 4, 5};

  using iterator_t = collections::detail::_static_vector_const_iterator<decltype(v)>;

  SECTION("construction"){
    const auto it = iterator_t{v.data() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
    REQUIRE(1 == *it);
  }

  SECTION("operator->") {
    struct A { int x; };
    constexpr auto test_vec = collections::static_vector<A, 5>{{0}, {1}, {2}, {3}, {4}};
    REQUIRE(0 == collections::detail::_static_vector_const_iterator<decltype(test_vec)>{test_vec.data() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&test_vec)}->x);
  }

  SECTION("operator++"){
    auto it = iterator_t{v.data() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
    ++it;
    REQUIRE(2 == *it);

#ifdef _WITE_CONFIG_DEBUG
    SECTION("asserts in debug if incrementing past the end of the parent vector") {
      ++it; ++it; ++it; ++it;
      WITE_REQUIRE_ASSERTS_WITH(++it, "static_vector:operator++: incrementing past end");
    }
#endif
  }
}