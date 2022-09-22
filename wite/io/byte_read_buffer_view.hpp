#pragma once

#include <wite/env/environment.hpp>

#include <wite/io/byte_buffer_read.hpp>
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/concepts.hpp>

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
  
  template <typename Value_T>
    requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>)
  Value_T read() {
    const auto out = io::read<Value_T>({_get_pos, _data.end()});
    std::advance(_get_pos, sizeof(Value_T));

    return out;
  }

  template <typename Value_T>
    requires is_buffer_writeable<Value_T> and is_encoded<Value_T>
  typename Value_T::value_type read() {
    const auto out = io::read<Value_T>({_get_pos, _data.end()});
    std::advance(_get_pos , sizeof(out));

    return out;
  }

  template <typename Value_T>
  auto try_read() noexcept {
    const auto out = io::try_read<Value_T>({_get_pos , _data.end()});

    // TODO: this should check for success before advancing the read pointer.
    std::advance(_get_pos , _value_size<Value_T>());

    return out;
  }

  template <typename... Value_Ts>
    requires(sizeof...(Value_Ts) > 1)
  auto read() {
    const auto values = io::read<Value_Ts...>(_data);

    std::advance(_get_pos , _byte_count<Value_Ts...>());

    return values;
  }

  template <typename... Value_Ts>
    requires(sizeof...(Value_Ts) > 1)
  auto try_read() noexcept {
    const auto out = io::try_read<Value_Ts...>(_data);
    std::advance(_get_pos ,
                 std::min<ptrdiff_t>(_byte_count<Value_Ts...>(),
                                     std::distance(_get_pos , _data.end())));

    return out;
  }

  template <typename Value_T>
  auto read(std::endian endianness) {
    const auto out = io::read<Value_T>({_get_pos , _data.end()}, endianness);
    std::advance(_get_pos , _value_size<Value_T>());

    return out;
  }

private:
  template <typename Value_T>
    requires(not common::is_pod_like<Value_T>)
  static constexpr auto _value_size() noexcept {
    // This will fail to build if the type satisfies the requirements but doesn't have a value_type alias in it.
    // In that case, a new overload of this function will need to be added for the new type.
    return sizeof(typename Value_T::value_type);
  }

  template <typename Value_T>
    requires common::is_pod_like<Value_T>
  static constexpr auto _value_size() noexcept {
    return sizeof(Value_T);
  }

  template <size_t CURRENT, typename T, typename... Ts>
  static constexpr auto _recursive_byte_count() {
    if constexpr (sizeof...(Ts) == 0) {
      return CURRENT + _value_size<T>();
    } else {
      return _recursive_byte_count<CURRENT + _value_size<T>(), Ts...>();
    }
  }

  template <typename... Ts>
  static constexpr auto _byte_count() {
    return _recursive_byte_count<0, Ts...>();
  }

  std::span<const io::byte> _data;
  std::span<const io::byte>::iterator _get_pos;
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
