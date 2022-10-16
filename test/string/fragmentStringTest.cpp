#include <wite/string/fragment_string.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace wite;

TEST_CASE("fragment_string tests", "[string]") {
  SECTION("construct") {
    SECTION("from a single C-string") {
      SECTION("has zero length for empty string") {
        REQUIRE(0 == fragment_string{""}.length());
      }

      SECTION("narrow chars") {
        const auto fs = fragment_string{"hello!"};

        REQUIRE(6 == fs.length());
        REQUIRE(std::string{"hello!"} == fs.to_str());
      }

      SECTION("wide chars") {
        const auto fs = fragment_wstring{L"hello!"};

        REQUIRE(6 == fs.length());
        REQUIRE(std::wstring{L"hello!"} == fs.to_str());
      }
    }
  }

  SECTION("concatenate") {
    SECTION("narrow chars") {
      const auto fs_1 = fragment_string{"me "};
      REQUIRE(std::string{"me "} == fs_1.to_str());

      const auto fs_2 = fs_1 + "thinks ";
      REQUIRE(std::string{"me thinks "} == fs_2.to_str());

      const auto fs_3 = fragment_string{"it "};
      REQUIRE(std::string{"it "} == fs_3.to_str());

      const auto fs_4 = fs_2 + fs_3;
      REQUIRE(std::string{"me thinks it "} == fs_4.to_str());

      const auto fs_5 = fs_4 + "is " + "a " + "weasle";
      REQUIRE(std::string{"me thinks it is a weasle"} == fs_5.to_str());
    }

    SECTION("wide chars") {
      const auto fs_1 = fragment_wstring{L"me "};
      REQUIRE(std::wstring{L"me "} == fs_1.to_str());

      const auto fs_2 = fs_1 + L"thinks ";
      REQUIRE(std::wstring{L"me thinks "} == fs_2.to_str());

      const auto fs_3 = fragment_wstring{L"it "};
      REQUIRE(std::wstring{L"it "} == fs_3.to_str());

      const auto fs_4 = fs_2 + fs_3;
      REQUIRE(std::wstring{L"me thinks it "} == fs_4.to_str());

      const auto fs_5 = fs_4 + L"is " + L"a " + L"weasle";
      REQUIRE(std::wstring{L"me thinks it is a weasle"} == fs_5.to_str());
    }
  }

  SECTION("length and size") {
    const auto fs = fragment_string{"Lorem"} + " ipsum" + " dolor" + " sit";

    SECTION("length() and size() return the same value") {
      REQUIRE(fs.length() == fs.size());
    }

    SECTION("length() and max_size() return the same value") {
      REQUIRE(fs.length() == fs.max_size());
    }

    SECTION("length() and capacity() return the same value") {
      REQUIRE(fs.length() == fs.capacity());
    }
  }

  SECTION("element access") {
    const char* s1 = "abcdefghij";
    const char* s2 = "lmnopqrs";

    const auto fs = fragment_string{s1} + "k" + s2 + "tuvwxyz";

    const auto equivalent_str = std::string_view{"abcdefghijklmnopqrstuvwxyz"};

    SECTION("operator[]") {
      for (auto i = 0u; i < equivalent_str.length(); ++i) {
        REQUIRE(equivalent_str[i] == fs[i]);
      }
    }

    SECTION("at() returns the same as operator[]") {
      for (auto i = 0u; i < fs.length(); ++i) {
        REQUIRE(fs[i] == fs.at(i));
      }
    }

    SECTION("at() throws out of range if requested element is beyond the end of the string") {
      REQUIRE_THROWS_AS(fs.at(26), std::out_of_range);
    }

    SECTION("front() returns the first character") {
      REQUIRE('a' == fs.front());
    }

#ifdef _WITE_CONFIG_DEBUG
    SECTION("front() asserts if the string has zero length") {
      REQUIRE_THROWS_AS(fragment_string("").front(), wite::assertion_error);
    }
#endif

    SECTION("back() returns the last character") {
      REQUIRE('z' == fs.back());
    }

#ifdef _WITE_CONFIG_DEBUG
    SECTION("back() asserts if the string has zero length") {
      REQUIRE_THROWS_AS(fragment_string("").back(), wite::assertion_error);
    }
#endif
  }

  SECTION("empty() returns the correct value") {
    SECTION("when the string is non-empty") {
      REQUIRE_FALSE(fragment_string("some chars").empty());
    }

    SECTION("when the string is empty") {
      const auto fs = fragment_string{""} + "" + "" + "";
      REQUIRE(fs.empty());
    }
  }

  SECTION("compare()...") {
    const auto fs = fragment_string{"defg"} + "hijk" + "lmnop";

    SECTION("with std::string") {
      REQUIRE(0 == fs.compare(std::string("defghijklmnop")));
      REQUIRE(0 > fs.compare(std::string("defghjjklmnop")));
      REQUIRE(0 < fs.compare(std::string("defghiiklmnop")));
      REQUIRE(0 < fs.compare(std::string("defghijklmno")));
      REQUIRE(0 > fs.compare(std::string("defghijklmnopq")));
      REQUIRE(0 < fs.compare(std::string("cdefghjjklmn")));
    }

    SECTION("with const char*") {
      REQUIRE(0 == fs.compare("defghijklmnop"));
      REQUIRE(0 > fs.compare("defghjjklmnop"));
      REQUIRE(0 < fs.compare("defghiiklmnop"));
      REQUIRE(0 < fs.compare("defghijklmno"));
      REQUIRE(0 > fs.compare("defghijklmnopq"));
      REQUIRE(0 < fs.compare("cdefghjjklmn"));
    }

    SECTION("with std::string_view") {
      REQUIRE(0 == fs.compare(std::string_view("defghijklmnop")));
      REQUIRE(0 > fs.compare(std::string_view("defghjjklmnop")));
      REQUIRE(0 < fs.compare(std::string_view("defghiiklmnop")));
      REQUIRE(0 < fs.compare(std::string_view("defghijklmno")));
      REQUIRE(0 > fs.compare(std::string_view("defghijklmnopq")));
      REQUIRE(0 < fs.compare(std::string_view("cdefghjjklmn")));
    }

    SECTION("with fragment_string") {
      REQUIRE(0 == fs.compare(fragment_string{"defgh"} + "ijklmnop"));
      REQUIRE(0 == fs.compare(fragment_string{"de"} + "fgh" + "ij" + "klm" + "no" + "p"));

      REQUIRE(0 > fs.compare(fragment_string{"defgh"} + "jjklmnop"));
      REQUIRE(0 > fs.compare(fragment_string{"de"} + "fgh" + "jj" + "klm" + "no" + "p"));

      REQUIRE(0 < fs.compare(fragment_string{"defgh"} + "iiklmnop"));
      REQUIRE(0 < fs.compare(fragment_string{"de"} + "fgh" + "ii" + "klm" + "no" + "p"));

      REQUIRE(0 < fs.compare(fragment_string{"defgh"} + "ijklmno"));
      REQUIRE(0 < fs.compare(fragment_string{"de"} + "fgh" + "ij" + "klm" + "no"));

      REQUIRE(0 > fs.compare(fragment_string{"defgh"} + "ijklmnopq"));
      REQUIRE(0 > fs.compare(fragment_string{"de"} + "fgh" + "ij" + "klm" + "no" + "pq"));

      REQUIRE(0 < fs.compare(fragment_string{"cdefgh"} + "jjklmn"));
      REQUIRE(0 < fs.compare(fragment_string{"cde"} + "fgh" + "jj" + "klm" + "n"));
    }
  }

  SECTION("starts_with()") {
    const auto fs = fragment_string("xyz01") + "23456" + "789";

    SECTION("a single character") {
      REQUIRE_FALSE(fragment_string().starts_with('?'));
      REQUIRE(fs.starts_with('x'));
      REQUIRE_FALSE(fs.starts_with('y'));
    }

    SECTION("a string view") {
      REQUIRE_FALSE(fragment_string().starts_with(std::string_view("anything")));
      REQUIRE(fs.starts_with(std::string_view("xyz0123")));
      REQUIRE_FALSE(fs.starts_with(std::string_view("yz0123")));
      REQUIRE_FALSE(fs.starts_with(std::string_view("xyz0123456789A")));
    }

    SECTION("a c-string") {
      REQUIRE_FALSE(fragment_string().starts_with("anything"));
      REQUIRE(fs.starts_with("xyz0123"));
      REQUIRE_FALSE(fs.starts_with("yz0123"));
      REQUIRE_FALSE(fs.starts_with("xyz0123456789A"));
    }

    SECTION("a fragment_string") {
      REQUIRE_FALSE(fragment_string().starts_with(fragment_string("any") + "thing"));
      REQUIRE(fs.starts_with(fragment_string("xyz") + "0123"));
      REQUIRE_FALSE(fs.starts_with(fragment_string("y") + "z0123"));
      REQUIRE_FALSE(fs.starts_with(fragment_string("xyz012345678") + "9A"));
    }
  }

  SECTION("ends_with()") {
    const auto fs = fragment_string("x") + "yx0123456" + "789" + "ABCDE";

    SECTION("a single character") {
      REQUIRE_FALSE(fragment_string().ends_with('?'));
      REQUIRE(fs.ends_with('E'));
      REQUIRE_FALSE(fs.ends_with('D'));
    }

    SECTION("a string view") {
      REQUIRE_FALSE(fragment_string().ends_with(std::string_view("anything")));
      REQUIRE(fs.ends_with(std::string_view("89ABCDE")));
      REQUIRE_FALSE(fs.ends_with(std::string_view("BC")));
      REQUIRE_FALSE(fs.ends_with(std::string_view("wxyz0123456789ABCDE")));
    }

    SECTION("a c-string") {
      REQUIRE_FALSE(fragment_string().ends_with("anything"));
      REQUIRE(fs.ends_with("89ABCDE"));
      REQUIRE_FALSE(fs.ends_with("BC"));
      REQUIRE_FALSE(fs.ends_with("wxyz0123456789ABCDE"));
    }

    SECTION("a fragment_string") {
      REQUIRE_FALSE(fragment_string().ends_with(fragment_string("any") + "thing"));
      REQUIRE(fs.ends_with(fragment_string("9A") + "BCDE"));
      REQUIRE_FALSE(fs.ends_with(fragment_string("A") + "BCD"));
      REQUIRE_FALSE(fs.ends_with(fragment_string("xyz012345678") + "9ABCDEF"));
    }
  }

  SECTION("contains()") {
    const auto fs = fragment_string("defghij") + "klmnopqr" + "st" + "uvw";

    SECTION("a single character") {
      REQUIRE_FALSE(fragment_string().contains('?'));

      SECTION("valid leters return true")
      {
        const auto valid_letter =
            GENERATE('d', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w');

        REQUIRE(fs.contains(valid_letter));
      }

      SECTION("invalid letters return false") {
        const auto invalid_letter = GENERATE('a', 'b', 'c', 'x', 'y', 'z');

        REQUIRE_FALSE(fs.contains(invalid_letter));
      }
    }

    SECTION("a string view") {
      REQUIRE_FALSE(fragment_string().contains(std::string_view("anything")));
      REQUIRE(fs.contains(std::string_view("ijkl")));
      REQUIRE_FALSE(fs.contains(std::string_view("ijkkl")));
      REQUIRE_FALSE(fs.contains(std::string_view("cdefjhijklmnopqrstuvw")));
    }

    SECTION("a c-string") {
      REQUIRE_FALSE(fragment_string().contains("anything"));
      REQUIRE(fs.contains("ijkl"));
      REQUIRE_FALSE(fs.contains("ijkkl"));
      REQUIRE_FALSE(fs.contains("cdefjhijklmnopqrstuvw"));
    }

    SECTION("a fragment_string") {
      REQUIRE_FALSE(fragment_string().contains(fragment_string("any") + "thing"));
      REQUIRE(fs.contains(fragment_string("ijkl") + "m"));
      REQUIRE_FALSE(fs.contains(fragment_string("cde") + "fghijk"));
      REQUIRE_FALSE(fs.contains(fragment_string("cdef") + "g" + "hijklmnopqrstuvw"));
    }
  }

  SECTION("substr()") {
    const auto fs = fragment_string("defghij") + "klmnopqr" + "st" + "uvw";

    SECTION("single fragment substring from beginning") {
      REQUIRE(std::string{"defgh"} == fs.substr(0, 5));
    }

    SECTION("multi-fragment substring from beginning") {
      REQUIRE(std::string{"defghijklm"} == fs.substr(0, 10));
    }

    SECTION("multi-fragment substring from middle") {
      REQUIRE(std::string{"qrstuv"} == fs.substr(13, 6));
    }

    SECTION("multi-fragment right sub-string") {
      REQUIRE(std::string("tuvw") == fs.substr(16, 4));
    }

    SECTION("throw std::out_of_range when position is beyond the end of the string") {
      REQUIRE_THROWS_AS(fs.substr(20), std::out_of_range);
    }

    SECTION("clips substring length to length of string") {
      REQUIRE(std::string("tuvw") == fs.substr(16, 10));
    }
  }
}

TEST_CASE("fragment_string iterator", "[string]") {
  const char* s1 = "first fragment";
  const char* s2 = "second fragment";

  const auto fs = fragment_string{s1} + " " + s2;
  const auto fs2 = fragment_string{"A second string"} + "parts to get" + "correct fragment count";

  SECTION("get begin") {
    REQUIRE('f' == *fs.begin());
  }

  SECTION("end points to the null char at the end of the last fragment") {
    auto end = fs.end();
    REQUIRE(s2 + std::strlen(s2) == (&(*(--end)) + 1));
  }

  SECTION("operator++") {
    auto it = fs.begin();

    SECTION("iterates over the first fragment...") {
      for (auto i = 0u; i < std::strlen(s1); ++i) {
        REQUIRE(s1[i] == *it);
        ++it;
      }

      SECTION("and then traverses across fagments...") {
        REQUIRE(' ' == *it);
        ++it;

        for (auto i = 0u; i < std::strlen(s2); ++i) {
          REQUIRE(s2[i] == *it);
          ++it;
        }

        SECTION("and finishes at the end") {
          REQUIRE(fs.end() == it);
        }

#ifdef _WITE_CONFIG_DEBUG
        SECTION("incrementing out past the end asserts in debug") {
          REQUIRE_THROWS_AS(++it, wite::assertion_error);
        }
#endif
      }
    }
  }

  SECTION("operator--") {
    auto it = fs.end();

    SECTION("iterates over the last fragment") {
      for (auto i = static_cast<int64_t>(std::strlen(s2) - 1); i >= 0; --i) {
        --it;
        REQUIRE(s2[i] == *it);
      }

      SECTION("and then traverses across fagments...") {
        --it;
        REQUIRE(' ' == *it);

        for (auto i = static_cast<int64_t>(std::strlen(s1) - 1); i >= 0; --i) {
          --it;
          REQUIRE(s1[i] == *it);
        }

        SECTION("and finishes at the beginning") {
          REQUIRE(fs.begin() == it);
        }

#ifdef _WITE_CONFIG_DEBUG
        SECTION("decrementing past the beginning asserts in debug") {
          REQUIRE_THROWS_AS(--it, wite::assertion_error);
        }
#endif
      }
    }
  }

  SECTION("operator+=") {
    // clang-format off
    auto [test_name, direction, it] = GENERATE_REF(table<const char*, int, decltype(fs)::iterator>({
      {"positive offset", 1, fs.begin()},
      {"negative offset", -1, --fs.end()}
    }));
    // clang-format on

    const auto reference_string = std::string{"first fragment second fragment"};
    auto expected               = (direction > 0 ? reference_string.begin() : std::prev(reference_string.end()));

    SECTION(test_name) {
      SECTION("increment within fragment") {
        it += direction * 5;
        expected += direction * 5;
        REQUIRE(*expected == *it);
      }

      SECTION("increment up to fragment boundary") {
        it += direction * 13;
        expected += direction * 13;
        REQUIRE(*expected == *it);
      }

      SECTION("increment over fragment boundary") {
        it += direction * 14;
        expected += direction * 14;
        REQUIRE(*expected == *it);
      }

      SECTION("increment accross multiple fragments") {
        it += direction * 20;
        expected += direction * 20;
        REQUIRE(*expected == *it);
      }

#ifdef _WITE_CONFIG_DEBUG
      SECTION("increment outside string asserts in debug") {
        REQUIRE_THROWS_AS(it += direction * 30, wite::assertion_error);
      }
#endif
    }
  }

  SECTION("operator-=") {
    // clang-format off
    auto [test_name, direction, it] = GENERATE_REF(table<const char*, int, decltype(fs)::iterator>({
      {"positive offset", 1, --fs.end()},
      {"negative offset", -1, fs.begin()}
    }));
    // clang-format on

    const auto reference_string = std::string{"first fragment second fragment"};
    auto expected               = (direction > 0 ? std::prev(reference_string.end()) : reference_string.begin());

    SECTION(test_name) {
      SECTION("increment within fragment") {
        it -= direction * 5;
        expected -= direction * 5;
        REQUIRE(*expected == *it);
      }

      SECTION("increment up to fragment boundary") {
        it -= direction * 13;
        expected -= direction * 13;
        REQUIRE(*expected == *it);
      }

      SECTION("increment to fragment boundary") {
        it -= direction * 14;
        expected -= direction * 14;
        REQUIRE(*expected == *it);
      }

      SECTION("increment accross multiple fragments") {
        it -= direction * 20;
        expected -= direction * 20;
        REQUIRE(*expected == *it);
      }

#ifdef _WITE_CONFIG_DEBUG
      SECTION("increment outside string asserts in debug") {
        REQUIRE_THROWS_AS(it -= direction * 30, wite::assertion_error);
      }
#endif
    }
  }

  SECTION("add integer offset") {
    SECTION("smaller than fragment size") {
      const auto it_1 = fs.begin();
      const auto it_2 = it_1 + 4;
      REQUIRE('t' == *it_2);
    }

    SECTION("accross a fragment boundary") {
      const auto it_1 = fs.begin();
      const auto it_2 = it_1 + 20;
      REQUIRE('d' == *it_2);
    }

    SECTION("negative offset") {
      const auto it_1 = fs.begin() + 20;
      const auto it_2 = it_1 + (-10);
      REQUIRE('m' == *it_2);
    }

  #ifdef _WITE_CONFIG_DEBUG
    SECTION("increment outside string asserts in debug") {
      REQUIRE_THROWS_AS(fs.begin() + 30, wite::assertion_error);
    }
  #endif
  }

  SECTION("subtract integer offset") {
    SECTION("smaller than fragment size") {
      const auto it_1 = --fs.end();
      const auto it_2 = it_1 - 4;
      REQUIRE('g' == *it_2);
    }

    SECTION("accross a fragment boundary") {
      const auto it_1 = fs.begin() + 20;
      const auto it_2 = it_1 - 6;
      REQUIRE(' ' == *it_2);
    }

    SECTION("negative offset") {
      const auto it_1 = fs.begin();
      const auto it_2 = it_1 - (-10);
      REQUIRE('m' == *it_2);
    }

  #ifdef _WITE_CONFIG_DEBUG
    SECTION("increment outside string asserts in debug") {
      REQUIRE_THROWS_AS(fs.end() - 31, wite::assertion_error);
    }
  #endif
  }

  SECTION("distance between iterators") {
    SECTION("zero distance"){
      const auto it_1 = fs.begin();
      const auto it_2 = it_1;
      REQUIRE(0 == it_1 - it_2);
      REQUIRE(0 == it_2 - it_1);
    }
      
    SECTION("within a fragment") {
      const auto it_1 = fs.begin();
      const auto it_2 = it_1 + 5;
      REQUIRE(5 == it_2 - it_1);
      REQUIRE(-5 == it_1 - it_2);
    }
    
    SECTION("across fragments") {
      const auto it_1 = fs.begin() + 3;
      const auto it_2 = it_1 + 18;
      REQUIRE(18 == it_2 - it_1);
      REQUIRE(-18 == it_1 - it_2);
    }

#ifdef _WITE_CONFIG_DEBUG
    SECTION("asserts when comparing iterators from different strings in debug") {
      REQUIRE_THROWS_AS(fs.begin() - fs2.begin(), wite::assertion_error);
    }
#endif
  }

  SECTION("Comparison operators") {
    SECTION("equal-to compares as expected") {
      REQUIRE(      fs.begin() == fs.begin());
      REQUIRE_FALSE(fs.begin() == (fs.begin() + 1));

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts when comparing iterators from different strings in debug") {
        REQUIRE_THROWS_AS(fs.begin() == fs2.begin(), wite::assertion_error);
      }
#endif
    }

    SECTION("not-equal-to compares as expected") {
      REQUIRE(      fs.begin() != (fs.begin() + 1));
      REQUIRE_FALSE(fs.begin() != fs.begin());

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts when comparing iterators from different strings in debug") {
        REQUIRE_THROWS_AS(fs.begin() != fs2.begin(), wite::assertion_error);
      }
#endif
    }

    SECTION("less-than compares as expected") {
      REQUIRE(      (fs.begin() + 1) < (fs.begin() + 2));
      REQUIRE_FALSE((fs.begin() + 1) < (fs.begin() + 1));
      REQUIRE_FALSE((fs.begin() + 2) < (fs.begin() + 1));

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts when comparing iterators from different strings in debug") {
        REQUIRE_THROWS_AS(fs.begin() < fs2.begin(), wite::assertion_error);
      }
#endif
    }

    SECTION("less-than-or-equal-to compares as expected") {
      REQUIRE(      (fs.begin() + 1) <= (fs.begin() + 2));
      REQUIRE(      (fs.begin() + 1) <= (fs.begin() + 1));
      REQUIRE_FALSE((fs.begin() + 2) <= (fs.begin() + 1));

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts when comparing iterators from different strings in debug") {
        REQUIRE_THROWS_AS(fs.begin() <= fs2.begin(), wite::assertion_error);
      }
#endif
    }

    SECTION("greater-than compares as expected") {
      REQUIRE(      (fs.begin() + 3) > (fs.begin() + 2));
      REQUIRE_FALSE((fs.begin() + 2) > (fs.begin() + 2));
      REQUIRE_FALSE((fs.begin() + 2) > (fs.begin() + 3));

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts when comparing iterators from different strings in debug") {
        REQUIRE_THROWS_AS(fs.begin() > fs2.begin(), wite::assertion_error);
      }
#endif
    }

    SECTION("greater-than-or-equal-to compares as expected") {
      REQUIRE(      (fs.begin() + 3) >= (fs.begin() + 2));
      REQUIRE(      (fs.begin() + 2) >= (fs.begin() + 2));
      REQUIRE_FALSE((fs.begin() + 2) >= (fs.begin() + 3));

#ifdef _WITE_CONFIG_DEBUG
      SECTION("asserts when comparing iterators from different strings in debug") {
        REQUIRE_THROWS_AS(fs.begin() >= fs2.begin(), wite::assertion_error);
      }
#endif
    }
  }

  SECTION("standard library iterator utilities compatibility") {
    SECTION("std::distance") {
      REQUIRE(3 == std::distance(fs.begin() + 13, fs.begin() + 16));
    }

    SECTION("std::next") {
      REQUIRE('e' == *std::next(fs.begin(), 16));
    }

    SECTION("std::prev") {
      REQUIRE('t' == *std::prev(fs.begin() + 16, 3));
    }

    SECTION("std::advance") {
      auto it = fs.begin();
      std::advance(it, 16);
      REQUIRE('e' == *it);
    }
  }
}

TEST_CASE("fragment_string reverse iterator", "[string]") {
    const char* s1 = "first fragment";
    const char* s2 = "second fragment";

    const auto fs  = fragment_string{s1} + " " + s2;

    REQUIRE("tnemgarf dnoces tnemgarf tsrif" == std::string(fs.rbegin(), fs.rend()));
}