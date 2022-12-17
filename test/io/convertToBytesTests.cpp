#include <wite/io/byte_buffer.hpp>

#include "../utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <array>
#include <cstdint>
#include <cstddef>

using namespace wite;

TEMPLATE_TEST_CASE("from bytes", "[buffer_io]", io::byte, uint8_t, unsigned char) {
  SECTION("uint64") {
    const auto bytes = std::array<TestType, sizeof(uint64_t)>{TestType(0x11),
                                                               TestType(0xFF),
                                                               TestType(0x22),
                                                               TestType(0xEE),
                                                               TestType(0x33),
                                                               TestType(0xDD),
                                                               TestType(0x44),
                                                               TestType(0xCC)};

    REQUIRE(uint64_t{0xCC44DD33EE22FF11} == io::from_bytes<uint64_t>(bytes));
  }

  SECTION("uint32") {
    const auto bytes =
        std::array<TestType, sizeof(uint32_t)>{TestType(0x22), TestType(0xEE), TestType(0x33), TestType(0xDD)};

    REQUIRE(uint32_t{0xDD33EE22} == io::from_bytes<io::little_endian<uint32_t>>(bytes));
    REQUIRE(uint32_t{0x22EE33DD} == io::from_bytes<io::big_endian<uint32_t>>(bytes));
  }

  SECTION("uint16") {
    const auto bytes = std::array<TestType, sizeof(uint16_t)>{TestType(0xD3), TestType(0x4D)};
    REQUIRE(uint16_t{0x4DD3} == io::from_bytes<io::little_endian<uint16_t>>(bytes));
    REQUIRE(uint16_t{0xD34D} == io::from_bytes<io::big_endian<uint16_t>>(bytes));
  }

  SECTION("uint8_t") {
    const auto bytes = std::array<TestType, sizeof(uint8_t)>{TestType(0xCC)};
    REQUIRE(uint8_t{0xCC} == io::from_bytes<uint8_t>(bytes));
  }

  SECTION("bool") {
    const auto true_bytes = std::array<TestType, sizeof(bool)>{TestType{true}};
    REQUIRE(true == io::from_bytes<bool>(true_bytes));

    const auto false_bytes = std::array<TestType, sizeof(bool)>{TestType{false}};
    REQUIRE(false == io::from_bytes<bool>(false_bytes));
  }

  SECTION("double") {
    auto bytes   = std::array<TestType, sizeof(double)>{};
    const auto d = 1.35743452e+55;
    std::copy_n(reinterpret_cast<const TestType*>(&d), sizeof(d), bytes.begin());

    REQUIRE(d == io::from_bytes<double>(bytes));
  }

  SECTION("float") {
    auto bytes   = std::array<TestType, sizeof(float)>{};
    const auto f = 1.3574e+10f;
    std::copy_n(reinterpret_cast<const TestType*>(&f), sizeof(f), bytes.begin());

    REQUIRE(f == io::from_bytes<float>(bytes));
  }
}

TEMPLATE_TEST_CASE("try from bytes", "[buffer_io]", io::byte, uint8_t, unsigned char) {
  SECTION("uint64") {
    const auto bytes = std::array<TestType, sizeof(uint64_t)>{TestType(0x11),
                                                              TestType(0xFF),
                                                              TestType(0x22),
                                                              TestType(0xEE),
                                                              TestType(0x33),
                                                              TestType(0xDD),
                                                              TestType(0x44),
                                                              TestType(0xCC)};

    REQUIRE(io::try_from_bytes<uint64_t>(bytes).ok());
    REQUIRE(uint64_t{0xCC44DD33EE22FF11} == io::try_from_bytes<uint64_t>(bytes).value());
  }

  SECTION("uint32") {
    const auto bytes =
        std::array<TestType, sizeof(uint32_t)>{TestType(0x22), TestType(0xEE), TestType(0x33), TestType(0xDD)};

    REQUIRE(io::try_from_bytes<io::little_endian<uint32_t>>(bytes).ok());
    REQUIRE(uint32_t{0xDD33EE22} == io::try_from_bytes<io::little_endian<uint32_t>>(bytes).value());
    REQUIRE(uint32_t{0x22EE33DD} == io::try_from_bytes<io::big_endian<uint32_t>>(bytes).value());
  }

  SECTION("uint16") {
    const auto bytes = std::array<TestType, sizeof(uint16_t)>{TestType(0xD3), TestType(0x4D)};

    REQUIRE(io::try_from_bytes<io::little_endian<uint16_t>>(bytes).ok());
    REQUIRE(uint16_t{0x4DD3} == io::try_from_bytes<io::little_endian<uint16_t>>(bytes).value());

    REQUIRE(io::try_from_bytes<io::big_endian<uint16_t>>(bytes).ok());
    REQUIRE(uint16_t{0xD34D} == io::try_from_bytes<io::big_endian<uint16_t>>(bytes).value());
  }

  SECTION("uint8_t") {
    const auto bytes = std::array<TestType, sizeof(uint8_t)>{TestType(0xCC)};

    REQUIRE(io::try_from_bytes<uint8_t>(bytes).ok());
    REQUIRE(uint8_t{0xCC} == io::try_from_bytes<uint8_t>(bytes).value());
  }

  SECTION("bool") {
    const auto true_bytes = std::array<TestType, sizeof(bool)>{TestType{true}};
    REQUIRE(io::try_from_bytes<bool>(true_bytes).ok());
    REQUIRE(true == io::try_from_bytes<bool>(true_bytes).value());

    const auto false_bytes = std::array<TestType, sizeof(bool)>{TestType{false}};
    REQUIRE(io::try_from_bytes<bool>(false_bytes).ok());
    REQUIRE(false == io::try_from_bytes<bool>(false_bytes).value());
  }

  SECTION("double") {
    auto bytes   = std::array<TestType, sizeof(double)>{};
    const auto d = 1.35743452e+55;
    std::copy_n(reinterpret_cast<const TestType*>(&d), sizeof(d), bytes.begin());

    REQUIRE(io::try_from_bytes<double>(bytes).ok());
    REQUIRE(d == io::try_from_bytes<double>(bytes).value());
  }

  SECTION("float") {
    auto bytes   = std::array<TestType, sizeof(float)>{};
    const auto f = 1.3574e+10f;
    std::copy_n(reinterpret_cast<const TestType*>(&f), sizeof(f), bytes.begin());

    REQUIRE(io::try_from_bytes<float>(bytes).ok());
    REQUIRE(f == io::try_from_bytes<float>(bytes).value());
  }
}

TEST_CASE("to bytes", "[buffer_io]") {
  SECTION("uint64") {
    const auto bytes = std::array<io::byte, sizeof(uint64_t)>{io::byte(0x11),
                                                               io::byte(0xFF),
                                                               io::byte(0x22),
                                                               io::byte(0xEE),
                                                               io::byte(0x33),
                                                               io::byte(0xDD),
                                                               io::byte(0x44),
                                                               io::byte(0xCC)};

    REQUIRE(test::ranges_equal(bytes, io::to_bytes(0xCC44DD33EE22FF11)));
  }

  SECTION("uint32") {
    const auto bytes =
        std::array<io::byte, sizeof(uint32_t)>{io::byte(0x22), io::byte(0xEE), io::byte(0x33), io::byte(0xDD)};

    REQUIRE(test::ranges_equal(bytes, io::to_bytes<io::little_endian<uint32_t>>(0xDD33EE22)));
    REQUIRE(test::ranges_equal(bytes, io::to_bytes<io::big_endian<uint32_t>>(0x22EE33DD)));
  }

  SECTION("uint16") {
    const auto bytes = std::array<io::byte, sizeof(uint16_t)>{io::byte(0xD3), io::byte(0x4D)};
    REQUIRE(test::ranges_equal(bytes, io::to_bytes(uint16_t{0x4DD3})));
    REQUIRE(test::ranges_equal(bytes, io::to_bytes<io::big_endian<uint16_t>>(uint16_t{0xD34D})));
  }

  SECTION("uint8_t") {
    const auto bytes = std::array<io::byte, sizeof(uint8_t)>{io::byte(0xCC)};
    REQUIRE(test::ranges_equal(bytes, io::to_bytes<uint8_t>(uint8_t{0xCC})));
  }

  SECTION("bool") {
    const auto true_bytes = std::array<io::byte, sizeof(bool)>{io::byte{true}};
    REQUIRE(test::ranges_equal(true_bytes, io::to_bytes(true)));

    const auto false_bytes = std::array<io::byte, sizeof(bool)>{io::byte{false}};
    REQUIRE(test::ranges_equal(false_bytes, io::to_bytes(false)));
  }

  SECTION("double") {
    auto bytes   = std::array<io::byte, sizeof(double)>{};
    const auto d = 1.35743452e+55;
    std::copy_n(reinterpret_cast<const io::byte*>(&d), sizeof(d), bytes.begin());

    REQUIRE(test::ranges_equal(bytes, io::to_bytes(d)));
  }

  SECTION("float") {
    auto bytes   = std::array<io::byte, sizeof(float)>{};
    const auto f = 1.3574e+10f;
    std::copy_n(reinterpret_cast<const io::byte*>(&f), sizeof(f), bytes.begin());

    REQUIRE(test::ranges_equal(bytes, io::to_bytes(f)));
  }
}

TEST_CASE("try_to_bytes", "[buffer_io]") {
  SECTION("successful conversion") {
    REQUIRE(io::try_to_bytes(int(999)).ok());
    REQUIRE(test::ranges_equal(io::static_byte_buffer<4>{io::byte(0x12), io::byte(0x34), io::byte(0x56), io::byte(0x78)},
                               io::try_to_bytes(0x78563412).value()));
  }

  SECTION("Failing conversion") {
    REQUIRE(io::try_to_bytes<uint32_t, io::static_byte_buffer<3>>(0x12345678).is_error());
    REQUIRE(io::write_error::insufficient_buffer == io::try_to_bytes<uint32_t, io::static_byte_buffer<3>>(0x12345678).error());
  }
}
