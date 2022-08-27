#include <wite/io/bytes_buffer.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <sstream>
#include <vector>

using namespace wite;

TEST_CASE("Values from vector buffer", "[buffer_io]") {
  const auto vec_buffer = std::vector<std::byte>{std::byte{0x67},
                                                 std::byte{0x45},
                                                 std::byte{0x23},
                                                 std::byte{0x01},
                                                 std::byte{0xEF},
                                                 std::byte{0xCD},
                                                 std::byte{0xAB},
                                                 std::byte{0x89}};

  SECTION("Read int") {
    REQUIRE(uint32_t(0x01234567) == io::buffers::read<uint32_t>(vec_buffer));
  }

  SECTION("Read 2 shorts") {
    REQUIRE(uint32_t(0x4567) == io::buffers::read<uint16_t>(vec_buffer));
    REQUIRE(uint32_t(0x0123) == io::buffers::read<uint16_t>({std::next(vec_buffer.begin(), 2), vec_buffer.end()}));
  }

  SECTION("Read past the end of the buffer is an error") {
    const auto read_buf = std::span{std::next(vec_buffer.begin(), 6), vec_buffer.end()};
    REQUIRE_THROWS_AS(io::buffers::read<uint32_t>(read_buf), std::out_of_range);
  }
}

TEST_CASE("Values from pointers to array", "[buffer_io]") {
  constexpr auto buf_size               = 8;
  const std::byte data_buffer[buf_size] = {std::byte{0x67},
                                              std::byte{0x45},
                                              std::byte{0x23},
                                              std::byte{0x01},
                                              std::byte{0xEF},
                                              std::byte{0xCD},
                                              std::byte{0xAB},
                                              std::byte{0x89}};

  const std::byte* buf_begin = data_buffer;
  const std::byte* buf_end   = buf_begin + buf_size;
  
  SECTION("Read int") {
    REQUIRE(uint32_t(0x01234567) == io::buffers::read<uint32_t>({buf_begin, buf_end}));
  }

  SECTION("Read 2 shorts") {
    REQUIRE(uint32_t(0x4567) == io::buffers::read<uint16_t>({buf_begin, buf_end}));
    REQUIRE(uint32_t(0x0123) == io::buffers::read<uint16_t>({std::next(buf_begin, 2), buf_end}));
  }

  SECTION("Read past the end of the buffer is an error") {
    const auto read_buf = std::span{std::next(buf_begin, 6), buf_end};
    REQUIRE_THROWS_AS(io::buffers::read<uint32_t>(read_buf), std::out_of_range);
  }
}

TEST_CASE("Byte streams", "[buffer_io]") {
  SECTION("Read int") {
    std::stringstream stream("\x67\x45\x23\x01\xEF\xCD\xAB\x89");

    REQUIRE(uint32_t(0x01234567) == io::buffers::read<uint32_t>(stream));
    REQUIRE(uint32_t(0x89ABCDEF) == io::buffers::read<uint32_t>(stream));
  }

  SECTION("Read past the end of the buffer is an error") {
    std::stringstream stream("\xAB\x89\x67");

    REQUIRE_THROWS_AS(io::buffers::read<uint32_t>(stream), std::out_of_range);
  }
}