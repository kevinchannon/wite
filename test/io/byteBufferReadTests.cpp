/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#include <wite/io/byte_buffer.hpp>
#include <wite/io/byte_stream.hpp>  // This is here to make sure that things build in the presence of each other
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>

#include "../utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_template_test_macros.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace wite;

#ifndef _WITE_COMPILER_MSVC
namespace std {
template <typename T>
initializer_list(const std::initializer_list<T>&) -> initializer_list<T>;
}
#endif

TEMPLATE_TEST_CASE("read from raw byte array tests", "[buffer_io]", io::byte, uint8_t, unsigned char) {
  SECTION("unchecked_read") {
    SECTION("returns value and next read position", "[buffer_io]") {
      const auto data = std::array<TestType, 8>{TestType(0x67),
                                                  TestType(0x45),
                                                  TestType(0x23),
                                                  TestType(0x01),
                                                  TestType(0xEF),
                                                  TestType(0xCD),
                                                  TestType(0xAB),
                                                  TestType(0x89)};

      const TestType* buf = data.data();

      const auto [value, next] = io::unchecked_read<uint32_t>(buf);

      REQUIRE(uint32_t{0x01234567} == value);
      REQUIRE(buf + 4 == next);
    }
  }

  SECTION("read") {
    SECTION("single value") {
      SECTION("vector buffer", "[buffer_io]") {
        const auto vec_buffer = std::vector<TestType>{TestType(0x67),
                                                      TestType(0x45),
                                                      TestType(0x23),
                                                      TestType(0x01),
                                                      TestType(0xEF),
                                                      TestType(0xCD),
                                                      TestType(0xAB),
                                                      TestType(0x89)};

        SECTION("Little-endian") {
          SECTION("Read int") {
            REQUIRE(uint32_t(0x01234567) == io::read_with_endian<uint32_t>(vec_buffer, io::endian::little));
            REQUIRE(uint32_t(0x01234567) == io::read<io::little_endian<uint32_t>>(vec_buffer));
          }

          SECTION("Read 2 shorts") {
            REQUIRE(uint32_t(0x4567) == io::read_with_endian<uint16_t>(vec_buffer, io::endian::little));
            REQUIRE(uint32_t(0x4567) == io::read<io::little_endian<uint16_t>>(vec_buffer));
            REQUIRE(uint32_t(0x0123) ==
                    io::read<io::little_endian<uint16_t>>(std::span{std::next(vec_buffer.begin(), 2), vec_buffer.end()}));
          }

          SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
            const auto read_buf = std::span{std::next(vec_buffer.begin(), 6), vec_buffer.end()};
            REQUIRE_THROWS_AS(io::read_with_endian<uint32_t>(read_buf, io::endian::little), std::out_of_range);
            REQUIRE_THROWS_AS(io::read<io::little_endian<uint32_t>>(read_buf), std::out_of_range);
          }
        }

        SECTION("Big-endian") {
          SECTION("Read int") {
            REQUIRE(uint32_t(0x67452301) == io::read_with_endian<uint32_t>(vec_buffer, io::endian::big));
            REQUIRE(uint32_t(0x67452301) == io::read<io::big_endian<uint32_t>>(vec_buffer));
          }

          SECTION("Read 2 shorts") {
            REQUIRE(uint32_t(0x6745) == io::read_with_endian<uint16_t>(vec_buffer, io::endian::big));
            REQUIRE(uint32_t(0x6745) == io::read<io::big_endian<uint16_t>>(vec_buffer));
            REQUIRE(uint32_t(0x2301) == io::read_with_endian<uint16_t>(
                                            std::span{std::next(vec_buffer.begin(), 2), vec_buffer.end()}, io::endian::big));
          }

          SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
            const auto read_buf = std::span{std::next(vec_buffer.begin(), 6), vec_buffer.end()};
            REQUIRE_THROWS_AS(io::read_with_endian<uint32_t>(read_buf, io::endian::big), std::out_of_range);
            REQUIRE_THROWS_AS(io::read<io::big_endian<uint32_t>>(read_buf), std::out_of_range);
          }
        }

        SECTION("Range value") {
          SECTION("Buffer size is OK") {
            const auto v = io::read_range(vec_buffer, std::vector<uint32_t>(2, 0));

            REQUIRE(v.size() == 2);
            REQUIRE(uint32_t{0x01234567} == v[0]);
            REQUIRE(uint32_t{0x89ABCDEF} == v[1]);
          }

          SECTION("Buffer too small") {
            REQUIRE_THROWS_AS(io::read_range(vec_buffer, std::vector<uint32_t>(3, 0)), std::out_of_range);
          }
        }
      }

      SECTION("pointers to array", "[buffer_io]") {
        constexpr auto buf_size              = 8;
        const TestType data_buffer[buf_size] = {TestType(0x67),
                                                TestType(0x45),
                                                TestType(0x23),
                                                TestType(0x01),
                                                TestType(0xEF),
                                                TestType(0xCD),
                                                TestType(0xAB),
                                                TestType(0x89)};

        const TestType* buf_begin = data_buffer;
        const TestType* buf_end   = buf_begin + buf_size;

        SECTION("Read int") {
          REQUIRE(uint32_t(0x01234567) == io::read<uint32_t>(std::span{buf_begin, buf_end}));
        }

        SECTION("Read 2 shorts") {
          REQUIRE(uint32_t(0x4567) == io::read<uint16_t>(std::span{buf_begin, buf_end}));
          REQUIRE(uint32_t(0x0123) == io::read<uint16_t>(std::span{std::next(buf_begin, 2), buf_end}));
        }

        SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
          const auto read_buf = std::span{std::next(buf_begin, 6), buf_end};
          REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf), std::out_of_range);
        }
      }
    }

    SECTION("multiple-values") {
      // clang-format off
      const auto buffer = std::array<TestType, sizeof(uint32_t) + sizeof(uint16_t) + sizeof(std::array<uint8_t, 4>) + sizeof(bool) + sizeof(uint32_t)>{
        TestType(0x78), TestType(0x56), TestType(0x34), TestType(0x12),
        TestType(0xAB), TestType(0xCD),
        TestType(0x00), TestType(0x01), TestType(0x02), TestType(0x03),
        TestType{true},
        TestType(0x98), TestType(0xBA), TestType(0xDC), TestType(0xFE)
      };
      // clang-format on

      SECTION("are correctly read") {
        const auto [a, b, c, d, e] = io::read<uint32_t, io::big_endian<uint16_t>, std::array<uint8_t, 4>, bool, uint32_t>(buffer);

        REQUIRE(a == uint32_t{0x12345678});
        REQUIRE(b == uint16_t{0xABCD});
        REQUIRE(c == std::array<uint8_t, 4>{0x00, 0x01, 0x02, 0x03});
        REQUIRE(d == true);
        REQUIRE(e == uint32_t{0xFEDCBA98});
      }

      SECTION("throws out_of_range if the buffer is too small") {
        const auto read_from_buffer = [&]() {
          io::read<uint32_t, io::big_endian<uint16_t>, std::array<uint8_t, 5>, bool, uint32_t>(buffer);
        };
        REQUIRE_THROWS_AS(read_from_buffer(), std::out_of_range);
      }
    }
  }

  SECTION("try_read") {
    SECTION("single value") {
      SECTION("returns value on good read") {
        const auto data = std::array<TestType, 4>{TestType(0x67), TestType(0x45), TestType(0xAB), TestType(0xFF)};

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
        const auto data = std::array<TestType, 3>{TestType(0x67), TestType(0x45), TestType(0xAB)};

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
      SECTION("read from buffer succeeds") {
        // clang-format off
        const auto buffer = std::array<TestType, sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
          TestType(0x78), TestType(0x56), TestType(0x34), TestType(0x12),
          TestType(0xAB), TestType(0xCD),
          TestType{true},
          TestType(0x98), TestType(0xBA), TestType(0xDC), TestType(0xFE)
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
        const auto buffer = std::array<TestType, sizeof(uint32_t) + sizeof(uint16_t)>{
          TestType(0x78), TestType(0x56), TestType(0x34), TestType(0x12),
          TestType(0xAB), TestType(0xCD)
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

  SECTION("read_at") {
    SECTION("single value") {
      const auto data = std::vector<TestType>{TestType(0x67),
                                                TestType(0x45),
                                                TestType(0x23),
                                                TestType(0x01),
                                                TestType(0xEF),
                                                TestType(0xCD),
                                                TestType(0xAB),
                                                TestType(0x89)};

      SECTION("scalar value") {
        SECTION("reads at the correct offset") {
          REQUIRE(uint32_t{0x89ABCDEF} == io::read_at<uint32_t>(4, data));
        }

        SECTION("throws std::out_of_range if the read goes past the end of the buffer") {
          REQUIRE_THROWS_AS(io::read_at<uint32_t>(5, data), std::out_of_range);
        }

        SECTION("throws std::out_of_range if the read starts past the end of the buffer") {
          REQUIRE_THROWS_AS(io::read_at<uint32_t>(9, data), std::out_of_range);
        }

        SECTION("throws std::invalid_argument for pathological read offset") {
          REQUIRE_THROWS_AS(io::read_at<uint32_t>(std::numeric_limits<size_t>::max() - sizeof(uint32_t) + 1, data),
                            std::invalid_argument);
        }
      }

      SECTION("range value") {
        SECTION("reads at the correct offset") {
          REQUIRE(std::vector<uint16_t>{0xEF01, 0xABCD} == io::read_range_at(3, data, std::vector<uint16_t>(2, 0)));
        }

        SECTION("throws std::out_of_range if the read goes past the end of the buffer") {
          REQUIRE_THROWS_AS(io::read_range_at(5, data, std::vector<uint16_t>(2, 0)), std::out_of_range);
        }

        SECTION("throws std::out_of_range if the read starts past the end of the buffer") {
          REQUIRE_THROWS_AS(io::read_range_at(9, data, std::vector<uint16_t>(2, 0)), std::out_of_range);
        }

        SECTION("throws std::invalid_argument for pathological read offset") {
          REQUIRE_THROWS_AS(
              io::read_range_at(std::numeric_limits<size_t>::max() - sizeof(uint32_t) + 1, data, std::vector<uint16_t>(2, 0)),
              std::invalid_argument);
        }
      }
    }

    SECTION("multiple values") {
      // clang-format off
      const auto buffer = std::array<TestType, sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
        TestType(0x78), TestType(0x56), TestType(0x34), TestType(0x12),
        TestType(0xAB), TestType(0xCD),
        TestType{true},
        TestType(0x98), TestType(0xBA), TestType(0xDC), TestType(0xFE)
      };
      // clang-format on

      SECTION("reads multiple values from buffer at the specified position") {
        const auto [b, c] = io::read_at<io::big_endian<uint16_t>, bool>(4, buffer);

        REQUIRE(b == uint16_t{0xABCD});
        REQUIRE(c == true);
      }

      SECTION("throws if it tries to read past the end of the buffer") {
        // We need this lambda because Catch doesn't seem to handle commas in the REQUIRE_THROWS_AS macro very well on all
        // platforms.
        const auto read_from_buffer = [&]() { return io::read_at<io::big_endian<uint16_t>, bool>(10, buffer); };

        REQUIRE_THROWS_AS(read_from_buffer(), std::out_of_range);
      }
    }
  }

  SECTION("try_read_at") {
    SECTION("single value") {
      const auto data = std::array<TestType, 8>{TestType(0x67),
                                                  TestType(0x45),
                                                  TestType(0xAB),
                                                  TestType(0xFF),
                                                  TestType(0x01),
                                                  TestType(0x23),
                                                  TestType(0x45),
                                                  TestType(0x67)};

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
      const auto buffer = std::array<TestType, sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
        TestType(0x78), TestType(0x56), TestType(0x34), TestType(0x12),
        TestType(0xAB), TestType(0xCD),
        TestType{true},
        TestType(0x98), TestType(0xBA), TestType(0xDC), TestType(0xFE)
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
