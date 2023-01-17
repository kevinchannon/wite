/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#define WITE_NO_EXCEPTIONS

#include <wite/io/byte_buffer.hpp>
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

TEST_CASE("read from raw byte array tests (no exceptions)", "[buffer_io]") {
  SECTION("unchecked_read") {
    SECTION("returns value and next read position", "[buffer_io]") {
      const auto data = io::static_byte_buffer<8>{io::byte(0x67),
                                                  io::byte(0x45),
                                                  io::byte(0x23),
                                                  io::byte(0x01),
                                                  io::byte(0xEF),
                                                  io::byte(0xCD),
                                                  io::byte(0xAB),
                                                  io::byte(0x89)};

      const io::byte* buf = data.data();

      const auto [value, next] = io::unchecked_read<uint32_t>(buf);

      REQUIRE(uint32_t{0x01234567} == value);
      REQUIRE(buf + 4 == next);
    }
  }

  SECTION("try_read") {
    SECTION("single value") {
      SECTION("returns value on good read") {
        const auto data = io::static_byte_buffer<4>{io::byte(0x67), io::byte(0x45), io::byte(0xAB), io::byte(0xFF)};

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

        SECTION("range value") {
          const auto val = io::try_read_range(data, std::vector<uint16_t>(2, uint16_t{}));
          REQUIRE(val.ok());
          REQUIRE(std::vector<uint16_t>{0x4567, 0xFFAB} == val.value());
        }
      }

      SECTION("returns error on bad read") {
        const auto data = io::static_byte_buffer<3>{io::byte(0x67), io::byte(0x45), io::byte(0xAB)};

        SECTION("for scalar values") {
          const auto val = io::try_read<uint32_t>(data);
          REQUIRE(val.is_error());
          REQUIRE(io::read_error::insufficient_buffer == val.error());
        }

        SECTION("for range values") {
          const auto val = io::try_read_range(data, std::vector<uint16_t>(2, uint16_t{}));
          REQUIRE(val.is_error());
          REQUIRE(io::read_error::insufficient_buffer == val.error());
        }
      }
    }

    SECTION("multiple values") {
      SECTION("read from bufffer succeeds") {
        // clang-format off
        const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
          io::byte(0x78), io::byte(0x56), io::byte(0x34), io::byte(0x12),
          io::byte(0xAB), io::byte(0xCD),
          io::byte{true},
          io::byte(0x98), io::byte(0xBA), io::byte(0xDC), io::byte(0xFE)
        };
        // clang-format on

        const auto [a, b, c, d] = io::try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(buffer);

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
          io::byte(0x78), io::byte(0x56), io::byte(0x34), io::byte(0x12),
          io::byte(0xAB), io::byte(0xCD)
        };
        // clang-format on
        const auto [a, b, c, d] = io::try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(buffer);

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

  SECTION("try_read_at") {
    SECTION("single value") {
      const auto data = io::static_byte_buffer<8>{io::byte(0x67),
                                                  io::byte(0x45),
                                                  io::byte(0xAB),
                                                  io::byte(0xFF),
                                                  io::byte(0x01),
                                                  io::byte(0x23),
                                                  io::byte(0x45),
                                                  io::byte(0x67)};

      SECTION("scalar value") {
        SECTION("returns value on good read") {
          SECTION("with default endianness") {
            const auto val = io::try_read_at<uint32_t>(2, data);
            REQUIRE(val.ok());
            REQUIRE(uint32_t{0x2301FFAB} == val.value());
          }

          SECTION("with specified endianness") {
            const auto val = io::try_read_at<io::big_endian<uint32_t>>(3, data);
            REQUIRE(val.ok());
            REQUIRE(uint32_t{0xFF012345} == val.value());
          }
        }

        SECTION("returns error if the read goes past the end of the buffer") {
          const auto val = io::try_read_at<uint32_t>(5, data);
          REQUIRE(val.is_error());
          REQUIRE(io::read_error::insufficient_buffer == val.error());
        }

        SECTION("returns error if the read starts past the end of the buffer") {
          const auto val = io::try_read_at<uint32_t>(9, data);
          REQUIRE(val.is_error());
          REQUIRE(io::read_error::insufficient_buffer == val.error());
        }

        SECTION("returns error for pathological read offset") {
          const auto val = io::try_read_at<uint32_t>(std::numeric_limits<size_t>::max() - sizeof(uint32_t) + 1, data);
          REQUIRE(val.is_error());
          REQUIRE(io::read_error::invalid_position_offset == val.error());
        }
      }

      SECTION("range value") {
        SECTION("returns value on good read") {
          const auto val = io::try_read_range_at(3, data, std::vector<uint16_t>(2, 0));
          REQUIRE(val.ok());
          REQUIRE(std::vector<uint16_t>{0x01FF, 0x4523} == val.value());
        }

        SECTION("returns error if the read goes past the end of the buffer") {
          const auto val = io::try_read_range_at(5, data, std::vector<uint16_t>(2, 0));
          REQUIRE(val.is_error());
          REQUIRE(io::read_error::insufficient_buffer == val.error());
        }

        SECTION("returns error if the read starts past the end of the buffer") {
          const auto val = io::try_read_range_at(9, data, std::vector<uint16_t>(2, 0));
          REQUIRE(val.is_error());
          REQUIRE(io::read_error::insufficient_buffer == val.error());
        }

        SECTION("returns error for pathological read offset") {
          const auto val =
              io::try_read_range_at(std::numeric_limits<size_t>::max() - sizeof(uint32_t) + 1, data, std::vector<uint16_t>(2, 0));
          REQUIRE(val.is_error());
          REQUIRE(io::read_error::invalid_position_offset == val.error());
        }
      }
    }

    SECTION("multiple values") {
      // clang-format off
      const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
        io::byte(0x78), io::byte(0x56), io::byte(0x34), io::byte(0x12),
        io::byte(0xAB), io::byte(0xCD),
        io::byte{true},
        io::byte(0x98), io::byte(0xBA), io::byte(0xDC), io::byte(0xFE)
      };
      // clang-format on

      SECTION("reads multiple values from buffer at the specified position") {
        const auto [b, c] = io::try_read_at<io::big_endian<uint16_t>, bool>(4, buffer);

        REQUIRE(b.ok());
        REQUIRE(uint16_t{0xABCD} == b.value());

        REQUIRE(c.ok());
        REQUIRE(true == c.value());
      }

      SECTION("returns read_error::insufficient_buffer if it tries to read past the end of the buffer") {
        const auto [b, c] = io::try_read_at<io::big_endian<uint16_t>, bool>(9, buffer);

        REQUIRE(b.ok());
        REQUIRE(uint16_t{0xDCFE} == b.value());

        REQUIRE(c.is_error());
        REQUIRE(io::read_error::insufficient_buffer == c.error());
      }
    }
  }
}
