#include <wite/collections/static_vector.hpp>

#include "../utils.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <vector>

using namespace std::string_literals;
using namespace wite;

using TestContainer_t = collections::static_vector<int, 20>;

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

  SECTION("can hold types that are not default-constructable", "[static_vector]"){
    struct A {
      explicit A(int a) : a{a} {}

      A() = delete;

      int a;
    };

    auto vec = collections::static_vector<A, 10>{};
    REQUIRE(vec.empty());

    vec.push_back(A{10});
    REQUIRE(1 == vec.size());
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

  SECTION("comparison works", "[static_vector]") {
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

  SECTION("resize() changes the size of the vector as expected", "[static_vector]") {
    auto v = collections::static_vector<int, 5>{1, 2};

    SECTION("if the new size is greater than the existing size") {
      SECTION("And the new values are default constructed") {
        v.resize(4);

        REQUIRE(4 == v.size());

        REQUIRE(int{} == v[2]);
        REQUIRE(int{} == v[3]);
      }

      SECTION("And the new values are as specified") {
        v.resize(4, 20);

        REQUIRE(4 == v.size());

        REQUIRE(20 == v[2]);
        REQUIRE(20 == v[3]);
      }
    }

    SECTION("if the new size is smaller than the existing size"){
      v.resize(1);

      REQUIRE(1 == v.size());
      REQUIRE(1 == v[0]);
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

TEMPLATE_TEST_CASE("Static vector const iterator operations tests",
                   "[collections]",
                   collections::detail::_static_vector_const_iterator<TestContainer_t>,
                   collections::detail::_static_vector_iterator<TestContainer_t>) {
  auto v = TestContainer_t{1, 2, 3, 4, 5};

  using iterator_t = TestType;

  SECTION("construction") {
    const auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
    REQUIRE(1 == *it);
  }

  SECTION("operator->") {
    struct A {
      int x;
    };
    _WITE_RELEASE_CONSTEXPR auto test_vec = collections::static_vector<A, 5>{{0}, {1}, {2}, {3}, {4}};
    REQUIRE(0 == collections::detail::_static_vector_const_iterator<decltype(test_vec)> {
      test_vec.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&test_vec)
    } -> x);
  }

  SECTION("pre-increment operator") {
    auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
    ++it;
    REQUIRE(2 == *it);

#ifdef _WITE_CONFIG_DEBUG
    SECTION("asserts in debug if incrementing past the end of the parent vector") {
      ++it;
      ++it;
      ++it;
      ++it;
      WITE_REQUIRE_ASSERTS_WITH(++it, "static_vector::operator++: incrementing past end");
    }
#endif
  }

  SECTION("post-increment operator") {
    auto it            = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
    const auto it_prev = it++;

    REQUIRE(1 == *it_prev);
    REQUIRE(2 == *it);

#ifdef _WITE_CONFIG_DEBUG
    SECTION("asserts in debug if incrementing past the end of the parent vector") {
      ++it;
      ++it;
      ++it;
      ++it;
      WITE_REQUIRE_ASSERTS_WITH(it++, "static_vector::operator++: incrementing past end");
    }
#endif
  }

  SECTION("pre-decrement operator") {
    auto it = iterator_t{v.ptr() + v.size() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
    --it;
    REQUIRE(5 == *it);

#ifdef _WITE_CONFIG_DEBUG
    SECTION("asserts in debug if decrementing past the beginning of the parent vector") {
      --it;
      --it;
      --it;
      --it;
      WITE_REQUIRE_ASSERTS_WITH(--it, "static_vector::operator--: decrementing past beginning");
    }
#endif
  }

  SECTION("post-decrement operator") {
    auto it            = iterator_t{v.ptr() + v.size() - 1 _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
    const auto it_prev = it--;

    REQUIRE(5 == *it_prev);
    REQUIRE(4 == *it);

#ifdef _WITE_CONFIG_DEBUG
    SECTION("asserts in debug if decrementing past the beginning of the parent vector") {
      --it;
      --it;
      --it;
      WITE_REQUIRE_ASSERTS_WITH(it--, "static_vector::operator--: decrementing past beginning");
    }
#endif
  }

  SECTION("increment assignment") {
    SECTION("with positive increment") {
      auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      it += 3;
      REQUIRE(4 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if incrementing past the end of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it += 3, "static_vector::operator+=: incrementing past end");
      }
#endif
    }

    SECTION("with negative increment") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      it += -3;
      REQUIRE(3 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if decrementing past the begining of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it += -3, "static_vector::operator+=: decrementing past beginning");
      }
#endif
    }
  }

  SECTION("decrement assignment") {
    SECTION("with positive decrement") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      it -= 3;
      REQUIRE(3 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if incrementing past the beginning of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it -= 3, "static_vector::operator-=: decrementing past beginning");
      }
#endif
    }

    SECTION("with negative increment") {
      auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      it -= -3;
      REQUIRE(4 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if decrementing past the end of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it -= -3, "static_vector::operator-=: incrementing past end");
      }
#endif
    }
  }

  SECTION("positive offset operator") {
    SECTION("with positive increment") {
      auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)} + 3;
      REQUIRE(4 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if incrementing past the end of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it + 3, "static_vector::operator+: incrementing past end");
      }
#endif
    }

    SECTION("with negative increment") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)} + (-3);
      REQUIRE(3 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if decrementing past the beginning of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it + (-3), "static_vector::operator+: decrementing past beginning");
      }
#endif
    }
  }

  SECTION("negative offset operator") {
    SECTION("with positive increment") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)} - 3;
      REQUIRE(3 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if incrementing past the end of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it - 3, "static_vector::operator-: decrementing past beginning");
      }
#endif
    }

    SECTION("with negative increment") {
      auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)} - (-3);
      REQUIRE(4 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if incrementing past the beginning of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it - (-3), "static_vector::operator-: incrementing past end");
      }
#endif
    }
  }

  SECTION("distance between iterators") {
    const auto begin = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
    const auto end   = iterator_t{v.ptr() + v.size() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};

    SECTION("manual offset calculation works") {
      REQUIRE(0 == begin - begin);
      REQUIRE(5 == end - begin);
      REQUIRE(-5 == begin - end);
    }

    SECTION("std::distance works") {
      REQUIRE(5 == std::distance(begin, end));
      REQUIRE(-5 == std::distance(end, begin));
    }

#ifdef _WITE_CONFIG_DEBUG
    SECTION("asserts in debug if distance calculation happens for iterators that have different parent containers") {
      const auto not_v       = collections::static_vector<int, 20>{6, 7, 8, 9};
      const auto wrong_begin = iterator_t{not_v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&not_v)};

      WITE_REQUIRE_ASSERTS_WITH(
          end - wrong_begin,
          "static_vector::operator-: distance comparison between two iterators with different parent containers");
      WITE_REQUIRE_ASSERTS_WITH(
          std::distance(wrong_begin, end),
          "static_vector::operator-: distance comparison between two iterators with different parent containers");
    }
#endif
  }

  SECTION("square-bracket operator") {
    const auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
    REQUIRE(1 == it[0]);
    REQUIRE(2 == it[1]);
    REQUIRE(3 == it[2]);
    REQUIRE(4 == it[3]);
    REQUIRE(5 == it[4]);

#ifdef _WITE_CONFIG_DEBUG
    SECTION("asserts in debug if offset is too large") {
      WITE_REQUIRE_ASSERTS_WITH(it[5], "static_vector::operator[]: index out of range");
    }

    SECTION("asserts in debug if offset is negative") {
      WITE_REQUIRE_ASSERTS_WITH(it[-1], "static_vector::operator[]: negative indices are invalid");
    }
#endif
  }

  SECTION("comparison") {
    SECTION("equality") {
      const auto it_1 = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      const auto it_2 = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};

      SECTION("equal iterators are equal") {
        REQUIRE(it_1 == it_2);
      }

      SECTION("unequal iterators are not equal") {
        REQUIRE_FALSE(it_1 == iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)} + 1);
      }
    }

    SECTION("inequality") {
      const auto it_1 = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      const auto it_2 = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};

      SECTION("equal iterators are not not equal") {
        REQUIRE_FALSE(it_1 != it_2);
      }

      SECTION("unequal iterators are not equal") {
        REQUIRE(it_1 != iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)} + 1);
      }
    }

    SECTION("less-than") {
      const auto it_1 = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      const auto it_2 = it_1 + 1;

      REQUIRE(it_1 < it_2);
      REQUIRE_FALSE(it_2 < it_1);

      const auto it_3 = it_1;
      REQUIRE_FALSE(it_1 < it_3);
      REQUIRE_FALSE(it_3 < it_1);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if comparison happens for iterators that have different parent containers") {
        const auto not_v  = collections::static_vector<int, 20>{6, 7, 8, 9};
        const auto bad_it = iterator_t{not_v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&not_v)};

        WITE_REQUIRE_ASSERTS_WITH(
            bad_it < it_1, "static_vector::operator<=>: comparison between two iterators with different parent containers");
      }
#endif
    }

    SECTION("greater-than") {
      const auto it_1 = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)} + 1;
      const auto it_2 = it_1 - 1;

      REQUIRE(it_1 > it_2);
      REQUIRE_FALSE(it_2 > it_1);

      const auto it_3 = it_1;
      REQUIRE_FALSE(it_1 > it_3);
      REQUIRE_FALSE(it_3 > it_1);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if comparison happens for iterators that have different parent containers") {
        const auto not_v  = collections::static_vector<int, 20>{6, 7, 8, 9};
        const auto bad_it = iterator_t{not_v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&not_v)};

        WITE_REQUIRE_ASSERTS_WITH(
            bad_it > it_1, "static_vector::operator<=>: comparison between two iterators with different parent containers");
      }
#endif
    }

    SECTION("less-than-or-equal-to") {
      const auto it_1 = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      const auto it_2 = it_1 + 1;

      REQUIRE(it_1 <= it_2);
      REQUIRE_FALSE(it_2 <= it_1);

      const auto it_3 = it_1;
      REQUIRE(it_1 <= it_3);
      REQUIRE(it_3 <= it_1);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if comparison happens for iterators that have different parent containers") {
        const auto not_v  = collections::static_vector<int, 20>{6, 7, 8, 9};
        const auto bad_it = iterator_t{not_v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&not_v)};

        WITE_REQUIRE_ASSERTS_WITH(
            bad_it <= it_1, "static_vector::operator<=>: comparison between two iterators with different parent containers");
      }
#endif
    }

    SECTION("greater-than-or-equal-to") {
      const auto it_1 = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)} + 1;
      const auto it_2 = it_1 - 1;

      REQUIRE(it_1 >= it_2);
      REQUIRE_FALSE(it_2 >= it_1);

      const auto it_3 = it_1;
      REQUIRE(it_1 >= it_3);
      REQUIRE(it_3 >= it_1);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if comparison happens for iterators that have different parent containers") {
        const auto not_v  = collections::static_vector<int, 20>{6, 7, 8, 9};
        const auto bad_it = iterator_t{not_v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&not_v)};

        WITE_REQUIRE_ASSERTS_WITH(
            bad_it >= it_1, "static_vector::operator<=>: comparison between two iterators with different parent containers");
      }
#endif
    }
  }
}

TEST_CASE("Mutating static vector iterator operations", "[collections]") {
  auto v = TestContainer_t{1, 2, 3, 4, 5};

  using iterator_t = collections::detail::_static_vector_iterator<TestContainer_t>;

  SECTION("dereference allows editing") {
    SECTION("via operator*") {
      const auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      *it           = 6;

      REQUIRE(6 == v[0]);
    }

    SECTION("via operator->"){
      struct A {
        int x;
      };

      collections::static_vector<A, 5> test_vec = {A{0}, A{1}, A{2}, A{3}, A{4}};
      const auto it = collections::detail::_static_vector_iterator<decltype(test_vec)> {
        test_vec.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&test_vec)
      };

      it->x = 5;

      REQUIRE(5 == test_vec[0].x);
    }
  }

  SECTION("operations that return reference to self return mutating versions") {
    SECTION("pre-increment operator") {
      auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      auto it_2 = ++it;

      *it_2 = 10;
      REQUIRE(10 == *it);
    }

    SECTION("post-increment operator") {
      auto it            = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      const auto it_prev = it++;

      *it_prev = 6;
      REQUIRE(6 == v[0]);
    }

    SECTION("pre-decrement operator") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      auto it_2 = --it;

      *it_2 = 10;
      REQUIRE(10 == *it);
    }

    SECTION("post-decrement operator") {
      auto it            = iterator_t{v.ptr() + v.size() - 1 _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      const auto it_prev = it--;

      *it_prev = 6;
      REQUIRE(6 == v[4]);
    }

    SECTION("increment assignment") {
      auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      const auto it_2 = it += 3;

      *it_2 = 10;
      REQUIRE(10 == *it);
    }

    SECTION("decrement assignment") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      const auto it_2 = it -= 3;

      *it_2 = 10;
      REQUIRE(10 == *it);
    }

    SECTION("positive offset operator") {
      auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)} + 3;
      *it = 10;
      REQUIRE(10 == v[3]);
    }

    SECTION("negative offset operator") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)} - 3;
      *it = 10;
      REQUIRE(10 == v[2]);
    }

    SECTION("square-bracket operator") {
      const auto it = iterator_t{v.ptr() _WITE_STATIC_VEC_ITER_DEBUG_ARG(&v)};
      it[3] = 10;
      REQUIRE(10 == it[3]);
    }
  }
}
