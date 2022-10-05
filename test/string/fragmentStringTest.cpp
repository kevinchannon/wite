#include <wite/string/fragment_string.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace wite;

TEST_CASE("fragment_string tests", "[string]") {
  SECTION("construct") {
    SECTION("from a single C-string") {
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

  SECTION("length and size match") {
    const auto fs = fragment_string{"Lorem ipsum dolor sit"};
    REQUIRE(fs.length() == fs.size());
  }
}

TEST_CASE("fragment_string iterator", "[string]") {
  const char* s1 = "first fragment";
  const char* s2 = "second fragment";

  const auto fs = fragment_string{s1} + " " + s2;

  SECTION("get begin") {
    REQUIRE('f' == *fs.begin());
  }

  SECTION("end points to the null char at the end of the last fragment") {
    const auto end = fs.end();
    REQUIRE((s2 + std::strlen(s2) == &(*end)));
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
        SECTION("incrementing out past the end throws std::out_of_range in debug") {
          REQUIRE_THROWS_AS(++it, std::out_of_range);
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
      }
    }
  }
}
