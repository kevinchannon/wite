#pragma once

#include <wite/env/environment.hpp>

#include <wite/io/byte_buffer_read.hpp>
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/concepts.hpp>
#include <wite/io/byte_utilities.hpp>

#include <bit>
#include <cstddef>
#include <span>
#include <type_traits>
#include <iterator>

#if !_WITE_HAS_CONCEPTS
#error "C++20 concepts are require, but the compiler doesn't support them"
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

class byte_read_buffer_view {
 public:

  explicit byte_read_buffer_view(std::span<const io::byte> buf) : _data{std::move(buf)}, _get_pos{_data.begin()} {}

  byte_read_buffer_view(std::span<const io::byte> buf, typename std::span<const io::byte>::size_type offset)
      : _data{std::move(buf)}, _get_pos{std::next(_data.begin(), offset)} {}

  byte_read_buffer_view& seek(size_t position) {
    if (position > _data.size()) {
      throw std::out_of_range{"Cannot seek past end of buffer"};
    }

    unchecked_seek(position);
    return *this;
  }

  result<bool, read_error> try_seek(size_t position) noexcept {
    if (position > _data.size()) {
      return read_error::invalid_position_offset;
    }

    unchecked_seek(position);
    return true;
  }

  void unchecked_seek(size_t position) {
    _get_pos = std::next(_data.begin(), position);
  }

  _WITE_NODISCARD std::ptrdiff_t read_position() const noexcept { return std::distance(_data.begin(), _get_pos); }
  
  template <typename... Value_Ts>
  auto read() {
    const auto out = io::read<Value_Ts...>({_get_pos, _data.end()});
    std::advance(_get_pos, byte_count<Value_Ts...>());

    return out;
  }

  template <typename... Value_Ts>
  auto try_read() noexcept {
    const auto out = io::try_read<Value_Ts...>({_get_pos, _data.end()});
    std::advance(_get_pos ,
                 std::min<ptrdiff_t>(byte_count<Value_Ts...>(),
                                     std::distance(_get_pos , _data.end())));

    return out;
  }

  template <typename Value_T>
  auto read(std::endian endianness) {
    const auto out = io::read<Value_T>({_get_pos , _data.end()}, endianness);
    std::advance(_get_pos, value_size<Value_T>());

    return out;
  }

private:

  std::span<const io::byte> _data;
  std::span<const io::byte>::iterator _get_pos;
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
