# include <wite/maths/io.hpp>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

using namespace wite::maths;

TEST_CASE("value_range") {
  const auto closed_closed_d = closed_value_range{1.1, 10.5};
  const auto closed_closed_i = closed_value_range{2, 4};

  const auto open_open_d = open_value_range{1.1, 10.5};
  const auto open_open_i = open_value_range{2, 4};

  const auto closed_open_d = value_range<double, range_boundary::closed, range_boundary::open>{1.1, 10.5};
  const auto closed_open_i = value_range<int, range_boundary::closed, range_boundary::open>{2, 4};

  const auto open_closed_d = value_range<double, range_boundary::open, range_boundary::closed>{1.1, 10.5};
  const auto open_closed_i = value_range<int, range_boundary::open, range_boundary::closed>{2, 4};

  SECTION("narrow chars") {
    std::stringstream ss;

    SECTION("closed-closed") {
      SECTION("double range") {
        ss << closed_closed_d;
        REQUIRE("[ 1.1, 10.5 ]" == ss.str());
      }

      SECTION("int range") {
        ss << closed_closed_i;
        REQUIRE("[ 2, 4 ]" == ss.str());
      }
    }

    SECTION("open-open") {
      SECTION("double range") {
        ss << open_open_d;
        REQUIRE("( 1.1, 10.5 )" == ss.str());
      }

      SECTION("int range") {
        ss << open_open_i;
        REQUIRE("( 2, 4 )" == ss.str());
      }
    }

    SECTION("closed-open") {
      SECTION("double range") {
        ss << closed_open_d;
        REQUIRE("[ 1.1, 10.5 )" == ss.str());
      }

      SECTION("int range") {
        ss << closed_open_i;
        REQUIRE("[ 2, 4 )" == ss.str());
      }
    }

    SECTION("open-closed") {
      SECTION("double range") {
        ss << open_closed_d;
        REQUIRE("( 1.1, 10.5 ]" == ss.str());
      }

      SECTION("int range") {
        ss << open_closed_i;
        REQUIRE("( 2, 4 ]" == ss.str());
      }
    }
  }

  SECTION("wide chars") {
    std::wstringstream ss;

    SECTION("closed-closed") {
      SECTION("double range") {
        ss << closed_closed_d;
        REQUIRE(std::wstring(L"[ 1.1, 10.5 ]") == ss.str());
      }

      SECTION("int range") {
        ss << closed_closed_i;
        REQUIRE(std::wstring(L"[ 2, 4 ]") == ss.str());
      }
    }

    SECTION("open-open") {
      SECTION("double range") {
        ss << open_open_d;
        REQUIRE(std::wstring(L"( 1.1, 10.5 )") == ss.str());
      }

      SECTION("int range") {
        ss << open_open_i;
        REQUIRE(std::wstring(L"( 2, 4 )") == ss.str());
      }
    }

    SECTION("closed-open") {
      SECTION("double range") {
        ss << closed_open_d;
        REQUIRE(std::wstring(L"[ 1.1, 10.5 )") == ss.str());
      }

      SECTION("int range") {
        ss << closed_open_i;
        REQUIRE(std::wstring(L"[ 2, 4 )") == ss.str());
      }
    }

    SECTION("open-closed") {
      SECTION("double range") {
        ss << open_closed_d;
        REQUIRE(std::wstring(L"( 1.1, 10.5 ]") == ss.str());
      }

      SECTION("int range") {
        ss << open_closed_i;
        REQUIRE(std::wstring(L"( 2, 4 ]") == ss.str());
      }
    }
  }
}
