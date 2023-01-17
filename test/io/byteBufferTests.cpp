/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#include <wite/io/byte_buffer.hpp>
#include <wite/io/byte_stream.hpp>  // This is here to make sure that things build in eachothers presence.
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>


#include "../utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstdint>

using namespace wite;

#ifndef _WITE_COMPILER_MSVC
namespace std {
template <typename T>
initializer_list(const std::initializer_list<T>&) -> initializer_list<T>;
}
#endif

TEST_CASE("Byte buffers write-read tests", "[buffer_io]") {
  auto buffer = io::static_byte_buffer<32>{};

  SECTION("Double value") {
    const auto val = 2.718;

    REQUIRE(sizeof(double) == io::write(buffer, val));
    REQUIRE(val == io::read<double>(buffer));
  }

  SECTION("Uint32 value") {
    const auto val = uint32_t{0xCDCDCDCD};

    REQUIRE(sizeof(uint32_t) == io::write(buffer, val));
    REQUIRE(val == io::read<uint32_t>(buffer));
  }

  SECTION("Bool value") {
    const auto val = GENERATE(true, false);

    REQUIRE(sizeof(bool) == io::write(buffer, val));
    REQUIRE(val == io::read<bool>(buffer));
  }
}

TEST_CASE("Write-read using encodings", "[buffer_io]") {
  auto buffer = io::static_byte_buffer<32>{};

  SECTION("Uint64") {
    const auto val = uint64_t{0x0011223344556677};

    SECTION("Little-endian") {
      REQUIRE(sizeof(uint64_t) == io::write(buffer, io::little_endian{val}));
      REQUIRE(0x77 == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x66 == io::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x55 == io::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x44 == io::to_integer<uint8_t>(buffer[3]));
      REQUIRE(0x33 == io::to_integer<uint8_t>(buffer[4]));
      REQUIRE(0x22 == io::to_integer<uint8_t>(buffer[5]));
      REQUIRE(0x11 == io::to_integer<uint8_t>(buffer[6]));
      REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[7]));

      REQUIRE(val == io::read<io::little_endian<uint64_t>>(buffer));
    }

    SECTION("Big-endian") {
      REQUIRE(sizeof(uint64_t) == io::write(buffer, io::big_endian{val}));
      REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x11 == io::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x22 == io::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x33 == io::to_integer<uint8_t>(buffer[3]));
      REQUIRE(0x44 == io::to_integer<uint8_t>(buffer[4]));
      REQUIRE(0x55 == io::to_integer<uint8_t>(buffer[5]));
      REQUIRE(0x66 == io::to_integer<uint8_t>(buffer[6]));
      REQUIRE(0x77 == io::to_integer<uint8_t>(buffer[7]));

      REQUIRE(val == io::read<io::big_endian<uint64_t>>(buffer));
    }
  }

  SECTION("Uint32") {
    const auto val = uint32_t{0x01234567};

    SECTION("Little-endian") {
      REQUIRE(sizeof(uint32_t) == io::write(buffer, io::little_endian{val}));
      REQUIRE(0x67 == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x45 == io::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x23 == io::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x01 == io::to_integer<uint8_t>(buffer[3]));

      REQUIRE(val == io::read<io::little_endian<uint32_t>>(buffer));
    }

    SECTION("Big-endian") {
      REQUIRE(sizeof(uint32_t) == io::write(buffer, io::big_endian{val}));
      REQUIRE(0x01 == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x23 == io::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x45 == io::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x67 == io::to_integer<uint8_t>(buffer[3]));

      REQUIRE(val == io::read<io::big_endian<uint32_t>>(buffer));
    }
  }

  SECTION("Uint16") {
    const auto val = uint16_t{0xABCD};

    SECTION("Little-endian") {
      REQUIRE(sizeof(uint16_t) == io::write(buffer, io::little_endian{val}));
      REQUIRE(0xCD == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0xAB == io::to_integer<uint8_t>(buffer[1]));

      REQUIRE(val == io::read<io::little_endian<uint16_t>>(buffer));
    }

    SECTION("Big-endian") {
      REQUIRE(sizeof(uint16_t) == io::write(buffer, io::big_endian{val}));
      REQUIRE(0xAB == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0xCD == io::to_integer<uint8_t>(buffer[1]));

      REQUIRE(val == io::read<io::big_endian<uint16_t>>(buffer));
    }
  }
}