#include "wite/binascii/hexlify.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <string>

using namespace wite;
using namespace std::string_literals;

#define WITE_MAKE_BYTE_TEST_TUPLE(val) \
  { io::byte(0x##val), #val }

TEST_CASE("Hexlify and unhexlify") {
  SECTION("converts a byte into the correct char pair") {
    // clang-format off
    const auto [byte, byte_str] = GENERATE(table<io::byte, std::string>({
      WITE_MAKE_BYTE_TEST_TUPLE(00), WITE_MAKE_BYTE_TEST_TUPLE(01), WITE_MAKE_BYTE_TEST_TUPLE(02), WITE_MAKE_BYTE_TEST_TUPLE(03),
      WITE_MAKE_BYTE_TEST_TUPLE(04), WITE_MAKE_BYTE_TEST_TUPLE(05), WITE_MAKE_BYTE_TEST_TUPLE(06), WITE_MAKE_BYTE_TEST_TUPLE(07),
      WITE_MAKE_BYTE_TEST_TUPLE(08), WITE_MAKE_BYTE_TEST_TUPLE(09), WITE_MAKE_BYTE_TEST_TUPLE(0A), WITE_MAKE_BYTE_TEST_TUPLE(0B),
      WITE_MAKE_BYTE_TEST_TUPLE(0C), WITE_MAKE_BYTE_TEST_TUPLE(0D), WITE_MAKE_BYTE_TEST_TUPLE(0E), WITE_MAKE_BYTE_TEST_TUPLE(0F),
      WITE_MAKE_BYTE_TEST_TUPLE(10), WITE_MAKE_BYTE_TEST_TUPLE(11), WITE_MAKE_BYTE_TEST_TUPLE(12), WITE_MAKE_BYTE_TEST_TUPLE(13),
      WITE_MAKE_BYTE_TEST_TUPLE(14), WITE_MAKE_BYTE_TEST_TUPLE(15), WITE_MAKE_BYTE_TEST_TUPLE(16), WITE_MAKE_BYTE_TEST_TUPLE(17),
      WITE_MAKE_BYTE_TEST_TUPLE(18), WITE_MAKE_BYTE_TEST_TUPLE(19), WITE_MAKE_BYTE_TEST_TUPLE(1A), WITE_MAKE_BYTE_TEST_TUPLE(1B),
      WITE_MAKE_BYTE_TEST_TUPLE(1C), WITE_MAKE_BYTE_TEST_TUPLE(1D), WITE_MAKE_BYTE_TEST_TUPLE(1E), WITE_MAKE_BYTE_TEST_TUPLE(1F),
      WITE_MAKE_BYTE_TEST_TUPLE(20), WITE_MAKE_BYTE_TEST_TUPLE(21), WITE_MAKE_BYTE_TEST_TUPLE(22), WITE_MAKE_BYTE_TEST_TUPLE(23),
      WITE_MAKE_BYTE_TEST_TUPLE(24), WITE_MAKE_BYTE_TEST_TUPLE(25), WITE_MAKE_BYTE_TEST_TUPLE(26), WITE_MAKE_BYTE_TEST_TUPLE(27),
      WITE_MAKE_BYTE_TEST_TUPLE(28), WITE_MAKE_BYTE_TEST_TUPLE(29), WITE_MAKE_BYTE_TEST_TUPLE(2A), WITE_MAKE_BYTE_TEST_TUPLE(2B),
      WITE_MAKE_BYTE_TEST_TUPLE(2C), WITE_MAKE_BYTE_TEST_TUPLE(2D), WITE_MAKE_BYTE_TEST_TUPLE(2E), WITE_MAKE_BYTE_TEST_TUPLE(2F),
      WITE_MAKE_BYTE_TEST_TUPLE(30), WITE_MAKE_BYTE_TEST_TUPLE(31), WITE_MAKE_BYTE_TEST_TUPLE(32), WITE_MAKE_BYTE_TEST_TUPLE(33),
      WITE_MAKE_BYTE_TEST_TUPLE(34), WITE_MAKE_BYTE_TEST_TUPLE(35), WITE_MAKE_BYTE_TEST_TUPLE(36), WITE_MAKE_BYTE_TEST_TUPLE(37),
      WITE_MAKE_BYTE_TEST_TUPLE(38), WITE_MAKE_BYTE_TEST_TUPLE(39), WITE_MAKE_BYTE_TEST_TUPLE(3A), WITE_MAKE_BYTE_TEST_TUPLE(3B),
      WITE_MAKE_BYTE_TEST_TUPLE(3C), WITE_MAKE_BYTE_TEST_TUPLE(3D), WITE_MAKE_BYTE_TEST_TUPLE(3E), WITE_MAKE_BYTE_TEST_TUPLE(3F),
      WITE_MAKE_BYTE_TEST_TUPLE(40), WITE_MAKE_BYTE_TEST_TUPLE(41), WITE_MAKE_BYTE_TEST_TUPLE(42), WITE_MAKE_BYTE_TEST_TUPLE(43),
      WITE_MAKE_BYTE_TEST_TUPLE(44), WITE_MAKE_BYTE_TEST_TUPLE(45), WITE_MAKE_BYTE_TEST_TUPLE(46), WITE_MAKE_BYTE_TEST_TUPLE(47),
      WITE_MAKE_BYTE_TEST_TUPLE(48), WITE_MAKE_BYTE_TEST_TUPLE(49), WITE_MAKE_BYTE_TEST_TUPLE(4A), WITE_MAKE_BYTE_TEST_TUPLE(4B),
      WITE_MAKE_BYTE_TEST_TUPLE(4C), WITE_MAKE_BYTE_TEST_TUPLE(4D), WITE_MAKE_BYTE_TEST_TUPLE(4E), WITE_MAKE_BYTE_TEST_TUPLE(4F),
      WITE_MAKE_BYTE_TEST_TUPLE(50), WITE_MAKE_BYTE_TEST_TUPLE(51), WITE_MAKE_BYTE_TEST_TUPLE(52), WITE_MAKE_BYTE_TEST_TUPLE(53),
      WITE_MAKE_BYTE_TEST_TUPLE(54), WITE_MAKE_BYTE_TEST_TUPLE(55), WITE_MAKE_BYTE_TEST_TUPLE(56), WITE_MAKE_BYTE_TEST_TUPLE(57),
      WITE_MAKE_BYTE_TEST_TUPLE(58), WITE_MAKE_BYTE_TEST_TUPLE(59), WITE_MAKE_BYTE_TEST_TUPLE(5A), WITE_MAKE_BYTE_TEST_TUPLE(5B),
      WITE_MAKE_BYTE_TEST_TUPLE(5C), WITE_MAKE_BYTE_TEST_TUPLE(5D), WITE_MAKE_BYTE_TEST_TUPLE(5E), WITE_MAKE_BYTE_TEST_TUPLE(5F),
      WITE_MAKE_BYTE_TEST_TUPLE(60), WITE_MAKE_BYTE_TEST_TUPLE(61), WITE_MAKE_BYTE_TEST_TUPLE(62), WITE_MAKE_BYTE_TEST_TUPLE(63),
      WITE_MAKE_BYTE_TEST_TUPLE(64), WITE_MAKE_BYTE_TEST_TUPLE(65), WITE_MAKE_BYTE_TEST_TUPLE(66), WITE_MAKE_BYTE_TEST_TUPLE(67),
      WITE_MAKE_BYTE_TEST_TUPLE(68), WITE_MAKE_BYTE_TEST_TUPLE(69), WITE_MAKE_BYTE_TEST_TUPLE(6A), WITE_MAKE_BYTE_TEST_TUPLE(6B),
      WITE_MAKE_BYTE_TEST_TUPLE(6C), WITE_MAKE_BYTE_TEST_TUPLE(6D), WITE_MAKE_BYTE_TEST_TUPLE(6E), WITE_MAKE_BYTE_TEST_TUPLE(6F),
      WITE_MAKE_BYTE_TEST_TUPLE(70), WITE_MAKE_BYTE_TEST_TUPLE(71), WITE_MAKE_BYTE_TEST_TUPLE(72), WITE_MAKE_BYTE_TEST_TUPLE(73),
      WITE_MAKE_BYTE_TEST_TUPLE(74), WITE_MAKE_BYTE_TEST_TUPLE(75), WITE_MAKE_BYTE_TEST_TUPLE(76), WITE_MAKE_BYTE_TEST_TUPLE(77),
      WITE_MAKE_BYTE_TEST_TUPLE(78), WITE_MAKE_BYTE_TEST_TUPLE(79), WITE_MAKE_BYTE_TEST_TUPLE(7A), WITE_MAKE_BYTE_TEST_TUPLE(7B),
      WITE_MAKE_BYTE_TEST_TUPLE(7C), WITE_MAKE_BYTE_TEST_TUPLE(7D), WITE_MAKE_BYTE_TEST_TUPLE(7E), WITE_MAKE_BYTE_TEST_TUPLE(7F),
      WITE_MAKE_BYTE_TEST_TUPLE(80), WITE_MAKE_BYTE_TEST_TUPLE(81), WITE_MAKE_BYTE_TEST_TUPLE(82), WITE_MAKE_BYTE_TEST_TUPLE(83),
      WITE_MAKE_BYTE_TEST_TUPLE(84), WITE_MAKE_BYTE_TEST_TUPLE(85), WITE_MAKE_BYTE_TEST_TUPLE(86), WITE_MAKE_BYTE_TEST_TUPLE(87),
      WITE_MAKE_BYTE_TEST_TUPLE(88), WITE_MAKE_BYTE_TEST_TUPLE(89), WITE_MAKE_BYTE_TEST_TUPLE(8A), WITE_MAKE_BYTE_TEST_TUPLE(8B),
      WITE_MAKE_BYTE_TEST_TUPLE(8C), WITE_MAKE_BYTE_TEST_TUPLE(8D), WITE_MAKE_BYTE_TEST_TUPLE(8E), WITE_MAKE_BYTE_TEST_TUPLE(8F),
      WITE_MAKE_BYTE_TEST_TUPLE(90), WITE_MAKE_BYTE_TEST_TUPLE(91), WITE_MAKE_BYTE_TEST_TUPLE(92), WITE_MAKE_BYTE_TEST_TUPLE(93),
      WITE_MAKE_BYTE_TEST_TUPLE(94), WITE_MAKE_BYTE_TEST_TUPLE(95), WITE_MAKE_BYTE_TEST_TUPLE(96), WITE_MAKE_BYTE_TEST_TUPLE(97),
      WITE_MAKE_BYTE_TEST_TUPLE(98), WITE_MAKE_BYTE_TEST_TUPLE(99), WITE_MAKE_BYTE_TEST_TUPLE(9A), WITE_MAKE_BYTE_TEST_TUPLE(9B),
      WITE_MAKE_BYTE_TEST_TUPLE(9C), WITE_MAKE_BYTE_TEST_TUPLE(9D), WITE_MAKE_BYTE_TEST_TUPLE(9E), WITE_MAKE_BYTE_TEST_TUPLE(9F),
      WITE_MAKE_BYTE_TEST_TUPLE(A0), WITE_MAKE_BYTE_TEST_TUPLE(A1), WITE_MAKE_BYTE_TEST_TUPLE(A2), WITE_MAKE_BYTE_TEST_TUPLE(A3),
      WITE_MAKE_BYTE_TEST_TUPLE(A4), WITE_MAKE_BYTE_TEST_TUPLE(A5), WITE_MAKE_BYTE_TEST_TUPLE(A6), WITE_MAKE_BYTE_TEST_TUPLE(A7),
      WITE_MAKE_BYTE_TEST_TUPLE(A8), WITE_MAKE_BYTE_TEST_TUPLE(A9), WITE_MAKE_BYTE_TEST_TUPLE(AA), WITE_MAKE_BYTE_TEST_TUPLE(AB),
      WITE_MAKE_BYTE_TEST_TUPLE(AC), WITE_MAKE_BYTE_TEST_TUPLE(AD), WITE_MAKE_BYTE_TEST_TUPLE(AE), WITE_MAKE_BYTE_TEST_TUPLE(AF),
      WITE_MAKE_BYTE_TEST_TUPLE(B0), WITE_MAKE_BYTE_TEST_TUPLE(B1), WITE_MAKE_BYTE_TEST_TUPLE(B2), WITE_MAKE_BYTE_TEST_TUPLE(B3),
      WITE_MAKE_BYTE_TEST_TUPLE(B4), WITE_MAKE_BYTE_TEST_TUPLE(B5), WITE_MAKE_BYTE_TEST_TUPLE(B6), WITE_MAKE_BYTE_TEST_TUPLE(B7),
      WITE_MAKE_BYTE_TEST_TUPLE(B8), WITE_MAKE_BYTE_TEST_TUPLE(B9), WITE_MAKE_BYTE_TEST_TUPLE(BA), WITE_MAKE_BYTE_TEST_TUPLE(BB),
      WITE_MAKE_BYTE_TEST_TUPLE(BC), WITE_MAKE_BYTE_TEST_TUPLE(BD), WITE_MAKE_BYTE_TEST_TUPLE(BE), WITE_MAKE_BYTE_TEST_TUPLE(BF),
      WITE_MAKE_BYTE_TEST_TUPLE(C0), WITE_MAKE_BYTE_TEST_TUPLE(C1), WITE_MAKE_BYTE_TEST_TUPLE(C2), WITE_MAKE_BYTE_TEST_TUPLE(C3),
      WITE_MAKE_BYTE_TEST_TUPLE(C4), WITE_MAKE_BYTE_TEST_TUPLE(C5), WITE_MAKE_BYTE_TEST_TUPLE(C6), WITE_MAKE_BYTE_TEST_TUPLE(C7),
      WITE_MAKE_BYTE_TEST_TUPLE(C8), WITE_MAKE_BYTE_TEST_TUPLE(C9), WITE_MAKE_BYTE_TEST_TUPLE(CA), WITE_MAKE_BYTE_TEST_TUPLE(CB),
      WITE_MAKE_BYTE_TEST_TUPLE(CC), WITE_MAKE_BYTE_TEST_TUPLE(CD), WITE_MAKE_BYTE_TEST_TUPLE(CE), WITE_MAKE_BYTE_TEST_TUPLE(CF),
      WITE_MAKE_BYTE_TEST_TUPLE(D0), WITE_MAKE_BYTE_TEST_TUPLE(D1), WITE_MAKE_BYTE_TEST_TUPLE(D2), WITE_MAKE_BYTE_TEST_TUPLE(D3),
      WITE_MAKE_BYTE_TEST_TUPLE(D4), WITE_MAKE_BYTE_TEST_TUPLE(D5), WITE_MAKE_BYTE_TEST_TUPLE(D6), WITE_MAKE_BYTE_TEST_TUPLE(D7),
      WITE_MAKE_BYTE_TEST_TUPLE(D8), WITE_MAKE_BYTE_TEST_TUPLE(D9), WITE_MAKE_BYTE_TEST_TUPLE(DA), WITE_MAKE_BYTE_TEST_TUPLE(DB),
      WITE_MAKE_BYTE_TEST_TUPLE(DC), WITE_MAKE_BYTE_TEST_TUPLE(DD), WITE_MAKE_BYTE_TEST_TUPLE(DE), WITE_MAKE_BYTE_TEST_TUPLE(DF),
      WITE_MAKE_BYTE_TEST_TUPLE(E0), WITE_MAKE_BYTE_TEST_TUPLE(E1), WITE_MAKE_BYTE_TEST_TUPLE(E2), WITE_MAKE_BYTE_TEST_TUPLE(E3),
      WITE_MAKE_BYTE_TEST_TUPLE(E4), WITE_MAKE_BYTE_TEST_TUPLE(E5), WITE_MAKE_BYTE_TEST_TUPLE(E6), WITE_MAKE_BYTE_TEST_TUPLE(E7),
      WITE_MAKE_BYTE_TEST_TUPLE(E8), WITE_MAKE_BYTE_TEST_TUPLE(E9), WITE_MAKE_BYTE_TEST_TUPLE(EA), WITE_MAKE_BYTE_TEST_TUPLE(EB),
      WITE_MAKE_BYTE_TEST_TUPLE(EC), WITE_MAKE_BYTE_TEST_TUPLE(ED), WITE_MAKE_BYTE_TEST_TUPLE(EE), WITE_MAKE_BYTE_TEST_TUPLE(EF),
      WITE_MAKE_BYTE_TEST_TUPLE(F0), WITE_MAKE_BYTE_TEST_TUPLE(F1), WITE_MAKE_BYTE_TEST_TUPLE(F2), WITE_MAKE_BYTE_TEST_TUPLE(F3),
      WITE_MAKE_BYTE_TEST_TUPLE(F4), WITE_MAKE_BYTE_TEST_TUPLE(F5), WITE_MAKE_BYTE_TEST_TUPLE(F6), WITE_MAKE_BYTE_TEST_TUPLE(F7),
      WITE_MAKE_BYTE_TEST_TUPLE(F8), WITE_MAKE_BYTE_TEST_TUPLE(F9), WITE_MAKE_BYTE_TEST_TUPLE(FA), WITE_MAKE_BYTE_TEST_TUPLE(FB),
      WITE_MAKE_BYTE_TEST_TUPLE(FC), WITE_MAKE_BYTE_TEST_TUPLE(FD), WITE_MAKE_BYTE_TEST_TUPLE(FE), WITE_MAKE_BYTE_TEST_TUPLE(FF)
    }));
    // clang-format on

    SECTION(byte_str) {
      SECTION("hexlify") {
        const auto encoded_str = binascii::hexlify(io::dynamic_byte_buffer{byte});
        REQUIRE(byte_str == encoded_str);

        SECTION("unhexlify") {
          REQUIRE(io::dynamic_byte_buffer{byte} == binascii::unhexlify(encoded_str));
        }
      }
    }
  }

  SECTION("Converts a byte string into bytes") {

    const auto [test_name, str] = GENERATE(table<std::string, std::string>({
        {"upper-case", "0123456789ABCDEF"},
        {"lower-case", "0123456789abcdef"}
    }));

    const auto expected = io::static_byte_buffer<8>{io::byte(0x01),
                                                    io::byte(0x23),
                                                    io::byte(0x45),
                                                    io::byte(0x67),
                                                    io::byte(0x89),
                                                    io::byte(0xAB),
                                                    io::byte(0xCD),
                                                    io::byte(0xEF)};

    SECTION(test_name) {
      const auto bytes = binascii::unhexlify(str);

      REQUIRE(expected.size() == bytes.size());
      REQUIRE(io::to_integer<uint32_t>(expected[0]) == io::to_integer<uint32_t>(bytes[0]));
      REQUIRE(io::to_integer<uint32_t>(expected[1]) == io::to_integer<uint32_t>(bytes[1]));
      REQUIRE(io::to_integer<uint32_t>(expected[2]) == io::to_integer<uint32_t>(bytes[2]));
      REQUIRE(io::to_integer<uint32_t>(expected[3]) == io::to_integer<uint32_t>(bytes[3]));
      REQUIRE(io::to_integer<uint32_t>(expected[4]) == io::to_integer<uint32_t>(bytes[4]));
      REQUIRE(io::to_integer<uint32_t>(expected[5]) == io::to_integer<uint32_t>(bytes[5]));
      REQUIRE(io::to_integer<uint32_t>(expected[6]) == io::to_integer<uint32_t>(bytes[6]));
      REQUIRE(io::to_integer<uint32_t>(expected[7]) == io::to_integer<uint32_t>(bytes[7]));
    }
  }
}