#include <wite/io/bytes_buffer.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstddef>
#include <iterator>
#include <sstream>
#include <vector>

using namespace wite;

// Helps the REQUIRE_THROW_AS macro to work in some cases.
#define COMMA ,

TEST_CASE("Values from vector buffer", "[buffer_io]") {
  const auto vec_buffer = std::vector<std::byte>{std::byte{0x67},
                                                 std::byte{0x45},
                                                 std::byte{0x23},
                                                 std::byte{0x01},
                                                 std::byte{0xEF},
                                                 std::byte{0xCD},
                                                 std::byte{0xAB},
                                                 std::byte{0x89}};

  SECTION("Little-endian") {
    SECTION("Read int") {
      REQUIRE(uint32_t(0x01234567) == io::buffers::read<uint32_t>(vec_buffer, std::endian::little));
      REQUIRE(uint32_t(0x01234567) == io::buffers::read<uint32_t, std::endian::little>(vec_buffer));
    }

    SECTION("Read 2 shorts") {
      REQUIRE(uint32_t(0x4567) == io::buffers::read<uint16_t>(vec_buffer, std::endian::little));
      REQUIRE(uint32_t(0x4567) == io::buffers::read<uint16_t, std::endian::little>(vec_buffer));
      REQUIRE(uint32_t(0x0123) ==
              io::buffers::read<uint16_t, std::endian::little>({std::next(vec_buffer.begin(), 2), vec_buffer.end()}));
    }

    SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
      const auto read_buf = std::span{std::next(vec_buffer.begin(), 6), vec_buffer.end()};
      REQUIRE_THROWS_AS(io::buffers::read<uint32_t>(read_buf, std::endian::little), std::out_of_range);
      REQUIRE_THROWS_AS(io::buffers::read<uint32_t COMMA std::endian::little>(read_buf), std::out_of_range);
    }
  }

  SECTION("Big-endian") {
    SECTION("Read int") {
      REQUIRE(uint32_t(0x67452301) == io::buffers::read<uint32_t>(vec_buffer, std::endian::big));
      REQUIRE(uint32_t(0x67452301) == io::buffers::read<uint32_t, std::endian::big>(vec_buffer));
    }

    SECTION("Read 2 shorts") {
      REQUIRE(uint32_t(0x6745) == io::buffers::read<uint16_t>(vec_buffer, std::endian::big));
      REQUIRE(uint32_t(0x6745) == io::buffers::read<uint16_t, std::endian::big>(vec_buffer));
      REQUIRE(uint32_t(0x2301) ==
              io::buffers::read<uint16_t>({std::next(vec_buffer.begin(), 2), vec_buffer.end()}, std::endian::big));
    }

    SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
      const auto read_buf = std::span{std::next(vec_buffer.begin(), 6), vec_buffer.end()};
      REQUIRE_THROWS_AS(io::buffers::read<uint32_t>(read_buf, std::endian::big), std::out_of_range);
      REQUIRE_THROWS_AS(io::buffers::read<uint32_t COMMA std::endian::big>(read_buf), std::out_of_range);
    }
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

  SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
    const auto read_buf = std::span{std::next(buf_begin, 6), buf_end};
    REQUIRE_THROWS_AS(io::buffers::read<uint32_t>(read_buf), std::out_of_range);
  }
}

TEST_CASE("Write values to byte arrays", "[buffer_io]") {
  auto array_buffer = std::array<std::byte, 10>{};

  SECTION("Little-endian") {
    SECTION("Write int at start of buffer") {
      io::buffers::write(array_buffer, 0x89ABCDEF, std::endian::little);

      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[0]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[1]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[3]));

      REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
    }

    SECTION("Write int not at start of buffer") {
      io::buffers::write({std::next(array_buffer.begin(), 2), array_buffer.end()}, 0x89ABCDEF, std::endian::little);

      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[3]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[4]));
      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[5]));

      REQUIRE(std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == std::byte{0}; }));
      REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
    }

    SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
      REQUIRE_THROWS_AS(io::buffers::write({std::next(array_buffer.begin(), 7), array_buffer.end()}, 0x89ABCDEF, std::endian::little),
                        std::out_of_range);
    }
  }

  SECTION("Big-endian") {
    SECTION("Write int at start of buffer") {
      io::buffers::write(array_buffer, 0x89ABCDEF, std::endian::big);

      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[0]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[1]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[3]));

      REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
    }

    SECTION("Write int not at start of buffer") {
      io::buffers::write({std::next(array_buffer.begin(), 2), array_buffer.end()}, 0x89ABCDEF, std::endian::big);

      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[3]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[4]));
      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[5]));

      REQUIRE(std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == std::byte{0}; }));
      REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
    }

    SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
      REQUIRE_THROWS_AS(io::buffers::write({std::next(array_buffer.begin(), 7), array_buffer.end()}, 0x89ABCDEF, std::endian::big),
                        std::out_of_range);
    }
  }
}

TEST_CASE("byte_read_buffer_view tests", "[buffer_io]") {
  const auto array_buffer = std::array{std::byte{0x67},
                                       std::byte{0x45},
                                       std::byte{0x23},
                                       std::byte{0x01},
                                       std::byte{0xEF},
                                       std::byte{0xCD},
                                       std::byte{0xAB},
                                       std::byte{0x89}};

  SECTION("Little-endian") {
    SECTION("Read int") {
      auto read_buf = io::buffers::byte_read_buffer_view(array_buffer);

      SECTION("Dynamic endianness") {
        REQUIRE(uint32_t(0x01234567) == io::buffers::read<uint32_t>(read_buf, std::endian::little));
        REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
      }

      SECTION("Static endianness") {
        REQUIRE(uint32_t(0x01234567) == io::buffers::read<uint32_t, std::endian::little>(read_buf));
        REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
      }

      SECTION("Default endianness") {
        REQUIRE(uint32_t(0x01234567) == io::buffers::read<uint32_t>(read_buf));
        REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
      }
    }

    SECTION("Read 2 shorts") {
      auto read_buf = io::buffers::byte_read_buffer_view(array_buffer);

      REQUIRE(uint32_t(0x4567) == io::buffers::read<uint16_t>(read_buf, std::endian::little));
      REQUIRE(uint32_t(0x0123) == io::buffers::read<uint16_t>(read_buf, std::endian::little));
    }

    SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
      auto read_buf = io::buffers::byte_read_buffer_view(array_buffer, 6);

      REQUIRE_THROWS_AS(io::buffers::read<uint32_t>(read_buf, std::endian::little), std::out_of_range);
    }
  }

  SECTION("Big-endian") {
    SECTION("Read int") {
      auto read_buf = io::buffers::byte_read_buffer_view(array_buffer);

      REQUIRE(uint32_t(0x67452301) == io::buffers::read<uint32_t>(read_buf, std::endian::big));
      REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
    }

    SECTION("Read 2 shorts") {
      auto read_buf = io::buffers::byte_read_buffer_view(array_buffer);

      REQUIRE(uint32_t(0x6745) == io::buffers::read<uint16_t>(read_buf, std::endian::big));
      REQUIRE(uint32_t(0x2301) == io::buffers::read<uint16_t>(read_buf, std::endian::big));
    }

    SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
      auto read_buf = io::buffers::byte_read_buffer_view(array_buffer, 6);

      REQUIRE_THROWS_AS(io::buffers::read<uint32_t>(read_buf, std::endian::big), std::out_of_range);
    }
  }
}

TEST_CASE("byte_write_buffer_view tests", "[buffer_io]") {
  auto array_buffer = std::array<std::byte, 10>{};

  SECTION("Little-endian") {
    SECTION("Write int at start of buffer") {
      auto write_buffer = io::buffers::byte_write_buffer_view{array_buffer};
      io::buffers::write(write_buffer, 0x89ABCDEF, std::endian::little);
      REQUIRE(std::next(write_buffer.data.begin(), 4) == write_buffer.write_position);

      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[0]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[1]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[3]));

      REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));

      SECTION("and then another write to the buffer") {
        io::buffers::write(write_buffer, 0x01234567, std::endian::little);
        REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);

        REQUIRE(0x67 == std::to_integer<uint8_t>(array_buffer[4]));
        REQUIRE(0x45 == std::to_integer<uint8_t>(array_buffer[5]));
        REQUIRE(0x23 == std::to_integer<uint8_t>(array_buffer[6]));
        REQUIRE(0x01 == std::to_integer<uint8_t>(array_buffer[7]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));

        SECTION("and then another write throws std::out_of_range") {
          REQUIRE_THROWS_AS(io::buffers::write(write_buffer, 0x01234567, std::endian::little), std::out_of_range);

          SECTION("and the buffer is not written to") {
            REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);
            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
          }
        }
      }
    }
  }

  SECTION("Big-endian") {
    SECTION("Write int at start of buffer") {
      auto write_buffer = io::buffers::byte_write_buffer_view{array_buffer};
      io::buffers::write(write_buffer, 0x89ABCDEF, std::endian::big);
      REQUIRE(std::next(write_buffer.data.begin(), 4) == write_buffer.write_position);

      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[0]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[1]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[3]));

      REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));

      SECTION("and then another write to the buffer") {
        io::buffers::write(write_buffer, 0x01234567, std::endian::big);
        REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);

        REQUIRE(0x01 == std::to_integer<uint8_t>(array_buffer[4]));
        REQUIRE(0x23 == std::to_integer<uint8_t>(array_buffer[5]));
        REQUIRE(0x45 == std::to_integer<uint8_t>(array_buffer[6]));
        REQUIRE(0x67 == std::to_integer<uint8_t>(array_buffer[7]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));

        SECTION("and then another write throws std::out_of_range") {
          REQUIRE_THROWS_AS(io::buffers::write(write_buffer, 0x01234567, std::endian::big), std::out_of_range);

          SECTION("and the buffer is not written to") {
            REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);
            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
          }
        }
      }
    }
  }
}

TEST_CASE("Byte streams", "[buffer_io]") {
  SECTION("Read int") {
    std::stringstream stream("\x67\x45\x23\x01\xEF\xCD\xAB\x89");

    REQUIRE(uint32_t(0x01234567) == io::buffers::read<uint32_t>(stream));
    REQUIRE(uint32_t(0x89ABCDEF) == io::buffers::read<uint32_t>(stream));
  }

  SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
    std::stringstream stream("\xAB\x89\x67");

    REQUIRE_THROWS_AS(io::buffers::read<uint32_t>(stream), std::out_of_range);
  }
}