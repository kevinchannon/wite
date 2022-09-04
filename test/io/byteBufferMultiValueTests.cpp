#include <wite/io/byte_buffer.hpp>
#include <wite/io/byte_stream.hpp>  // This is here to make sure that things build in eachothers presence.
#include <wite/io/types.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <string>

using namespace std::string_literals;

using namespace wite;

#define COMMA ,

TEST_CASE("Write multiple values to buffer", "[buffer_io]") {

  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;
  const auto d = uint32_t{0xFEDCBA98};

  auto buffer  = io::static_byte_buffer<sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)>{};

  const auto [test_name, writer] =
      GENERATE_REF(table<std::string, std::function<void()>>({
        {"Direct buffer access"s, [&]() { io::write(buffer, a, io::big_endian{b}, c, d); }},
        {"Via read buffer view"s, [&]() { 
          auto write_view = io::byte_write_buffer_view{buffer};
          io::write(write_view, a, io::big_endian{b}, c, d);
        }}}));

  SECTION(test_name) {
    writer();
  }

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

TEST_CASE("Write multiple values from buffer throws out_of_range if the buffer is too small", "[buffer_io]") {
  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;
  const auto d = uint32_t{0xFEDCBA98};

  auto buffer = io::static_byte_buffer<sizeof(a) + sizeof(b) + sizeof(c)>{};

  REQUIRE_THROWS_AS(io::write(buffer, a, io::big_endian{b}, c, d), std::out_of_range);
}

TEST_CASE("Read multiple values from buffer", "[buffer_io]") {
  // clang-format off
  const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
    std::byte{0x78}, std::byte{0x56}, std::byte{0x34}, std::byte{0x12},
    std::byte{0xAB}, std::byte{0xCD},
    std::byte{true},
    std::byte{0x98}, std::byte{0xBA}, std::byte{0xDC}, std::byte{0xFE}
  };
  // clang-format on

  const auto [a, b, c, d] = io::read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(buffer);

  REQUIRE( a == uint32_t{0x12345678});
  REQUIRE( b == uint16_t{0xABCD});
  REQUIRE( c == true);
  REQUIRE( d == uint32_t{0xFEDCBA98});
}

TEST_CASE("Read multiple values from buffer throws out_of_range if the buffer is too small", "[buffer_io]") {
  // clang-format off
  const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool)>{
    std::byte{0x78}, std::byte{0x56}, std::byte{0x34}, std::byte{0x12},
    std::byte{0xAB}, std::byte{0xCD},
    std::byte{true}
  };
  // clang-format on

  REQUIRE_THROWS_AS(io::read<uint32_t COMMA io::big_endian<uint16_t> COMMA bool COMMA uint32_t>(buffer), std::out_of_range);
}

TEST_CASE("Try read multiple values from buffer", "[buffer_io]") {
  // clang-format off
  const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
    std::byte{0x78}, std::byte{0x56}, std::byte{0x34}, std::byte{0x12},
    std::byte{0xAB}, std::byte{0xCD},
    std::byte{true},
    std::byte{0x98}, std::byte{0xBA}, std::byte{0xDC}, std::byte{0xFE}
  };
  // clang-format on

  const auto [a, b, c, d] = io::try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(buffer);

  REQUIRE(a.ok());
  REQUIRE(a.value() == uint32_t{0x12345678});

  REQUIRE(b.ok());
  REQUIRE(b.value() == uint16_t{0xABCD});

  REQUIRE(c.ok());
  REQUIRE(c.value() == true);

  REQUIRE(d.ok());
  REQUIRE(d.value() == uint32_t{0xFEDCBA98});
}

TEST_CASE("Try read multiple values inserts errors if the buffer is too small", "[buffer_io]") {
  // clang-format off
  const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t)>{
    std::byte{0x78}, std::byte{0x56}, std::byte{0x34}, std::byte{0x12},
    std::byte{0xAB}, std::byte{0xCD}
  };
  // clang-format on

  const auto [a, b, c, d] = io::try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(buffer);

  REQUIRE(a.ok());
  REQUIRE(a.value() == uint32_t{0x12345678});

  REQUIRE(b.ok());
  REQUIRE(b.value() == uint16_t{0xABCD});

  REQUIRE(c.is_error());
  REQUIRE(io::read_error::insufficient_buffer == c.error());

  REQUIRE(d.is_error());
  REQUIRE(io::read_error::insufficient_buffer == d.error());
}

TEST_CASE("Try write multiple values to buffer", "[buffer_io]") {
  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;
  const auto d = uint32_t{0xFEDCBA98};

  auto buffer = io::static_byte_buffer<sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)>{};

  const auto [rc_a, rc_b, rc_c, rc_d] = io::try_write(buffer, a, io::big_endian{b}, c, d);

  REQUIRE(rc_a.ok());
  REQUIRE(true == rc_a.value());

  REQUIRE(rc_b.ok());
  REQUIRE(true == rc_b.value());

  REQUIRE(rc_c.ok());
  REQUIRE(true == rc_c.value());

  REQUIRE(rc_d.ok());
  REQUIRE(true == rc_d.value());

  REQUIRE(uint32_t{0x78} == std::to_integer<uint32_t>(buffer[0]));
  REQUIRE(uint32_t{0x56} == std::to_integer<uint32_t>(buffer[1]));
  REQUIRE(uint32_t{0x34} == std::to_integer<uint32_t>(buffer[2]));
  REQUIRE(uint32_t{0x12} == std::to_integer<uint32_t>(buffer[3]));

  REQUIRE(uint32_t{0xAB} == std::to_integer<uint32_t>(buffer[4]));
  REQUIRE(uint32_t{0xCD} == std::to_integer<uint32_t>(buffer[5]));

  REQUIRE(uint32_t{true} == std::to_integer<uint32_t>(buffer[6]));

  REQUIRE(uint32_t{0x98} == std::to_integer<uint32_t>(buffer[7]));
  REQUIRE(uint32_t{0xBA} == std::to_integer<uint32_t>(buffer[8]));
  REQUIRE(uint32_t{0xDC} == std::to_integer<uint32_t>(buffer[9]));
  REQUIRE(uint32_t{0xFE} == std::to_integer<uint32_t>(buffer[10]));
}

TEST_CASE("Try write multiple values from buffer inserts errors if the buffer is too small", "[buffer_io]") {
  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;
  const auto d = uint32_t{0xFEDCBA98};

  auto buffer = io::static_byte_buffer<sizeof(a) + sizeof(b)>{};

  const auto [rc_a, rc_b, rc_c, rc_d] = io::try_write(buffer, a, io::big_endian{b}, c, d);

  REQUIRE(rc_a.ok());
  REQUIRE(true == rc_a.value());

  REQUIRE(rc_b.ok());
  REQUIRE(true == rc_b.value());

  REQUIRE(rc_c.is_error());
  REQUIRE(io::write_error::insufficient_buffer == rc_c.error());

  REQUIRE(rc_d.is_error());
  REQUIRE(io::write_error::insufficient_buffer == rc_d.error());

  REQUIRE(uint32_t{0x78} == std::to_integer<uint32_t>(buffer[0]));
  REQUIRE(uint32_t{0x56} == std::to_integer<uint32_t>(buffer[1]));
  REQUIRE(uint32_t{0x34} == std::to_integer<uint32_t>(buffer[2]));
  REQUIRE(uint32_t{0x12} == std::to_integer<uint32_t>(buffer[3]));

  REQUIRE(uint32_t{0xAB} == std::to_integer<uint32_t>(buffer[4]));
  REQUIRE(uint32_t{0xCD} == std::to_integer<uint32_t>(buffer[5]));
}
