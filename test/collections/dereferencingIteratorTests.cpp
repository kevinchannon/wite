#include <wite/collections/dereferencing_iterator.hpp>
#include <wite/collections/static_vector.hpp>

#include <test/utils.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

using namespace wite;

using TestContainer_t = collections::static_vector<int, 20>;

TEMPLATE_TEST_CASE("Dereferencing const iterator operations tests",
                   "[collections]",
                   collections::dereferencing_const_iterator<TestContainer_t>,
                   collections::dereferencing_iterator<TestContainer_t>) {
  auto v = TestContainer_t{1, 2, 3, 4, 5};

  using iterator_t = TestType;

  SECTION("construction") {
    const auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
    REQUIRE(1 == *it);
  }

  SECTION("operator->") {
    struct A {
      int x;
    };
    _WITE_RELEASE_CONSTEXPR auto test_vec = collections::static_vector<A, 5>{{0}, {1}, {2}, {3}, {4}};
    REQUIRE(0 == collections::dereferencing_const_iterator<decltype(test_vec)> {
      test_vec.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&test_vec) _WITE_DEREF_ITER_DEBUG_ARG(test_vec.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(test_vec.ptr() + test_vec.size())
    } -> x);
  }

  SECTION("pre-increment operator") {
    auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
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
    auto it            = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
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
    auto it = iterator_t{v.ptr() + v.size() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
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
    auto it            = iterator_t{v.ptr() + v.size() - 1 _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
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
      auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      it += 3;
      REQUIRE(4 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if incrementing past the end of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it += 3, "static_vector::operator+=: incrementing past end");
      }
#endif
    }

    SECTION("with negative increment") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
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
      auto it = iterator_t{v.ptr() + v.size() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      it -= 3;
      REQUIRE(3 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if incrementing past the beginning of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it -= 3, "static_vector::operator-=: decrementing past beginning");
      }
#endif
    }

    SECTION("with negative increment") {
      auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
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
      auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())} + 3;
      REQUIRE(4 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if incrementing past the end of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it + 3, "static_vector::operator+: incrementing past end");
      }
#endif
    }

    SECTION("with negative increment") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())} + (-3);
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
      auto it = iterator_t{v.ptr() + v.size() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())} - 3;
      REQUIRE(3 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if incrementing past the end of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it - 3, "static_vector::operator-: decrementing past beginning");
      }
#endif
    }

    SECTION("with negative increment") {
      auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())} - (-3);
      REQUIRE(4 == *it);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if incrementing past the beginning of the parent vector") {
        WITE_REQUIRE_ASSERTS_WITH(it - (-3), "static_vector::operator-: incrementing past end");
      }
#endif
    }
  }

  SECTION("distance between iterators") {
    const auto begin = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
    const auto end   = iterator_t{v.ptr() + v.size() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};

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
      auto not_v       = collections::static_vector<int, 20>{6, 7, 8, 9};
      const auto wrong_begin = iterator_t{not_v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&not_v) _WITE_DEREF_ITER_DEBUG_ARG(not_v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(not_v.ptr() + v.size())};

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
    const auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
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
      const auto it_1 = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      const auto it_2 = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};

      SECTION("equal iterators are equal") {
        REQUIRE(it_1 == it_2);
      }

      SECTION("unequal iterators are not equal") {
        REQUIRE_FALSE(it_1 == iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())} + 1);
      }
    }

    SECTION("inequality") {
      const auto it_1 = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      const auto it_2 = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};

      SECTION("equal iterators are not not equal") {
        REQUIRE_FALSE(it_1 != it_2);
      }

      SECTION("unequal iterators are not equal") {
        REQUIRE(it_1 != iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())} + 1);
      }
    }

    SECTION("less-than") {
      const auto it_1 = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      const auto it_2 = it_1 + 1;

      REQUIRE(it_1 < it_2);
      REQUIRE_FALSE(it_2 < it_1);

      const auto it_3 = it_1;
      REQUIRE_FALSE(it_1 < it_3);
      REQUIRE_FALSE(it_3 < it_1);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if comparison happens for iterators that have different parent containers") {
        auto not_v  = collections::static_vector<int, 20>{6, 7, 8, 9};
        const auto bad_it = iterator_t{not_v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&not_v) _WITE_DEREF_ITER_DEBUG_ARG(not_v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(not_v.ptr() + v.size())};

        WITE_REQUIRE_ASSERTS_WITH(
            bad_it < it_1, "static_vector::operator<=>: comparison between two iterators with different parent containers");
      }
#endif
    }

    SECTION("greater-than") {
      const auto it_1 = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())} + 1;
      const auto it_2 = it_1 - 1;

      REQUIRE(it_1 > it_2);
      REQUIRE_FALSE(it_2 > it_1);

      const auto it_3 = it_1;
      REQUIRE_FALSE(it_1 > it_3);
      REQUIRE_FALSE(it_3 > it_1);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if comparison happens for iterators that have different parent containers") {
        auto not_v  = collections::static_vector<int, 20>{6, 7, 8, 9};
        const auto bad_it = iterator_t{not_v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&not_v) _WITE_DEREF_ITER_DEBUG_ARG(not_v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(not_v.ptr() + v.size())};

        WITE_REQUIRE_ASSERTS_WITH(
            bad_it > it_1, "static_vector::operator<=>: comparison between two iterators with different parent containers");
      }
#endif
    }

    SECTION("less-than-or-equal-to") {
      const auto it_1 = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      const auto it_2 = it_1 + 1;

      REQUIRE(it_1 <= it_2);
      REQUIRE_FALSE(it_2 <= it_1);

      const auto it_3 = it_1;
      REQUIRE(it_1 <= it_3);
      REQUIRE(it_3 <= it_1);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if comparison happens for iterators that have different parent containers") {
        auto not_v  = collections::static_vector<int, 20>{6, 7, 8, 9};
        const auto bad_it = iterator_t{not_v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&not_v) _WITE_DEREF_ITER_DEBUG_ARG(not_v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(not_v.ptr() + v.size())};

        WITE_REQUIRE_ASSERTS_WITH(
            bad_it <= it_1, "static_vector::operator<=>: comparison between two iterators with different parent containers");
      }
#endif
    }

    SECTION("greater-than-or-equal-to") {
      const auto it_1 = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())} + 1;
      const auto it_2 = it_1 - 1;

      REQUIRE(it_1 >= it_2);
      REQUIRE_FALSE(it_2 >= it_1);

      const auto it_3 = it_1;
      REQUIRE(it_1 >= it_3);
      REQUIRE(it_3 >= it_1);

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts in debug if comparison happens for iterators that have different parent containers") {
        auto not_v  = collections::static_vector<int, 20>{6, 7, 8, 9};
        const auto bad_it = iterator_t{not_v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&not_v) _WITE_DEREF_ITER_DEBUG_ARG(not_v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(not_v.ptr() + v.size())};

        WITE_REQUIRE_ASSERTS_WITH(
            bad_it >= it_1, "static_vector::operator<=>: comparison between two iterators with different parent containers");
      }
#endif
    }
  }
}

TEST_CASE("Mutating dereferencing iterator operations", "[collections]") {
  auto v = TestContainer_t{1, 2, 3, 4, 5};

  using iterator_t = collections::dereferencing_iterator<TestContainer_t>;

  SECTION("dereference allows editing") {
    SECTION("via operator*") {
      const auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      *it           = 6;

      REQUIRE(6 == v[0]);
    }

    SECTION("via operator->"){
      struct A {
        int x;
      };

      collections::static_vector<A, 5> test_vec = {A{0}, A{1}, A{2}, A{3}, A{4}};
      const auto it = collections::dereferencing_iterator<decltype(test_vec)> {
          test_vec.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&test_vec) _WITE_DEREF_ITER_DEBUG_ARG(test_vec.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(test_vec.ptr() + v.size())
      };

      it->x = 5;

      REQUIRE(5 == test_vec[0].x);
    }
  }

  SECTION("operations that return reference to self return mutating versions") {
    SECTION("pre-increment operator") {
      auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      auto it_2 = ++it;

      *it_2 = 10;
      REQUIRE(10 == *it);
    }

    SECTION("post-increment operator") {
      auto it            = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      const auto it_prev = it++;

      *it_prev = 6;
      REQUIRE(6 == v[0]);
    }

    SECTION("pre-decrement operator") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      auto it_2 = --it;

      *it_2 = 10;
      REQUIRE(10 == *it);
    }

    SECTION("post-decrement operator") {
      auto it            = iterator_t{v.ptr() + v.size() - 1 _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      const auto it_prev = it--;

      *it_prev = 6;
      REQUIRE(6 == v[4]);
    }

    SECTION("increment assignment") {
      auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      const auto it_2 = it += 3;

      *it_2 = 10;
      REQUIRE(10 == *it);
    }

    SECTION("decrement assignment") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      const auto it_2 = it -= 3;

      *it_2 = 10;
      REQUIRE(10 == *it);
    }

    SECTION("positive offset operator") {
      auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())} + 3;
      *it = 10;
      REQUIRE(10 == v[3]);
    }

    SECTION("negative offset operator") {
      auto it = iterator_t{v.ptr() + v.size() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())} - 3;
      *it = 10;
      REQUIRE(10 == v[2]);
    }

    SECTION("square-bracket operator") {
      const auto it = iterator_t{v.ptr() _WITE_DEREF_ITER_DEBUG_ARG(&v) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr()) _WITE_DEREF_ITER_DEBUG_ARG(v.ptr() + v.size())};
      it[3] = 10;
      REQUIRE(10 == it[3]);
    }
  }
}