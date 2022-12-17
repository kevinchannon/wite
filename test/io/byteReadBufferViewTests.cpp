#include <wite/io/byte_buffer.hpp>
#include <wite/io/byte_stream.hpp>  // This is here to make sure that things build in the presence of each other.
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

TEMPLATE_TEST_CASE("byte_read_buffer_view tests", "[buffer_io]", io::byte, uint8_t, unsigned char) {
  SECTION("construction"){
    const auto data = std::array<TestType, 12>{TestType(0xCD),
                                                 TestType(0x53),
                                                 TestType(0x72),
                                                 TestType(0xF3),
                                                 TestType(0x0E),
                                                 TestType(0x52),
                                                 TestType(0x06),
                                                 TestType(0xB3),
                                                 TestType(0x6A),
                                                 TestType(0xEE),
                                                 TestType(0x27),
                                                 TestType(0xFF)};

    SECTION("initialises to the start of the buffer") {
      REQUIRE(0 == io::byte_read_buffer_view{data}.read_position());
    }

    SECTION("has the correct read position when offset is specified") {
      REQUIRE(5 == io::byte_read_buffer_view(data, 5).read_position());
    }

    SECTION("throws std::out_of_range if offset is bigger than buffer range") {
      REQUIRE_THROWS_AS(io::byte_read_buffer_view(data, 13), std::out_of_range);
    }

    SECTION("try_make returns buffer with correct position") {
      const auto buf_result = io::try_make_byte_read_buffer_view(data, 8);
      REQUIRE(buf_result.ok());
      REQUIRE(8 == buf_result.value().read_position());
    }
  }

  SECTION("byte_read_buffer_view::seek") {
    const auto data = std::array<TestType, 10>{};
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
    const auto data = std::array<TestType, 10>{};
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
      const auto array_buffer = std::array{TestType(0x67),
                                           TestType(0x45),
                                           TestType(0x23),
                                           TestType(0x01),
                                           TestType(0xEF),
                                           TestType(0xCD),
                                           TestType(0xAB),
                                           TestType(0x89)};

      SECTION("scalar value") {
        SECTION("Little-endian") {
          SECTION("Read int") {
            auto read_buf = io::byte_read_buffer_view(array_buffer);

            SECTION("Dynamic endianness") {
              REQUIRE(uint32_t(0x01234567) == read_buf.template read<uint32_t>(io::endian::little));
              REQUIRE(4 == read_buf.read_position());
            }

            SECTION("Static endianness") {
              REQUIRE(uint32_t(0x01234567) == read_buf.template read<io::little_endian<uint32_t>>());
              REQUIRE(4 == read_buf.read_position());

              read_buf.seek(0);
              REQUIRE(uint32_t(0x67452301) == read_buf.template read<io::big_endian<uint32_t>>());
              REQUIRE(4 == read_buf.read_position());
            }

            SECTION("Default endianness") {
              REQUIRE(uint32_t(0x01234567) == read_buf.template read<uint32_t>());
              REQUIRE(4 == read_buf.read_position());
            }
          }

          SECTION("Read 2 shorts") {
            auto read_buf = io::byte_read_buffer_view(array_buffer);

            REQUIRE(uint32_t(0x4567) == read_buf.template read<uint16_t>(io::endian::little));
            REQUIRE(uint32_t(0x0123) == read_buf.template read<uint16_t>(io::endian::little));
          }

          SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
            auto read_buf = io::byte_read_buffer_view(array_buffer, 6);

            REQUIRE_THROWS_AS(read_buf.template read<io::little_endian<uint32_t>>(), std::out_of_range);
          }
        }

        SECTION("Big-endian") {
          auto buffer = io::byte_read_buffer_view(array_buffer);
          SECTION("Read int") {
            SECTION("Dynamic endianness") {
              REQUIRE(uint32_t(0x67452301) == buffer.template read<uint32_t>(io::endian::big));
              REQUIRE(4 == buffer.read_position());
            }

            SECTION("Static endianness") {
              REQUIRE(uint32_t(0x67452301) == buffer.template read<io::big_endian<uint32_t>>());
              REQUIRE(4 == buffer.read_position());
            }
          }

          SECTION("Read 2 shorts") {
            REQUIRE(uint32_t(0x6745) == buffer.template read<io::big_endian<uint16_t>>());
            REQUIRE(uint32_t(0x2301) == buffer.template read<io::big_endian<uint16_t>>());

            REQUIRE(4 == buffer.read_position());
          }

          SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
            auto read_buf = io::byte_read_buffer_view(array_buffer, 6);

            REQUIRE_THROWS_AS(read_buf.template read<io::big_endian<uint32_t>>(), std::out_of_range);
          }
        }
      }

      SECTION("range value") {
        auto read_buf = io::byte_read_buffer_view{array_buffer, 1};

        SECTION("return value on good read") {
          REQUIRE(std::vector<uint16_t>{0x2345, 0xEF01, 0xABCD} == read_buf.read_range(std::vector<uint16_t>(3, 0)));
        }

        SECTION("throws std::out_of_range if reading past the end of the buffer") {
#ifndef _WITE_CONFIG_DEBUG
#ifdef _WITE_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overread"
#endif  // _WITE_COMPILER_GCC
#endif  // _WITE_CONFIG_DEBUG
          REQUIRE_THROWS_AS(read_buf.read_range(std::vector<uint16_t>(8, 0)), std::out_of_range);
#ifndef _WITE_CONFIG_DEBUG
#ifdef _WITE_COMPILER_GCC
#pragma GCC diagnostic pop
#endif  // _WITE_COMPILER_GCC
#endif  // _WITE_CONFIG_DEBUG
        }
      }
    }

    SECTION("multiple values") {
      // clang-format off
      const auto data = std::array<TestType, sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
        TestType(0x78), TestType(0x56), TestType(0x34), TestType(0x12),
        TestType(0xAB), TestType(0xCD),
        TestType{true},
        TestType(0x98), TestType(0xBA), TestType(0xDC), TestType(0xFE)
      };
      // clang-format on

      auto read_view = io::byte_read_buffer_view{data};

      SECTION("are correctly read") {
        const auto [a, b, c, d] = read_view.template read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>();

        REQUIRE(a == uint32_t{0x12345678});
        REQUIRE(b == uint16_t{0xABCD});
        REQUIRE(c == true);
        REQUIRE(d == uint32_t{0xFEDCBA98});
      }

      SECTION("throws out_of_range if the buffer is too small") {
        const auto read_from_buffer = [&]() {
          read_view.template read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t, bool>();
        };
        REQUIRE_THROWS_AS(read_from_buffer(), std::out_of_range);
      }
    }
  }

  SECTION("try_read") {
    SECTION("single value") {
      SECTION("returns value on good read") {
        const auto data = std::array<TestType, 4>{TestType(0x67), TestType(0x45), TestType(0xAB), TestType(0xFF)};
        auto buffer     = io::byte_read_buffer_view{data};

        SECTION("scalar value") {
          SECTION("with default endianness") {
            const auto val = buffer.template try_read<uint32_t>();
            REQUIRE(val.ok());
            REQUIRE(uint32_t{0xFFAB4567} == val.value());
            REQUIRE(4 == buffer.read_position());
          }

          SECTION("with specified endianness adapter") {
            const auto val = buffer.template try_read<io::big_endian<uint32_t>>();
            REQUIRE(val.ok());
            REQUIRE(uint32_t{0x6745ABFF} == val.value());
            REQUIRE(4 == buffer.read_position());
          }

          SECTION("read 2 shorts") {
            const auto first_value = buffer.template try_read<uint16_t>();
            REQUIRE(first_value.ok());
            REQUIRE(uint32_t{0x4567} == first_value.value());
            REQUIRE(2 == buffer.read_position());

            const auto second_value = buffer.template try_read<uint16_t>();
            REQUIRE(second_value.ok());
            REQUIRE(uint32_t{0xFFAB} == second_value.value());
            REQUIRE(4 == buffer.read_position());
          }

          SECTION("Read past the end of the buffer returns error value") {
            auto read_buf = io::byte_read_buffer_view(data, 2);

            const auto val = read_buf.template try_read<uint32_t>();
            REQUIRE(val.is_error());
            REQUIRE(io::read_error::insufficient_buffer == val.error());
          }
        }

        SECTION("range value") {
          SECTION("returns OK value on good read") {
            const auto val = buffer.try_read_range(std::vector<uint8_t>(3, 0));
            REQUIRE(val.ok());
            REQUIRE(std::vector<uint8_t>{0x67, 0x45, 0xAB} == val.value());
            REQUIRE(3 == buffer.read_position());
          }

          SECTION("returns error if the read goes past the end of the buffer") {
#ifndef _WITE_CONFIG_DEBUG
#ifdef _WITE_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#pragma GCC diagnostic ignored "-Wstringop-overread"
#endif  // _WITE_COMPILER_GCC
#endif  // _WITE_CONFIG_DEBUG
            const auto val = buffer.try_read_range(std::vector<uint8_t>(5, 0));
#ifndef _WITE_CONFIG_DEBUG
#ifdef _WITE_COMPILER_GCC
#pragma GCC diagnostic pop
#endif  // _WITE_COMPILER_GCC
#endif  // _WITE_CONFIG_DEBUG

            REQUIRE(val.is_error());
            REQUIRE(io::read_error::insufficient_buffer == val.error());
            REQUIRE(4 == buffer.read_position());
          }
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

        auto read_view = io::byte_read_buffer_view{buffer};

        const auto [a, b, c, d] = read_view.template try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>();

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

        auto read_view          = io::byte_read_buffer_view{buffer};
        const auto [a, b, c, d] = read_view.template try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>();

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
