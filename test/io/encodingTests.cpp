/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>

using namespace wite;

TEST_CASE("little_endian tests", "[buffer_io]") {
  SECTION("byte_begin points to the first byte of the value") {
    const auto bytes = std::array<uint8_t, 4>{0x32, 0x40, 0xBD, 0x3F};

    const auto val = *reinterpret_cast<const uint32_t*>(bytes.data());
    REQUIRE((io::system_native_endianness == io::endian::little ? 0x3FBD4032 : 0x3240BD3F) == val);
    REQUIRE((io::system_native_endianness == io::endian::little ? uint32_t{0x32} : uint32_t{0x3F}) ==
            io::to_integer<uint32_t>(*io::little_endian{val}.byte_begin<io::byte>()));
  }

  SECTION("byte_count returns the size of the encoded value") {
    REQUIRE(2 == io::little_endian{uint16_t{}}.byte_count());
    REQUIRE(4 == io::little_endian{uint32_t{}}.byte_count());
    REQUIRE(8 == io::little_endian{uint64_t{}}.byte_count());
  }
}

TEST_CASE("bg_endian tests", "[buffer_io]") {
  SECTION("byte_begin points to the first byte of the value") {
    const auto bytes = std::array<uint8_t, 4>{0x32, 0x40, 0xBD, 0x3F};

    const auto val = *reinterpret_cast<const uint32_t*>(bytes.data());
    REQUIRE((io::system_native_endianness == io::endian::little ? 0x3FBD4032 : 0x3240BD3F) == val);
    REQUIRE((io::system_native_endianness == io::endian::little ? uint32_t{0x3F} : uint32_t{0x32}) == io::to_integer<uint32_t>(*io::big_endian{val}.byte_begin<io::byte>()));
  }

  SECTION("byte_count returns the size of the encoded value") {
    REQUIRE(2 == io::big_endian{uint16_t{}}.byte_count());
    REQUIRE(4 == io::big_endian{uint32_t{}}.byte_count());
    REQUIRE(8 == io::big_endian{uint64_t{}}.byte_count());
  }
}