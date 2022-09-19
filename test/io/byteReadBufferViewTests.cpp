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

TEST_CASE("byte_read_buffer_view tests", "[buffer_io]") {
  SECTION("read") {
    SECTION("single value") {
      const auto array_buffer = std::array{io::byte{0x67},
                                           io::byte{0x45},
                                           io::byte{0x23},
                                           io::byte{0x01},
                                           io::byte{0xEF},
                                           io::byte{0xCD},
                                           io::byte{0xAB},
                                           io::byte{0x89}};

      SECTION("Little-endian") {
        SECTION("Read int") {
          auto read_buf = io::byte_read_buffer_view(array_buffer);

          SECTION("Dynamic endianness") {
            REQUIRE(uint32_t(0x01234567) == io::read<uint32_t>(read_buf, io::endian::little));
            REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
          }

          SECTION("Static endianness") {
            REQUIRE(uint32_t(0x01234567) == io::read<io::little_endian<uint32_t>>(read_buf));
            REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
          }

          SECTION("Default endianness") {
            REQUIRE(uint32_t(0x01234567) == io::read<uint32_t>(read_buf));
            REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
          }
        }

        SECTION("Read 2 shorts") {
          auto read_buf = io::byte_read_buffer_view(array_buffer);

          REQUIRE(uint32_t(0x4567) == io::read<uint16_t>(read_buf, io::endian::little));
          REQUIRE(uint32_t(0x0123) == io::read<uint16_t>(read_buf, io::endian::little));
        }

        SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
          auto read_buf = io::byte_read_buffer_view(array_buffer, 6);

          REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf, io::endian::little), std::out_of_range);
        }
      }

      SECTION("Big-endian") {
        SECTION("Read int") {
          auto read_buf = io::byte_read_buffer_view(array_buffer);
          SECTION("Dynmic endianness") {
            REQUIRE(uint32_t(0x67452301) == io::read<uint32_t>(read_buf, io::endian::big));
            REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
          }

          SECTION("Static endianness") {
            REQUIRE(uint32_t(0x67452301) == io::read<io::big_endian<uint32_t>>(read_buf));
            REQUIRE(std::next(read_buf.data.begin(), 4) == read_buf.read_position);
          }
        }

        SECTION("Read 2 shorts") {
          auto read_buf = io::byte_read_buffer_view(array_buffer);

          REQUIRE(uint32_t(0x6745) == io::read<uint16_t>(read_buf, io::endian::big));
          REQUIRE(uint32_t(0x2301) == io::read<uint16_t>(read_buf, io::endian::big));
        }

        SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
          auto read_buf = io::byte_read_buffer_view(array_buffer, 6);

          REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf, io::endian::big), std::out_of_range);
        }
      }
    }

    SECTION("multiple values") {
      // clang-format off
      const auto data = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
        io::byte{0x78}, io::byte{0x56}, io::byte{0x34}, io::byte{0x12},
        io::byte{0xAB}, io::byte{0xCD},
        io::byte{true},
        io::byte{0x98}, io::byte{0xBA}, io::byte{0xDC}, io::byte{0xFE}
      };
      // clang-format on

      auto read_view          = io::byte_read_buffer_view{data};

      SECTION("are correctly read") {
        const auto [a, b, c, d] = io::read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(read_view);

        REQUIRE(a == uint32_t{0x12345678});
        REQUIRE(b == uint16_t{0xABCD});
        REQUIRE(c == true);
        REQUIRE(d == uint32_t{0xFEDCBA98});
      }

      SECTION("throws out_of_range if the buffer is too small") {
        const auto read_from_buffer = [&]() { io::read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t, bool>(read_view); };
        REQUIRE_THROWS_AS(read_from_buffer(), std::out_of_range);
      }
    }
  }

  SECTION("try_read") {
    SECTION("returns value on good read", "[buffer_io]") {
      const auto data = io::static_byte_buffer<4>{io::byte{0x67}, io::byte{0x45}, io::byte{0xAB}, io::byte{0xFF}};
      auto buffer     = io::byte_read_buffer_view{data};

      SECTION("with default endianness") {
        const auto val = io::try_read<uint32_t>(buffer);
        REQUIRE(val.ok());
        REQUIRE(uint32_t{0xFFAB4567} == val.value());
        REQUIRE(4 == std::distance(buffer.data.begin(), buffer.read_position));
      }

      SECTION("with specified endianness adapter") {
        const auto val = io::try_read<io::big_endian<uint32_t>>(buffer);
        REQUIRE(val.ok());
        REQUIRE(uint32_t{0x6745ABFF} == val.value());
        REQUIRE(4 == std::distance(buffer.data.begin(), buffer.read_position));
      }
    }
  }

  SECTION("read_at") {
    SECTION("single value") {
      const auto data = io::dynamic_byte_buffer{io::byte{0x67},
                                                io::byte{0x45},
                                                io::byte{0x23},
                                                io::byte{0x01},
                                                io::byte{0xEF},
                                                io::byte{0xCD},
                                                io::byte{0xAB},
                                                io::byte{0x89}};

      auto buffer = io::byte_read_buffer_view{data};

      SECTION("reads at the correct offset") {
        REQUIRE(uint32_t{0x89ABCDEF} == io::read_at<uint32_t>(4, buffer));
        REQUIRE(8 == std::distance(buffer.data.begin(), buffer.read_position));
      }

      SECTION("throws std::out_of_range if the read goes past the end of the buffer") {
        REQUIRE_THROWS_AS(io::read_at<uint32_t>(5, buffer), std::out_of_range);
      }

      SECTION("throws std::out_of_range if the read starts past the end of the buffer") {
        REQUIRE_THROWS_AS(io::read_at<uint32_t>(9, buffer), std::out_of_range);
      }

      SECTION("throws std::invalid_argument for pathological read offset") {
        REQUIRE_THROWS_AS(io::read_at<uint32_t>(std::numeric_limits<size_t>::max() - sizeof(uint32_t) + 1, buffer),
                          std::invalid_argument);
      }
    }

    SECTION("multiple values") {
      // clang-format off
    const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
      io::byte{0x78}, io::byte{0x56}, io::byte{0x34}, io::byte{0x12},
      io::byte{0xAB}, io::byte{0xCD},
      io::byte{true},
      io::byte{0x98}, io::byte{0xBA}, io::byte{0xDC}, io::byte{0xFE}
    };
      // clang-format on

      auto buffer_view = io::byte_read_buffer_view{buffer};

      SECTION("reads multiple values from buffer at the specified position") {
        const auto [b, c] = io::read_at<io::big_endian<uint16_t>, bool>(4, buffer_view);

        REQUIRE(b == uint16_t{0xABCD});
        REQUIRE(c == true);
      }

      SECTION("throws if it tries to read past the end of the buffer") {
        const auto read_from_buffer = [&]() { return io::read_at<io::big_endian<uint16_t>, bool>(10, buffer_view); };
        REQUIRE_THROWS_AS(read_from_buffer(), std::out_of_range);
      }
    }
  }
}