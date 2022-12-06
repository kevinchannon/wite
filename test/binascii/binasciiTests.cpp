#include <test/utils.hpp>
#include <wite/binascii/hexlify.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace wite;

TEST_CASE("From hex chars tests", "[binascii]") {
  SECTION("uint8_t") {
    SECTION("works for valid string") {
      REQUIRE(uint8_t{0xAB} == binascii::from_hex_chars<uint8_t>("Ab"));
    }

    SECTION("throws std::invalid_argument if the string is too short") {
      WITE_REQ_THROWS(binascii::from_hex_chars<uint8_t>("9"), std::invalid_argument, "Invalid sequence length for type");
    }

    SECTION("throws std::invalid_argument if the string is too long") {
      WITE_REQ_THROWS(binascii::from_hex_chars<uint8_t>("679"), std::invalid_argument, "Invalid sequence length for type");
    }

    SECTION("throws std::invalid_argument if the string contains invalid characters") {
      WITE_REQ_THROWS(binascii::from_hex_chars<uint8_t>("FG"), std::invalid_argument, "Invalid hex char");
    }
  }

  SECTION("uint16_t") {
    SECTION("works for valid string") {
      REQUIRE(uint16_t{0xDCBA} == binascii::from_hex_chars<uint16_t>("BadC"));
    }

    SECTION("throws std::invalid_argument if the string is too short") {
      WITE_REQ_THROWS(binascii::from_hex_chars<uint16_t>("98B"), std::invalid_argument, "Invalid sequence length for type");
    }

    SECTION("throws std::invalid_argument if the string is too long") {
      WITE_REQ_THROWS(binascii::from_hex_chars<uint16_t>("6798B"), std::invalid_argument, "Invalid sequence length for type");
    }

    SECTION("throws std::invalid_argument if the string contains invalid characters") {
      WITE_REQ_THROWS(binascii::from_hex_chars<uint16_t>("DEFG"), std::invalid_argument, "Invalid hex char");
    }
  }

  SECTION("uint32_t") {
    SECTION("works for valid string") {
      REQUIRE(uint32_t{0xFEDCBA98} == binascii::from_hex_chars<uint32_t>("98BADcfE"));
    }

    SECTION("throws std::invalid_argument if the string is too short") {
      WITE_REQ_THROWS(binascii::from_hex_chars<uint32_t>("98BADC"), std::invalid_argument, "Invalid sequence length for type");
    }

    SECTION("throws std::invalid_argument if the string is too long") {
      WITE_REQ_THROWS(
          binascii::from_hex_chars<uint32_t>("6798BADCFE"), std::invalid_argument, "Invalid sequence length for type");
    }

    SECTION("throws std::invalid_argument if the string contains invalid characters") {
      WITE_REQ_THROWS(binascii::from_hex_chars<uint32_t>("89ABCDEG"), std::invalid_argument, "Invalid hex char");
    }
  }

  SECTION("uint64_t") {
    SECTION("works for valid string") {
      REQUIRE(uint64_t{0xFEDCBA9876543210} == binascii::from_hex_chars<uint64_t>("1032547698BADcfE"));
    }

    SECTION("throws std::invalid_argument if the string is too short") {
      WITE_REQ_THROWS(
          binascii::from_hex_chars<uint64_t>("1032547698BADcf"), std::invalid_argument, "Invalid sequence length for type");
    }

    SECTION("throws std::invalid_argument if the string is too long") {
      WITE_REQ_THROWS(
          binascii::from_hex_chars<uint64_t>("1032547698BADcfE0"), std::invalid_argument, "Invalid sequence length for type");
    }

    SECTION("throws std::invalid_argument if the string contains invalid characters") {
      WITE_REQ_THROWS(binascii::from_hex_chars<uint64_t>("1032547698BADxfE"), std::invalid_argument, "Invalid hex char");
    }
  }
}

TEST_CASE("Try from hex chars tests", "[binascii]") {
  SECTION("uint8_t") {
    SECTION("works for valid string") {
      REQUIRE(binascii::try_from_hex_chars<uint8_t>("Ab").ok());
      REQUIRE(uint8_t{0xAB} == binascii::try_from_hex_chars<uint8_t>("Ab").value());
    }

    SECTION("returns invalid_sequence_length if the string is too short") {
      REQUIRE(binascii::try_from_hex_chars<uint8_t>("9").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_sequence_length == binascii::try_from_hex_chars<uint8_t>("9").error());
    }

    SECTION("returns invalid_sequence_length if the string is too long") {
      REQUIRE(binascii::try_from_hex_chars<uint8_t>("679").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_sequence_length == binascii::try_from_hex_chars<uint8_t>("679").error());
    }

    SECTION("returns invalid_hex_char if the string contains invalid characters") {
      REQUIRE(binascii::try_from_hex_chars<uint8_t>("FG").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_hex_char == binascii::try_from_hex_chars<uint8_t>("FG").error());
    }
  }

  SECTION("uint16_t") {
    SECTION("works for valid string") {
      REQUIRE(binascii::try_from_hex_chars<uint16_t>("BadC").ok());
      REQUIRE(uint16_t{0xDCBA} == binascii::try_from_hex_chars<uint16_t>("BadC").value());
    }

    SECTION("returns invalid_sequence_length if the string is too short") {
      REQUIRE(binascii::try_from_hex_chars<uint16_t>("98B").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_sequence_length == binascii::try_from_hex_chars<uint16_t>("98B").error());
    }

    SECTION("returns invalid_sequence_length if the string is too long") {
      REQUIRE(binascii::try_from_hex_chars<uint16_t>("6798B").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_sequence_length == binascii::try_from_hex_chars<uint16_t>("6798B").error());
    }

    SECTION("returns invalid_hex_char if the string contains invalid characters") {
      REQUIRE(binascii::try_from_hex_chars<uint16_t>("DEFG").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_hex_char == binascii::try_from_hex_chars<uint16_t>("DEFG").error());
    }
  }

  SECTION("uint32_t") {
    SECTION("works for valid string") {
      REQUIRE(binascii::try_from_hex_chars<uint32_t>("98BADcfE").ok());
      REQUIRE(uint32_t{0xFEDCBA98} == binascii::try_from_hex_chars<uint32_t>("98BADcfE").value());
    }

    SECTION("returns invalid_sequence_length if the string is too short") {
      REQUIRE(binascii::try_from_hex_chars<uint32_t>("98BADC").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_sequence_length ==
              binascii::try_from_hex_chars<uint32_t>("98BADC").error());
    }

    SECTION("returns invalid_sequence_length if the string is too long") {
      REQUIRE(binascii::try_from_hex_chars<uint32_t>("6798BADCFE").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_sequence_length ==
              binascii::try_from_hex_chars<uint32_t>("6798BADCFE").error());
    }

    SECTION("returns invalid_hex_char if the string contains invalid characters") {
      REQUIRE(binascii::try_from_hex_chars<uint32_t>("89ABCDEG").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_hex_char == binascii::try_from_hex_chars<uint32_t>("89ABCDEG").error());
    }
  }

  SECTION("uint64_t") {
    SECTION("works for valid string") {
      REQUIRE(binascii::try_from_hex_chars<uint64_t>("1032547698BADcfE").ok());
      REQUIRE(uint64_t{0xFEDCBA9876543210} == binascii::try_from_hex_chars<uint64_t>("1032547698BADcfE").value());
    }

    SECTION("returns invalid_sequence_length if the string is too short") {
      REQUIRE(binascii::try_from_hex_chars<uint64_t>("1032547698BADcf").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_sequence_length ==
              binascii::try_from_hex_chars<uint64_t>("1032547698BADcf").error());
    }

    SECTION("returns invalid_sequence_length if the string is too long") {
      REQUIRE(binascii::try_from_hex_chars<uint64_t>("1032547698BADcfE0").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_sequence_length ==
              binascii::try_from_hex_chars<uint64_t>("1032547698BADcfE0").error());
    }

    SECTION("returns invalid_hex_char if the string contains invalid characters") {
      REQUIRE(binascii::try_from_hex_chars<uint64_t>("1032547698BADxfE").is_error());
      REQUIRE(binascii::from_hex_chars_error::invalid_hex_char ==
              binascii::try_from_hex_chars<uint64_t>("1032547698BADxfE").error());
    }
  }
}

TEST_CASE("Unhexlify fixed size data", "[binascii]") {
  SECTION("8 uint8_t values") {
    REQUIRE(std::array<uint8_t, 8>{0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10} ==
            binascii::unhexlify<8, uint8_t>("FEDCBA9876543210"));
  }

  SECTION("6 io::byte values") {
    REQUIRE(
        std::array<io::byte, 6>{io::byte{0xFE}, io::byte{0xDC}, io::byte{0xBA}, io::byte{0x98}, io::byte{0x76}, io::byte{0x54}} ==
        binascii::unhexlify<6, io::byte>("FEDCBA987654"));
  }

  SECTION("throws std::invalid_argument if the output size doesn't match the sequence length") {
    const auto thrower = [](const char* s) { std::ignore = binascii::unhexlify<10, uint8_t>(s); };
    WITE_REQ_THROWS(thrower("1234567891234567891"), std::invalid_argument, "Invalid sequence length");
    WITE_REQ_THROWS(thrower("123456789123456789100"), std::invalid_argument, "Invalid sequence length");
  }
}
