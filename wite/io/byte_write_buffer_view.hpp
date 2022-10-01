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

class byte_write_buffer_view {
 public:
  using buffer_type = std::span<io::byte>;
  using size_type   = typename std::span<const io::byte>::size_type;

  explicit byte_write_buffer_view(buffer_type buf) : _data{std::move(buf)}, _put_pos{_data.begin()} {}

  byte_write_buffer_view(buffer_type buf, size_type offset)
      : byte_write_buffer_view{std::move(buf)} {
    seek(offset);
  }

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

  template <typename... Value_Ts>
  write_result_t try_write(Value_Ts&&... values) noexcept {
    const auto result = io::try_write({_put_pos, _data.end()}, std::forward<Value_Ts>(values)...);
    if (result.ok()) {
      std::advance(_put_pos, result.value());
    }

    return result;
  }

  template <typename Value_T>
  size_t write(Value_T value, endian endianness) {
    const auto bytes_written = io::write({_put_pos, _data.end()}, value, endianness);
    std::advance(_put_pos, bytes_written);

    return bytes_written;
  }


private:
  buffer_type _data;
  buffer_type::iterator _put_pos;
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
