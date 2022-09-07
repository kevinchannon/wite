#pragma once

#include <wite/configure/features.hpp>

#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/concepts.hpp>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <iterator>
#include <wite/compatibility/span.hpp>
#include <stdexcept>
#include <type_traits>
#include <tuple>
#include <utility>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <typename Value_T>
  requires is_buffer_readable<Value_T>
auto unchecked_read(auto buffer) noexcept
#else
template <typename Value_T, typename Buffer_T>
auto unchecked_read(std::enable_if_t<is_iterator_v<Buffer_T>, Buffer_T> buffer)
#endif
{
  if constexpr (std::is_base_of_v<io::encoding, Value_T>) {
    using OutputValue_t = typename Value_T::value_type;

    auto out = OutputValue_t{};

    if constexpr (std::is_same_v<io::little_endian<OutputValue_t>, Value_T>) {
      std::copy_n(buffer, sizeof(Value_T), reinterpret_cast<std::byte*>(&out));
    } else if constexpr (std::is_same_v<io::big_endian<OutputValue_t>, Value_T>) {
        std::copy_n(
            buffer, sizeof(Value_T), std::make_reverse_iterator(std::next(reinterpret_cast<std::byte*>(&out), sizeof(Value_T))));
    }
    else {
      static_assert(std::is_same_v<io::little_endian<OutputValue_t>, Value_T>, "Invalid encoding type");
    }

    return std::pair<OutputValue_t, decltype(buffer)>{out, std::next(buffer, sizeof(out))};
  } else {
    auto out = Value_T{};
    std::copy_n(buffer, sizeof(Value_T), reinterpret_cast<std::byte*>(&out));
    return std::pair<Value_T, decltype(buffer)>{out, std::next(buffer, sizeof(out))};
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_readable<Value_T>
auto read(const std::span<const std::byte>& buffer) {
#else
auto read(const std::span<const std::byte>& buffer, std::enable_if_t<is_buffer_readable<Value_T>>) {
#endif
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

namespace detail {

  template<typename FirstValue_T, typename... OtherValue_Ts>
  auto _recursive_read(const std::span<const std::byte>& buffer) {
    auto first_value = std::make_tuple(read<FirstValue_T>(buffer));

    if constexpr (sizeof...(OtherValue_Ts) > 0) {
      auto other_values = _recursive_read<OtherValue_Ts...>(
          std::span<const std::byte>{std::next(buffer.begin(), sizeof(std::tuple_element_t<0, decltype(first_value)>)), buffer.end()});

      return std::tuple_cat(first_value, other_values);
    }
    else {
      return first_value;
    }
  }

}  // namespace detail

template <typename... Value_Ts>
#if _WITE_HAS_CONCEPTS
requires(sizeof...(Value_Ts) > 1) auto read(const std::span<const std::byte>& buffer) {
#else
auto read(const std::span<const std::byte>& buffer, std::enable_if_t<(sizeof...(Value_Ts) != 1)>) {
#endif
  return detail::_recursive_read<Value_Ts...>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_readable<Value_T> auto from_bytes(const std::span<const std::byte>& buffer) {
#else
auto from_bytes(const std::span<const std::byte>& buffer, std::enable_if_t<is_buffer_readable<Value_T>>) {
#endif
  return read<Value_T>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>) 
read_result_t<Value_T> try_read(const std::span<const std::byte>& buffer) noexcept {
#else
std::enable_if_t<is_buffer_readable<Value_T> && (! is_encoded<Value_T>), read_result_t<Value_T>> try_read(
    const std::span<const std::byte>& buffer) noexcept {
#endif
  if (buffer.size() < sizeof(Value_T)) {
    return read_error::insufficient_buffer;
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_readable<Value_T> and is_encoded<Value_T> 
read_result_t<typename Value_T::value_type> try_read(const std::span<const std::byte>& buffer) noexcept {
#else
std::enable_if_t<is_buffer_readable<Value_T> && is_encoded<Value_T>, read_result_t<typename Value_T::value_type>> try_read(
    const std::span<const std::byte>& buffer) noexcept {
#endif
  if (buffer.size() < sizeof(typename Value_T::value_type)) {
    return read_error::insufficient_buffer;
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>)
read_result_t<Value_T> try_from_bytes(const std::span<const std::byte>& buffer) noexcept {
#else
std::enable_if_t<is_buffer_readable<Value_T> && (!is_encoded<Value_T>), read_result_t<Value_T>> try_from_bytes(
    const std::span<const std::byte>& buffer) noexcept {
#endif
  return try_read<Value_T>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer::read {

  template<typename Value_T>
  #if _WITE_HAS_CONCEPTS
  requires((not std::is_standard_layout_v<Value_T>) or (not std::is_trivial_v<Value_T>))
  constexpr auto value_size() noexcept {
  #else
  constexpr auto value_size(
      std::enable_if_t<((! std::is_standard_layout_v<Value_T>) || (! std::is_trivial_v<Value_T>))>) noexcept {
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

  template <typename FirstValue_T, typename... OtherValue_Ts>
  auto _recursive_try_read(const std::span<const std::byte>& buffer) noexcept {
    auto first_value = std::make_tuple(try_read<FirstValue_T>(buffer));

    if constexpr (sizeof...(OtherValue_Ts) > 0) {

      constexpr auto increment = value_size<typename std::tuple_element_t<0, decltype(first_value)>>();

      auto other_values = _recursive_try_read<OtherValue_Ts...>(std::span<const std::byte>{
        std::get<0>(first_value).ok() ? std::next(buffer.begin(), increment) : buffer.end(),
          buffer.end()}
      );

      return std::tuple_cat(first_value, other_values);
    } else {
      return first_value;
    }
  }

}  // namespace detail

template <typename... Value_Ts>
#if _WITE_HAS_CONCEPTS
requires(sizeof...(Value_Ts) > 1) auto try_read(const std::span<const std::byte>& buffer) noexcept {
#else
auto try_read(std::enable_if_t<sizeof...(Value_Ts) != 1, const std::span<const std::byte>& > buffer) noexcept {
#endif
  return detail::buffer::read::_recursive_try_read<Value_Ts...>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
#if _WITE_HAS_CONCEPTS
requires is_buffer_readable<Value_T> Value_T read(const std::span<const std::byte>& buffer, std::endian endianness) {
#else
std::enable_if_t<is_buffer_readable<Value_T>, Value_T> read(const std::span<const std::byte>& buffer, endian endianness) {
#endif
  if (std::endian::little == endianness) {
    return read<io::little_endian<Value_T>>(buffer);
  } else {
    return read<io::big_endian<Value_T>>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
