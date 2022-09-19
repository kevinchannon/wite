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

TEST_CASE("byte_write_buffer_view tests", "[bufer_io]") {
  SECTION("write") {
    SECTION("single value") {
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

            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

            SECTION("and then another write to the buffer (default endianness)") {
              REQUIRE(sizeof(uint32_t) == io::write(write_buffer, 0x463235F9));
              REQUIRE(std::next(write_buffer.data.begin(), 12) == write_buffer.write_position);

              REQUIRE((io::endian::native == io::endian::little ? 0xF9 : 0x46) == io::to_integer<uint8_t>(array_buffer[8]));
              REQUIRE((io::endian::native == io::endian::little ? 0x35 : 0x32) == io::to_integer<uint8_t>(array_buffer[9]));
              REQUIRE((io::endian::native == io::endian::little ? 0x32 : 0x35) == io::to_integer<uint8_t>(array_buffer[10]));
              REQUIRE((io::endian::native == io::endian::little ? 0x46 : 0xF9) == io::to_integer<uint8_t>(array_buffer[11]));

              REQUIRE(std::all_of(
                  std::next(array_buffer.begin(), 12), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

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

            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

            SECTION("and then another write throws std::out_of_range") {
              REQUIRE_THROWS_AS(io::write(write_buffer, 0x01234567, io::endian::big), std::out_of_range);

              SECTION("and the buffer is not written to") {
                REQUIRE(std::next(write_buffer.data.begin(), 8) == write_buffer.write_position);
                REQUIRE(std::all_of(
                    std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
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

    SECTION("multiple values") {
      const auto a = uint32_t{0x12345678};
      const auto b = uint16_t{0xABCD};
      const auto c = true;
      const auto d = uint32_t{0xFEDCBA98};

      constexpr auto data_size = sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d);

      auto buffer = io::static_byte_buffer<data_size>{};
      auto write_view = io::byte_write_buffer_view{buffer};

      SECTION("returns number of bytes written on success") {
        REQUIRE(data_size == io::write(write_view, a, io::big_endian{b}, c, d));
        REQUIRE(std::next(write_view.data.begin(), data_size) == write_view.write_position);

        SECTION("and writes the correct values to the buffer") {
          REQUIRE(uint32_t{0x78} == io::to_integer<uint32_t>(buffer[0]));
          REQUIRE(uint32_t{0x56} == io::to_integer<uint32_t>(buffer[1]));
          REQUIRE(uint32_t{0x34} == io::to_integer<uint32_t>(buffer[2]));
          REQUIRE(uint32_t{0x12} == io::to_integer<uint32_t>(buffer[3]));

          REQUIRE(uint32_t{0xAB} == io::to_integer<uint32_t>(buffer[4]));
          REQUIRE(uint32_t{0xCD} == io::to_integer<uint32_t>(buffer[5]));

          REQUIRE(uint32_t{true} == io::to_integer<uint32_t>(buffer[6]));

          REQUIRE(uint32_t{0x98} == io::to_integer<uint32_t>(buffer[7]));
          REQUIRE(uint32_t{0xBA} == io::to_integer<uint32_t>(buffer[8]));
          REQUIRE(uint32_t{0xDC} == io::to_integer<uint32_t>(buffer[9]));
          REQUIRE(uint32_t{0xFE} == io::to_integer<uint32_t>(buffer[10]));
        }
      }

      SECTION("throws out_of_range if the buffer is too small") {
        const auto write_to_buffer = [&]() { io::write(write_view, a, io::big_endian{b}, c, d, a); };
        REQUIRE_THROWS_AS(write_to_buffer(), std::out_of_range);
        REQUIRE(data_size == std::distance(write_view.data.begin(), write_view.write_position));
      }
    }
  }

  SECTION("write_at") {
    SECTION("single value") {
      auto data = io::static_byte_buffer<12>{};
      auto buffer = io::byte_write_buffer_view{data};

      SECTION("writes at the correct position") {
        const auto val = double{3.14156e+10};
        const auto pos = ptrdiff_t{3};

        REQUIRE(pos + sizeof(val) == io::write_at(pos, buffer, val));
        REQUIRE(pos + sizeof(val) == std::distance(buffer.data.begin(), buffer.write_position));

        REQUIRE(val == io::read<double>({std::next(data.begin(), pos), data.end()}));

        REQUIRE(0x00 == io::to_integer<uint8_t>(data[0]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(data[1]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(data[2]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(data[pos + sizeof(val)]));
      }

      SECTION("raises exception if writing past the end of a buffer") {
        REQUIRE_THROWS_AS(io::write_at(5, buffer, double{}), std::out_of_range);
        REQUIRE(0 == std::distance(buffer.data.begin(), buffer.write_position));
      }

      SECTION("raises exception if starting past the end of a buffer") {
        REQUIRE_THROWS_AS(io::write_at(13, buffer, double{}), std::out_of_range);
        REQUIRE(0 == std::distance(buffer.data.begin(), buffer.write_position));
      }

      SECTION("handles large and pathological offset") {
        REQUIRE_THROWS_AS(io::write_at(std::numeric_limits<size_t>::max() - sizeof(double) + 1, buffer, double{}), 
          std::invalid_argument);
        REQUIRE(0 == std::distance(buffer.data.begin(), buffer.write_position));
      }
    }

    SECTION("multiple values") {
      const auto a = uint32_t{0x12345678};
      const auto b = uint16_t{0xABCD};
      const auto c = true;
      const auto d = uint32_t{0xFEDCBA98};

      constexpr auto data_size = sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d);

      auto buffer = io::static_byte_buffer<1 + data_size>{};
      auto write_view = io::byte_write_buffer_view{buffer};

      SECTION("returns number of bytes written on success") {
        REQUIRE(1 + data_size == io::write_at(1, write_view, a, io::big_endian{b}, c, d));
        REQUIRE(1 + data_size == std::distance(write_view.data.begin(), write_view.write_position));

        SECTION("and writes the correct data") {
          REQUIRE(uint32_t{0x78} == io::to_integer<uint32_t>(buffer[1]));
          REQUIRE(uint32_t{0x56} == io::to_integer<uint32_t>(buffer[2]));
          REQUIRE(uint32_t{0x34} == io::to_integer<uint32_t>(buffer[3]));
          REQUIRE(uint32_t{0x12} == io::to_integer<uint32_t>(buffer[4]));

          REQUIRE(uint32_t{0xAB} == io::to_integer<uint32_t>(buffer[5]));
          REQUIRE(uint32_t{0xCD} == io::to_integer<uint32_t>(buffer[6]));

          REQUIRE(uint32_t{true} == io::to_integer<uint32_t>(buffer[7]));

          REQUIRE(uint32_t{0x98} == io::to_integer<uint32_t>(buffer[8]));
          REQUIRE(uint32_t{0xBA} == io::to_integer<uint32_t>(buffer[9]));
          REQUIRE(uint32_t{0xDC} == io::to_integer<uint32_t>(buffer[10]));
          REQUIRE(uint32_t{0xFE} == io::to_integer<uint32_t>(buffer[11]));
        }
      }

      SECTION("throws out_of_range if the buffer is too small") {
        const auto write_to_buffer = [&]() { io::write_at(2, write_view, a, io::big_endian{b}, c, d, a); };
        REQUIRE_THROWS_AS(write_to_buffer(), std::out_of_range);
      }
    }
  }

  SECTION("try_write") {
    SECTION("single value") {
      SECTION("returns number of bytes written on good write") {
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

      SECTION("returns error on bad write") {
        auto data = io::static_byte_buffer<3>{};

        const auto result = io::try_write(data, uint32_t{0xCDCDCDCD});
        REQUIRE(result.is_error());
        REQUIRE(io::write_error::insufficient_buffer == result.error());
      }
    }

    SECTION("multiple values") {
      const auto a = uint32_t{0x12345678};
      const auto b = uint16_t{0xABCD};
      const auto c = true;
      const auto d = uint32_t{0xFEDCBA98};

      constexpr auto data_size = sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d);

      auto buffer = io::static_byte_buffer<data_size>{};
      auto write_view = io::byte_write_buffer_view{buffer};

      SECTION("returns the number of bytes written on success") {
        const auto result = io::try_write(write_view, a, io::big_endian{b}, c, d);
        REQUIRE(result.ok());
        REQUIRE(data_size == result.value());
        REQUIRE(data_size == std::distance(write_view.data.begin(), write_view.write_position));

        SECTION("and writes the bytes correctly") {
          REQUIRE(uint32_t{0x78} == io::to_integer<uint32_t>(buffer[0]));
          REQUIRE(uint32_t{0x56} == io::to_integer<uint32_t>(buffer[1]));
          REQUIRE(uint32_t{0x34} == io::to_integer<uint32_t>(buffer[2]));
          REQUIRE(uint32_t{0x12} == io::to_integer<uint32_t>(buffer[3]));

          REQUIRE(uint32_t{0xAB} == io::to_integer<uint32_t>(buffer[4]));
          REQUIRE(uint32_t{0xCD} == io::to_integer<uint32_t>(buffer[5]));

          REQUIRE(uint32_t{true} == io::to_integer<uint32_t>(buffer[6]));

          REQUIRE(uint32_t{0x98} == io::to_integer<uint32_t>(buffer[7]));
          REQUIRE(uint32_t{0xBA} == io::to_integer<uint32_t>(buffer[8]));
          REQUIRE(uint32_t{0xDC} == io::to_integer<uint32_t>(buffer[9]));
          REQUIRE(uint32_t{0xFE} == io::to_integer<uint32_t>(buffer[10]));
        }
      }

      SECTION("returns error if the buffer is too small") {
        const auto result = io::try_write(write_view, a, io::big_endian{b}, c, d, int{});

        REQUIRE(result.is_error());
        REQUIRE(io::write_error::insufficient_buffer == result.error());
        REQUIRE(0 == std::distance(write_view.data.begin(), write_view.write_position));
      }
    }
  }

  SECTION("try_write_at") {
    SECTION("single value") {
      auto data   = io::static_byte_buffer<12>{};
      auto buffer = io::byte_write_buffer_view{data};

      SECTION("via byte_write_buffer_view writes at the correct position") {
        const auto val    = double{3.14156e+10};
        const auto pos    = ptrdiff_t{3};
        const auto result = io::try_write_at(pos, buffer, val);
        REQUIRE(result.ok());
        REQUIRE(pos + sizeof(val) == result.value());
        REQUIRE(pos + sizeof(val) == std::distance(buffer.data.begin(), buffer.write_position));

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
        REQUIRE(0 == std::distance(buffer.data.begin(), buffer.write_position));
      }

      SECTION("returns insufficient buffer error if starting past the end of a buffer") {
        const auto result = io::try_write_at(13, buffer, double{});
        REQUIRE(result.is_error());
        REQUIRE(io::write_error::insufficient_buffer == result.error());
        REQUIRE(0 == std::distance(buffer.data.begin(), buffer.write_position));
      }

      SECTION("handles large and pathological offset") {
        const auto result = io::try_write_at(std::numeric_limits<size_t>::max() - sizeof(double) + 1, buffer, double{});
        REQUIRE(result.is_error());
        REQUIRE(io::write_error::invalid_position_offset == result.error());
        REQUIRE(0 == std::distance(buffer.data.begin(), buffer.write_position));
      }
    }

    SECTION("multiple values") {
      const auto a = uint32_t{0x12345678};
      const auto b = uint16_t{0xABCD};
      const auto c = true;
      const auto d = uint32_t{0xFEDCBA98};

      constexpr auto data_size = sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d);

      auto buffer     = io::static_byte_buffer<1 + data_size>{};
      auto write_view = io::byte_write_buffer_view{buffer};

      SECTION("returns the number of bytes written on success") {
        const auto result = io::try_write_at(1, write_view, a, io::big_endian{b}, c, d);
        REQUIRE(result.ok());
        REQUIRE(1 + data_size == result.value());
        REQUIRE(1 + data_size == std::distance(write_view.data.begin(), write_view.write_position));

        SECTION("and writes the bytes correctly") {
          REQUIRE(uint32_t{0x78} == io::to_integer<uint32_t>(buffer[1]));
          REQUIRE(uint32_t{0x56} == io::to_integer<uint32_t>(buffer[2]));
          REQUIRE(uint32_t{0x34} == io::to_integer<uint32_t>(buffer[3]));
          REQUIRE(uint32_t{0x12} == io::to_integer<uint32_t>(buffer[4]));

          REQUIRE(uint32_t{0xAB} == io::to_integer<uint32_t>(buffer[5]));
          REQUIRE(uint32_t{0xCD} == io::to_integer<uint32_t>(buffer[6]));

          REQUIRE(uint32_t{true} == io::to_integer<uint32_t>(buffer[7]));

          REQUIRE(uint32_t{0x98} == io::to_integer<uint32_t>(buffer[8]));
          REQUIRE(uint32_t{0xBA} == io::to_integer<uint32_t>(buffer[9]));
          REQUIRE(uint32_t{0xDC} == io::to_integer<uint32_t>(buffer[10]));
          REQUIRE(uint32_t{0xFE} == io::to_integer<uint32_t>(buffer[11]));
        }
      }

      SECTION("returns error when writing fails") {
        const auto result = io::try_write_at(2, write_view, a, b, c, d);
        REQUIRE(result.is_error());
        REQUIRE(0 == std::distance(write_view.data.begin(), write_view.write_position));
      }
    }
  }
}
