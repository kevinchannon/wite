/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#define WITE_NO_EXCEPTIONS

#include <test/utils.hpp>
#include <wite/binascii/hexlify.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace wite;

TEST_CASE("Try from hex chars tests (no except)", "[binascii]") {
  SECTION("uint8_t") {
    SECTION("works for valid string") {
      REQUIRE(binascii::try_from_hex_chars<uint8_t>("Ab").ok());
      REQUIRE(uint8_t{0xAB} == binascii::try_from_hex_chars<uint8_t>("Ab").value());
    }

    SECTION("returns invalid_sequence_length if the string is too short") {
      REQUIRE(binascii::try_from_hex_chars<uint8_t>("9").is_error());
      REQUIRE(binascii::error::invalid_sequence_length == binascii::try_from_hex_chars<uint8_t>("9").error());
    }

    SECTION("returns invalid_sequence_length if the string is too long") {
      REQUIRE(binascii::try_from_hex_chars<uint8_t>("679").is_error());
      REQUIRE(binascii::error::invalid_sequence_length == binascii::try_from_hex_chars<uint8_t>("679").error());
    }

    SECTION("returns invalid_hex_char if the string contains invalid characters") {
      REQUIRE(binascii::try_from_hex_chars<uint8_t>("FG").is_error());
      REQUIRE(binascii::error::invalid_hex_char == binascii::try_from_hex_chars<uint8_t>("FG").error());
    }
  }

  SECTION("uint16_t") {
    SECTION("works for valid string") {
      REQUIRE(binascii::try_from_hex_chars<uint16_t>("BadC").ok());
      REQUIRE(uint16_t{0xDCBA} == binascii::try_from_hex_chars<uint16_t>("BadC").value());
    }

    SECTION("returns invalid_sequence_length if the string is too short") {
      REQUIRE(binascii::try_from_hex_chars<uint16_t>("98B").is_error());
      REQUIRE(binascii::error::invalid_sequence_length == binascii::try_from_hex_chars<uint16_t>("98B").error());
    }

    SECTION("returns invalid_sequence_length if the string is too long") {
      REQUIRE(binascii::try_from_hex_chars<uint16_t>("6798B").is_error());
      REQUIRE(binascii::error::invalid_sequence_length == binascii::try_from_hex_chars<uint16_t>("6798B").error());
    }

    SECTION("returns invalid_hex_char if the string contains invalid characters") {
      REQUIRE(binascii::try_from_hex_chars<uint16_t>("DEFG").is_error());
      REQUIRE(binascii::error::invalid_hex_char == binascii::try_from_hex_chars<uint16_t>("DEFG").error());
    }
  }

  SECTION("uint32_t") {
    SECTION("works for valid string") {
      REQUIRE(binascii::try_from_hex_chars<uint32_t>("98BADcfE").ok());
      REQUIRE(uint32_t{0xFEDCBA98} == binascii::try_from_hex_chars<uint32_t>("98BADcfE").value());
    }

    SECTION("returns invalid_sequence_length if the string is too short") {
      REQUIRE(binascii::try_from_hex_chars<uint32_t>("98BADC").is_error());
      REQUIRE(binascii::error::invalid_sequence_length == binascii::try_from_hex_chars<uint32_t>("98BADC").error());
    }

    SECTION("returns invalid_sequence_length if the string is too long") {
      REQUIRE(binascii::try_from_hex_chars<uint32_t>("6798BADCFE").is_error());
      REQUIRE(binascii::error::invalid_sequence_length == binascii::try_from_hex_chars<uint32_t>("6798BADCFE").error());
    }

    SECTION("returns invalid_hex_char if the string contains invalid characters") {
      REQUIRE(binascii::try_from_hex_chars<uint32_t>("89ABCDEG").is_error());
      REQUIRE(binascii::error::invalid_hex_char == binascii::try_from_hex_chars<uint32_t>("89ABCDEG").error());
    }
  }

  SECTION("uint64_t") {
    SECTION("works for valid string") {
      REQUIRE(binascii::try_from_hex_chars<uint64_t>("1032547698BADcfE").ok());
      REQUIRE(uint64_t{0xFEDCBA9876543210} == binascii::try_from_hex_chars<uint64_t>("1032547698BADcfE").value());
    }

    SECTION("returns invalid_sequence_length if the string is too short") {
      REQUIRE(binascii::try_from_hex_chars<uint64_t>("1032547698BADcf").is_error());
      REQUIRE(binascii::error::invalid_sequence_length == binascii::try_from_hex_chars<uint64_t>("1032547698BADcf").error());
    }

    SECTION("returns invalid_sequence_length if the string is too long") {
      REQUIRE(binascii::try_from_hex_chars<uint64_t>("1032547698BADcfE0").is_error());
      REQUIRE(binascii::error::invalid_sequence_length == binascii::try_from_hex_chars<uint64_t>("1032547698BADcfE0").error());
    }

    SECTION("returns invalid_hex_char if the string contains invalid characters") {
      REQUIRE(binascii::try_from_hex_chars<uint64_t>("1032547698BADxfE").is_error());
      REQUIRE(binascii::error::invalid_hex_char == binascii::try_from_hex_chars<uint64_t>("1032547698BADxfE").error());
    }
  }
}

TEST_CASE("Unhexlify fixed size data (no except)", "[binascii]") {
  SECTION("without exceptions") {
    SECTION("8 uint8_t values") {
      REQUIRE(binascii::try_unhexlify<8, uint8_t>("FEDCBA9876543210").ok());
      REQUIRE(std::array<uint8_t, 8>{0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10} ==
              binascii::try_unhexlify<8, uint8_t>("FEDCBA9876543210").value());
    }
    
    SECTION("returns invalid_sequence_length if the output size doesn't match the sequence length") {
      REQUIRE(binascii::try_unhexlify<10, uint8_t>("1234567891234567891").is_error());
      REQUIRE(binascii::error::invalid_sequence_length == binascii::try_unhexlify<10, uint8_t>("1234567891234567891").error());

      REQUIRE(binascii::try_unhexlify<10, uint8_t>("123456789123456789100").is_error());
      REQUIRE(binascii::error::invalid_sequence_length == binascii::try_unhexlify<10, uint8_t>("123456789123456789100").error());
    }

    SECTION("throws std::invalid_argument if any of the input characters are not valid hex chars") {
      REQUIRE(binascii::try_unhexlify<4, uint8_t>("76543X10").is_error());
      REQUIRE(binascii::error::invalid_hex_char == binascii::try_unhexlify<4, uint8_t>("76543X10").error());
    }
  }
}
