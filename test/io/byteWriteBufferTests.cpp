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
#include <list>
#include <numbers>

using namespace wite;

#ifndef _WITE_COMPILER_MSVC
namespace std {
template <typename T>
initializer_list(const std::initializer_list<T>&) -> initializer_list<T>;
}
#endif

namespace {

template<typename T>
auto extract_byte(T val, size_t n) -> uint32_t { return *(reinterpret_cast<const uint8_t*>(&val) + n); }
}

TEST_CASE("Write values to byte arrays", "[buffer_io]") {
  SECTION("write") {
    SECTION("single value") {
      auto array_buffer = io::static_byte_buffer<10>{};

      SECTION("Little-endian") {
        SECTION("Write int at start of buffer") {
          SECTION("Dynamic endianness") {
            REQUIRE(sizeof(uint32_t) == io::write_with_endian(array_buffer, 0x89ABCDEF, io::endian::little));

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
          REQUIRE(sizeof(uint32_t) == io::write_with_endian(std::next(array_buffer.begin(), 2), array_buffer.end(),
                                                            0x89ABCDEF,
                                                            io::endian::little));

          REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[2]));
          REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[3]));
          REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[4]));
          REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[5]));

          REQUIRE(
              std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == io::byte{0}; }));
          REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
        }

        SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
          REQUIRE_THROWS_AS(
              io::write_with_endian(std::next(array_buffer.begin(), 7), array_buffer.end(), 0x89ABCDEF, io::endian::little),
                            std::out_of_range);
        }
      }

      SECTION("Big-endian") {
        SECTION("Write int at start of buffer") {
          REQUIRE(sizeof(uint32_t) == io::write_with_endian(array_buffer, 0x89ABCDEF, io::endian::big));

          REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[0]));
          REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[1]));
          REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[2]));
          REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[3]));

          REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
        }

        SECTION("Write int not at start of buffer") {
          REQUIRE(sizeof(uint32_t) ==
                  io::write_with_endian(std::next(array_buffer.begin(), 2), array_buffer.end(), 0x89ABCDEF, io::endian::big));

          REQUIRE(0x89 == io::to_integer<uint8_t>(array_buffer[2]));
          REQUIRE(0xAB == io::to_integer<uint8_t>(array_buffer[3]));
          REQUIRE(0xCD == io::to_integer<uint8_t>(array_buffer[4]));
          REQUIRE(0xEF == io::to_integer<uint8_t>(array_buffer[5]));

          REQUIRE(
              std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == io::byte{0}; }));
          REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == io::byte{0}; }));
        }

        SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
          REQUIRE_THROWS_AS(
              io::write_with_endian(std::next(array_buffer.begin(), 7), array_buffer.end(), 0x89ABCDEF, io::endian::big),
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

    SECTION("range values") {
      auto buffer = io::static_byte_buffer<16>{};

      SECTION("dynamic ranges") {
        const auto values         = {uint32_t{0x12345678}, uint32_t{0xABCDEF77}, uint32_t{0xCDCDCDCD}, uint32_t{0x17456434}};
        constexpr auto value_size = sizeof(decltype(values)::value_type);

        SECTION("writes the correct number of bytes") {
          REQUIRE(value_size * values.size() == io::write(buffer, values));

          SECTION("and the bytes have the right values") {
            REQUIRE(0x78 == io::to_integer<uint8_t>(buffer[0]));
            REQUIRE(0x56 == io::to_integer<uint8_t>(buffer[1]));
            REQUIRE(0x34 == io::to_integer<uint8_t>(buffer[2]));
            REQUIRE(0x12 == io::to_integer<uint8_t>(buffer[3]));

            REQUIRE(0x77 == io::to_integer<uint8_t>(buffer[4]));
            REQUIRE(0xEF == io::to_integer<uint8_t>(buffer[5]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(buffer[6]));
            REQUIRE(0xAB == io::to_integer<uint8_t>(buffer[7]));

            REQUIRE(0xCD == io::to_integer<uint8_t>(buffer[8]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(buffer[9]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(buffer[10]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(buffer[11]));

            REQUIRE(0x34 == io::to_integer<uint8_t>(buffer[12]));
            REQUIRE(0x64 == io::to_integer<uint8_t>(buffer[13]));
            REQUIRE(0x45 == io::to_integer<uint8_t>(buffer[14]));
            REQUIRE(0x17 == io::to_integer<uint8_t>(buffer[15]));
          }
        }

        SECTION("writes R-value ranges correctly") {
          REQUIRE(3 * sizeof(uint32_t) == io::write(buffer, std::vector<uint32_t>{1, 2, 3}));

          REQUIRE(0x01 == io::to_integer<uint8_t>(buffer[ 0]));
          REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[ 1]));
          REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[ 2]));
          REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[ 3]));

          REQUIRE(0x02 == io::to_integer<uint8_t>(buffer[ 4]));
          REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[ 5]));
          REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[ 6]));
          REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[ 7]));

          REQUIRE(0x03 == io::to_integer<uint8_t>(buffer[ 8]));
          REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[ 9]));
          REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[10]));
          REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[11]));
        }
      }

      SECTION("static ranges") {
        const auto values         = std::array{uint32_t{0x12345678}, uint32_t{0xABCDEF77}, uint32_t{0xCDCDCDCD}, uint32_t{0x17456434}};
        constexpr auto value_size = sizeof(decltype(values)::value_type);
        auto buffer_1               = io::static_byte_buffer<4 * value_size>{};

        SECTION("writes the correct number of bytes") {
          REQUIRE(value_size * values.size() == io::write(buffer_1, values));

          SECTION("and the bytes have the right values") {
            REQUIRE(0x78 == io::to_integer<uint8_t>(buffer_1[0]));
            REQUIRE(0x56 == io::to_integer<uint8_t>(buffer_1[1]));
            REQUIRE(0x34 == io::to_integer<uint8_t>(buffer_1[2]));
            REQUIRE(0x12 == io::to_integer<uint8_t>(buffer_1[3]));

            REQUIRE(0x77 == io::to_integer<uint8_t>(buffer_1[4]));
            REQUIRE(0xEF == io::to_integer<uint8_t>(buffer_1[5]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(buffer_1[6]));
            REQUIRE(0xAB == io::to_integer<uint8_t>(buffer_1[7]));

            REQUIRE(0xCD == io::to_integer<uint8_t>(buffer_1[8]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(buffer_1[9]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(buffer_1[10]));
            REQUIRE(0xCD == io::to_integer<uint8_t>(buffer_1[11]));

            REQUIRE(0x34 == io::to_integer<uint8_t>(buffer_1[12]));
            REQUIRE(0x64 == io::to_integer<uint8_t>(buffer_1[13]));
            REQUIRE(0x45 == io::to_integer<uint8_t>(buffer_1[14]));
            REQUIRE(0x17 == io::to_integer<uint8_t>(buffer_1[15]));
          }
        }
      }

      SECTION("throws std::out_of_range if the buffer is too small") {
        const auto too_many_values = {1, 2, 3, 4, 5};
        REQUIRE_THROWS_AS(io::write(buffer, too_many_values), std::out_of_range);
      }

      SECTION("can be used with multiple value overloads") {
        const auto a = int32_t{3};
        const auto b = std::vector<uint16_t> {1, 2, 3};
        const auto c = '9';

        REQUIRE(11 == io::write(buffer, a, b, c));

        REQUIRE(0x03 == io::to_integer<uint8_t>(buffer[0]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[1]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[2]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[3]));

        REQUIRE(0x01 == io::to_integer<uint8_t>(buffer[4]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[5]));
        REQUIRE(0x02 == io::to_integer<uint8_t>(buffer[6]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[7]));
        REQUIRE(0x03 == io::to_integer<uint8_t>(buffer[8]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(buffer[9]));

        REQUIRE(0x39 == io::to_integer<uint8_t>(buffer[10]));
      }
    }
  }

  SECTION("write_at") {
    SECTION("single value") {
      auto data = io::static_byte_buffer<18>{};

      SECTION("directly to data writes at the correct position") {
        const auto val = double{3.14156e+10};
        const auto pos = ptrdiff_t{3};

        REQUIRE(pos + sizeof(val) == io::write_at(pos, data, val));

        REQUIRE(val == io::read<double>(std::span{std::next(data.begin(), pos), data.end()}));

        REQUIRE(0x00 == io::to_integer<uint8_t>(data[0]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(data[1]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(data[2]));
        REQUIRE(0x00 == io::to_integer<uint8_t>(data[pos + sizeof(val)]));
      }

      SECTION("raises exception if writing past the end of a buffer") {
        const auto pos = 1 + data.size() - sizeof(double);
        REQUIRE_THROWS_AS(io::write_at(pos, data, double{}), std::out_of_range);
      }

      SECTION("raises exception if starting past the end of a buffer") {
        const auto pos = 1 + data.size();
        REQUIRE_THROWS_AS(io::write_at(pos, data, double{}), std::out_of_range);
      }

      SECTION("handles large and pathological offset") {
        REQUIRE_THROWS_AS(io::write_at(std::numeric_limits<size_t>::max() - sizeof(double) + 1, data, double{}),
                          std::invalid_argument);
      }

      SECTION("range values are written correctly") {
        const auto values         = {uint32_t{0x12345678}, uint32_t{0xABCDEF77}, uint32_t{0xCDCDCDCD}, uint32_t{0x17456434}};
        constexpr auto value_size = sizeof(decltype(values)::value_type);

        const auto pos = size_t{2};

        REQUIRE(pos + value_size * values.size() == io::write_at(pos, data, values));

        REQUIRE(0x78 == io::to_integer<uint8_t>(data[2]));
        REQUIRE(0x56 == io::to_integer<uint8_t>(data[3]));
        REQUIRE(0x34 == io::to_integer<uint8_t>(data[4]));
        REQUIRE(0x12 == io::to_integer<uint8_t>(data[5]));

        REQUIRE(0x77 == io::to_integer<uint8_t>(data[6]));
        REQUIRE(0xEF == io::to_integer<uint8_t>(data[7]));
        REQUIRE(0xCD == io::to_integer<uint8_t>(data[8]));
        REQUIRE(0xAB == io::to_integer<uint8_t>(data[9]));

        REQUIRE(0xCD == io::to_integer<uint8_t>(data[10]));
        REQUIRE(0xCD == io::to_integer<uint8_t>(data[11]));
        REQUIRE(0xCD == io::to_integer<uint8_t>(data[12]));
        REQUIRE(0xCD == io::to_integer<uint8_t>(data[13]));

        REQUIRE(0x34 == io::to_integer<uint8_t>(data[14]));
        REQUIRE(0x64 == io::to_integer<uint8_t>(data[15]));
        REQUIRE(0x45 == io::to_integer<uint8_t>(data[16]));
        REQUIRE(0x17 == io::to_integer<uint8_t>(data[17]));
      }

      SECTION("R-value range values are written correctly") {
        const auto values         = {uint32_t{0x12345678}, uint32_t{0xABCDEF77}, uint32_t{0xCDCDCDCD}, uint32_t{0x17456434}};
        constexpr auto value_size = sizeof(decltype(values)::value_type);

        const auto pos = size_t{2};

        REQUIRE(
            pos + value_size * values.size() ==
            io::write_at(pos, data, std::vector<uint32_t>{uint32_t{0x12345678}, uint32_t{0xABCDEF77}, uint32_t{0xCDCDCDCD}, uint32_t{0x17456434}}));

        REQUIRE(0x78 == io::to_integer<uint8_t>(data[2]));
        REQUIRE(0x56 == io::to_integer<uint8_t>(data[3]));
        REQUIRE(0x34 == io::to_integer<uint8_t>(data[4]));
        REQUIRE(0x12 == io::to_integer<uint8_t>(data[5]));

        REQUIRE(0x77 == io::to_integer<uint8_t>(data[6]));
        REQUIRE(0xEF == io::to_integer<uint8_t>(data[7]));
        REQUIRE(0xCD == io::to_integer<uint8_t>(data[8]));
        REQUIRE(0xAB == io::to_integer<uint8_t>(data[9]));

        REQUIRE(0xCD == io::to_integer<uint8_t>(data[10]));
        REQUIRE(0xCD == io::to_integer<uint8_t>(data[11]));
        REQUIRE(0xCD == io::to_integer<uint8_t>(data[12]));
        REQUIRE(0xCD == io::to_integer<uint8_t>(data[13]));

        REQUIRE(0x34 == io::to_integer<uint8_t>(data[14]));
        REQUIRE(0x64 == io::to_integer<uint8_t>(data[15]));
        REQUIRE(0x45 == io::to_integer<uint8_t>(data[16]));
        REQUIRE(0x17 == io::to_integer<uint8_t>(data[17]));
      }
    }

    SECTION("multiple values") {
      const auto a = uint32_t{0x12345678};
      const auto b = uint16_t{0xABCD};
      const auto c = std::list<double>{std::numbers::pi, std::numbers::e, };
      const auto d = true;
      const auto e = uint32_t{0xFEDCBA98};

      constexpr auto data_size = sizeof(a) + sizeof(b) + 2 * sizeof(double) + sizeof(d) + sizeof(e);

      auto buffer = io::static_byte_buffer<1 + data_size>{};

      SECTION("returns number of bytes written on success") {
        REQUIRE(1 + data_size == io::write_at(1, buffer, a, io::big_endian{b}, c, d, e));

        SECTION("and writes the correct data") {
          // a
          REQUIRE(uint32_t{0x78} == io::to_integer<uint32_t>(buffer[1]));
          REQUIRE(uint32_t{0x56} == io::to_integer<uint32_t>(buffer[2]));
          REQUIRE(uint32_t{0x34} == io::to_integer<uint32_t>(buffer[3]));
          REQUIRE(uint32_t{0x12} == io::to_integer<uint32_t>(buffer[4]));
              
          // b
          REQUIRE(uint32_t{0xAB} == io::to_integer<uint32_t>(buffer[5]));
          REQUIRE(uint32_t{0xCD} == io::to_integer<uint32_t>(buffer[6]));
              
          // c.0
          REQUIRE(extract_byte(std::numbers::pi, 0) == io::to_integer<uint32_t>(buffer[7]));
          REQUIRE(extract_byte(std::numbers::pi, 1) == io::to_integer<uint32_t>(buffer[8]));
          REQUIRE(extract_byte(std::numbers::pi, 2) == io::to_integer<uint32_t>(buffer[9]));
          REQUIRE(extract_byte(std::numbers::pi, 3) == io::to_integer<uint32_t>(buffer[10]));
          REQUIRE(extract_byte(std::numbers::pi, 4) == io::to_integer<uint32_t>(buffer[11]));
          REQUIRE(extract_byte(std::numbers::pi, 5) == io::to_integer<uint32_t>(buffer[12]));
          REQUIRE(extract_byte(std::numbers::pi, 6) == io::to_integer<uint32_t>(buffer[13]));
          REQUIRE(extract_byte(std::numbers::pi, 7) == io::to_integer<uint32_t>(buffer[14]));

          // c.1
          REQUIRE(extract_byte(std::numbers::e, 0) == io::to_integer<uint32_t>(buffer[15]));
          REQUIRE(extract_byte(std::numbers::e, 1) == io::to_integer<uint32_t>(buffer[16]));
          REQUIRE(extract_byte(std::numbers::e, 2) == io::to_integer<uint32_t>(buffer[17]));
          REQUIRE(extract_byte(std::numbers::e, 3) == io::to_integer<uint32_t>(buffer[18]));
          REQUIRE(extract_byte(std::numbers::e, 4) == io::to_integer<uint32_t>(buffer[19]));
          REQUIRE(extract_byte(std::numbers::e, 5) == io::to_integer<uint32_t>(buffer[20]));
          REQUIRE(extract_byte(std::numbers::e, 6) == io::to_integer<uint32_t>(buffer[21]));
          REQUIRE(extract_byte(std::numbers::e, 7) == io::to_integer<uint32_t>(buffer[22]));

          // d
          REQUIRE(uint32_t{true} == io::to_integer<uint32_t>(buffer[23]));
              
          // e
          REQUIRE(uint32_t{0x98} == io::to_integer<uint32_t>(buffer[24]));
          REQUIRE(uint32_t{0xBA} == io::to_integer<uint32_t>(buffer[25]));
          REQUIRE(uint32_t{0xDC} == io::to_integer<uint32_t>(buffer[26]));
          REQUIRE(uint32_t{0xFE} == io::to_integer<uint32_t>(buffer[27]));
        }
      }

      SECTION("throws out_of_range if the buffer is too small") {
        const auto write_to_buffer = [&]() { io::write_at(2, buffer, a, io::big_endian{b}, c, d, a); };
        REQUIRE_THROWS_AS(write_to_buffer(), std::out_of_range);
      }
    }
  }

  SECTION("try_write") {
    SECTION("single value") {
      SECTION("returns number of bytes written on good write") {
        auto data = io::static_byte_buffer<20>{};

        SECTION("with default endianness") {
          const auto val    = uint32_t{0xFE01CD23};
          const auto result = io::try_write(data, val);

          REQUIRE(result.ok());
          REQUIRE(sizeof(val) == result.value());

          REQUIRE(uint32_t{0x23} == io::to_integer<uint32_t>(data[0]));
          REQUIRE(uint32_t{0xCD} == io::to_integer<uint32_t>(data[1]));
          REQUIRE(uint32_t{0x01} == io::to_integer<uint32_t>(data[2]));
          REQUIRE(uint32_t{0xFE} == io::to_integer<uint32_t>(data[3]));
        }

        SECTION("with specified endianness") {
          const auto val    = uint32_t{0x23CD01FE};
          const auto result = io::try_write(data, io::big_endian{val});

          REQUIRE(result.ok());
          REQUIRE(sizeof(val) == result.value());

          REQUIRE(uint32_t{0x23} == io::to_integer<uint32_t>(data[0]));
          REQUIRE(uint32_t{0xCD} == io::to_integer<uint32_t>(data[1]));
          REQUIRE(uint32_t{0x01} == io::to_integer<uint32_t>(data[2]));
          REQUIRE(uint32_t{0xFE} == io::to_integer<uint32_t>(data[3]));
        }

        SECTION("range values") {
          SECTION("static ranges") {
            const auto values = std::array{1.123f, 2.456f, 3.789f};
            const auto result = io::try_write(data, values);

            REQUIRE(result.ok());
            REQUIRE(sizeof(float) * values.size() == result.value());

            REQUIRE(extract_byte(values[0], 0) == io::to_integer<uint32_t>(data[0]));
            REQUIRE(extract_byte(values[0], 1) == io::to_integer<uint32_t>(data[1]));
            REQUIRE(extract_byte(values[0], 2) == io::to_integer<uint32_t>(data[2]));
            REQUIRE(extract_byte(values[0], 3) == io::to_integer<uint32_t>(data[3]));

            REQUIRE(extract_byte(values[1], 0) == io::to_integer<uint32_t>(data[4]));
            REQUIRE(extract_byte(values[1], 1) == io::to_integer<uint32_t>(data[5]));
            REQUIRE(extract_byte(values[1], 2) == io::to_integer<uint32_t>(data[6]));
            REQUIRE(extract_byte(values[1], 3) == io::to_integer<uint32_t>(data[7]));

            REQUIRE(extract_byte(values[2], 0) == io::to_integer<uint32_t>(data[8]));
            REQUIRE(extract_byte(values[2], 1) == io::to_integer<uint32_t>(data[9]));
            REQUIRE(extract_byte(values[2], 2) == io::to_integer<uint32_t>(data[10]));
            REQUIRE(extract_byte(values[2], 3) == io::to_integer<uint32_t>(data[11]));
          }

          SECTION("dynamoc ranges") {
            const auto values = std::vector{1.123f, 2.456f, 3.789f};
            const auto result = io::try_write(data, values);

            REQUIRE(result.ok());
            REQUIRE(sizeof(float) * values.size() == result.value());

            REQUIRE(extract_byte(values[0], 0) == io::to_integer<uint32_t>(data[0]));
            REQUIRE(extract_byte(values[0], 1) == io::to_integer<uint32_t>(data[1]));
            REQUIRE(extract_byte(values[0], 2) == io::to_integer<uint32_t>(data[2]));
            REQUIRE(extract_byte(values[0], 3) == io::to_integer<uint32_t>(data[3]));

            REQUIRE(extract_byte(values[1], 0) == io::to_integer<uint32_t>(data[4]));
            REQUIRE(extract_byte(values[1], 1) == io::to_integer<uint32_t>(data[5]));
            REQUIRE(extract_byte(values[1], 2) == io::to_integer<uint32_t>(data[6]));
            REQUIRE(extract_byte(values[1], 3) == io::to_integer<uint32_t>(data[7]));

            REQUIRE(extract_byte(values[2], 0) == io::to_integer<uint32_t>(data[8]));
            REQUIRE(extract_byte(values[2], 1) == io::to_integer<uint32_t>(data[9]));
            REQUIRE(extract_byte(values[2], 2) == io::to_integer<uint32_t>(data[10]));
            REQUIRE(extract_byte(values[2], 3) == io::to_integer<uint32_t>(data[11]));
          }

          SECTION("R-value ranges") {
            const auto result = io::try_write(data, std::vector{1.123f, 2.456f, 3.789f});

            REQUIRE(result.ok());

            const auto values = std::vector{1.123f, 2.456f, 3.789f};
            REQUIRE(sizeof(float) * values.size() == result.value());

            REQUIRE(extract_byte(values[0], 0) == io::to_integer<uint32_t>(data[0]));
            REQUIRE(extract_byte(values[0], 1) == io::to_integer<uint32_t>(data[1]));
            REQUIRE(extract_byte(values[0], 2) == io::to_integer<uint32_t>(data[2]));
            REQUIRE(extract_byte(values[0], 3) == io::to_integer<uint32_t>(data[3]));

            REQUIRE(extract_byte(values[1], 0) == io::to_integer<uint32_t>(data[4]));
            REQUIRE(extract_byte(values[1], 1) == io::to_integer<uint32_t>(data[5]));
            REQUIRE(extract_byte(values[1], 2) == io::to_integer<uint32_t>(data[6]));
            REQUIRE(extract_byte(values[1], 3) == io::to_integer<uint32_t>(data[7]));

            REQUIRE(extract_byte(values[2], 0) == io::to_integer<uint32_t>(data[8]));
            REQUIRE(extract_byte(values[2], 1) == io::to_integer<uint32_t>(data[9]));
            REQUIRE(extract_byte(values[2], 2) == io::to_integer<uint32_t>(data[10]));
            REQUIRE(extract_byte(values[2], 3) == io::to_integer<uint32_t>(data[11]));
          }
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
      const auto e = std::vector{uint32_t{1}, uint32_t{2}, uint32_t{3}, uint32_t{4}};

      constexpr auto data_size = sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d) + sizeof(uint32_t) * 4;

      auto buffer = io::static_byte_buffer<data_size>{};

      SECTION("returns the number of bytes written") {
        const auto result = io::try_write(buffer, a, io::big_endian{b}, c, d, e);
        REQUIRE(result.ok());
        REQUIRE(data_size == result.value());

        SECTION("and writes the bytes correctly") {
          // a
          REQUIRE(uint32_t{0x78} == io::to_integer<uint32_t>(buffer[ 0]));
          REQUIRE(uint32_t{0x56} == io::to_integer<uint32_t>(buffer[ 1]));
          REQUIRE(uint32_t{0x34} == io::to_integer<uint32_t>(buffer[ 2]));
          REQUIRE(uint32_t{0x12} == io::to_integer<uint32_t>(buffer[ 3]));
              
          // b
          REQUIRE(uint32_t{0xAB} == io::to_integer<uint32_t>(buffer[ 4]));
          REQUIRE(uint32_t{0xCD} == io::to_integer<uint32_t>(buffer[ 5]));
              
          // c
          REQUIRE(uint32_t{true} == io::to_integer<uint32_t>(buffer[ 6]));
              
          // d
          REQUIRE(uint32_t{0x98} == io::to_integer<uint32_t>(buffer[ 7]));
          REQUIRE(uint32_t{0xBA} == io::to_integer<uint32_t>(buffer[ 8]));
          REQUIRE(uint32_t{0xDC} == io::to_integer<uint32_t>(buffer[ 9]));
          REQUIRE(uint32_t{0xFE} == io::to_integer<uint32_t>(buffer[10]));

          // e.0
          REQUIRE(uint32_t{0x01} == io::to_integer<uint32_t>(buffer[11]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[12]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[13]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[14]));

          // e.1
          REQUIRE(uint32_t{0x02} == io::to_integer<uint32_t>(buffer[15]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[16]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[17]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[18]));

          // e.2
          REQUIRE(uint32_t{0x03} == io::to_integer<uint32_t>(buffer[19]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[20]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[21]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[22]));

          // e.3
          REQUIRE(uint32_t{0x04} == io::to_integer<uint32_t>(buffer[23]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[24]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[25]));
          REQUIRE(uint32_t{0x00} == io::to_integer<uint32_t>(buffer[26]));
        }
      }

      SECTION("returns error if the buffer is too small") {
        const auto result = io::try_write(buffer, a, io::big_endian{b}, c, d, e, int{});

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

        REQUIRE(val == io::read<double>(std::span{std::next(data.begin(), pos), data.end()}));

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


TEST_CASE("Write values to non-byte arrays", "[buffer_io]") {
  SECTION("write") {
    SECTION("single value") {
      auto array_buffer = std::array<uint8_t, 10>{};

      SECTION("Little-endian") {
        SECTION("Write int at start of buffer") {
          SECTION("Dynamic endianness") {
            REQUIRE(sizeof(uint32_t) == io::write_with_endian(array_buffer, 0x89ABCDEF, io::endian::little));

            REQUIRE(0xEF == array_buffer[0]);
            REQUIRE(0xCD == array_buffer[1]);
            REQUIRE(0xAB == array_buffer[2]);
            REQUIRE(0x89 == array_buffer[3]);

            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == 0; }));
          }

          SECTION("Static endianness") {
            REQUIRE(sizeof(uint32_t) == io::write(array_buffer, io::little_endian{0x89ABCDEF}));
          
            REQUIRE(0xEF == array_buffer[0]);
            REQUIRE(0xCD == array_buffer[1]);
            REQUIRE(0xAB == array_buffer[2]);
            REQUIRE(0x89 == array_buffer[3]);
          
            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == 0; }));
          }
          
          SECTION("Default endianness") {
            REQUIRE(sizeof(uint32_t) == io::write(array_buffer, 0x89ABCDEF));
          
            REQUIRE((io::endian::native == io::endian::little ? 0xEF : 0x89) == array_buffer[0]);
            REQUIRE((io::endian::native == io::endian::little ? 0xCD : 0xAB) == array_buffer[1]);
            REQUIRE((io::endian::native == io::endian::little ? 0xAB : 0xCD) == array_buffer[2]);
            REQUIRE((io::endian::native == io::endian::little ? 0x89 : 0xEF) == array_buffer[3]);
          
            REQUIRE(
                std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == 0; }));
          }
        }

        SECTION("Write int not at start of buffer") {
          REQUIRE(sizeof(uint32_t) ==
                  io::write_with_endian(std::next(array_buffer.begin(), 2), array_buffer.end(), 0x89ABCDEF, io::endian::little));

          REQUIRE(0xEF == array_buffer[2]);
          REQUIRE(0xCD == array_buffer[3]);
          REQUIRE(0xAB == array_buffer[4]);
          REQUIRE(0x89 == array_buffer[5]);

          REQUIRE(
              std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == 0; }));
          REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == 0; }));
        }

        SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
          REQUIRE_THROWS_AS(
              io::write_with_endian(std::next(array_buffer.begin(), 7), array_buffer.end(), 0x89ABCDEF, io::endian::little),
              std::out_of_range);
        }
      }

      SECTION("Big-endian") {
        SECTION("Write int at start of buffer") {
          REQUIRE(sizeof(uint32_t) == io::write_with_endian(array_buffer, 0x89ABCDEF, io::endian::big));

          REQUIRE(0x89 == array_buffer[0]);
          REQUIRE(0xAB == array_buffer[1]);
          REQUIRE(0xCD == array_buffer[2]);
          REQUIRE(0xEF == array_buffer[3]);

          REQUIRE(std::all_of(std::next(array_buffer.begin(), 4), array_buffer.end(), [](auto&& x) { return x == 0; }));
        }

        SECTION("Write int not at start of buffer") {
          REQUIRE(sizeof(uint32_t) ==
                  io::write_with_endian(std::next(array_buffer.begin(), 2), array_buffer.end(), 0x89ABCDEF, io::endian::big));

          REQUIRE(0x89 == array_buffer[2]);
          REQUIRE(0xAB == array_buffer[3]);
          REQUIRE(0xCD == array_buffer[4]);
          REQUIRE(0xEF == array_buffer[5]);

          REQUIRE(
              std::all_of(array_buffer.begin(), std::next(array_buffer.begin(), 2), [](auto&& x) { return x == 0; }));
          REQUIRE(std::all_of(std::next(array_buffer.begin(), 6), array_buffer.end(), [](auto&& x) { return x == 0; }));
        }

        SECTION("Write past the end of the buffer fails with std::out_of_range exception") {
          REQUIRE_THROWS_AS(
              io::write_with_endian(std::next(array_buffer.begin(), 7), array_buffer.end(), 0x89ABCDEF, io::endian::big),
              std::out_of_range);
        }
      }
    }

    SECTION("multiple values") {
      const auto a = uint32_t{0x12345678};
      const auto b = uint16_t{0xABCD};
      const auto c = true;
      const auto d = uint32_t{0xFEDCBA98};
    
      auto buffer = std::array<unsigned char, sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)>{};
    
      SECTION("returns number of bytes written on success") {
        REQUIRE(11 == io::write(buffer, a, io::big_endian{b}, c, d));
    
        REQUIRE(uint32_t{0x78} == buffer[0]);
        REQUIRE(uint32_t{0x56} == buffer[1]);
        REQUIRE(uint32_t{0x34} == buffer[2]);
        REQUIRE(uint32_t{0x12} == buffer[3]);
   
        REQUIRE(uint32_t{0xAB} == buffer[4]);
        REQUIRE(uint32_t{0xCD} == buffer[5]);
   
        REQUIRE(uint32_t{true} == buffer[6]);
   
        REQUIRE(uint32_t{0x98} == buffer[7]);
        REQUIRE(uint32_t{0xBA} == buffer[8]);
        REQUIRE(uint32_t{0xDC} == buffer[9]);
        REQUIRE(uint32_t{0xFE} == buffer[10]);
      }
    
      SECTION("throws out_of_range if the buffer is too small") {
        const auto write_to_buffer = [&]() { io::write(buffer, a, io::big_endian{b}, c, d, a); };
        REQUIRE_THROWS_AS(write_to_buffer(), std::out_of_range);
      }
    }
    
    SECTION("range values") {
      auto buffer = std::vector<unsigned char>(16, 0);
    
      SECTION("dynamic ranges") {
        const auto values         = {uint32_t{0x12345678}, uint32_t{0xABCDEF77}, uint32_t{0xCDCDCDCD}, uint32_t{0x17456434}};
        constexpr auto value_size = sizeof(decltype(values)::value_type);
    
        SECTION("writes the correct number of bytes") {
          REQUIRE(value_size * values.size() == io::write(buffer, values));
    
          SECTION("and the bytes have the right values") {
            REQUIRE(0x78 == buffer[0]);
            REQUIRE(0x56 == buffer[1]);
            REQUIRE(0x34 == buffer[2]);
            REQUIRE(0x12 == buffer[3]);

            REQUIRE(0x77 == buffer[4]);
            REQUIRE(0xEF == buffer[5]);
            REQUIRE(0xCD == buffer[6]);
            REQUIRE(0xAB == buffer[7]);

            REQUIRE(0xCD == buffer[8]);
            REQUIRE(0xCD == buffer[9]);
            REQUIRE(0xCD == buffer[10]);
            REQUIRE(0xCD == buffer[11]);

            REQUIRE(0x34 == buffer[12]);
            REQUIRE(0x64 == buffer[13]);
            REQUIRE(0x45 == buffer[14]);
            REQUIRE(0x17 == buffer[15]);
          }
        }
    
        SECTION("writes R-value ranges correctly") {
          REQUIRE(3 * sizeof(uint32_t) == io::write(buffer, std::vector<uint32_t>{1, 2, 3}));

          REQUIRE(0x01 == buffer[0]);
          REQUIRE(0x00 == buffer[1]);
          REQUIRE(0x00 == buffer[2]);
          REQUIRE(0x00 == buffer[3]);

          REQUIRE(0x02 == buffer[4]);
          REQUIRE(0x00 == buffer[5]);
          REQUIRE(0x00 == buffer[6]);
          REQUIRE(0x00 == buffer[7]);

          REQUIRE(0x03 == buffer[8]);
          REQUIRE(0x00 == buffer[9]);
          REQUIRE(0x00 == buffer[10]);
          REQUIRE(0x00 == buffer[11]);
        }
      }
    
      SECTION("static ranges") {
        const auto values = std::array{uint32_t{0x12345678}, uint32_t{0xABCDEF77}, uint32_t{0xCDCDCDCD}, uint32_t{0x17456434}};
        constexpr auto value_size = sizeof(decltype(values)::value_type);
        auto buffer_1             = std::array<uint8_t, 4 * value_size>{};
    
        SECTION("writes the correct number of bytes") {
          REQUIRE(value_size * values.size() == io::write(buffer_1, values));
    
          SECTION("and the bytes have the right values") {
            REQUIRE(0x78 == buffer_1[0]);
            REQUIRE(0x56 == buffer_1[1]);
            REQUIRE(0x34 == buffer_1[2]);
            REQUIRE(0x12 == buffer_1[3]);
    
            REQUIRE(0x77 == buffer_1[4]);
            REQUIRE(0xEF == buffer_1[5]);
            REQUIRE(0xCD == buffer_1[6]);
            REQUIRE(0xAB == buffer_1[7]);
    
            REQUIRE(0xCD == buffer_1[8]);
            REQUIRE(0xCD == buffer_1[9]);
            REQUIRE(0xCD == buffer_1[10]);
            REQUIRE(0xCD == buffer_1[11]);
    
            REQUIRE(0x34 == buffer_1[12]);
            REQUIRE(0x64 == buffer_1[13]);
            REQUIRE(0x45 == buffer_1[14]);
            REQUIRE(0x17 == buffer_1[15]);
          }
        }
      }
    
      SECTION("throws std::out_of_range if the buffer is too small") {
        const auto too_many_values = {1, 2, 3, 4, 5};
        REQUIRE_THROWS_AS(io::write(buffer, too_many_values), std::out_of_range);
      }
    
      SECTION("can be used with multiple value overloads") {
        const auto a = int32_t{3};
        const auto b = std::vector<uint16_t>{1, 2, 3};
        const auto c = '9';
    
        REQUIRE(11 == io::write(buffer, a, b, c));
    
        REQUIRE(0x03 == buffer[0]);
        REQUIRE(0x00 == buffer[1]);
        REQUIRE(0x00 == buffer[2]);
        REQUIRE(0x00 == buffer[3]);
    
        REQUIRE(0x01 == buffer[4]);
        REQUIRE(0x00 == buffer[5]);
        REQUIRE(0x02 == buffer[6]);
        REQUIRE(0x00 == buffer[7]);
        REQUIRE(0x03 == buffer[8]);
        REQUIRE(0x00 == buffer[9]);
    
        REQUIRE(0x39 == buffer[10]);
      }
    }
  }

  SECTION("write_at") {
    SECTION("single value") {
      auto data = std::array<uint8_t, 18>{};
  
      SECTION("directly to data writes at the correct position") {
        const auto val = double{3.14156e+10};
        const auto pos = ptrdiff_t{3};
  
        REQUIRE(pos + sizeof(val) == io::write_at(pos, data, val));
  
        // REQUIRE(val == io::read<double>({std::next(data.begin(), pos), data.end()}));
  
        REQUIRE(0x00 == data[0]);
        REQUIRE(0x00 == data[1]);
        REQUIRE(0x00 == data[2]);
        REQUIRE(0x00 == data[pos + sizeof(val)]);
      }
  
      SECTION("raises exception if writing past the end of a buffer") {
        const auto pos = 1 + data.size() - sizeof(double);
        REQUIRE_THROWS_AS(io::write_at(pos, data, double{}), std::out_of_range);
      }
  
      SECTION("raises exception if starting past the end of a buffer") {
        const auto pos = 1 + data.size();
        REQUIRE_THROWS_AS(io::write_at(pos, data, double{}), std::out_of_range);
      }
  
      SECTION("handles large and pathological offset") {
        REQUIRE_THROWS_AS(io::write_at(std::numeric_limits<size_t>::max() - sizeof(double) + 1, data, double{}),
                          std::invalid_argument);
      }
  
      SECTION("range values are written correctly") {
        const auto values         = {uint32_t{0x12345678}, uint32_t{0xABCDEF77}, uint32_t{0xCDCDCDCD}, uint32_t{0x17456434}};
        constexpr auto value_size = sizeof(decltype(values)::value_type);
  
        const auto pos = size_t{2};
  
        REQUIRE(pos + value_size * values.size() == io::write_at(pos, data, values));
  
        REQUIRE(0x78 == data[2]);
        REQUIRE(0x56 == data[3]);
        REQUIRE(0x34 == data[4]);
        REQUIRE(0x12 == data[5]);
  
        REQUIRE(0x77 == data[6]);
        REQUIRE(0xEF == data[7]);
        REQUIRE(0xCD == data[8]);
        REQUIRE(0xAB == data[9]);
  
        REQUIRE(0xCD == data[10]);
        REQUIRE(0xCD == data[11]);
        REQUIRE(0xCD == data[12]);
        REQUIRE(0xCD == data[13]);
  
        REQUIRE(0x34 == data[14]);
        REQUIRE(0x64 == data[15]);
        REQUIRE(0x45 == data[16]);
        REQUIRE(0x17 == data[17]);
      }
  
      SECTION("R-value range values are written correctly") {
        const auto values         = {uint32_t{0x12345678}, uint32_t{0xABCDEF77}, uint32_t{0xCDCDCDCD}, uint32_t{0x17456434}};
        constexpr auto value_size = sizeof(decltype(values)::value_type);
  
        const auto pos = size_t{2};
  
        REQUIRE(pos + value_size * values.size() ==
                io::write_at(pos,
                             data,
                             std::vector<uint32_t>{
                                 uint32_t{0x12345678}, uint32_t{0xABCDEF77}, uint32_t{0xCDCDCDCD}, uint32_t{0x17456434}}));
  
        REQUIRE(0x78 == data[2]);
        REQUIRE(0x56 == data[3]);
        REQUIRE(0x34 == data[4]);
        REQUIRE(0x12 == data[5]);
  
        REQUIRE(0x77 == data[6]);
        REQUIRE(0xEF == data[7]);
        REQUIRE(0xCD == data[8]);
        REQUIRE(0xAB == data[9]);
  
        REQUIRE(0xCD == data[10]);
        REQUIRE(0xCD == data[11]);
        REQUIRE(0xCD == data[12]);
        REQUIRE(0xCD == data[13]);
  
        REQUIRE(0x34 == data[14]);
        REQUIRE(0x64 == data[15]);
        REQUIRE(0x45 == data[16]);
        REQUIRE(0x17 == data[17]);
      }
    }
  
    SECTION("multiple values") {
      const auto a = uint32_t{0x12345678};
      const auto b = uint16_t{0xABCD};
      const auto c = std::list<double>{
          std::numbers::pi,
          std::numbers::e,
      };
      const auto d = true;
      const auto e = uint32_t{0xFEDCBA98};
  
      constexpr auto data_size = sizeof(a) + sizeof(b) + 2 * sizeof(double) + sizeof(d) + sizeof(e);
  
      auto buffer = std::vector<unsigned char>(1 + data_size, 0);
  
      SECTION("returns number of bytes written on success") {
        REQUIRE(1 + data_size == io::write_at(1, buffer, a, io::big_endian{b}, c, d, e));
  
        SECTION("and writes the correct data") {
          // a
          REQUIRE(uint32_t{0x78} == buffer[1]);
          REQUIRE(uint32_t{0x56} == buffer[2]);
          REQUIRE(uint32_t{0x34} == buffer[3]);
          REQUIRE(uint32_t{0x12} == buffer[4]);
  
          // b
          REQUIRE(uint32_t{0xAB} == buffer[5]);
          REQUIRE(uint32_t{0xCD} == buffer[6]);
  
          // c.0
          REQUIRE(extract_byte(std::numbers::pi, 0) == buffer[7]);
          REQUIRE(extract_byte(std::numbers::pi, 1) == buffer[8]);
          REQUIRE(extract_byte(std::numbers::pi, 2) == buffer[9]);
          REQUIRE(extract_byte(std::numbers::pi, 3) == buffer[10]);
          REQUIRE(extract_byte(std::numbers::pi, 4) == buffer[11]);
          REQUIRE(extract_byte(std::numbers::pi, 5) == buffer[12]);
          REQUIRE(extract_byte(std::numbers::pi, 6) == buffer[13]);
          REQUIRE(extract_byte(std::numbers::pi, 7) == buffer[14]);
  
          // c.1
          REQUIRE(extract_byte(std::numbers::e, 0) == buffer[15]);
          REQUIRE(extract_byte(std::numbers::e, 1) == buffer[16]);
          REQUIRE(extract_byte(std::numbers::e, 2) == buffer[17]);
          REQUIRE(extract_byte(std::numbers::e, 3) == buffer[18]);
          REQUIRE(extract_byte(std::numbers::e, 4) == buffer[19]);
          REQUIRE(extract_byte(std::numbers::e, 5) == buffer[20]);
          REQUIRE(extract_byte(std::numbers::e, 6) == buffer[21]);
          REQUIRE(extract_byte(std::numbers::e, 7) == buffer[22]);
  
          // d
          REQUIRE(uint32_t{true} == buffer[23]);
  
          // e
          REQUIRE(uint32_t{0x98} == buffer[24]);
          REQUIRE(uint32_t{0xBA} == buffer[25]);
          REQUIRE(uint32_t{0xDC} == buffer[26]);
          REQUIRE(uint32_t{0xFE} == buffer[27]);
        }
      }
  
      SECTION("throws out_of_range if the buffer is too small") {
        const auto write_to_buffer = [&]() { io::write_at(2, buffer, a, io::big_endian{b}, c, d, a); };
        REQUIRE_THROWS_AS(write_to_buffer(), std::out_of_range);
      }
    }
  }
  
  SECTION("try_write") {
    SECTION("single value") {
      SECTION("returns number of bytes written on good write") {
        auto data = std::array<uint8_t, 20>{};
  
        SECTION("with default endianness") {
          const auto val    = uint32_t{0xFE01CD23};
          const auto result = io::try_write(data, val);
  
          REQUIRE(result.ok());
          REQUIRE(sizeof(val) == result.value());
  
          REQUIRE(uint32_t{0x23} == data[0]);
          REQUIRE(uint32_t{0xCD} == data[1]);
          REQUIRE(uint32_t{0x01} == data[2]);
          REQUIRE(uint32_t{0xFE} == data[3]);
        }
  
        SECTION("with specified endianness") {
          const auto val    = uint32_t{0x23CD01FE};
          const auto result = io::try_write(data, io::big_endian{val});
  
          REQUIRE(result.ok());
          REQUIRE(sizeof(val) == result.value());
  
          REQUIRE(uint32_t{0x23} == data[0]);
          REQUIRE(uint32_t{0xCD} == data[1]);
          REQUIRE(uint32_t{0x01} == data[2]);
          REQUIRE(uint32_t{0xFE} == data[3]);
        }
  
        SECTION("range values") {
          SECTION("static ranges") {
            const auto values = std::array{1.123f, 2.456f, 3.789f};
            const auto result = io::try_write(data, values);
  
            REQUIRE(result.ok());
            REQUIRE(sizeof(float) * values.size() == result.value());
  
            REQUIRE(extract_byte(values[0], 0) == data[0]);
            REQUIRE(extract_byte(values[0], 1) == data[1]);
            REQUIRE(extract_byte(values[0], 2) == data[2]);
            REQUIRE(extract_byte(values[0], 3) == data[3]);
  
            REQUIRE(extract_byte(values[1], 0) == data[4]);
            REQUIRE(extract_byte(values[1], 1) == data[5]);
            REQUIRE(extract_byte(values[1], 2) == data[6]);
            REQUIRE(extract_byte(values[1], 3) == data[7]);
  
            REQUIRE(extract_byte(values[2], 0) == data[8]);
            REQUIRE(extract_byte(values[2], 1) == data[9]);
            REQUIRE(extract_byte(values[2], 2) == data[10]);
            REQUIRE(extract_byte(values[2], 3) == data[11]);
          }
  
          SECTION("dynamoc ranges") {
            const auto values = std::vector{1.123f, 2.456f, 3.789f};
            const auto result = io::try_write(data, values);
  
            REQUIRE(result.ok());
            REQUIRE(sizeof(float) * values.size() == result.value());
  
            REQUIRE(extract_byte(values[0], 0) == data[0]);
            REQUIRE(extract_byte(values[0], 1) == data[1]);
            REQUIRE(extract_byte(values[0], 2) == data[2]);
            REQUIRE(extract_byte(values[0], 3) == data[3]);
  
            REQUIRE(extract_byte(values[1], 0) == data[4]);
            REQUIRE(extract_byte(values[1], 1) == data[5]);
            REQUIRE(extract_byte(values[1], 2) == data[6]);
            REQUIRE(extract_byte(values[1], 3) == data[7]);
  
            REQUIRE(extract_byte(values[2], 0) == data[8]);
            REQUIRE(extract_byte(values[2], 1) == data[9]);
            REQUIRE(extract_byte(values[2], 2) == data[10]);
            REQUIRE(extract_byte(values[2], 3) == data[11]);
          }
  
          SECTION("R-value ranges") {
            const auto result = io::try_write(data, std::vector{1.123f, 2.456f, 3.789f});
  
            REQUIRE(result.ok());
  
            const auto values = std::vector{1.123f, 2.456f, 3.789f};
            REQUIRE(sizeof(float) * values.size() == result.value());
  
            REQUIRE(extract_byte(values[0], 0) == data[0]);
            REQUIRE(extract_byte(values[0], 1) == data[1]);
            REQUIRE(extract_byte(values[0], 2) == data[2]);
            REQUIRE(extract_byte(values[0], 3) == data[3]);
  
            REQUIRE(extract_byte(values[1], 0) == data[4]);
            REQUIRE(extract_byte(values[1], 1) == data[5]);
            REQUIRE(extract_byte(values[1], 2) == data[6]);
            REQUIRE(extract_byte(values[1], 3) == data[7]);
  
            REQUIRE(extract_byte(values[2], 0) == data[8]);
            REQUIRE(extract_byte(values[2], 1) == data[9]);
            REQUIRE(extract_byte(values[2], 2) == data[10]);
            REQUIRE(extract_byte(values[2], 3) == data[11]);
          }
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
      const auto e = std::vector{uint32_t{1}, uint32_t{2}, uint32_t{3}, uint32_t{4}};
  
      constexpr auto data_size = sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d) + sizeof(uint32_t) * 4;
  
      auto buffer = std::vector<unsigned char>(data_size, 0);
  
      SECTION("returns the number of bytes written") {
        const auto result = io::try_write(buffer, a, io::big_endian{b}, c, d, e);
        REQUIRE(result.ok());
        REQUIRE(data_size == result.value());
  
        SECTION("and writes the bytes correctly") {
          // a
          REQUIRE(uint32_t{0x78} == buffer[0]);
          REQUIRE(uint32_t{0x56} == buffer[1]);
          REQUIRE(uint32_t{0x34} == buffer[2]);
          REQUIRE(uint32_t{0x12} == buffer[3]);
  
          // b
          REQUIRE(uint32_t{0xAB} == buffer[4]);
          REQUIRE(uint32_t{0xCD} == buffer[5]);
  
          // c
          REQUIRE(uint32_t{true} == buffer[6]);
  
          // d
          REQUIRE(uint32_t{0x98} == buffer[7]);
          REQUIRE(uint32_t{0xBA} == buffer[8]);
          REQUIRE(uint32_t{0xDC} == buffer[9]);
          REQUIRE(uint32_t{0xFE} == buffer[10]);
  
          // e.0
          REQUIRE(uint32_t{0x01} == buffer[11]);
          REQUIRE(uint32_t{0x00} == buffer[12]);
          REQUIRE(uint32_t{0x00} == buffer[13]);
          REQUIRE(uint32_t{0x00} == buffer[14]);
  
          // e.1
          REQUIRE(uint32_t{0x02} == buffer[15]);
          REQUIRE(uint32_t{0x00} == buffer[16]);
          REQUIRE(uint32_t{0x00} == buffer[17]);
          REQUIRE(uint32_t{0x00} == buffer[18]);
  
          // e.2
          REQUIRE(uint32_t{0x03} == buffer[19]);
          REQUIRE(uint32_t{0x00} == buffer[20]);
          REQUIRE(uint32_t{0x00} == buffer[21]);
          REQUIRE(uint32_t{0x00} == buffer[22]);
  
          // e.3
          REQUIRE(uint32_t{0x04} == buffer[23]);
          REQUIRE(uint32_t{0x00} == buffer[24]);
          REQUIRE(uint32_t{0x00} == buffer[25]);
          REQUIRE(uint32_t{0x00} == buffer[26]);
        }
      }
  
      SECTION("returns error if the buffer is too small") {
        const auto result = io::try_write(buffer, a, io::big_endian{b}, c, d, e, int{});
  
        REQUIRE(result.is_error());
        REQUIRE(io::write_error::insufficient_buffer == result.error());
      }
    }
  }
  
  SECTION("try_write_at") {
    SECTION("single value") {
      auto data = std::array<uint8_t, 12>{};
  
      SECTION("directly to data writes at the correct position") {
        const auto val    = double{3.14156e+10};
        const auto pos    = ptrdiff_t{3};
        const auto result = io::try_write_at(pos, data, val);
        REQUIRE(result.ok());
        REQUIRE(pos + sizeof(val) == result.value());
  
        // REQUIRE(val == io::read<double>({std::next(data.begin(), pos), data.end()}));
  
        REQUIRE(0x00 == data[0]);
        REQUIRE(0x00 == data[1]);
        REQUIRE(0x00 == data[2]);
        REQUIRE(0x00 == data[pos + sizeof(val)]);
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
  
      SECTION("returns error if the buffer is too small") {
        const auto result = io::try_write_at(2, buffer, a, io::big_endian{b}, c, d);
  
        REQUIRE(result.is_error());
        REQUIRE(io::write_error::insufficient_buffer == result.error());
      }
    }
  }
}
