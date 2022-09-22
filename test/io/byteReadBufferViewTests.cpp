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
  SECTION("byte_read_buffer_view::seek") {
    const auto data = io::static_byte_buffer<10>{};
    auto view       = io::byte_read_buffer_view{data};

    SECTION("moves the view to the correct position") {
      view.seek(2);
      REQUIRE(2 == view.read_position());
    }

    SECTION("throws std::out_of_range if the position is past the end of the buffer") {
      REQUIRE_THROWS_AS(view.seek(11), std::out_of_range);
    }
  }

  SECTION("byte_read_buffer_view::try_seek") {
    const auto data = io::static_byte_buffer<10>{};
    auto view       = io::byte_read_buffer_view{data};

    SECTION("moves the view to the correct position") {
      const auto result = view.try_seek(2);
      REQUIRE(result.ok());
      REQUIRE(2 == view.read_position());
    }

    SECTION("returns read_error::invalid_position_offset if the position is past the end of the buffer") {
      REQUIRE(io::read_error::invalid_position_offset == view.try_seek(11).error());
    }
  }

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
            REQUIRE(uint32_t(0x01234567) == read_buf.read<uint32_t>(io::endian::little));
            REQUIRE(4 == read_buf.read_position());
          }

          SECTION("Static endianness") {
            REQUIRE(uint32_t(0x01234567) == read_buf.read<io::little_endian<uint32_t>>());
            REQUIRE(4 == read_buf.read_position());

            read_buf.seek(0);
            REQUIRE(uint32_t(0x67452301) == read_buf.read<io::big_endian<uint32_t>>());
            REQUIRE(4 == read_buf.read_position());
          }

          SECTION("Default endianness") {
            REQUIRE(uint32_t(0x01234567) == read_buf.read<uint32_t>());
            REQUIRE(4 == read_buf.read_position());
          }
        }

        SECTION("Read 2 shorts") {
          auto read_buf = io::byte_read_buffer_view(array_buffer);

          REQUIRE(uint32_t(0x4567) == read_buf.read<uint16_t>(io::endian::little));
          REQUIRE(uint32_t(0x0123) == read_buf.read<uint16_t>(io::endian::little));
        }

        SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
          auto read_buf = io::byte_read_buffer_view(array_buffer, 6);

          REQUIRE_THROWS_AS(read_buf.read<io::little_endian<uint32_t>>(), std::out_of_range);
        }
      }

      SECTION("Big-endian") {
        auto buffer = io::byte_read_buffer_view(array_buffer);
        SECTION("Read int") {
          SECTION("Dynmic endianness") {
            REQUIRE(uint32_t(0x67452301) == buffer.read<uint32_t>(io::endian::big));
            REQUIRE(4 == buffer.read_position());
          }

          SECTION("Static endianness") {
            REQUIRE(uint32_t(0x67452301) == buffer.read<io::big_endian<uint32_t>>());
            REQUIRE(4 == buffer.read_position());
          }
        }

        SECTION("Read 2 shorts") {
          REQUIRE(uint32_t(0x6745) == buffer.read<io::big_endian<uint16_t>>());
          REQUIRE(uint32_t(0x2301) == buffer.read<io::big_endian<uint16_t>>());

          REQUIRE(4 == buffer.read_position());
        }

        SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
          auto read_buf = io::byte_read_buffer_view(array_buffer, 6);

          REQUIRE_THROWS_AS(read_buf.read<io::big_endian<uint32_t>>(), std::out_of_range);
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

      auto read_view = io::byte_read_buffer_view{data};

      SECTION("are correctly read") {
        const auto [a, b, c, d] = read_view.read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>();

        REQUIRE(a == uint32_t{0x12345678});
        REQUIRE(b == uint16_t{0xABCD});
        REQUIRE(c == true);
        REQUIRE(d == uint32_t{0xFEDCBA98});
      }

      SECTION("throws out_of_range if the buffer is too small") {
        const auto read_from_buffer = [&]() {
          read_view.read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t, bool>();
        };
        REQUIRE_THROWS_AS(read_from_buffer(), std::out_of_range);
      }
    }
  }

  SECTION("try_read") {
    SECTION("single value") {
      SECTION("returns value on good read", "[buffer_io]") {
        const auto data = io::static_byte_buffer<4>{io::byte{0x67}, io::byte{0x45}, io::byte{0xAB}, io::byte{0xFF}};
        auto buffer     = io::byte_read_buffer_view{data};

        SECTION("with default endianness") {
          const auto val = buffer.try_read<uint32_t>();
          REQUIRE(val.ok());
          REQUIRE(uint32_t{0xFFAB4567} == val.value());
          REQUIRE(4 == buffer.read_position());
        }

        SECTION("with specified endianness adapter") {
          const auto val = buffer.try_read<io::big_endian<uint32_t>>();
          REQUIRE(val.ok());
          REQUIRE(uint32_t{0x6745ABFF} == val.value());
          REQUIRE(4 == buffer.read_position());
        }
      }
    }

    SECTION("multiple values") {
      SECTION("read from bufffer succeeds") {
        // clang-format off
        const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
          io::byte{0x78}, io::byte{0x56}, io::byte{0x34}, io::byte{0x12},
          io::byte{0xAB}, io::byte{0xCD},
          io::byte{true},
          io::byte{0x98}, io::byte{0xBA}, io::byte{0xDC}, io::byte{0xFE}
        };
        // clang-format on

        auto read_view = io::byte_read_buffer_view{buffer};

        const auto [a, b, c, d] = read_view.try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>();

        REQUIRE(a.ok());
        REQUIRE(a.value() == uint32_t{0x12345678});

        REQUIRE(b.ok());
        REQUIRE(b.value() == uint16_t{0xABCD});

        REQUIRE(c.ok());
        REQUIRE(c.value() == true);

        REQUIRE(d.ok());
        REQUIRE(d.value() == uint32_t{0xFEDCBA98});
      }

      SECTION("inserts errors if the buffer is too small") {
        // clang-format off
        const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t)>{
          io::byte{0x78}, io::byte{0x56}, io::byte{0x34}, io::byte{0x12},
          io::byte{0xAB}, io::byte{0xCD}
        };
        // clang-format on

        auto read_view          = io::byte_read_buffer_view{buffer};
        const auto [a, b, c, d] = read_view.try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>();

        REQUIRE(a.ok());
        REQUIRE(a.value() == uint32_t{0x12345678});

        REQUIRE(b.ok());
        REQUIRE(b.value() == uint16_t{0xABCD});

        REQUIRE(c.is_error());
        REQUIRE(io::read_error::insufficient_buffer == c.error());

        REQUIRE(d.is_error());
        REQUIRE(io::read_error::insufficient_buffer == d.error());
      }
    }
  }
}