#include <wite/io/byte_stream.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <sstream>

using namespace wite;

// Helps the REQUIRE_THROW_AS macro to work in some cases.
#define COMMA ,

TEST_CASE("Byte streams", "[buffer_io]") {
  SECTION("Read int") {
    std::stringstream stream("\x67\x45\x23\x01\xEF\xCD\xAB\x89");

    REQUIRE(uint32_t(0x01234567) == io::stream::read<uint32_t>(stream));
    REQUIRE(uint32_t(0x89ABCDEF) == io::stream::read<uint32_t>(stream));
  }

  SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
    std::stringstream stream("\xAB\x89\x67");

    REQUIRE_THROWS_AS(io::stream::read<uint32_t>(stream), std::out_of_range);
  }
}
