#pragma once

#include <wite/env/features.hpp>

#include <wite/io/byte_buffer_write.hpp>
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/concepts.hpp>

#include <bit>
#include <cstddef>
#include <iterator>
#include <span>
#include <type_traits>

#if !_WITE_HAS_CONCEPTS
#error "C++20 concepts are require, but the compiler doesn't support them"
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

struct byte_write_buffer_view {
  explicit byte_write_buffer_view(std::span<io::byte> buf) : data{std::move(buf)}, write_position{data.begin()} {}

  byte_write_buffer_view(std::span<io::byte> buf, typename std::span<const io::byte>::size_type offset)
      : data{std::move(buf)}, write_position{std::next(data.begin(), offset)} {}

  std::span<io::byte> data;
  std::span<io::byte>::iterator write_position;
};

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
size_t write(byte_write_buffer_view& buffer, Value_T value) {
  const auto bytes_written = write<Value_T>({buffer.write_position, buffer.data.end()}, value);
  std::advance(buffer.write_position, bytes_written);

  return bytes_written;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename... Value_Ts>
size_t write(byte_write_buffer_view& buffer, Value_T first_value, Value_Ts... other_values) {
  auto out = write(buffer, first_value);

  if constexpr (sizeof...(other_values) > 0) {
    out += write(buffer, other_values...);
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_writeable<Value_T>
size_t write_at(size_t position, byte_write_buffer_view& buffer, Value_T value) {
  buffer.write_position = std::next(buffer.data.begin(), write_at<Value_T>(position, buffer.data, value));
  return std::distance(buffer.data.begin(), buffer.write_position);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
write_result_t try_write(byte_write_buffer_view& buffer, Value_T value) {
  const auto result = try_write<Value_T>({buffer.write_position, buffer.data.end()}, value);
  if (result.ok()) {
    std::advance(buffer.write_position, sizeof(Value_T));
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_writeable<Value_T>
write_result_t try_write_at(size_t position, byte_write_buffer_view& buffer, Value_T value) {
  const auto result = try_write_at<Value_T>(position, buffer.data, value);
  if (result.ok()) {
    buffer.write_position = std::next(buffer.data.begin(), result.value());
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename... Value_Ts>
write_result_t try_write(byte_write_buffer_view& buffer, Value_T first_value, Value_Ts... other_values) noexcept {
  const auto result = try_write(buffer.data, first_value, other_values...);
  if (result.ok()) {
    std::advance(buffer.write_position, result.value());
  }

  return result;  
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename... Value_Ts>
auto try_write_at(size_t position, byte_write_buffer_view& buffer, Value_T first_value, Value_Ts... other_values) noexcept {
  const auto result = try_write_at(position, buffer.data, first_value, other_values...);
  if (result.ok()) {
    buffer.write_position = std::next(buffer.data.begin(), result.value());
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
size_t write(byte_write_buffer_view& buffer, Value_T value, endian endianness) {
  const auto bytes_written = write<Value_T>({buffer.write_position, buffer.data.end()}, value, endianness);
  std::advance(buffer.write_position, bytes_written);

  return bytes_written;
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
