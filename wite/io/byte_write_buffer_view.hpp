#pragma once

#include <wite/io/byte_buffer_write.hpp>
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/concepts.hpp>

#include <bit>
#include <cstddef>
#include <iterator>
#include <span>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

struct byte_write_buffer_view {
  explicit byte_write_buffer_view(std::span<std::byte> buf) : data{std::move(buf)}, write_position{data.begin()} {}

  byte_write_buffer_view(std::span<std::byte> buf, typename std::span<const std::byte>::size_type offset)
      : data{std::move(buf)}, write_position{std::next(data.begin(), offset)} {}

  std::span<std::byte> data;
  std::span<std::byte>::iterator write_position;
};

///////////////////////////////////////////////////////////////////////////////

template <std::endian ENDIANNESS = std::endian::native, typename Value_T>
requires is_buffer_writeable<Value_T>
void write(byte_write_buffer_view& buffer, Value_T value) {
  write<ENDIANNESS, Value_T>({buffer.write_position, buffer.data.end()}, value);
  std::advance(buffer.write_position, sizeof(Value_T));
}

///////////////////////////////////////////////////////////////////////////////

template <std::endian ENDIANNESS = std::endian::native, typename Value_T>
requires is_buffer_writeable<Value_T>
write_result_t try_write(byte_write_buffer_view& buffer, Value_T value) {
  const auto result = try_write<ENDIANNESS, Value_T>({buffer.write_position, buffer.data.end()}, value);
  if (result.ok()) {
    std::advance(buffer.write_position, sizeof(Value_T));
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
void write(byte_write_buffer_view& buffer, Value_T value, std::endian endianness) {
  write<Value_T>({buffer.write_position, buffer.data.end()}, value, endianness);
  std::advance(buffer.write_position, sizeof(Value_T));
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
