#include <wite/io/bytes_buffer.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <cstddef>
#include <iterator>
#include <sstream>
#include <vector>

using namespace wite;

TEST_CASE("Values from vector buffer", "[buffer_io]") {
  const auto vec_buffer = std::vector<std::byte>{std::byte{0x78},
                                                 std::byte{0x56},
                                                 std::byte{0x34},
                                                 std::byte{0x12},
                                                 std::byte{0xEF},
                                                 std::byte{0xCD},
                                                 std::byte{0xAB},
                                                 std::byte{0x89}};

  SECTION("Read int") {
    REQUIRE(uint32_t(0x12345678) == io::buffers::read<uint32_t>(vec_buffer));
  }

  SECTION("Read 2 shorts") {
    REQUIRE(uint32_t(0x5678) == io::buffers::read<uint16_t>(vec_buffer));
    REQUIRE(uint32_t(0x1234) == io::buffers::read<uint16_t>({std::next(vec_buffer.begin(), 2), vec_buffer.end()}));
  }
}

TEST_CASE("Byte streams", "[buffer_io]") {
  SECTION("Read int") {
    std::stringstream stream("\x78\x56\x34\x12");

    REQUIRE(uint32_t(0x12345678) == io::buffers::read<uint32_t>(stream));
  }
}