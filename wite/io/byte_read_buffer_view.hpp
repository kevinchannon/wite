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

template<byte_range_like ByteRange_T>
class byte_read_buffer_view {
 public:
  using buffer_type = std::decay_t<ByteRange_T>;
  using size_type   = buffer_type::size_type;

  explicit byte_read_buffer_view(const buffer_type& buf) : _data{buf}, _get_pos{_data.begin()} {}

  #ifndef WITE_NO_EXCEPTIONS

  byte_read_buffer_view(const buffer_type& buf, size_type offset)
      : byte_read_buffer_view{buf} {
    seek(offset);
  }

  byte_read_buffer_view& seek(size_type position) {
    if (position > _data.size()) {
      throw std::out_of_range{"Cannot seek past end of buffer"};
    }

    unchecked_seek(position);
    return *this;
  }

  #endif

  result<bool, read_error> try_seek(size_type position) noexcept {
    if (position > _data.size()) {
      return read_error::invalid_position_offset;
    }

    unchecked_seek(position);
    return true;
  }

  void unchecked_seek(size_t position) noexcept {
    _get_pos = std::next(_data.begin(), position);
  }

  _WITE_NODISCARD size_type read_position() const noexcept { return std::distance(_data.begin(), _get_pos); }
  
  #ifndef WITE_NO_EXCEPTIONS

  template <typename... Value_Ts>
  auto read() {
    auto out = io::read<Value_Ts...>(std::span{_get_pos, _data.end()});
    std::advance(_get_pos, byte_count<Value_Ts...>());

    return out;
  }

  template<typename Range_T>
  auto read_range(Range_T&& range) {
    auto out = io::read_range(std::span{_get_pos, _data.end()}, std::forward<Range_T>(range));
    std::advance(_get_pos, byte_count(out));

    return out;
  }

  template <typename Value_T>
  auto read(std::endian endianness) {
    const auto out = io::read_with_endian<Value_T>(std::span{_get_pos, _data.end()}, endianness);
    std::advance(_get_pos, value_size<Value_T>());

    return out;
  }

  #endif

  template <typename... Value_Ts>
  auto try_read() noexcept {
    auto out = io::try_read<Value_Ts...>(std::span{_get_pos, _data.end()});
    std::advance(_get_pos ,
                 std::min<ptrdiff_t>(byte_count<Value_Ts...>(),
                                     std::distance(_get_pos , _data.end())));

    return out;
  }

  template <typename Range_T>
  auto try_read_range(Range_T&& range) noexcept {
    auto out = io::try_read_range(std::span{_get_pos, _data.end()}, std::forward<Range_T>(range));
    if (out.ok()) {
      std::advance(_get_pos, byte_count(out.value()));
    } else {
      _get_pos = _data.end();
    }

    return out;
  }

private:

  const buffer_type& _data;
  buffer_type::const_iterator _get_pos;
};

///////////////////////////////////////////////////////////////////////////////

template<byte_range_like R>
byte_read_buffer_view(const R& r)->byte_read_buffer_view<R>;

template <byte_range_like R>
byte_read_buffer_view(const R& r, size_t) -> byte_read_buffer_view<R>;

///////////////////////////////////////////////////////////////////////////////

template<byte_range_like ByteRange_T>
result<byte_read_buffer_view<ByteRange_T>, read_error> try_make_byte_read_buffer_view(
    const ByteRange_T& buffer,
    size_t offset) noexcept {
  auto out = byte_read_buffer_view {std::move(buffer)};
  if (auto result = out.try_seek(offset); result.is_error()) {
    return result.error();
  }

  return out;
}


}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
