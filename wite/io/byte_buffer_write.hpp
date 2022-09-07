#pragma once

#include <wite/configure/features.hpp>

#include <wite/io/concepts.hpp>
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <iterator>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <typename Value_T>
requires is_buffer_writeable<Value_T>
void unchecked_write(auto buffer, Value_T value) {
#else
template <typename Value_T, typename Buffer_T>
void unchecked_write(std::enable_if_t<is_iterator_v<Buffer_T>, Buffer_T> buffer, std::enable_if_t<is_buffer_writeable<Value_T>, Value_T> value) {
#endif
  if constexpr (is_encoded<Value_T>) {
    using RawValue_t = typename Value_T::value_type;

    if constexpr (std::is_same_v<little_endian<RawValue_t>, Value_T>) {
      std::copy_n(reinterpret_cast<std::byte*>(&value.value), sizeof(value.value), buffer);
    } else if constexpr (std::is_same_v<big_endian<RawValue_t>, Value_T>) {
      std::copy_n(std::make_reverse_iterator(std::next(reinterpret_cast<std::byte*>(&value.value), sizeof(value.value))),
                  sizeof(value.value),
                  buffer);
    }
  } else {
    std::copy_n(reinterpret_cast<std::byte*>(&value), sizeof(value), buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_writeable<Value_T>
void write(std::span<std::byte> buffer, Value_T value) {
#else
void write(std::span<std::byte> buffer, std::enable_if_t<is_buffer_writeable<Value_T>, Value_T> value) {
#endif
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  unchecked_write<Value_T>(buffer.begin(), value);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename... Value_Ts>
void write(std::span<std::byte> buffer, Value_T first_value, Value_Ts... other_values) {
  write(buffer, first_value);

  if constexpr (sizeof...(other_values) > 0) {
    write(std::span<std::byte>{std::next(buffer.begin(), sizeof(first_value)), buffer.end()}, other_values...);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename Result_T = static_byte_buffer<sizeof(Value_T)>>
#if _WITE_HAS_CONCEPTS
requires is_buffer_writeable<Value_T> Result_T to_bytes(Value_T value) {
#else
std::enable_if_t<is_buffer_writeable<Value_T>, Result_T> to_bytes(Value_T value) {
#endif
  auto out = Result_T{};

  write(out, value);

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_writeable<Value_T> write_result_t try_write(std::span<std::byte> buffer, Value_T value) {
#else
    std::enable_if_t<is_buffer_writeable<Value_T>, write_result_t> try_write(std::span<std::byte> buffer, Value_T value) {
#endif
  if (buffer.size() < sizeof(Value_T)) {
    return write_error::insufficient_buffer;
  }

  unchecked_write<Value_T>(buffer.begin(), value);

  return true;
}

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer::write {

  template <typename Value_T>
  #if _WITE_HAS_CONCEPTS
  requires((not std::is_standard_layout_v<Value_T>) or (not std::is_trivial_v<Value_T>))
  constexpr auto value_size() noexcept {
  #else
  constexpr auto value_size(
      std::enable_if_t<(! std::is_standard_layout_v<Value_T>) || (! std::is_trivial_v<Value_T>)>) noexcept {
  #endif
    // This will fail to build if the type satisfies the reuirements but doesn't have a value_type alias in it.
    // In that case, a new overload of this function will need to be added for the new type.
    return sizeof(typename Value_T::value_type);
  }

  template <typename Value_T>
  #if _WITE_HAS_CONCEPTS
  requires(std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>)
  constexpr auto value_size() noexcept {
  #else
  constexpr auto value_size(std::enable_if_t<std::is_standard_layout_v<Value_T> && std::is_trivial_v<Value_T>>) noexcept {
  #endif
    return sizeof(Value_T);
  }

  template <typename Value_T, typename... Value_Ts>
  auto _recursive_try_write(std::span<std::byte> buffer, Value_T first_value, Value_Ts... other_values) {
    auto first_result = std::make_tuple(try_write(buffer, first_value));

    if constexpr (sizeof...(other_values) > 0) {
      auto other_results =
          _recursive_try_write(std::span<std::byte>{std::get<0>(first_result).ok()
                                             ? std::next(buffer.begin(), value_size<Value_T>())
                                             : buffer.end(),
                                         buffer.end()},
                    other_values...);

      return std::tuple_cat(first_result, other_results);
    } else {
      return first_result;
    }
  }

}  // namespace detail::buffer::write

template <typename Value_T, typename... Value_Ts>
auto try_write(std::span<std::byte> buffer, Value_T first_value, Value_Ts... other_values) noexcept {
  return detail::buffer::write::_recursive_try_write(buffer, first_value, other_values...);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename Result_T = static_byte_buffer<sizeof(Value_T)>>
#if _WITE_HAS_CONCEPTS
requires is_buffer_writeable<Value_T> result<Result_T, write_error> try_to_bytes(Value_T value) {
#else
std::enable_if_t<is_buffer_writeable<Value_T>, result<Result_T, write_error>> try_to_bytes(Value_T value) {
#endif
  auto out = Result_T{};

  const auto result = try_write(out, value);
  if (result.ok()) {
    return {out};
  } else {
    return {result.error()};
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_writeable<Value_T>
void write(std::span<std::byte> buffer, Value_T value, std::endian endianness) {
#else
void write(std::span<std::byte> buffer, std::enable_if_t<is_buffer_writeable<Value_T>, Value_T> value, endian endianness) {
#endif
  if (std::endian::little == endianness) {
    write(buffer, little_endian{value});
  } else {
    write(buffer, big_endian{value});
  }
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
