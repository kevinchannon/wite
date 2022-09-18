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

TEST_CASE("Values from vector buffer", "[buffer_io]") {
  const auto vec_buffer = std::vector<io::byte>{io::byte{0x67},
                                                io::byte{0x45},
                                                io::byte{0x23},
                                                io::byte{0x01},
                                                io::byte{0xEF},
                                                io::byte{0xCD},
                                                io::byte{0xAB},
                                                io::byte{0x89}};

  SECTION("Little-endian") {
    SECTION("Read int") {
      REQUIRE(uint32_t(0x01234567) == io::read<uint32_t>(vec_buffer, io::endian::little));
      REQUIRE(uint32_t(0x01234567) == io::read<io::little_endian<uint32_t>>(vec_buffer));
    }

    SECTION("Read 2 shorts") {
      REQUIRE(uint32_t(0x4567) == io::read<uint16_t>(vec_buffer, io::endian::little));
      REQUIRE(uint32_t(0x4567) == io::read<io::little_endian<uint16_t>>(vec_buffer));
      REQUIRE(uint32_t(0x0123) == io::read<io::little_endian<uint16_t>>({std::next(vec_buffer.begin(), 2), vec_buffer.end()}));
    }

    SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
      const auto read_buf = std::span{std::next(vec_buffer.begin(), 6), vec_buffer.end()};
      REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf, io::endian::little), std::out_of_range);
      REQUIRE_THROWS_AS(io::read<io::little_endian<uint32_t>>(read_buf), std::out_of_range);
    }
  }

  SECTION("Big-endian") {
    SECTION("Read int") {
      REQUIRE(uint32_t(0x67452301) == io::read<uint32_t>(vec_buffer, io::endian::big));
      REQUIRE(uint32_t(0x67452301) == io::read<io::big_endian<uint32_t>>(vec_buffer));
    }

    SECTION("Read 2 shorts") {
      REQUIRE(uint32_t(0x6745) == io::read<uint16_t>(vec_buffer, io::endian::big));
      REQUIRE(uint32_t(0x6745) == io::read<io::big_endian<uint16_t>>(vec_buffer));
      REQUIRE(uint32_t(0x2301) == io::read<uint16_t>({std::next(vec_buffer.begin(), 2), vec_buffer.end()}, io::endian::big));
    }

    SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
      const auto read_buf = std::span{std::next(vec_buffer.begin(), 6), vec_buffer.end()};
      REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf, io::endian::big), std::out_of_range);
      REQUIRE_THROWS_AS(io::read<io::big_endian<uint32_t>>(read_buf), std::out_of_range);
    }
  }
}

TEST_CASE("Values from pointers to array", "[buffer_io]") {
  constexpr auto buf_size              = 8;
  const io::byte data_buffer[buf_size] = {io::byte{0x67},
                                          io::byte{0x45},
                                          io::byte{0x23},
                                          io::byte{0x01},
                                          io::byte{0xEF},
                                          io::byte{0xCD},
                                          io::byte{0xAB},
                                          io::byte{0x89}};

  const io::byte* buf_begin = data_buffer;
  const io::byte* buf_end   = buf_begin + buf_size;

  SECTION("Read int") {
    REQUIRE(uint32_t(0x01234567) == io::read<uint32_t>({buf_begin, buf_end}));
  }

  SECTION("Read 2 shorts") {
    REQUIRE(uint32_t(0x4567) == io::read<uint16_t>({buf_begin, buf_end}));
    REQUIRE(uint32_t(0x0123) == io::read<uint16_t>({std::next(buf_begin, 2), buf_end}));
  }

  SECTION("Read past the end of the buffer fails with std::out_of_range exception") {
    const auto read_buf = std::span{std::next(buf_begin, 6), buf_end};
    REQUIRE_THROWS_AS(io::read<uint32_t>(read_buf), std::out_of_range);
  }
}

TEST_CASE("try_read returns value on good read", "[buffer_io]") {
  SECTION("From raw buffer") {
    const auto data = io::static_byte_buffer<4>{io::byte{0x67}, io::byte{0x45}, io::byte{0xAB}, io::byte{0xFF}};

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
  }

  SECTION("Via byte_read_buffer_view") {
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

TEST_CASE("try_read returns error on bad read", "[buffer_io]") {
  const auto data = io::static_byte_buffer<3>{io::byte{0x67}, io::byte{0x45}, io::byte{0xAB}};

  const auto val = io::try_read<uint32_t>(data);
  REQUIRE(val.is_error());
  REQUIRE(io::read_error::insufficient_buffer == val.error());
}

TEST_CASE("unchecked_read returns value and next read position", "[buffer_io]") {
  const auto data = io::static_byte_buffer<8>{io::byte{0x67},
                                              io::byte{0x45},
                                              io::byte{0x23},
                                              io::byte{0x01},
                                              io::byte{0xEF},
                                              io::byte{0xCD},
                                              io::byte{0xAB},
                                              io::byte{0x89}};

  const io::byte* buf = data.data();

  const auto [value, next] = io::unchecked_read<uint32_t>(buf);

  REQUIRE(uint32_t{0x01234567} == value);
  REQUIRE(buf + 4 == next);
}

TEST_CASE("Direct buffer read_at", "[buffer_io]") {
  const auto data = io::dynamic_byte_buffer{io::byte{0x67},
                                            io::byte{0x45},
                                            io::byte{0x23},
                                            io::byte{0x01},
                                            io::byte{0xEF},
                                            io::byte{0xCD},
                                            io::byte{0xAB},
                                            io::byte{0x89}};

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

TEST_CASE("read_at via byte_read_buffer_view", "[buffer_io]") {
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
