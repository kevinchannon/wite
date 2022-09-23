#pragma once

#include <wite/env/features.hpp>

#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/concepts.hpp>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <iterator>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <tuple>
#include <utility>

#if !_WITE_HAS_CONCEPTS
#error "C++20 concepts are require, but the compiler doesn't support them"
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer::read {

  template <typename Value_T>
    requires((not std::is_standard_layout_v<Value_T>) or (not std::is_trivial_v<Value_T>))
  constexpr auto value_size() noexcept {
    // This will fail to build if the type satisfies the requirements but doesn't have a value_type alias in it.
    // In that case, a new overload of this function will need to be added for the new type.
    return sizeof(typename Value_T::value_type);
  }

  template <typename Value_T>
    requires(std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>)
  constexpr auto value_size() noexcept {
    return sizeof(Value_T);
  }

  template <size_t CURRENT, typename T, typename... Ts>
  static constexpr auto _recursive_byte_count() {
    if constexpr (sizeof...(Ts) == 0) {
      return CURRENT + value_size<T>();
    } else {
      return _recursive_byte_count<CURRENT + value_size<T>(), Ts...>();
    }
  }

  template <typename... Ts>
  static constexpr auto byte_count() {
    return _recursive_byte_count<0, Ts...>();
  }
}  // namespace detail::buffer::read

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T>
auto unchecked_read(auto buffer_iterator) noexcept
{
  if constexpr (io::is_encoded<Value_T>) {
    using OutputValue_t = typename Value_T::value_type;

    auto out = OutputValue_t{};

    if constexpr (std::is_same_v<io::little_endian<OutputValue_t>, Value_T>) {
      std::copy_n(buffer_iterator, sizeof(Value_T), reinterpret_cast<io::byte*>(&out));
    } else if constexpr (std::is_same_v<io::big_endian<OutputValue_t>, Value_T>) {
        std::copy_n(
            buffer_iterator, sizeof(Value_T), std::make_reverse_iterator(std::next(reinterpret_cast<io::byte*>(&out), sizeof(Value_T))));
    }
    else {
      static_assert(std::is_same_v<io::little_endian<OutputValue_t>, Value_T>, "Invalid encoding type");
    }

    return std::pair<OutputValue_t, decltype(buffer_iterator)>{out, std::next(buffer_iterator, sizeof(out))};
  } else {
    auto out = Value_T{};
    std::copy_n(buffer_iterator, sizeof(Value_T), reinterpret_cast<io::byte*>(&out));
    return std::pair<Value_T, decltype(buffer_iterator)>{out, std::next(buffer_iterator, sizeof(out))};
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_readable<Value_T>
auto read(const std::span<const io::byte>& buffer) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T>
auto read_at(size_t position, const std::span<const io::byte>& buffer) {
  if (position + sizeof(Value_T) < position) {
    throw std::invalid_argument{"Buffer read position exceeds allowed value"};
  }

  if (position >= buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return read<Value_T>({std::next(buffer.begin(), position), buffer.end()});
}

///////////////////////////////////////////////////////////////////////////////

template<typename Value_T>
requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>)
read_result_t<Value_T> try_read(const std::span<const io::byte>& buffer) noexcept {
  if (buffer.size() < detail::buffer::read::value_size<Value_T>()) {
    return read_error::insufficient_buffer;
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T> and is_encoded<Value_T>
read_result_t<typename Value_T::value_type> try_read(const std::span<const io::byte>& buffer) noexcept {
  if (buffer.size() < detail::buffer::read::value_size<Value_T>()) {
    return read_error::insufficient_buffer;
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T> and is_encoded<Value_T>
read_result_t<typename Value_T::value_type> try_read_at(size_t position, const std::span<const io::byte>& buffer) noexcept {
  if (position + sizeof(Value_T) < position) {
    return read_error::invalid_position_offset;
  }

  if (position >= buffer.size()) {
    return read_error::insufficient_buffer;
  }

  return try_read<Value_T>(std::span<const io::byte>{std::next(buffer.begin(), position), buffer.end()});
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>)
read_result_t<Value_T> try_read_at(size_t position, const std::span<const io::byte>& buffer) noexcept {
  if (position + sizeof(Value_T) < position) {
    return read_error::invalid_position_offset;
  }

  if (position >= buffer.size()) {
    return read_error::insufficient_buffer;
  }

  return try_read<Value_T>(std::span<const io::byte>{std::next(buffer.begin(), position), buffer.end()});
}

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer::read {

  template<typename FirstValue_T, typename... OtherValue_Ts>
  auto _recursive_read(const std::span<const io::byte>& buffer) {
    auto [first_value, next_pos] = io::unchecked_read<FirstValue_T>(buffer.begin());

    if constexpr (sizeof...(OtherValue_Ts) > 0) {
      auto other_values = _recursive_read<OtherValue_Ts...>(
          std::span<const io::byte>{next_pos, buffer.end()});

      return std::tuple_cat(std::tuple{first_value}, other_values);
    }
    else {
      return std::tuple{first_value};
    }
  }

  template <typename FirstValue_T, typename... OtherValue_Ts>
  auto _recursive_try_read(const std::span<const io::byte>& buffer) noexcept {
    auto first_value = std::make_tuple(io::try_read<FirstValue_T>(buffer));

    if constexpr (sizeof...(OtherValue_Ts) > 0) {
      constexpr auto increment = value_size<typename std::tuple_element_t<0, decltype(first_value)>>();

      auto other_values = _recursive_try_read<OtherValue_Ts...>(std::span<const io::byte>{
          std::get<0>(first_value).ok() ? std::next(buffer.begin(), increment) : buffer.end(), buffer.end()});

      return std::tuple_cat(first_value, other_values);
    } else {
      return first_value;
    }
  }

  template <typename FirstValue_T, typename... OtherValue_Ts>
  auto _recursive_try_read_at(size_t position, const std::span<const io::byte>& buffer) noexcept {
    auto first_value = std::make_tuple(io::try_read_at<FirstValue_T>(position, buffer));

    if constexpr (sizeof...(OtherValue_Ts) > 0) {
      constexpr auto increment = value_size<typename std::tuple_element_t<0, decltype(first_value)>>();

      auto other_values = _recursive_try_read_at<OtherValue_Ts...>(position + increment, buffer);

      return std::tuple_cat(first_value, other_values);
    } else {
      return first_value;
    }
  }

}  // namespace detail::buffer::read

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
auto read(const std::span<const io::byte>& buffer) {
  if (detail::buffer::read::byte_count<Value_Ts...>() > buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return detail::buffer::read::_recursive_read<Value_Ts...>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
auto read_at(size_t position, const std::span<const io::byte>& buffer) {
  if (position + detail::buffer::read::byte_count<Value_Ts...>() < position) {
    throw std::invalid_argument{"Buffer read position exceeds allowed value"};
  }

  if (position + detail::buffer::read::byte_count<Value_Ts...>() > buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return detail::buffer::read::_recursive_read<Value_Ts...>({std::next(buffer.begin(), position), buffer.end()});
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
requires(sizeof...(Value_Ts) > 1)
auto try_read(const std::span<const io::byte>& buffer) noexcept {
  return detail::buffer::read::_recursive_try_read<Value_Ts...>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
auto try_read_at(size_t position, const std::span<const io::byte>& buffer) noexcept {
  return detail::buffer::read::_recursive_try_read_at<Value_Ts...>(position, buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_readable<Value_T>
Value_T read(const std::span<const io::byte>& buffer, endian endianness) {
  if (endian::little == endianness) {
    return read<io::little_endian<Value_T>>(buffer);
  } else {
    return read<io::big_endian<Value_T>>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T>
auto from_bytes(const std::span<const io::byte>& buffer) {
  return read<Value_T>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>)
read_result_t<Value_T> try_from_bytes(const std::span<const io::byte>& buffer) noexcept {
  return try_read<Value_T>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
