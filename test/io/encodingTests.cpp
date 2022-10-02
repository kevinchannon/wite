#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace wite;

TEST_CASE("little_endian tests", "[buffer_io]") {
  SECTION("begin_byte points to the first byte of the value") {
      const auto val = uint32_t{0x3240BD3F};
      REQUIRE(0x3F == io::to_integer<uint8_t>(*io::little_endian{val}.byte_begin()));
    }

  SECTION("byte_count returns the size of the encoded value") {
      REQUIRE(2 == io::little_endian{uint16_t{}}.byte_count());
      REQUIRE(4 == io::little_endian{uint32_t{}}.byte_count());
      REQUIRE(8 == io::little_endian{uint64_t{}}.byte_count());
  }
}
