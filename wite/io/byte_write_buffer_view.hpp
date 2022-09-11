#pragma once

#include <wite/env/features.hpp>

#include <wite/io/byte_buffer_write.hpp>
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/concepts.hpp>

#include <bit>
#include <cstddef>
#include <iterator>
#include <wite/compatibility/span.hpp>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

struct byte_write_buffer_view {
  explicit byte_write_buffer_view(std::span<io::byte> buf) : data{std::move(buf)}, write_position{data.begin()} {}

  byte_write_buffer_view(std::span<io::byte> buf, typename std::span<const io::byte>::size_type offset)
      : data{std::move(buf)}, write_position{std::next(data.begin(), offset)} {}

  std::span<io::byte> data;
  std::span<io::byte>::iterator write_position;
};

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_writeable<Value_T>
void write(byte_write_buffer_view& buffer, Value_T value) {
#else
void write(byte_write_buffer_view& buffer, std::enable_if_t<is_buffer_writeable<Value_T>, Value_T> value) {
#endif
  write<Value_T>({buffer.write_position, buffer.data.end()}, value);
  std::advance(buffer.write_position, sizeof(Value_T));
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename... Value_Ts>
void write(byte_write_buffer_view& buffer, Value_T first_value, Value_Ts... other_values) {
  write(buffer, first_value);

  if constexpr (sizeof...(other_values) > 0) {
    write(buffer, other_values...);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_writeable<Value_T>
write_result_t try_write(byte_write_buffer_view& buffer, Value_T value) {
#else
    std::enable_if_t<is_buffer_writeable<Value_T>, write_result_t> try_write(byte_write_buffer_view& buffer, Value_T value) {
#endif
  const auto result = try_write<Value_T>({buffer.write_position, buffer.data.end()}, value);
  if (result.ok()) {
    std::advance(buffer.write_position, sizeof(Value_T));
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer_view::write {

  template <typename Value_T>
  #if _WITE_HAS_CONCEPTS
  requires((not std::is_standard_layout_v<Value_T>) or (not std::is_trivial_v<Value_T>)) constexpr auto value_size() noexcept {
  #else
  constexpr auto value_size(std::enable_if_t<(! std::is_standard_layout_v<Value_T>) || (! std::is_trivial_v<Value_T>)>) noexcept {
  #endif
    // This will fail to build if the type satisfies the reuirements but doesn't have a value_type alias in it.
    // In that case, a new overload of this function will need to be added for the new type.
    return sizeof(typename Value_T::value_type);
  }

  template <typename Value_T>
  #if _WITE_HAS_CONCEPTS
  requires(std::is_standard_layout_v<Value_T>and std::is_trivial_v<Value_T>) constexpr auto value_size() noexcept {
  #else
  constexpr auto value_size(std::enable_if_t<std::is_standard_layout_v<Value_T> && std::is_trivial_v<Value_T>>) noexcept {
  #endif
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

}  // namespace detail::buffer_view::write

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename... Value_Ts>
auto try_write(byte_write_buffer_view& buffer, Value_T first_value, Value_Ts... other_values) noexcept {
  const auto out = try_write(buffer.data, first_value, other_values...);

  std::advance(buffer.write_position,
               std::min<ptrdiff_t>(detail::buffer_view::write::byte_count<Value_Ts...>(),
                                   std::distance(buffer.write_position, buffer.data.end())));

  return out;  
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_writeable<Value_T>
void write(byte_write_buffer_view& buffer, Value_T value, endian endianness) {
#else
void write(byte_write_buffer_view& buffer,
           std::enable_if_t<is_buffer_writeable<Value_T>, Value_T> value,
           endian endianness) {
#endif
  write<Value_T>({buffer.write_position, buffer.data.end()}, value, endianness);
  std::advance(buffer.write_position, sizeof(Value_T));
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
