#include <wite/io/byte_buffer.hpp>
#include <wite/io/byte_stream.hpp>  // This is here to make sure that things build in eachothers presence.
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>


#include "../utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <sstream>
#include <vector>

using namespace wite;

#ifndef _WITE_COMPILER_MSVC
namespace std {
template <typename T>
initializer_list(const std::initializer_list<T>&) -> initializer_list<T>;
}
#endif

TEST_CASE("Write values to byte arrays", "[buffer_io]") {
  auto array_buffer = io::static_byte_buffer<10>{};

  SECTION("Little-endian") {
    SECTION("Write int at start of buffer") {
      SECTION("Dynamic endianness") {
        REQUIRE(sizeof(uint32_t) == io::write(array_buffer, 0x89ABCDEF, io::endian::little));

        REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[0]));
        REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[1]));
        REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[2]));
        REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[3]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
      }

      SECTION("Static endianness") {
        REQUIRE(sizeof(uint32_t) == io::write(array_buffer, io::little_endian{0x89ABCDEF}));

        REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[0]));
        REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[1]));
        REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[2]));
        REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[3]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
      }

      SECTION("Default endianness") {
        REQUIRE(sizeof(uint32_t) == io::write(array_buffer, 0x89ABCDEF));

        REQUIRE((io::endian::native == io::endian::little ? 0xEF : 0x89) == io::to_integer<uint8_t>(array_buffer[0]));
        REQUIRE((io::endian::native == io::endian::little ? 0xCD : 0xAB) == io::to_integer<uint8_t>(array_buffer[1]));
        REQUIRE((io::endian::native == io::endian::little ? 0xAB : 0xCD) == io::to_integer<uint8_t>(array_buffer[2]));
        REQUIRE((io::endian::native == io::endian::little ? 0x89 : 0xEF) == io::to_integer<uint8_t>(array_buffer[3]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
      }
    }

    SECTION("Write int not at start of buffer") {
      REQUIRE(sizeof(uint32_t) == io::write({std::next(array_buffer.begin(), 2), array_buffer.end()}, 0x89ABCDEF, io::endian::little));

      REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[3]));
      REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[4]));
      REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[5]));

      REQUIRE(std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == io::byte{0}; }));
      REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
    }

    SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
      REQUIRE_THROWS_AS(io::write({std::next(array_buffer.begin(), 7), array_buffer.end()}, 0x89ABCDEF, io::endian::little),
                        std::out_of_range);
    }
  }

  SECTION("Big-endian") {
    SECTION("Write int at start of buffer") {
      REQUIRE(sizeof(uint32_t) == io::write(array_buffer, 0x89ABCDEF, io::endian::big));

      REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[0]));
      REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[1]));
      REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[3]));

      REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
    }

    SECTION("Write int not at start of buffer") {
      REQUIRE(sizeof(uint32_t) == io::write({std::next(array_buffer.begin(), 2), array_buffer.end()}, 0x89ABCDEF, io::endian::big));

      REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[3]));
      REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[4]));
      REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[5]));

      REQUIRE(std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == io::byte{0}; }));
      REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
    }

    SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
      REQUIRE_THROWS_AS(io::write({std::next(array_buffer.begin(), 7), array_buffer.end()}, 0x89ABCDEF, io::endian::big),
                        std::out_of_range);
    }
  }
}

TEST_CASE("byte_write_buffer_view tests", "[buffer_io]") {
  SECTION("Little-endian") {
    auto array_buffer = std::array<io::byte, 14>{};

    SECTION("Write int at start of buffer (dynamic endianness)") {
      auto write_buffer = io::byte_write_buffer_view{array_buffer};
      REQUIRE(sizeof(uint32_t) == io::write(write_buffer, 0x89ABCDEF, io::endian::little));
      REQUIRE(std::next(write_buffer.data.begin(), 4) == write_buffer.write_position);

      REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[0]));
      REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[1]));
      REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[3]));

      REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

      SECTION("and then another write to the buffer (static endianness)") {
        REQUIRE(sizeof(uint32_t) == io::write(write_buffer, io::little_endian{0x01234567}));
        REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);

        REQUIRE(0x67 == io::to_integer<uint8_t>(array_buffer[4]));
        REQUIRE(0x45 == io::to_integer<uint8_t>(array_buffer[5]));
        REQUIRE(0x23 == io::to_integer<uint8_t>(array_buffer[6]));
        REQUIRE(0x01 == io::to_integer<uint8_t>(array_buffer[7]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

        SECTION("and then another write to the buffer (default endianness)") {
          REQUIRE(sizeof(uint32_t) == io::write(write_buffer, 0x463235F9));
          REQUIRE(std::next(write_buffer.data.begin(), 12) == write_buffer.write_position);

          REQUIRE((io::endian::native == io::endian::little ? 0xF9 : 0x46) == io::to_integer<uint8_t>(array_buffer[8]));
          REQUIRE((io::endian::native == io::endian::little ? 0x35 : 0x32) == io::to_integer<uint8_t>(array_buffer[9]));
          REQUIRE((io::endian::native == io::endian::little ? 0x32 : 0x35) == io::to_integer<uint8_t>(array_buffer[10]));
          REQUIRE((io::endian::native == io::endian::little ? 0x46 : 0xF9) == io::to_integer<uint8_t>(array_buffer[11]));

          REQUIRE(
              std::all_of(std::next(array_buffer.begin(), 12), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

          SECTION("and then another write throws std::out_of_range") {
            REQUIRE_THROWS_AS(io::write(write_buffer, 0x01234567, io::endian::little), std::out_of_range);

            SECTION("and the buffer is not written to") {
              REQUIRE(std::next(write_buffer.data.begin(), 12) == write_buffer.write_position);
              REQUIRE(std::all_of(
                  std::next(array_buffer.begin(), 12), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
            }
          }
        }
      }
    }
  }

  SECTION("Big-endian") {
    auto array_buffer = std::array<io::byte, 10>{};

    SECTION("Write int at start of buffer (dynamic endianness)") {
      auto write_buffer = io::byte_write_buffer_view{array_buffer};
      REQUIRE(sizeof(uint32_t) == io::write(write_buffer, 0x89ABCDEF, io::endian::big));
      REQUIRE(std::next(write_buffer.data.begin(), 4) == write_buffer.write_position);

      REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[0]));
      REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[1]));
      REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[2]));
      REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[3]));

      REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

      SECTION("and then another write to the buffer (static endianness)") {
        REQUIRE(sizeof(uint32_t) == io::write(write_buffer, io::big_endian{0x01234567}));
        REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);

        REQUIRE(0x01 == io::to_integer<uint8_t>(array_buffer[4]));
        REQUIRE(0x23 == io::to_integer<uint8_t>(array_buffer[5]));
        REQUIRE(0x45 == io::to_integer<uint8_t>(array_buffer[6]));
        REQUIRE(0x67 == io::to_integer<uint8_t>(array_buffer[7]));

        REQUIRE(std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

        SECTION("and then another write throws std::out_of_range") {
          REQUIRE_THROWS_AS(io::write(write_buffer, 0x01234567, io::endian::big), std::out_of_range);

          SECTION("and the buffer is not written to") {
            REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);
            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
          }
        }
      }
    }
  }

  SECTION("Endian adapter interface") {
    auto raw_buffer   = io::static_byte_buffer<10>{};
    auto write_buffer = io::byte_write_buffer_view{raw_buffer};
    auto read_buffer  = io::byte_read_buffer_view{raw_buffer};

    const auto val_1 = uint32_t{0x01234567};
    REQUIRE(sizeof(uint32_t) == io::write(write_buffer, io::little_endian{val_1}));

    const auto val_2 = uint32_t{0x89ABCDEF};
    REQUIRE(sizeof(uint32_t) == io::write(write_buffer, io::big_endian{val_2}));

    const auto val_3 = int16_t{0x7D04};
    REQUIRE(sizeof(uint16_t) == io::write(write_buffer, val_3));

    REQUIRE(std::ranges::equal(io::static_byte_buffer<10>{io::byte{0x67},
                                                          io::byte{0x45},
                                                          io::byte{0x23},
                                                          io::byte{0x01},
                                                          io::byte{0x89},
                                                          io::byte{0xAB},
                                                          io::byte{0xCD},
                                                          io::byte{0xEF},
                                                          io::byte{0x04},
                                                          io::byte{0x7D}},
                               raw_buffer));

    REQUIRE(val_1 == io::read<uint32_t>(read_buffer));
    REQUIRE(val_2 == io::read<io::big_endian<uint32_t>>(read_buffer));
    REQUIRE(val_3 == io::read<io::little_endian<int16_t>>(read_buffer));
  }
}

TEST_CASE("Byte buffers write-read tests", "[buffer_io]") {
  auto buffer = io::static_byte_buffer<32>{};

  SECTION("Double value") {
    const auto val = 2.718;

    REQUIRE(sizeof(double) == io::write(buffer, val));
    REQUIRE(val == io::read<double>(buffer));
  }

  SECTION("Uint32 value") {
    const auto val = uint32_t{0xCDCDCDCD};

    REQUIRE(sizeof(uint32_t) == io::write(buffer, val));
    REQUIRE(val == io::read<uint32_t>(buffer));
  }

  SECTION("Bool value") {
    const auto val = GENERATE(true, false);

    REQUIRE(sizeof(bool) == io::write(buffer, val));
    REQUIRE(val == io::read<bool>(buffer));
  }
}

TEST_CASE("Write-read using encodings", "[buffer_io]") {
  auto buffer = io::static_byte_buffer<32>{};

  SECTION("Uint64") {
    const auto val = uint64_t{0x0011223344556677};

    SECTION("Little-endian") {
      REQUIRE(sizeof(uint64_t) == io::write(buffer, io::little_endian{val}));
      REQUIRE(0x77 == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x66 == io::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x55 == io::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x44 == io::to_integer<uint8_t>(buffer[3]));
      REQUIRE(0x33 == io::to_integer<uint8_t>(buffer[4]));
      REQUIRE(0x22 == io::to_integer<uint8_t>(buffer[5]));
      REQUIRE(0x11 == io::to_integer<uint8_t>(buffer[6]));
      REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[7]));

      REQUIRE(val == io::read<io::little_endian<uint64_t>>(buffer));
    }

    SECTION("Big-endian") {
      REQUIRE(sizeof(uint64_t) == io::write(buffer, io::big_endian{val}));
      REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x11 == io::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x22 == io::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x33 == io::to_integer<uint8_t>(buffer[3]));
      REQUIRE(0x44 == io::to_integer<uint8_t>(buffer[4]));
      REQUIRE(0x55 == io::to_integer<uint8_t>(buffer[5]));
      REQUIRE(0x66 == io::to_integer<uint8_t>(buffer[6]));
      REQUIRE(0x77 == io::to_integer<uint8_t>(buffer[7]));

      REQUIRE(val == io::read<io::big_endian<uint64_t>>(buffer));
    }
  }

  SECTION("Uint32") {
    const auto val = uint32_t{0x01234567};

    SECTION("Little-endian") {
      REQUIRE(sizeof(uint32_t) == io::write(buffer, io::little_endian{val}));
      REQUIRE(0x67 == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x45 == io::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x23 == io::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x01 == io::to_integer<uint8_t>(buffer[3]));

      REQUIRE(val == io::read<io::little_endian<uint32_t>>(buffer));
    }

    SECTION("Big-endian") {
      REQUIRE(sizeof(uint32_t) == io::write(buffer, io::big_endian{val}));
      REQUIRE(0x01 == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0x23 == io::to_integer<uint8_t>(buffer[1]));
      REQUIRE(0x45 == io::to_integer<uint8_t>(buffer[2]));
      REQUIRE(0x67 == io::to_integer<uint8_t>(buffer[3]));

      REQUIRE(val == io::read<io::big_endian<uint32_t>>(buffer));
    }
  }

  SECTION("Uint16") {
    const auto val = uint16_t{0xABCD};

    SECTION("Little-endian") {
      REQUIRE(sizeof(uint16_t) == io::write(buffer, io::little_endian{val}));
      REQUIRE(0xCD == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0xAB == io::to_integer<uint8_t>(buffer[1]));

      REQUIRE(val == io::read<io::little_endian<uint16_t>>(buffer));
    }

    SECTION("Big-endian") {
      REQUIRE(sizeof(uint16_t) == io::write(buffer, io::big_endian{val}));
      REQUIRE(0xAB == io::to_integer<uint8_t>(buffer[0]));
      REQUIRE(0xCD == io::to_integer<uint8_t>(buffer[1]));

      REQUIRE(val == io::read<io::big_endian<uint16_t>>(buffer));
    }
  }
}

TEST_CASE("try_write returns true on good write", "[buffer_io]") {
  SECTION("To raw buffer") {
    auto data = io::static_byte_buffer<4>{};

    SECTION("with default endianness") {
      const auto val = uint32_t{0xFE01CD23};
      const auto result = io::try_write(data, val);

      REQUIRE(result.ok());
      REQUIRE(sizeof(val) == result.value());
      REQUIRE(std::ranges::equal(io::static_byte_buffer<4>{io::byte{0x23}, io::byte{0xCD}, io::byte{0x01}, io::byte{0xFE}},
                                 data));
    }

    SECTION("with specified endianness") {
      const auto val = uint32_t{0x23CD01FE};
      const auto result = io::try_write(data, io::big_endian{val});

      REQUIRE(result.ok());
      REQUIRE(sizeof(val) == result.value());
      REQUIRE(std::ranges::equal(io::static_byte_buffer<4>{io::byte{0x23}, io::byte{0xCD}, io::byte{0x01}, io::byte{0xFE}},
                                 data));
    }
  }

  SECTION("Via byte_read_buffer_view") {
    auto data   = io::static_byte_buffer<4>{};
    auto buffer = io::byte_write_buffer_view{data};

    SECTION("with default endianness") {
      const auto val    = uint32_t{0xFE01CD23};
      const auto result = io::try_write(buffer, val);

      REQUIRE(result.ok());
      REQUIRE(sizeof(val) == result.value());
      REQUIRE(4 == std::distance(buffer.data.begin(), buffer.write_position));
      REQUIRE(std::ranges::equal(io::static_byte_buffer<4>{io::byte{0x23}, io::byte{0xCD}, io::byte{0x01}, io::byte{0xFE}},
                                 buffer.data));
    }

    SECTION("with specified endianness adapter") {
      const auto val    = uint32_t{0x23CD01FE};
      const auto result = io::try_write(buffer, io::big_endian{val});

      REQUIRE(result.ok());
      REQUIRE(sizeof(val) == result.value());
      REQUIRE(4 == std::distance(buffer.data.begin(), buffer.write_position));
      REQUIRE(std::ranges::equal(io::static_byte_buffer<4>{io::byte{0x23}, io::byte{0xCD}, io::byte{0x01}, io::byte{0xFE}},
                                 buffer.data));
    }
  }
}

TEST_CASE("try_write returns error on bad write", "[buffer_io]") {
  auto data = io::static_byte_buffer<3>{};

  const auto result = io::try_write(data, uint32_t{0xCDCDCDCD});
  REQUIRE(result.is_error());
  REQUIRE(io::write_error::insufficient_buffer == result.error());
}

TEST_CASE("write_at", "[buffer_io]") {
  auto data = io::static_byte_buffer<12>{};

  SECTION("directly to data writes at the correct position") {
    const auto val = double{3.14156e+10};
    const auto pos = ptrdiff_t{3};

    REQUIRE(pos + sizeof(val) == io::write_at(pos, data, val));

    REQUIRE(val == io::read<double>({std::next(data.begin(), pos), data.end()}));

    REQUIRE(0x00 == io::to_integer<uint8_t>(data[0]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[1]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[2]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[pos + sizeof(val)]));
  }

  SECTION("raises exception if writing past the end of a buffer") {
    REQUIRE_THROWS_AS(io::write_at(5, data, double{}), std::out_of_range);
  }

  SECTION("raises exception if starting past the end of a buffer") {
    REQUIRE_THROWS_AS(io::write_at(13, data, double{}), std::out_of_range);
  }

  SECTION("handles large and pathological offset") {
    REQUIRE_THROWS_AS(io::write_at(std::numeric_limits<size_t>::max() - sizeof(double) + 1, data, double{}),
                      std::invalid_argument);
  }

  auto buffer = io::byte_write_buffer_view{data};

  SECTION("via byte_write_buffer_view writes at the correct position") {
    const auto val = double{3.14156e+10};
    const auto pos = ptrdiff_t{3};

    REQUIRE(pos + sizeof(val) == io::write_at(pos, buffer, val));
    REQUIRE(std::next(buffer.data.begin(), pos + sizeof(val)) == buffer.write_position);

    REQUIRE(val == io::read<double>({std::next(data.begin(), pos), data.end()}));

    REQUIRE(0x00 == io::to_integer<uint8_t>(data[0]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[1]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[2]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[pos + sizeof(val)]));
  }

  SECTION("raises exception if writing past the end of a buffer") {
    REQUIRE_THROWS_AS(io::write_at(5, buffer, double{}), std::out_of_range);
  }

  SECTION("raises exception if starting past the end of a buffer") {
    REQUIRE_THROWS_AS(io::write_at(13, buffer, double{}), std::out_of_range);
  }

  SECTION("handles large and pathological offset") {
    REQUIRE_THROWS_AS(io::write_at(std::numeric_limits<size_t>::max() - sizeof(double) + 1, buffer, double{}),
                      std::invalid_argument);
  }
}

TEST_CASE("try_write_at", "[buffer_io]") {
  auto data = io::static_byte_buffer<12>{};

  SECTION("directly to data writes at the correct position") {
    const auto val = double{3.14156e+10};
    const auto pos = ptrdiff_t{3};
    const auto result = io::try_write_at(pos, data, val);
    REQUIRE(result.ok());
    REQUIRE(pos + sizeof(val) == result.value());

    REQUIRE(val == io::read<double>({std::next(data.begin(), pos), data.end()}));

    REQUIRE(0x00 == io::to_integer<uint8_t>(data[0]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[1]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[2]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[pos + sizeof(val)]));
  }

  SECTION("returns insufficient buffer error if writing past the end of a buffer") {
    const auto result = io::try_write_at(5, data, double{});
    REQUIRE(result.is_error());
    REQUIRE(io::write_error::insufficient_buffer == result.error());
  }

  SECTION("returns insufficient buffer error if starting past the end of a buffer") {
    const auto result = io::try_write_at(13, data, double{});
    REQUIRE(result.is_error());
    REQUIRE(io::write_error::insufficient_buffer == result.error());
  }

  SECTION("handles large and pathological offset") {
    const auto result = io::try_write_at(std::numeric_limits<size_t>::max() - sizeof(double) + 1, data, double{});
    REQUIRE(result.is_error());
    REQUIRE(io::write_error::invalid_position_offset == result.error());
  }

  auto buffer = io::byte_write_buffer_view{data};

  SECTION("via byte_write_buffer_view writes at the correct position") {
    const auto val    = double{3.14156e+10};
    const auto pos    = ptrdiff_t{3};
    const auto result = io::try_write_at(pos, buffer, val);
    REQUIRE(result.ok());
    REQUIRE(pos + sizeof(val) == result.value());

    REQUIRE(val == io::read<double>({std::next(data.begin(), pos), data.end()}));

    REQUIRE(0x00 == io::to_integer<uint8_t>(data[0]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[1]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[2]));
    REQUIRE(0x00 == io::to_integer<uint8_t>(data[pos + sizeof(val)]));
  }

  SECTION("returns insufficient buffer error if writing past the end of a buffer") {
    const auto result = io::try_write_at(5, buffer, double{});
    REQUIRE(result.is_error());
    REQUIRE(io::write_error::insufficient_buffer == result.error());
  }

  SECTION("returns insufficient buffer error if starting past the end of a buffer") {
    const auto result = io::try_write_at(13, buffer, double{});
    REQUIRE(result.is_error());
    REQUIRE(io::write_error::insufficient_buffer == result.error());
  }

  SECTION("handles large and pathological offset") {
    const auto result = io::try_write_at(std::numeric_limits<size_t>::max() - sizeof(double) + 1, buffer, double{});
    REQUIRE(result.is_error());
    REQUIRE(io::write_error::invalid_position_offset == result.error());
  }
}
