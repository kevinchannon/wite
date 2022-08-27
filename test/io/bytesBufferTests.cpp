#include <wite/io/bytes_buffer.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <vector>
#include <cstddef>
#include <sstream>
#include <iterator>

using namespace wite;

TEST_CASE("Bytes buffer to values", "[bytes_buffer]") {
  SECTION("Read int") {
    const auto buffer = std::vector<std::byte>{std::byte{0x78}, std::byte{0x56}, std::byte{0x34}, std::byte{0x12}};
    
    REQUIRE(uint32_t(0x12345678) == io::buffers::read<uint32_t>(buffer));
  }
}

TEST_CASE("Byte streams") {
  SECTION("Read int") {
    std::stringstream stream("\x78\x56\x34\x12");

    REQUIRE(uint32_t(0x12345678) == io::buffers::read<uint32_t>(stream));
  }
}