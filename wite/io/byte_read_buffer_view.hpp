#pragma once

#include <wite/io/byte_buffer_read.hpp>
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>

#include <bit>
#include <cstddef>
#include <span>
#include <type_traits>
#include <iterator>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

struct byte_read_buffer_view {
  explicit byte_read_buffer_view(std::span<const std::byte> buf) : data{std::move(buf)}, read_position{data.begin()} {}

  byte_read_buffer_view(std::span<const std::byte> buf, typename std::span<const std::byte>::size_type offset)
      : data{std::move(buf)}, read_position{std::next(data.begin(), offset)} {}

  std::span<const std::byte> data;
  std::span<const std::byte>::iterator read_position;
};

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T> and
    (not std::is_base_of_v<io::encoding, Value_T>) Value_T read(byte_read_buffer_view& buffer) {
  const auto out = read<Value_T, ENDIANNESS>({buffer.read_position, buffer.data.end()});
  std::advance(buffer.read_position, sizeof(Value_T));

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T> and
    (not std::is_base_of_v<io::encoding, Value_T>) read_result_t<Value_T> try_read(byte_read_buffer_view& buffer) {
  const auto out = try_read<Value_T, ENDIANNESS>({buffer.read_position, buffer.data.end()});
  std::advance(buffer.read_position, sizeof(Value_T));

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_base_of_v<io::encoding, Value_T> read_result_t<typename Value_T::value_type> try_read(
    byte_read_buffer_view& buffer) {
  using OuptutValue_t = typename Value_T::value_type;

  if constexpr (std::is_same_v<io::little_endian<OuptutValue_t>, Value_T>) {
    return try_read<OuptutValue_t, std::endian::little>(buffer);
  } else if constexpr (std::is_same_v<io::big_endian<OuptutValue_t>, Value_T>) {
    return try_read<OuptutValue_t, std::endian::big>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T> Value_T read(byte_read_buffer_view& buffer,
                                                                                        std::endian endienness) {
  const auto out = read<Value_T>({buffer.read_position, buffer.data.end()}, endienness);
  std::advance(buffer.read_position, sizeof(Value_T));

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_base_of_v<io::encoding, Value_T>
typename Value_T::value_type read(byte_read_buffer_view& buffer) {
  using OuptutValue_t = typename Value_T::value_type;

  if constexpr (std::is_same_v<io::little_endian<OuptutValue_t>, Value_T>) {
    return read<OuptutValue_t, std::endian::little>(buffer);
  } else if constexpr (std::is_same_v<io::big_endian<OuptutValue_t>, Value_T>) {
    return read<OuptutValue_t, std::endian::big>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
