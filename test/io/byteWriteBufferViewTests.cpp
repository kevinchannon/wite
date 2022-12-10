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
  SECTION("construction") {
    SECTION("initialises the write position to the start of the buffer") {
      auto data = io::static_byte_buffer<5>{};
      REQUIRE(0 == io::byte_write_buffer_view(data).write_position());
    }

    SECTION("intialises the write position to the correct position if an offset is specified") {
      auto data = io::static_byte_buffer<5>{};
      REQUIRE(2 == io::byte_write_buffer_view(data, 2).write_position());
    }

    SECTION("throws std::out_of_range if the offset is past the end of the buffer") {
      auto data = io::static_byte_buffer<10>{};
      REQUIRE_THROWS_AS(io::byte_write_buffer_view(data, 11), std::out_of_range);
    }
  }

  SECTION("byte_write_buffer_view::seek") {
    auto data = io::static_byte_buffer<10>{};
    auto view = io::byte_write_buffer_view{data};

    SECTION("moves the view to the correct position") {
      view.seek(2);
      REQUIRE(2 == view.write_position());
    }

    SECTION("throws std::out_of_range if the position is past the end of the buffer") {
      REQUIRE_THROWS_AS(view.seek(11), std::out_of_range);
    }
  }

  SECTION("byte_write_buffer_view::try_seek") {
    auto data = io::static_byte_buffer<10>{};
    auto view = io::byte_write_buffer_view{data};

    SECTION("moves the view to the correct position") {
      const auto result = view.try_seek(2);
      REQUIRE(result.ok());
      REQUIRE(2 == view.write_position());
    }

    SECTION("returns write_error::invalid_position_offset if the position is past the end of the buffer") {
      REQUIRE(io::write_error::invalid_position_offset == view.try_seek(11).error());
    }
  }

  SECTION("write") {
    SECTION("single value") {
      SECTION("scalar value") {
        SECTION("Little-endian") {
          auto array_buffer = std::array<io::byte, 14>{};

          SECTION("Write int at start of buffer (dynamic endianness)") {
            auto buffer = io::byte_write_buffer_view{array_buffer};
            REQUIRE(sizeof(uint32_t) == buffer.write_with_endian(0x89ABCDEF, io::endian::little));
            REQUIRE(4 == buffer.write_position());

            REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[0]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[1]));
            REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[2]));
            REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[3]));

            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

            SECTION("and then another write to the buffer (static endianness)") {
              REQUIRE(sizeof(uint32_t) == buffer.write(io::little_endian{0x01234567}));
              REQUIRE(8 == buffer.write_position());

              REQUIRE(0x67 == io::to_integer<uint8_t>(array_buffer[4]));
              REQUIRE(0x45 == io::to_integer<uint8_t>(array_buffer[5]));
              REQUIRE(0x23 == io::to_integer<uint8_t>(array_buffer[6]));
              REQUIRE(0x01 == io::to_integer<uint8_t>(array_buffer[7]));

              REQUIRE(
                  std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

              SECTION("and then another write to the buffer (default endianness)") {
                REQUIRE(sizeof(uint32_t) == buffer.write(0x463235F9));
                REQUIRE(12 == buffer.write_position());

                REQUIRE((io::endian::native == io::endian::little ? 0xF9 : 0x46) == io::to_integer<uint8_t>(array_buffer[8]));
                REQUIRE((io::endian::native == io::endian::little ? 0x35 : 0x32) == io::to_integer<uint8_t>(array_buffer[9]));
                REQUIRE((io::endian::native == io::endian::little ? 0x32 : 0x35) == io::to_integer<uint8_t>(array_buffer[10]));
                REQUIRE((io::endian::native == io::endian::little ? 0x46 : 0xF9) == io::to_integer<uint8_t>(array_buffer[11]));

                REQUIRE(std::all_of(
                    std::next(array_buffer.begin(), 12), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

                SECTION("and then another write throws std::out_of_range") {
                  REQUIRE_THROWS_AS(buffer.write_with_endian(0x01234567, io::endian::little), std::out_of_range);

                  SECTION("and the buffer is not written to") {
                    REQUIRE(12 == buffer.write_position());
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
            auto buffer = io::byte_write_buffer_view{array_buffer};
            REQUIRE(sizeof(uint32_t) == buffer.write_with_endian(0x89ABCDEF, io::endian::big));
            REQUIRE(4 == buffer.write_position());

            REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[0]));
            REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[1]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[2]));
            REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[3]));

            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

            SECTION("and then another write to the buffer (static endianness)") {
              REQUIRE(sizeof(uint32_t) == buffer.write(io::big_endian{0x01234567}));
              REQUIRE(8 == buffer.write_position());

              REQUIRE(0x01 == io::to_integer<uint8_t>(array_buffer[4]));
              REQUIRE(0x23 == io::to_integer<uint8_t>(array_buffer[5]));
              REQUIRE(0x45 == io::to_integer<uint8_t>(array_buffer[6]));
              REQUIRE(0x67 == io::to_integer<uint8_t>(array_buffer[7]));

              REQUIRE(
                  std::all_of(std::next(array_buffer.begin(), 8), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));

              SECTION("and then another write throws std::out_of_range") {
                REQUIRE_THROWS_AS(buffer.write_with_endian(0x01234567, io::endian::big), std::out_of_range);

                SECTION("and the buffer is not written to") {
                  REQUIRE(8 == buffer.write_position());
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

          const auto val_1 = uint32_t{0x01234567};
          REQUIRE(sizeof(uint32_t) == write_buffer.write(io::little_endian{val_1}));

          const auto val_2 = uint32_t{0x89ABCDEF};
          REQUIRE(sizeof(uint32_t) == write_buffer.write(io::big_endian{val_2}));

          const auto val_3 = int16_t{0x7D04};
          REQUIRE(sizeof(uint16_t) == write_buffer.write(val_3));

          REQUIRE(std::ranges::equal(io::static_byte_buffer<10>{io::byte(0x67),
                                                                io::byte(0x45),
                                                                io::byte(0x23),
                                                                io::byte(0x01),
                                                                io::byte(0x89),
                                                                io::byte(0xAB),
                                                                io::byte(0xCD),
                                                                io::byte(0xEF),
                                                                io::byte(0x04),
                                                                io::byte(0x7D)},
                                     raw_buffer));
        }
      }

      SECTION("range value") {
        auto data = io::static_byte_buffer<12>{};

        SECTION("returns the number of bytes written on success") {
          const auto v = std::vector<uint16_t>{0x0123, 0x4567, 0x89AB, 0xCDEF};
          REQUIRE(8 == io::byte_write_buffer_view{data}.write(v));

          REQUIRE(0x23 == io::to_integer<uint8_t>(data[0]));
          REQUIRE(0x01 == io::to_integer<uint8_t>(data[1]));
          REQUIRE(0x67 == io::to_integer<uint8_t>(data[2]));
          REQUIRE(0x45 == io::to_integer<uint8_t>(data[3]));
          REQUIRE(0xAB == io::to_integer<uint8_t>(data[4]));
          REQUIRE(0x89 == io::to_integer<uint8_t>(data[5]));
          REQUIRE(0xEF == io::to_integer<uint8_t>(data[6]));
          REQUIRE(0xCD == io::to_integer<uint8_t>(data[7]));
        }
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
        REQUIRE(data_size == write_view.write(a, io::big_endian{b}, c, d));
        REQUIRE(data_size == write_view.write_position());

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
        const auto initial_buffer_position = write_view.write_position();
        const auto write_to_buffer = [&]() { write_view.write(a, io::big_endian{b}, c, d, a); };
        REQUIRE_THROWS_AS(write_to_buffer(), std::out_of_range);

        // On error, the write position should remain in the position is was in before the function call that caused the error.
        REQUIRE(initial_buffer_position == write_view.write_position());
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
          const auto result = buffer.try_write(val);

          REQUIRE(result.ok());
          REQUIRE(sizeof(val) == result.value());
          REQUIRE(4 == buffer.write_position());
          REQUIRE(std::ranges::equal(io::static_byte_buffer<4>{io::byte(0x23), io::byte(0xCD), io::byte(0x01), io::byte(0xFE)},
                                     data));
        }

        SECTION("with specified endianness adapter") {
          const auto val    = uint32_t{0x23CD01FE};
          const auto result = buffer.try_write(io::big_endian{val});

          REQUIRE(result.ok());
          REQUIRE(sizeof(val) == result.value());
          REQUIRE(4 == buffer.write_position());
            REQUIRE(std::ranges::equal(io::static_byte_buffer<4>{io::byte(0x23), io::byte(0xCD), io::byte(0x01), io::byte(0xFE)},
                                     data));
        }
      }

      SECTION("returns error on bad write") {
        auto data   = io::static_byte_buffer<3>{};
        auto buffer = io::byte_write_buffer_view{data};

        const auto result = buffer.try_write(uint32_t{0xCDCDCDCD});
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
        const auto result = write_view.try_write(a, io::big_endian{b}, c, d);
        REQUIRE(result.ok());
        REQUIRE(data_size == result.value());
        REQUIRE(data_size == write_view.write_position());

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
        const auto result = write_view.try_write(a, io::big_endian{b}, c, d, int{});

        REQUIRE(result.is_error());
        REQUIRE(io::write_error::insufficient_buffer == result.error());
        REQUIRE(0 == write_view.write_position());
      }
    }
  }

}
