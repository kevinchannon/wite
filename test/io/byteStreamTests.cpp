#include <wite/io/byte_stream.hpp>
#include <wite/io/byte_buffer.hpp>  // This is here to make sure that things build in eachothers presence.

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <sstream>

using namespace wite;

TEST_CASE("Byte streams read", "[buffer_io]") {
  SECTION("Read int") {
    std::stringstream stream("\x67\x45\x23\x01\xEF\xCD\xAB\x89");

    REQUIRE(uint32_t(0x01234567) == io::read<uint32_t>(stream));
    REQUIRE(uint32_t(0x89ABCDEF) == io::read<uint32_t>(stream));
  }
}

TEST_CASE("Byte streams write tests", "[buffer_io]") {
  SECTION("Write int") {
    std::stringstream stream{};

    io::write(stream, uint32_t{0xACEACE99});

    REQUIRE("\x99\xCE\xEA\xAC" == stream.str());
  }
}

TEST_CASE("Byte streams write-read tests", "[buffer_io]") {
  std::stringstream stream;

  SECTION("Double value") {
    const auto val = 2.718;
    io::write(stream, val);
    
    REQUIRE(val == io::read<double>(stream));
  }

  SECTION("Uint32 value") {
    const auto val = uint32_t{0xCDCDCDCD};
    io::write(stream, val);

    REQUIRE(val == io::read<uint32_t>(stream));
  }

  SECTION("Bool value") {
    const auto val = GENERATE(true, false);
    io::write(stream, val);

    REQUIRE(val == io::read<bool>(stream));
  }
}

