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

template <typename Value_T>
requires is_buffer_writeable<Value_T>
void unchecked_write(auto buffer, Value_T value) {
  
  if constexpr (is_encoded<Value_T>) {
    
    using RawValue_t = typename Value_T::value_type;
    
    if constexpr (std::is_same_v<little_endian<RawValue_t>, Value_T>) {
      std::copy_n(reinterpret_cast<std::byte*>(&value.value), sizeof(value.value), buffer);
    } else if constexpr (std::is_same_v<big_endian<RawValue_t>, Value_T>) {
      std::copy_n(std::make_reverse_iterator(std::next(reinterpret_cast<std::byte*>(&value.value), sizeof(value.value))),
                  sizeof(value.value),
                  buffer);
    }
  }
  else {
    std::copy_n(reinterpret_cast<std::byte*>(&value), sizeof(value), buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
void write(std::span<std::byte> buffer, Value_T value) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  unchecked_write<Value_T>(buffer.begin(), value);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
write_result_t try_write(std::span<std::byte> buffer, Value_T value) {
  if (buffer.size() < sizeof(Value_T)) {
    return write_error::insufficient_buffer;
  }

  unchecked_write<Value_T>(buffer.begin(), value);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
void write(std::span<std::byte> buffer, Value_T value, std::endian endianness) {
  if (std::endian::little == endianness) {
    write(buffer, little_endian{value});
  } else {
    write(buffer, big_endian{value});
  }
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
