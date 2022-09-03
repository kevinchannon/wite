#include <wite/io/byte_buffer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <cstddef>

using namespace wite;

TEST_CASE("from bytes", "[buffer_io]") {
  SECTION("uint64") {
    const auto bytes = std::array<std::byte, sizeof(uint64_t)>{std::byte{0x11},
                                                               std::byte{0xFF},
                                                               std::byte{0x22},
                                                               std::byte{0xEE},
                                                               std::byte{0x33},
                                                               std::byte{0xDD},
                                                               std::byte{0x44},
                                                               std::byte{0xCC}};

    REQUIRE(uint64_t{0xCC44DD33EE22FF11} == io::from_bytes<uint64_t>(bytes));
  }

  SECTION("uint32") {
    const auto bytes =
        std::array<std::byte, sizeof(uint32_t)>{std::byte{0x22}, std::byte{0xEE}, std::byte{0x33}, std::byte{0xDD}};

    REQUIRE(uint32_t{0xDD33EE22} == io::from_bytes<io::little_endian<uint32_t>>(bytes));
    REQUIRE(uint32_t{0x22EE33DD} == io::from_bytes<io::big_endian<uint32_t>>(bytes));
  }

  SECTION("uint16") {
    const auto bytes = std::array<std::byte, sizeof(uint16_t)>{std::byte{0xD3}, std::byte{0x4D}};
    REQUIRE(uint16_t{0x4DD3} == io::from_bytes<io::little_endian<uint16_t>>(bytes));
    REQUIRE(uint16_t{0xD34D} == io::from_bytes<io::big_endian<uint16_t>>(bytes));
  }

  SECTION("uint8_t") {
    const auto bytes = std::array<std::byte, sizeof(uint8_t)>{std::byte{0xCC}};
    REQUIRE(uint8_t{0xCC} == io::from_bytes<uint8_t>(bytes));
  }

  SECTION("bool") {
    const auto true_bytes = std::array<std::byte, sizeof(bool)>{std::byte{true}};
    REQUIRE(true == io::from_bytes<bool>(true_bytes));

    const auto false_bytes = std::array<std::byte, sizeof(bool)>{std::byte{false}};
    REQUIRE(false == io::from_bytes<bool>(false_bytes));
  }

  SECTION("double") {
    auto bytes   = std::array<std::byte, sizeof(double)>{};
    const auto d = 1.35743452e+55;
    std::copy_n(reinterpret_cast<const std::byte*>(&d), sizeof(d), bytes.begin());

    REQUIRE(d == io::from_bytes<double>(bytes));
  }

  SECTION("float") {
    auto bytes   = std::array<std::byte, sizeof(float)>{};
    const auto f = 1.3574e+10f;
    std::copy_n(reinterpret_cast<const std::byte*>(&f), sizeof(f), bytes.begin());

    REQUIRE(f == io::from_bytes<float>(bytes));
  }
}

TEST_CASE("to bytes", "[buffer_io]") {
  SECTION("uint64") {
    const auto bytes = std::array<std::byte, sizeof(uint64_t)>{std::byte{0x11},
                                                               std::byte{0xFF},
                                                               std::byte{0x22},
                                                               std::byte{0xEE},
                                                               std::byte{0x33},
                                                               std::byte{0xDD},
                                                               std::byte{0x44},
                                                               std::byte{0xCC}};

    REQUIRE(std::ranges::equal(bytes, io::to_bytes(0xCC44DD33EE22FF11)));
  }

  SECTION("uint32") {
    const auto bytes =
        std::array<std::byte, sizeof(uint32_t)>{std::byte{0x22}, std::byte{0xEE}, std::byte{0x33}, std::byte{0xDD}};

    REQUIRE(std::ranges::equal(bytes, io::to_bytes<io::little_endian<uint32_t>>(0xDD33EE22)));
    REQUIRE(std::ranges::equal(bytes, io::to_bytes<io::big_endian<uint32_t>>(0x22EE33DD)));
  }

  SECTION("uint16") {
    const auto bytes = std::array<std::byte, sizeof(uint16_t)>{std::byte{0xD3}, std::byte{0x4D}};
    REQUIRE(std::ranges::equal(bytes, io::to_bytes(uint16_t{0x4DD3})));
    REQUIRE(std::ranges::equal(bytes, io::to_bytes<io::big_endian<uint16_t>>(uint16_t{0xD34D})));
  }

  SECTION("uint8_t") {
    const auto bytes = std::array<std::byte, sizeof(uint8_t)>{std::byte{0xCC}};
    REQUIRE(std::ranges::equal(bytes, io::to_bytes<uint8_t>(uint8_t{0xCC})));
  }

  SECTION("bool") {
    const auto true_bytes = std::array<std::byte, sizeof(bool)>{std::byte{true}};
    REQUIRE(std::ranges::equal(true_bytes, io::to_bytes(true)));

    const auto false_bytes = std::array<std::byte, sizeof(bool)>{std::byte{false}};
    REQUIRE(std::ranges::equal(false_bytes, io::to_bytes(false)));
  }

  SECTION("double") {
    auto bytes   = std::array<std::byte, sizeof(double)>{};
    const auto d = 1.35743452e+55;
    std::copy_n(reinterpret_cast<const std::byte*>(&d), sizeof(d), bytes.begin());

    REQUIRE(std::ranges::equal(bytes, io::to_bytes(d)));
  }

  SECTION("float") {
    auto bytes   = std::array<std::byte, sizeof(float)>{};
    const auto f = 1.3574e+10f;
    std::copy_n(reinterpret_cast<const std::byte*>(&f), sizeof(f), bytes.begin());

    REQUIRE(std::ranges::equal(bytes, io::to_bytes(f)));
  }
}

TEST_CASE("try_to_bytes", "[buffer_io]") {
  SECTION("successful conversion") {
    REQUIRE(io::try_to_bytes(int(999)).ok());
    REQUIRE(std::ranges::equal(io::static_byte_buffer<4>{std::byte{0x12}, std::byte{0x34}, std::byte{0x56}, std::byte{0x78}},
                               io::try_to_bytes(0x78563412).value()));
  }

  SECTION("Failing conversion") {
    REQUIRE(io::try_to_bytes<uint32_t, io::static_byte_buffer<3>>(0x12345678).is_error());
    REQUIRE(io::write_error::insufficient_buffer == io::try_to_bytes<uint32_t, io::static_byte_buffer<3>>(0x12345678).error());
  }
}
