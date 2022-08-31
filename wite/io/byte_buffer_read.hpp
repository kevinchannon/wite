#pragma once

#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <iterator>
#include <span>
#include <stdexcept>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires(std::is_standard_layout_v<Value_T>and std::is_trivial_v<Value_T> and
         (not std::is_base_of_v<io::encoding, Value_T>))
Value_T unchecked_read(auto buffer) {
  auto out = Value_T{};

  if constexpr (std::endian::little == ENDIANNESS) {
    std::copy_n(buffer, sizeof(Value_T), reinterpret_cast<std::byte*>(&out));
  } else {
    std::copy_n(
        buffer, sizeof(Value_T), std::make_reverse_iterator(std::next(reinterpret_cast<std::byte*>(&out), sizeof(Value_T))));
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires(std::is_standard_layout_v<Value_T>and std::is_trivial_v<Value_T> and
         (not std::is_base_of_v<io::encoding, Value_T>))
Value_T read(const std::span<const std::byte>& buffer) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return unchecked_read<Value_T, ENDIANNESS>(buffer.begin());
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires(std::is_standard_layout_v<Value_T>and std::is_trivial_v<Value_T> and (not std::is_base_of_v<io::encoding, Value_T>))
read_result_t<Value_T> try_read(const std::span<const std::byte>& buffer) {
  if (buffer.size() < sizeof(Value_T)) {
    return read_error::insufficient_buffer;
  }

  return unchecked_read<Value_T, ENDIANNESS>(buffer.begin());
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_base_of_v<io::encoding, Value_T>
typename Value_T::value_type read(const std::span<const std::byte>& buffer) {
  using OuptutValue_t = typename Value_T::value_type;

  if constexpr (std::is_same_v<io::little_endian<OuptutValue_t>, Value_T>) {
    return read<OuptutValue_t, std::endian::little>(buffer);
  } else if constexpr (std::is_same_v<io::big_endian<OuptutValue_t>, Value_T>) {
    return read<OuptutValue_t, std::endian::big>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_base_of_v<io::encoding, Value_T>
read_result_t<typename Value_T::value_type> try_read( const std::span<const std::byte>& buffer) {
  using OuptutValue_t = typename Value_T::value_type;

  if constexpr (std::is_same_v<io::little_endian<OuptutValue_t>, Value_T>) {
    return try_read<OuptutValue_t, std::endian::little>(buffer);
  } else if constexpr (std::is_same_v<io::big_endian<OuptutValue_t>, Value_T>) {
    return try_read<OuptutValue_t, std::endian::big>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
Value_T read(const std::span<const std::byte>& buffer, std::endian endianness) {
  if (std::endian::little == endianness) {
    return read<Value_T, std::endian::little>(buffer);
  } else {
    return read<Value_T, std::endian::big>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
