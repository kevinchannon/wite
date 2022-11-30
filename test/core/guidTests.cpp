#include <wite/core/guid.hpp>
#include <wite/core/io.hpp>

#include <catch2/catch_test_macros.hpp>

#include <sstream>

using namespace wite;

TEST_CASE("GUID IO tests", "[core]") {
  SECTION("GUID can be inserted into a stream") {
    const auto g = GUID{0x01234567, 0x89AB, 0xCDEF, {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};
    std::stringstream ss;

    ss << g;

    REQUIRE("{01234567-89AB-CDEF-0123-456789ABCDEF}");
  }
}
