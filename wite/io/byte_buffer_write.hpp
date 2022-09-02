#pragma once

#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/concepts.hpp>

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

template <std::endian ENDIANNESS = std::endian::native, typename Value_T>
requires is_buffer_writeable<Value_T> and (not std::is_base_of_v<io::encoding, Value_T>)
void unchecked_write(auto buffer, Value_T value) {
    if constexpr (std::endian::little == ENDIANNESS) {
      std::copy_n(reinterpret_cast<std::byte*>(&value), sizeof(Value_T), buffer);
    } else {
      std::copy_n(std::make_reverse_iterator(std::next(reinterpret_cast<std::byte*>(&value), sizeof(Value_T))),
                  sizeof(Value_T),
                  buffer);
    }
}

///////////////////////////////////////////////////////////////////////////////

template <std::endian ENDIANNESS = std::endian::native, typename Value_T>
requires is_buffer_writeable<Value_T> and std::is_base_of_v<io::encoding, Value_T>
void unchecked_write(auto buffer, Value_T value) {
  if constexpr (std::is_same_v<io::little_endian<typename Value_T::value_type>, Value_T>) {
    unchecked_write<std::endian::little>(buffer, value.value);
  } else if constexpr (std::is_same_v<io::big_endian<typename Value_T::value_type>, Value_T>) {
    unchecked_write<std::endian::big>(buffer, value.value);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <std::endian ENDIANNESS = std::endian::native, typename Value_T>
requires is_buffer_writeable<Value_T>
void write(std::span<std::byte> buffer, Value_T value) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  unchecked_write<ENDIANNESS, Value_T>(buffer.begin(), value);
}

///////////////////////////////////////////////////////////////////////////////

template <std::endian ENDIANNESS = std::endian::native, typename Value_T>
requires is_buffer_writeable<Value_T>
write_result_t try_write(std::span<std::byte> buffer, Value_T value) {
  if (buffer.size() < sizeof(Value_T)) {
    return write_error::insufficient_buffer;
  }

  unchecked_write<ENDIANNESS, Value_T>(buffer.begin(), value);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
void write(std::span<std::byte> buffer, Value_T value, std::endian endianness) {
  if (std::endian::little == endianness) {
    write<std::endian::little>(buffer, value);
  } else {
    write<std::endian::big>(buffer, value);
  }
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
