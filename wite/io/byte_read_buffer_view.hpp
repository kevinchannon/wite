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

  explicit byte_read_buffer_view(std::span<const io::byte> buf) : data{std::move(buf)}, read_position{data.begin()} {}

  byte_read_buffer_view(std::span<const io::byte> buf, typename std::span<const io::byte>::size_type offset)
      : data{std::move(buf)}, read_position{std::next(data.begin(), offset)} {}

  byte_read_buffer_view& seek(size_t position) {
    if (position > data.size()) {
      throw std::out_of_range{"Cannot seek past end of buffer"};
    }

    unchecked_seek(position);
    return *this;
  }

  result<bool, read_error> try_seek(size_t position) noexcept {
    if (position > data.size()) {
      return read_error::invalid_position_offset;
    }

    unchecked_seek(position);
    return true;
  }

  void unchecked_seek(size_t position) {
    read_position = std::next(data.begin(), position);
  }

  _WITE_NODISCARD std::ptrdiff_t read_pos() const noexcept { return std::distance(data.begin(), read_position); }
  
  template <typename Value_T>
    requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>)
  Value_T read() {
    const auto out = io::read<Value_T>({read_position, data.end()});
    std::advance(read_position, sizeof(Value_T));

    return out;
  }

  template <typename Value_T>
    requires is_buffer_writeable<Value_T> and is_encoded<Value_T>
  typename Value_T::value_type read() {
    const auto out = io::read<Value_T>({read_position, data.end()});
    std::advance(read_position, sizeof(out));

    return out;
  }

  std::span<const io::byte> data;
  std::span<const io::byte>::iterator read_position;
};

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer_view::read {

  template <typename Value_T>
    requires(not common::is_pod_like<Value_T>)
  constexpr auto value_size() noexcept {
    // This will fail to build if the type satisfies the requirements but doesn't have a value_type alias in it.
    // In that case, a new overload of this function will need to be added for the new type.
    return sizeof(typename Value_T::value_type);
  }

  template <typename Value_T>
    requires common::is_pod_like<Value_T>
  constexpr auto value_size() noexcept {
    return sizeof(Value_T);
  }

  template <size_t CURRENT, typename T, typename... Ts>
  constexpr auto _recursive_byte_count() {
    if constexpr (sizeof...(Ts) == 0) {
      return CURRENT + value_size<T>();
    } else {
      return _recursive_byte_count<CURRENT + value_size<T>(), Ts...>();
    }
  }

  template <typename... Ts>
  constexpr auto byte_count() {
    return _recursive_byte_count<0, Ts...>();
  }

}  // namespace detail::buffer_view::read

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
auto read_at(size_t position, byte_read_buffer_view& buffer) {
  const auto out = read_at<Value_T>(position, {buffer.data.begin(), buffer.data.end()});
  buffer.unchecked_seek(position + detail::buffer_view::read::value_size<Value_T>());

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
auto try_read(byte_read_buffer_view& buffer) noexcept {
  const auto out = try_read<Value_T>({buffer.read_position, buffer.data.end()});

  // TODO: this should check for success before advancing the read pointer.
  std::advance(buffer.read_position, detail::buffer_view::read::value_size<Value_T>());

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
auto try_read_at(size_t position, byte_read_buffer_view& buffer) noexcept {
  const auto out = try_read_at<Value_T>(position, buffer.data);
  if (out.ok()) {
    buffer.unchecked_seek(position + detail::buffer_view::read::value_size<Value_T>());
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
requires(sizeof...(Value_Ts) > 1)
auto read(byte_read_buffer_view& buffer) {
  const auto values = read<Value_Ts...>(buffer.data);
  
  std::advance(buffer.read_position, detail::buffer_view::read::byte_count<Value_Ts...>());

  return values;
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
auto read_at(size_t position, byte_read_buffer_view& buffer) {
  const auto values = read_at<Value_Ts...>(position, buffer.data);

  buffer.read_position = std::next(buffer.data.begin(), detail::buffer_view::read::byte_count<Value_Ts...>());

  return values;
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
requires(sizeof...(Value_Ts) > 1)
auto try_read(byte_read_buffer_view& buffer) noexcept {
  const auto out = try_read<Value_Ts...>(buffer.data);
  std::advance(
    buffer.read_position,
    std::min<ptrdiff_t>(
      detail::buffer_view::read::byte_count<Value_Ts...>(),
      std::distance(buffer.read_position, buffer.data.end())
      )
  );

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
auto try_read_at(size_t position, byte_read_buffer_view& buffer) noexcept {
  const auto out = try_read_at<Value_Ts...>(position, buffer.data);
  
  buffer.seek(std::min(position + detail::buffer_view::read::byte_count<Value_Ts...>(), buffer.data.size()));

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
auto read(byte_read_buffer_view& buffer, std::endian endianness) {
  const auto out = read<Value_T>({buffer.read_position, buffer.data.end()}, endianness);
  std::advance(buffer.read_position, detail::buffer_view::read::value_size<Value_T>());

  return out;
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
