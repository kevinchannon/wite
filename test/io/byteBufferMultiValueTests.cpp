#include <wite/io/byte_buffer.hpp>
#include <wite/io/byte_stream.hpp>  // This is here to make sure that things build in eachothers presence.
#include <wite/io/types.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstddef>

using namespace wite;

TEST_CASE("Write multiple values to buffer", "[buffer_io]") {

  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;
  const auto d = uint32_t{0xFEDCBA98};

  auto buffer  = io::static_byte_buffer<sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)>{};

  io::write(buffer, a, io::big_endian{b}, c, d);

  REQUIRE(uint32_t{0x78} == std::to_integer<uint32_t>(buffer[ 0]));
  REQUIRE(uint32_t{0x56} == std::to_integer<uint32_t>(buffer[ 1]));
  REQUIRE(uint32_t{0x34} == std::to_integer<uint32_t>(buffer[ 2]));
  REQUIRE(uint32_t{0x12} == std::to_integer<uint32_t>(buffer[ 3]));

  REQUIRE(uint32_t{0xAB} == std::to_integer<uint32_t>(buffer[ 4]));
  REQUIRE(uint32_t{0xCD} == std::to_integer<uint32_t>(buffer[ 5]));

  REQUIRE(uint32_t{true} == std::to_integer<uint32_t>(buffer[ 6]));

  REQUIRE(uint32_t{0x98} == std::to_integer<uint32_t>(buffer[ 7]));
  REQUIRE(uint32_t{0xBA} == std::to_integer<uint32_t>(buffer[ 8]));
  REQUIRE(uint32_t{0xDC} == std::to_integer<uint32_t>(buffer[ 9]));
  REQUIRE(uint32_t{0xFE} == std::to_integer<uint32_t>(buffer[10]));
}

TEST_CASE("Read multiple values from buffer", "[buffer_io]") {
  // clang-format off
  const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
    std::byte{0x78}, std::byte{0x56}, std::byte{0x34}, std::byte{0x12},
    std::byte{0xCD}, std::byte{0xAB},
    std::byte{true},
    std::byte{0x98}, std::byte{0xBA}, std::byte{0xDC}, std::byte{0xFE}
  };
  // clang-format on

  const auto [a, b, c, d] = io::read<uint32_t, uint16_t, bool, uint32_t>(buffer);

  REQUIRE( a == uint32_t{0x12345678});
  REQUIRE( b == uint16_t{0xABCD});
  REQUIRE( c == true);
  REQUIRE( d == uint32_t{0xFEDCBA98});
}
