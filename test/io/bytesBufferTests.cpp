#include <wite/io/bytes_buffer.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

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

}

TEST_CASE("Byte streams", "[buffer_io]") {
  SECTION("Read int") {
    std::stringstream stream("\x67\x45\x23\x01\xEF\xCD\xAB\x89");

    REQUIRE(uint32_t(0x01234567) == io::buffers::read<uint32_t>(stream));
    REQUIRE(uint32_t(0x89ABCDEF) == io::buffers::read<uint32_t>(stream));
  }
}