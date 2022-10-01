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
  explicit byte_write_buffer_view(std::span<io::byte> buf) : _data{std::move(buf)}, _put_pos{_data.begin()} {}

  byte_write_buffer_view(std::span<io::byte> buf, typename std::span<const io::byte>::size_type offset)
      : _data{std::move(buf)}, _put_pos{std::next(_data.begin(), offset)} {}

  byte_write_buffer_view& seek(size_t position) {
    if (position > _data.size()) {
      throw std::out_of_range{"Cannot seek past end of buffer"};
    }

    unchecked_seek(position);
    return *this;
  }

  result<bool, write_error> try_seek(size_t position) noexcept {
    if (position > _data.size()) {
      return write_error::invalid_position_offset;
    }

    unchecked_seek(position);
    return true;
  }

  void unchecked_seek(size_t position) { _put_pos = std::next(_data.begin(), position); }

  _WITE_NODISCARD std::ptrdiff_t write_position() const noexcept { return std::distance(_data.begin(), _put_pos); }

  template <typename... Value_Ts>
  size_t write(Value_Ts&&... values) {
    const auto bytes_written = io::write({_put_pos, _data.end()}, std::forward<Value_Ts>(values)...);
    std::advance(_put_pos, bytes_written);

    return bytes_written;
  }

  template <typename Value_T>
    requires is_buffer_writeable<Value_T>
  write_result_t try_write(Value_T value) {
    const auto result = io::try_write({_put_pos, _data.end()}, value);
    if (result.ok()) {
      std::advance(_put_pos, sizeof(Value_T));
    }

    return result;
  }

  template <typename Value_T, typename... Value_Ts>
  write_result_t try_write(Value_T first_value, Value_Ts... other_values) noexcept {
    const auto result = io::try_write(_data, first_value, other_values...);
    if (result.ok()) {
      std::advance(_put_pos, result.value());
    }

    return result;
  }

  template <typename Value_T>
    requires is_buffer_writeable<Value_T>
  size_t write(Value_T value, endian endianness) {
    const auto bytes_written = io::write<Value_T>({_put_pos, _data.end()}, value, endianness);
    std::advance(_put_pos, bytes_written);

    return bytes_written;
  }


private:
  std::span<io::byte> _data;
  std::span<io::byte>::iterator _put_pos;
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
