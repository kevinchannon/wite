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

  byte_write_buffer_view& seek(size_t position) {
    if (position > data.size()) {
      throw std::out_of_range{"Cannot seek past end of buffer"};
    }

    unchecked_seek(position);
    return *this;
  }

  result<bool, write_error> try_seek(size_t position) noexcept {
    if (position > data.size()) {
      return write_error::invalid_position_offset;
    }

    unchecked_seek(position);
    return true;
  }

  void unchecked_seek(size_t position) { write_position = std::next(data.begin(), position); }

  _WITE_NODISCARD std::ptrdiff_t write_pos() const noexcept { return std::distance(data.begin(), write_position); }

  template <typename Value_T>
    requires is_buffer_writeable<Value_T>
  size_t write(Value_T value) {
    const auto bytes_written = io::write<Value_T>({write_position, data.end()}, value);
    std::advance(write_position, bytes_written);

    return bytes_written;
  }

  template <typename Value_T, typename... Value_Ts>
  size_t write(Value_T first_value, Value_Ts... other_values) {
    auto out = this->write(first_value);

    if constexpr (sizeof...(other_values) > 0) {
      out += this->write(other_values...);
    }

    return out;
  }

  template <typename Value_T>
    requires is_buffer_writeable<Value_T>
  write_result_t try_write(Value_T value) {
    const auto result = io::try_write<Value_T>({write_position, data.end()}, value);
    if (result.ok()) {
      std::advance(write_position, sizeof(Value_T));
    }

    return result;
  }

  template <typename Value_T, typename... Value_Ts>
  write_result_t try_write(Value_T first_value, Value_Ts... other_values) noexcept {
    const auto result = io::try_write(data, first_value, other_values...);
    if (result.ok()) {
      std::advance(write_position, result.value());
    }

    return result;
  }

  template <typename Value_T>
    requires is_buffer_writeable<Value_T>
  size_t write(Value_T value, endian endianness) {
    const auto bytes_written = io::write<Value_T>({write_position, data.end()}, value, endianness);
    std::advance(write_position, bytes_written);

    return bytes_written;
  }

  std::span<io::byte> data;
  std::span<io::byte>::iterator write_position;
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
