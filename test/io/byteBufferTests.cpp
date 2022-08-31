#include <wite/io/byte_buffer.hpp>
#include <wite/io/byte_stream.hpp>  // This is here to make sure that things build in eachothers presence.
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>

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
      REQUIRE(uint32_t(0x01234567) == io::read<uint32_t>(vec_buffer, std::endian::little));
      REQUIRE(uint32_t(0x01234567) == io::read<uint32_t, std::endian::little>(vec_buffer));
    }

    SECTION("Read 2 shorts") {
      REQUIRE(uint32_t(0x4567) == io::read<uint16_t>(vec_buffer, std::endian::little));
      REQUIRE(uint32_t(0x4567) == io::read<uint16_t, std::endian::little>(vec_buffer));
      REQUIRE(uint32_t(0x0123) == io::read<uint16_t, std::endian::little>({std::next(vec_buffer.begin(), 2), vec_buffer.end()}));
    }

    SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
      const auto read_buf = std::span{std::next(vec_buffer.begin(), 6), vec_buffer.end()};
      REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf, std::endian::little), std::out_of_range);
      REQUIRE_THROWS_AS(io::read<uint32_t COMMA std::endian::little>(read_buf), std::out_of_range);
    }
  }

  SECTION("Big-endian") {
    SECTION("Read int") {
      REQUIRE(uint32_t(0x67452301) == io::read<uint32_t>(vec_buffer, std::endian::big));
      REQUIRE(uint32_t(0x67452301) == io::read<uint32_t, std::endian::big>(vec_buffer));
    }

    SECTION("Read 2 shorts") {
      REQUIRE(uint32_t(0x6745) == io::read<uint16_t>(vec_buffer, std::endian::big));
      REQUIRE(uint32_t(0x6745) == io::read<uint16_t, std::endian::big>(vec_buffer));
      REQUIRE(uint32_t(0x2301) == io::read<uint16_t>({std::next(vec_buffer.begin(), 2), vec_buffer.end()}, std::endian::big));
    }

    SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
      const auto read_buf = std::span{std::next(vec_buffer.begin(), 6), vec_buffer.end()};
      REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf, std::endian::big), std::out_of_range);
      REQUIRE_THROWS_AS(io::read<uint32_t COMMA std::endian::big>(read_buf), std::out_of_range);
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
    REQUIRE(uint32_t(0x01234567) == io::read<uint32_t>({buf_begin, buf_end}));
  }

  SECTION("Read 2 shorts") {
    REQUIRE(uint32_t(0x4567) == io::read<uint16_t>({buf_begin, buf_end}));
    REQUIRE(uint32_t(0x0123) == io::read<uint16_t>({std::next(buf_begin, 2), buf_end}));
  }

  SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
    const auto read_buf = std::span{std::next(buf_begin, 6), buf_end};
    REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf), std::out_of_range);
  }
}

TEST_CASE("Write values to byte arrays", "[buffer_io]") {
  auto array_buffer = std::array<std::byte, 10>{};

  SECTION("Little-endian") {
    SECTION("Write int at start of buffer") {
      SECTION("Dynamic endianness") {
        io::write(array_buffer, 0x89ABCDEF, std::endian::little);

        REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[0]));
        REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[1]));
        REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[2]));
        REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[3]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
      }

      SECTION("Static endianness") {
        io::write<std::endian::little>(array_buffer, 0x89ABCDEF);

        REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[0]));
        REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[1]));
        REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[2]));
        REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[3]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
      }

      SECTION("Default endianness") {
        io::write(array_buffer, 0x89ABCDEF);

        REQUIRE((std::endian::native == std::endian::little ? 0xEF : 0x89) == std::to_integer<uint8_t>(array_buffer[0]));
        REQUIRE((std::endian::native == std::endian::little ? 0xCD : 0xAB) == std::to_integer<uint8_t>(array_buffer[1]));
        REQUIRE((std::endian::native == std::endian::little ? 0xAB : 0xCD) == std::to_integer<uint8_t>(array_buffer[2]));
        REQUIRE((std::endian::native == std::endian::little ? 0x89 : 0xEF) == std::to_integer<uint8_t>(array_buffer[3]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
      }
    }

    SECTION("Write int not at start of buffer") {
      io::write({std::next(array_buffer.begin(), 2), array_buffer.end()}, 0x89ABCDEF, std::endian::little);

      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[3]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[4]));
      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[5]));

      REQUIRE(std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == std::byte{0}; }));
      REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
    }

    SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
      REQUIRE_THROWS_AS(io::write({std::next(array_buffer.begin(), 7), array_buffer.end()}, 0x89ABCDEF, std::endian::little),
                        std::out_of_range);
    }
  }

  SECTION("Big-endian") {
    SECTION("Write int at start of buffer") {
      io::write(array_buffer, 0x89ABCDEF, std::endian::big);

      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[0]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[1]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[3]));

      REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
    }

    SECTION("Write int not at start of buffer") {
      io::write({std::next(array_buffer.begin(), 2), array_buffer.end()}, 0x89ABCDEF, std::endian::big);

      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[3]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[4]));
      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[5]));

      REQUIRE(std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == std::byte{0}; }));
      REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
    }

    SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
      REQUIRE_THROWS_AS(io::write({std::next(array_buffer.begin(), 7), array_buffer.end()}, 0x89ABCDEF, std::endian::big),
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
      auto read_buf = io::byte_read_buffer_view(array_buffer);

      SECTION("Dynamic endianness") {
        REQUIRE(uint32_t(0x01234567) == io::read<uint32_t>(read_buf, std::endian::little));
        REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
      }

      SECTION("Static endianness") {
        REQUIRE(uint32_t(0x01234567) == io::read<uint32_t, std::endian::little>(read_buf));
        REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
      }

      SECTION("Default endianness") {
        REQUIRE(uint32_t(0x01234567) == io::read<uint32_t>(read_buf));
        REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
      }
    }

    SECTION("Read 2 shorts") {
      auto read_buf = io::byte_read_buffer_view(array_buffer);

      REQUIRE(uint32_t(0x4567) == io::read<uint16_t>(read_buf, std::endian::little));
      REQUIRE(uint32_t(0x0123) == io::read<uint16_t>(read_buf, std::endian::little));
    }

    SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
      auto read_buf = io::byte_read_buffer_view(array_buffer, 6);

      REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf, std::endian::little), std::out_of_range);
    }
  }

  SECTION("Big-endian") {
    SECTION("Read int") {
      auto read_buf = io::byte_read_buffer_view(array_buffer);
      SECTION("Dynmic endianness") {
        REQUIRE(uint32_t(0x67452301) == io::read<uint32_t>(read_buf, std::endian::big));
        REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
      }

      SECTION("Static endianness") {
        REQUIRE(uint32_t(0x67452301) == io::read<uint32_t, std::endian::big>(read_buf));
        REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
      }
    }

    SECTION("Read 2 shorts") {
      auto read_buf = io::byte_read_buffer_view(array_buffer);

      REQUIRE(uint32_t(0x6745) == io::read<uint16_t>(read_buf, std::endian::big));
      REQUIRE(uint32_t(0x2301) == io::read<uint16_t>(read_buf, std::endian::big));
    }

    SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
      auto read_buf = io::byte_read_buffer_view(array_buffer, 6);

      REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf, std::endian::big), std::out_of_range);
    }
  }
}

TEST_CASE("byte_write_buffer_view tests", "[buffer_io]") {
  SECTION("Little-endian") {
    auto array_buffer = std::array<std::byte, 14>{};

    SECTION("Write int at start of buffer (dynamic endianness)") {
      auto write_buffer = io::byte_write_buffer_view{array_buffer};
      io::write(write_buffer, 0x89ABCDEF, std::endian::little);
      REQUIRE(std::next(write_buffer.data.begin(), 4) == write_buffer.write_position);

      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[0]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[1]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[3]));

      REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));

      SECTION("and then another write to the buffer (static endianness)") {
        io::write<std::endian::little>(write_buffer, 0x01234567);
        REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);

        REQUIRE(0x67 == std::to_integer<uint8_t>(array_buffer[4]));
        REQUIRE(0x45 == std::to_integer<uint8_t>(array_buffer[5]));
        REQUIRE(0x23 == std::to_integer<uint8_t>(array_buffer[6]));
        REQUIRE(0x01 == std::to_integer<uint8_t>(array_buffer[7]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));

        SECTION("and then another write to the buffer (default endianness)") {
          io::write(write_buffer, 0x463235F9);
          REQUIRE(std::next(write_buffer.data.begin(), 12) == write_buffer.write_position);

          REQUIRE((std::endian::native == std::endian::little ? 0xF9 : 0x46) == std::to_integer<uint8_t>(array_buffer[8]));
          REQUIRE((std::endian::native == std::endian::little ? 0x35 : 0x32) == std::to_integer<uint8_t>(array_buffer[9]));
          REQUIRE((std::endian::native == std::endian::little ? 0x32 : 0x35) == std::to_integer<uint8_t>(array_buffer[10]));
          REQUIRE((std::endian::native == std::endian::little ? 0x46 : 0xF9) == std::to_integer<uint8_t>(array_buffer[11]));

          REQUIRE(
              std::all_of(std::next(array_buffer.begin(), 12), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));

          SECTION("and then another write throws std::out_of_range") {
            REQUIRE_THROWS_AS(io::write(write_buffer, 0x01234567, std::endian::little), std::out_of_range);

            SECTION("and the buffer is not written to") {
              REQUIRE(std::next(write_buffer.data.begin(), 12) == write_buffer.write_position);
              REQUIRE(std::all_of(
                  std::next(array_buffer.begin(), 12), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
            }
          }
        }
      }
    }
  }

  SECTION("Big-endian") {
    auto array_buffer = std::array<std::byte, 10>{};

    SECTION("Write int at start of buffer (dynamic endianness)") {
      auto write_buffer = io::byte_write_buffer_view{array_buffer};
      io::write(write_buffer, 0x89ABCDEF, std::endian::big);
      REQUIRE(std::next(write_buffer.data.begin(), 4) == write_buffer.write_position);

      REQUIRE(0x89 == std::to_integer<uint8_t>(array_buffer[0]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(array_buffer[1]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xEF == std::to_integer<uint8_t>(array_buffer[3]));

      REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));

      SECTION("and then another write to the buffer (static endianness)") {
        io::write<std::endian::big>(write_buffer, 0x01234567);
        REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);

        REQUIRE(0x01 == std::to_integer<uint8_t>(array_buffer[4]));
        REQUIRE(0x23 == std::to_integer<uint8_t>(array_buffer[5]));
        REQUIRE(0x45 == std::to_integer<uint8_t>(array_buffer[6]));
        REQUIRE(0x67 == std::to_integer<uint8_t>(array_buffer[7]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));

        SECTION("and then another write throws std::out_of_range") {
          REQUIRE_THROWS_AS(io::write(write_buffer, 0x01234567, std::endian::big), std::out_of_range);

          SECTION("and the buffer is not written to") {
            REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);
            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == std::byte{0}; }));
          }
        }
      }
    }
  }

  SECTION("Endian adapter interface") {
    auto raw_buffer   = io::stack_byte_buffer<10>{};
    auto write_buffer = io::byte_write_buffer_view{raw_buffer};
    auto read_buffer  = io::byte_read_buffer_view{raw_buffer};

    const auto val_1 = uint32_t{0x01234567};
    io::write(write_buffer, io::little_endian{val_1});

    const auto val_2 = uint32_t{0x89ABCDEF};
    io::write(write_buffer, io::big_endian{val_2});

    const auto val_3 = int16_t{0x7D04};
    io::write(write_buffer, val_3);

    REQUIRE(std::ranges::equal(io::stack_byte_buffer<10>{std::byte{0x67},
                                                         std::byte{0x45},
                                                         std::byte{0x23},
                                                         std::byte{0x01},
                                                         std::byte{0x89},
                                                         std::byte{0xAB},
                                                         std::byte{0xCD},
                                                         std::byte{0xEF},
                                                         std::byte{0x04},
                                                         std::byte{0x7D}},
                               raw_buffer));

    REQUIRE(val_1 == io::read<uint32_t>(read_buffer));
    REQUIRE(val_2 == io::read<io::big_endian<uint32_t>>(read_buffer));
    REQUIRE(val_3 == io::read<io::little_endian<int16_t>>(read_buffer));
  }
}

TEST_CASE("Byte buffers write-read tests", "[buffer_io]") {
  auto buffer = io::stack_byte_buffer<32>{};

  SECTION("Double value") {
    const auto val = 2.718;
    io::write(buffer, val);

    REQUIRE(val == io::read<double>(buffer));
  }

  SECTION("Uint32 value") {
    const auto val = uint32_t{0xCDCDCDCD};
    io::write(buffer, val);

    REQUIRE(val == io::read<uint32_t>(buffer));
  }

  SECTION("Bool value") {
    const auto val = GENERATE(true, false);
    io::write(buffer, val);

    REQUIRE(val == io::read<bool>(buffer));
  }
}

TEST_CASE("Write-read using encodings", "[buffer_io]") {
  auto buffer = io::stack_byte_buffer<32>{};

  SECTION("Uint64") {
    const auto val = uint64_t{0x0011223344556677};

    SECTION("Little-endian") {
      io::write(buffer, io::little_endian{val});
      REQUIRE(0x77 == std::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x66 == std::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x55 == std::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x44 == std::to_integer<uint8_t>(buffer[3]));
      REQUIRE(0x33 == std::to_integer<uint8_t>(buffer[4]));
      REQUIRE(0x22 == std::to_integer<uint8_t>(buffer[5]));
      REQUIRE(0x11 == std::to_integer<uint8_t>(buffer[6]));
      REQUIRE(0x00 == std::to_integer<uint8_t>(buffer[7]));

      REQUIRE(val == io::read<io::little_endian<uint64_t>>(buffer));
    }

    SECTION("Big-endian") {
      io::write(buffer, io::big_endian{val});
      REQUIRE(0x00 == std::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x11 == std::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x22 == std::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x33 == std::to_integer<uint8_t>(buffer[3]));
      REQUIRE(0x44 == std::to_integer<uint8_t>(buffer[4]));
      REQUIRE(0x55 == std::to_integer<uint8_t>(buffer[5]));
      REQUIRE(0x66 == std::to_integer<uint8_t>(buffer[6]));
      REQUIRE(0x77 == std::to_integer<uint8_t>(buffer[7]));

      REQUIRE(val == io::read<io::big_endian<uint64_t>>(buffer));
    }
  }

  SECTION("Uint32") {
    const auto val = uint32_t{0x01234567};

    SECTION("Little-endian") {
      io::write(buffer, io::little_endian{val});
      REQUIRE(0x67 == std::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x45 == std::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x23 == std::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x01 == std::to_integer<uint8_t>(buffer[3]));

      REQUIRE(val == io::read<io::little_endian<uint32_t>>(buffer));
    }

    SECTION("Big-endian") {
      io::write(buffer, io::big_endian{val});
      REQUIRE(0x01 == std::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x23 == std::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x45 == std::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x67 == std::to_integer<uint8_t>(buffer[3]));

      REQUIRE(val == io::read<io::big_endian<uint32_t>>(buffer));
    }
  }

  SECTION("Uint16") {
    const auto val = uint16_t{0xABCD};

    SECTION("Little-endian") {
      io::write(buffer, io::little_endian{val});
      REQUIRE(0xCD == std::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0xAB == std::to_integer<uint8_t>(buffer[1]));

      REQUIRE(val == io::read<io::little_endian<uint16_t>>(buffer));
    }

    SECTION("Big-endian") {
      io::write(buffer, io::big_endian{val});
      REQUIRE(0xAB == std::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0xCD == std::to_integer<uint8_t>(buffer[1]));

      REQUIRE(val == io::read<io::big_endian<uint16_t>>(buffer));
    }
  }
}

TEST_CASE("try_read returns value on good read") {
  SECTION("From raw buffer") {
    const auto data = io::stack_byte_buffer<4>{std::byte{0x67}, std::byte{0x45}, std::byte{0xAB}, std::byte{0xFF}};

    SECTION("with default endianness") {
      const auto val = io::try_read<uint32_t>(data);
      REQUIRE(val.ok());
      REQUIRE(uint32_t{0xFFAB4567} == val.value());
    }

    SECTION("with specified endianness") {
      const auto val = io::try_read<io::big_endian<uint32_t>>(data);
      REQUIRE(val.ok());
      REQUIRE(uint32_t{0x6745ABFF} == val.value());
    }
  }

  SECTION("Via byte_read_buffer_view") {
    const auto data = io::stack_byte_buffer<4>{std::byte{0x67}, std::byte{0x45}, std::byte{0xAB}, std::byte{0xFF}};
    auto buffer     = io::byte_read_buffer_view{data};

    SECTION("with default endianness") {
      const auto val = io::try_read<uint32_t>(buffer);
      REQUIRE(val.ok());
      REQUIRE(uint32_t{0xFFAB4567} == val.value());
      REQUIRE(std::next(buffer.data.begin(), 4) == buffer.read_position);
    }

    SECTION("with specified endianness") {
      const auto val = io::try_read<io::big_endian<uint32_t>>(buffer);
      REQUIRE(val.ok());
      REQUIRE(uint32_t{0x6745ABFF} == val.value());
      REQUIRE(std::next(buffer.data.begin(), 4) == buffer.read_position);
    }
  }
}

TEST_CASE("try_read returns error on bad read") {
  const auto data = io::stack_byte_buffer<3>{std::byte{0x67}, std::byte{0x45}, std::byte{0xAB}};

  const auto val = io::try_read<uint32_t>(data);
  REQUIRE(val.is_error());
  REQUIRE(io::read_error::insufficient_buffer == val.error());
}

TEST_CASE("try_write returns true on good write") {
  SECTION("To raw buffer") {
    auto data = io::stack_byte_buffer<4>{};

    SECTION("with default endianness") {
      const auto val = uint32_t{0xFE01CD23};

      REQUIRE(io::try_write(data, val).ok());
      REQUIRE(
          std::ranges::equal(io::stack_byte_buffer<4>{std::byte{0x23}, std::byte{0xCD}, std::byte{0x01}, std::byte{0xFE}}, data));
    }

    SECTION("with specified endianness") {
      const auto val = uint32_t{0x23CD01FE};

      REQUIRE(io::try_write(data, io::big_endian{val}).ok());
      REQUIRE(
          std::ranges::equal(io::stack_byte_buffer<4>{std::byte{0x23}, std::byte{0xCD}, std::byte{0x01}, std::byte{0xFE}}, data));
    }
  }
}

TEST_CASE("try_write returns error on bad write") {
  auto data = io::stack_byte_buffer<3>{};

  const auto result = io::try_write(data, uint32_t{0xCDCDCDCD});
  REQUIRE(result.is_error());
  REQUIRE(io::write_error::insufficient_buffer == result.error());
}
