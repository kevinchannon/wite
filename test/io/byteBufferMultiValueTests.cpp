#include <wite/io/byte_buffer.hpp>
#include <wite/io/byte_stream.hpp>  // This is here to make sure that things build in eachothers presence.
#include <wite/io/types.hpp>


#include "../utils.hpp"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <tuple>

using namespace std::string_literals;

using namespace wite;

TEST_CASE("Write multiple values to buffer", "[buffer_io]") {

  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;
  const auto d = uint32_t{0xFEDCBA98};

  auto buffer  = io::static_byte_buffer<sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)>{};

  const auto [test_name, write_to_buffer] =
      GENERATE_REF(table<std::string, std::function<size_t()>>({
        {"Direct buffer access"s, [&]() -> size_t { return io::write(buffer, a, io::big_endian{b}, c, d); }},
        {"Via write buffer view"s, [&]() -> size_t { 
          auto write_view = io::byte_write_buffer_view{buffer};
          return io::write(write_view, a, io::big_endian{b}, c, d);
        }}}));

  SECTION(test_name) {
    REQUIRE(11 == write_to_buffer());

    REQUIRE(uint32_t{0x78} == test::to_integer<uint32_t>(buffer[ 0]));
    REQUIRE(uint32_t{0x56} == test::to_integer<uint32_t>(buffer[ 1]));
    REQUIRE(uint32_t{0x34} == test::to_integer<uint32_t>(buffer[ 2]));
    REQUIRE(uint32_t{0x12} == test::to_integer<uint32_t>(buffer[ 3]));

    REQUIRE(uint32_t{0xAB} == test::to_integer<uint32_t>(buffer[ 4]));
    REQUIRE(uint32_t{0xCD} == test::to_integer<uint32_t>(buffer[ 5]));

    REQUIRE(uint32_t{true} == test::to_integer<uint32_t>(buffer[ 6]));

    REQUIRE(uint32_t{0x98} == test::to_integer<uint32_t>(buffer[ 7]));
    REQUIRE(uint32_t{0xBA} == test::to_integer<uint32_t>(buffer[ 8]));
    REQUIRE(uint32_t{0xDC} == test::to_integer<uint32_t>(buffer[ 9]));
    REQUIRE(uint32_t{0xFE} == test::to_integer<uint32_t>(buffer[10]));
  }
}

TEST_CASE("Write multiple values from buffer throws out_of_range if the buffer is too small", "[buffer_io]") {
  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;
  const auto d = uint32_t{0xFEDCBA98};

  auto buffer = io::static_byte_buffer<sizeof(a) + sizeof(b) + sizeof(c)>{};

  const auto [test_name, write_to_buffer] = GENERATE_REF(table<std::string, std::function<void()>>(
      {{"Direct buffer access"s, [&]() { io::write(buffer, a, io::big_endian{b}, c, d); }},
       {"Via write buffer view"s, [&]() {
          auto write_view = io::byte_write_buffer_view{buffer};
          io::write(write_view, a, io::big_endian{b}, c, d);
        }}}));

  SECTION(test_name) {
    REQUIRE_THROWS_AS(write_to_buffer(), std::out_of_range);
  }
}

TEST_CASE("Read multiple values from buffer", "[buffer_io]") {
  // clang-format off
  const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
    io::byte{0x78}, io::byte{0x56}, io::byte{0x34}, io::byte{0x12},
    io::byte{0xAB}, io::byte{0xCD},
    io::byte{true},
    io::byte{0x98}, io::byte{0xBA}, io::byte{0xDC}, io::byte{0xFE}
  };
  // clang-format on

  using Result_t = std::tuple<uint32_t, uint16_t, bool, uint32_t>;

  const auto [test_name, read_from_buffer] =
      GENERATE_REF(table<std::string, std::function<Result_t()>>(
      {{"Direct buffer access"s,
        [&]() -> Result_t {
              return io::read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(buffer); }},
       {"Via read buffer view"s, [&]() -> Result_t {
              auto read_view = io::byte_read_buffer_view{buffer};
              return io::read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(read_view);
            }}}));

  SECTION(test_name) {
    const auto [a, b, c, d] = read_from_buffer();

    REQUIRE( a == uint32_t{0x12345678});
    REQUIRE( b == uint16_t{0xABCD});
    REQUIRE( c == true);
    REQUIRE( d == uint32_t{0xFEDCBA98});
  }
}

TEST_CASE("Read multiple values from buffer throws out_of_range if the buffer is too small", "[buffer_io]") {
  // clang-format off
  const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool)>{
    io::byte{0x78}, io::byte{0x56}, io::byte{0x34}, io::byte{0x12},
    io::byte{0xAB}, io::byte{0xCD},
    io::byte{true}
  };
  // clang-format on

  using Result_t = std::tuple<uint32_t, uint16_t, bool, uint32_t>;

  const auto [test_name, read_from_buffer] = GENERATE_REF(table<std::string, std::function<Result_t()>>(
      {{"Direct buffer access"s,
        [&]() -> Result_t { return io::read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(buffer); }},
       {"Via read buffer view"s, [&]() -> Result_t {
          auto read_view = io::byte_read_buffer_view{buffer};
          return io::read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(read_view);
        }}}));

  SECTION(test_name) {
    REQUIRE_THROWS_AS(read_from_buffer(), std::out_of_range);
  }
}

TEST_CASE("Try read multiple values from buffer", "[buffer_io]") {
  // clang-format off
  const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t) + sizeof(bool) + sizeof(uint32_t)>{
    io::byte{0x78}, io::byte{0x56}, io::byte{0x34}, io::byte{0x12},
    io::byte{0xAB}, io::byte{0xCD},
    io::byte{true},
    io::byte{0x98}, io::byte{0xBA}, io::byte{0xDC}, io::byte{0xFE}
  };
  // clang-format on

  using Result_t = std::tuple<
    io::read_result_t<uint32_t>,
    io::read_result_t<uint16_t>,
    io::read_result_t<bool>,
    io::read_result_t<uint32_t>>;

  const auto [test_name, read_from_buffer] = GENERATE_REF(table<std::string, std::function<Result_t()>>(
      {{"Direct buffer access"s,
        [&]() -> Result_t { return io::try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(buffer); }},
       {"Via read buffer view"s, [&]() -> Result_t {
          auto read_view = io::byte_read_buffer_view{buffer};
          return io::try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(read_view);
        }}}));

  SECTION(test_name) {
    const auto [a, b, c, d] = read_from_buffer();

    REQUIRE(a.ok());
    REQUIRE(a.value() == uint32_t{0x12345678});

    REQUIRE(b.ok());
    REQUIRE(b.value() == uint16_t{0xABCD});

    REQUIRE(c.ok());
    REQUIRE(c.value() == true);

    REQUIRE(d.ok());
    REQUIRE(d.value() == uint32_t{0xFEDCBA98});
  }
}

TEST_CASE("Try read multiple values inserts errors if the buffer is too small", "[buffer_io]") {
  // clang-format off
  const auto buffer = io::static_byte_buffer<sizeof(uint32_t) + sizeof(uint16_t)>{
    io::byte{0x78}, io::byte{0x56}, io::byte{0x34}, io::byte{0x12},
    io::byte{0xAB}, io::byte{0xCD}
  };
  // clang-format on

  using Result_t =
      std::tuple<io::read_result_t<uint32_t>, io::read_result_t<uint16_t>, io::read_result_t<bool>, io::read_result_t<uint32_t>>;

  const auto [test_name, read_from_buffer] = GENERATE_REF(table<std::string, std::function<Result_t()>>(
      {{"Direct buffer access"s,
        [&]() -> Result_t { return io::try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(buffer); }},
       {"Via read buffer view"s, [&]() -> Result_t {
          auto read_view = io::byte_read_buffer_view{buffer};
          return io::try_read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>(read_view);
        }}}));

  SECTION(test_name) {
    const auto [a, b, c, d] = read_from_buffer();

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

TEST_CASE("Try write multiple values to buffer", "[buffer_io]") {
  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;
  const auto d = uint32_t{0xFEDCBA98};

  auto buffer = io::static_byte_buffer<sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d) + 1>{};

  using Result_t = io::write_result_t;

  const auto [test_name, write_to_buffer, offset, expected] = GENERATE_REF(table<std::string, std::function<Result_t(size_t)>, size_t, size_t>(
      {
        {
          "Direct buffer access"s,
          [&](size_t) -> Result_t { return io::try_write(buffer, a, io::big_endian{b}, c, d); },
          0,
          sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)
        },
        {
          "Via write buffer view"s,
          [&](size_t) -> Result_t {
            auto write_view = io::byte_write_buffer_view{buffer};
            return io::try_write(write_view, a, io::big_endian{b}, c, d);
          },
          0,
          sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)
        },
        {
          "Direct access at offset"s,
          [&](size_t pos) -> Result_t { return io::try_write_at(pos, buffer, a, io::big_endian{b}, c, d); },
          1,
          1 + sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)},
         {
           "Via write buffer view at offset"s,
            [&](size_t pos) -> Result_t {
              auto write_view = io::byte_write_buffer_view{buffer};
              return io::try_write_at(pos, write_view, a, io::big_endian{b}, c, d);
            },
           1,
           1 + sizeof(a) + sizeof(b) + sizeof(c) + sizeof(d)}
      }));

  SECTION(test_name) {
    const auto result = write_to_buffer(offset);
    REQUIRE(result.ok());
    REQUIRE(expected == result.value());

    REQUIRE(uint32_t{0x78} == test::to_integer<uint32_t>(buffer[offset + 0]));
    REQUIRE(uint32_t{0x56} == test::to_integer<uint32_t>(buffer[offset + 1]));
    REQUIRE(uint32_t{0x34} == test::to_integer<uint32_t>(buffer[offset + 2]));
    REQUIRE(uint32_t{0x12} == test::to_integer<uint32_t>(buffer[offset + 3]));

    REQUIRE(uint32_t{0xAB} == test::to_integer<uint32_t>(buffer[offset + 4]));
    REQUIRE(uint32_t{0xCD} == test::to_integer<uint32_t>(buffer[offset + 5]));

    REQUIRE(uint32_t{true} == test::to_integer<uint32_t>(buffer[offset + 6]));

    REQUIRE(uint32_t{0x98} == test::to_integer<uint32_t>(buffer[offset + 7]));
    REQUIRE(uint32_t{0xBA} == test::to_integer<uint32_t>(buffer[offset + 8]));
    REQUIRE(uint32_t{0xDC} == test::to_integer<uint32_t>(buffer[offset + 9]));
    REQUIRE(uint32_t{0xFE} == test::to_integer<uint32_t>(buffer[offset + 10]));
  }
}

TEST_CASE("try_write via byte_write_buffer_view increments write position correctly") {
  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;

  auto data = io::static_byte_buffer<sizeof(a) + sizeof(b) + sizeof(c)>{};
  auto buffer = io::byte_write_buffer_view{data};

  SECTION("when writing succeeds") {
    const auto result = io::try_write(buffer, a, b, c);
    REQUIRE(result.ok());
    REQUIRE(std::next(buffer.data.begin(), sizeof(a) + sizeof(b) + sizeof(c)) == buffer.write_position);
  }

  SECTION("when writing fails") {
    const auto result = io::try_write(buffer, a, b, c, int{});
    REQUIRE(result.is_error());
    REQUIRE(buffer.data.begin() == buffer.write_position);
  }
}

TEST_CASE("try_write_at via byte_write_buffer_view increments write position correctly") {
  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;

  constexpr auto offset = size_t{3};

  auto data   = io::static_byte_buffer<sizeof(a) + sizeof(b) + sizeof(c) + offset>{};
  auto buffer = io::byte_write_buffer_view{data};

  SECTION("when writing succeeds") {
    const auto result = io::try_write_at(offset, buffer, a, b, c);
    REQUIRE(result.ok());
    REQUIRE(std::next(buffer.data.begin(), offset + sizeof(a) + sizeof(b) + sizeof(c)) == buffer.write_position);
  }

  SECTION("when writing fails") {
    const auto result = io::try_write_at(offset, buffer, a, b, c, int{});
    REQUIRE(result.is_error());
    REQUIRE(buffer.data.begin() == buffer.write_position);
  }
}

TEST_CASE("Try write multiple values from buffer returns error if the buffer is too small", "[buffer_io]") {
  const auto a = uint32_t{0x12345678};
  const auto b = uint16_t{0xABCD};
  const auto c = true;
  const auto d = uint32_t{0xFEDCBA98};

  auto buffer = io::static_byte_buffer<sizeof(a) + sizeof(b)>{};

  using Result_t = io::write_result_t;

  const auto [test_name, write_to_buffer] = GENERATE_REF(table<std::string, std::function<Result_t()>>(
      {{"Direct buffer access"s, [&]() -> Result_t { return io::try_write(buffer, a, io::big_endian{b}, c, d); }},
       {"Via write buffer view"s, [&]() -> Result_t {
          auto write_view = io::byte_write_buffer_view{buffer};
          return io::try_write(write_view, a, io::big_endian{b}, c, d);
        }}}));

  SECTION(test_name) {
    const auto result = write_to_buffer();

    REQUIRE(result.is_error());
    REQUIRE(io::write_error::insufficient_buffer == result.error());
  }
}
