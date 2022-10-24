#include <wite/geometry/io.hpp>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

using namespace wite::geometry;

TEST_CASE("Write point to ostream", "[geometry]") {
  SECTION("narrow characters") {
    std::stringstream ss;
    ss << point_2d{1.2, 3.4};

    REQUIRE("[ 1.2, 3.4 ]" == ss.str());
  }

  SECTION("wide characters") {
    std::wstringstream ss;
    ss << point_2d{1.2, 3.4};

    REQUIRE(L"[ 1.2, 3.4 ]" == ss.str());
  }
}
