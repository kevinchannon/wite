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
  SECTION("write") {
    SECTION("single value") {
      auto array_buffer = io::static_byte_buffer<10>{};

      SECTION("Little-endian") {
        SECTION("Write int at start of buffer") {
          SECTION("Dynamic endianness") {
            REQUIRE(sizeof(uint32_t) == io::write(array_buffer, 0x89ABCDEF, io::endian::little));

            REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[0]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[1]));
            REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[2]));
            REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[3]));

            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
          }

          SECTION("Static endianness") {
            REQUIRE(sizeof(uint32_t) == io::write(array_buffer, io::little_endian{0x89ABCDEF}));

            REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[0]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[1]));
            REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[2]));
            REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[3]));

            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
          }

          SECTION("Default endianness") {
            REQUIRE(sizeof(uint32_t) == io::write(array_buffer, 0x89ABCDEF));

            REQUIRE((io::endian::native == io::endian::little ? 0xEF : 0x89) == io::to_integer<uint8_t>(array_buffer[0]));
            REQUIRE((io::endian::native == io::endian::little ? 0xCD : 0xAB) == io::to_integer<uint8_t>(array_buffer[1]));
            REQUIRE((io::endian::native == io::endian::little ? 0xAB : 0xCD) == io::to_integer<uint8_t>(array_buffer[2]));
            REQUIRE((io::endian::native == io::endian::little ? 0x89 : 0xEF) == io::to_integer<uint8_t>(array_buffer[3]));

            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
          }
        }

        SECTION("Write int not at start of buffer") {
          REQUIRE(sizeof(uint32_t) ==
                  io::write({std::next(array_buffer.begin(), 2), array_buffer.end()}, 0x89ABCDEF, io::endian::little));

          REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[2]));
          REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[3]));
          REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[4]));
          REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[5]));

          REQUIRE(
              std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == io::byte{0}; }));
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
          REQUIRE(sizeof(uint32_t) ==
                  io::write({std::next(array_buffer.begin(), 2), array_buffer.end()}, 0x89ABCDEF, io::endian::big));

          REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[2]));
          REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[3]));
          REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[4]));
          REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[5]));

          REQUIRE(
              std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == io::byte{0}; }));
          REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
        }

        SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
          REQUIRE_THROWS_AS(io::write({std::next(array_buffer.begin(), 7), array_buffer.end()}, 0x89ABCDEF, io::endian::big),
                            std::out_of_range);
        }
      }
    }

    SECTION("multiple values") {
      const auto a = uint32_t{0x12345678};
      const auto b = uint16_t{0xABCD};
      const auto c = true;
      const auto d = uint32_t{0xFEDCBA98};

      auto buffer = io::static_byte_buffer<sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)>{};
     
      SECTION("returns number of bytes written on success") {
        REQUIRE(11 == io::write(buffer, a, io::big_endian{b}, c, d));

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

      SECTION("throws out_of_range if the buffer is too small") {
        const auto write_to_buffer = [&]() { io::write(buffer, a, io::big_endian{b}, c, d, a); };
        REQUIRE_THROWS_AS(write_to_buffer(), std::out_of_range);
      }
    }
  }

  SECTION("write_at") {
    SECTION("single value") {
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
    }
  }

  SECTION("try_write") {
    SECTION("single value") {
      SECTION("returns number of bytes written on good write") {
        auto data = io::static_byte_buffer<4>{};

        SECTION("with default endianness") {
          const auto val    = uint32_t{0xFE01CD23};
          const auto result = io::try_write(data, val);

          REQUIRE(result.ok());
          REQUIRE(sizeof(val) == result.value());
          REQUIRE(std::ranges::equal(io::static_byte_buffer<4>{io::byte{0x23}, io::byte{0xCD}, io::byte{0x01}, io::byte{0xFE}},
                                     data));
        }

        SECTION("with specified endianness") {
          const auto val    = uint32_t{0x23CD01FE};
          const auto result = io::try_write(data, io::big_endian{val});

          REQUIRE(result.ok());
          REQUIRE(sizeof(val) == result.value());
          REQUIRE(std::ranges::equal(io::static_byte_buffer<4>{io::byte{0x23}, io::byte{0xCD}, io::byte{0x01}, io::byte{0xFE}},
                                     data));
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

      SECTION("returns the number of bytes written") {
        const auto result = io::try_write(buffer, a, io::big_endian{b}, c, d);
        REQUIRE(result.ok());
        REQUIRE(data_size == result.value());

        SECTION("and writes the bytes correctly") {
          REQUIRE(uint32_t{0x78} == io::to_integer<uint32_t>(buffer[ 0]));
          REQUIRE(uint32_t{0x56} == io::to_integer<uint32_t>(buffer[ 1]));
          REQUIRE(uint32_t{0x34} == io::to_integer<uint32_t>(buffer[ 2]));
          REQUIRE(uint32_t{0x12} == io::to_integer<uint32_t>(buffer[ 3]));
                                                                     
          REQUIRE(uint32_t{0xAB} == io::to_integer<uint32_t>(buffer[ 4]));
          REQUIRE(uint32_t{0xCD} == io::to_integer<uint32_t>(buffer[ 5]));
                                                                     
          REQUIRE(uint32_t{true} == io::to_integer<uint32_t>(buffer[ 6]));
                                                                     
          REQUIRE(uint32_t{0x98} == io::to_integer<uint32_t>(buffer[ 7]));
          REQUIRE(uint32_t{0xBA} == io::to_integer<uint32_t>(buffer[ 8]));
          REQUIRE(uint32_t{0xDC} == io::to_integer<uint32_t>(buffer[ 9]));
          REQUIRE(uint32_t{0xFE} == io::to_integer<uint32_t>(buffer[10]));
        }
      }

      SECTION("returns error if the buffer is too small") {
        const auto result = io::try_write(buffer, a, io::big_endian{b}, c, d, int{});

        REQUIRE(result.is_error());
        REQUIRE(io::write_error::insufficient_buffer == result.error());
      }
    }
  }

  SECTION("try_write_at") {
    SECTION("single value") {
      auto data = io::static_byte_buffer<12>{};

      SECTION("directly to data writes at the correct position") {
        const auto val    = double{3.14156e+10};
        const auto pos    = ptrdiff_t{3};
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
    }

    SECTION("multiple values") {
      const auto a = uint32_t{0x12345678};
      const auto b = uint16_t{0xABCD};
      const auto c = true;
      const auto d = uint32_t{0xFEDCBA98};

      constexpr auto data_size = sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d);

      auto buffer = io::static_byte_buffer<1 + data_size>{};

      SECTION("returns the number of bytes written") {
        const auto result = io::try_write_at(1, buffer, a, io::big_endian{b}, c, d);
        REQUIRE(result.ok());
        REQUIRE(1 + data_size == result.value());

        SECTION("and writes the bytes correctly") {
          REQUIRE(uint32_t{0x78} == io::to_integer<uint32_t>(buffer[ 1]));
          REQUIRE(uint32_t{0x56} == io::to_integer<uint32_t>(buffer[ 2]));
          REQUIRE(uint32_t{0x34} == io::to_integer<uint32_t>(buffer[ 3]));
          REQUIRE(uint32_t{0x12} == io::to_integer<uint32_t>(buffer[ 4]));
                                                                     
          REQUIRE(uint32_t{0xAB} == io::to_integer<uint32_t>(buffer[ 5]));
          REQUIRE(uint32_t{0xCD} == io::to_integer<uint32_t>(buffer[ 6]));
                                                                     
          REQUIRE(uint32_t{true} == io::to_integer<uint32_t>(buffer[ 7]));
                                                                     
          REQUIRE(uint32_t{0x98} == io::to_integer<uint32_t>(buffer[ 8]));
          REQUIRE(uint32_t{0xBA} == io::to_integer<uint32_t>(buffer[ 9]));
          REQUIRE(uint32_t{0xDC} == io::to_integer<uint32_t>(buffer[10]));
          REQUIRE(uint32_t{0xFE} == io::to_integer<uint32_t>(buffer[11]));
        }
      }

      SECTION("returns error if the buffer is too small") {
        const auto result = io::try_write_at(2, buffer, a, io::big_endian{b}, c, d);

        REQUIRE(result.is_error());
        REQUIRE(io::write_error::insufficient_buffer == result.error());
      }
    }
  }
}
