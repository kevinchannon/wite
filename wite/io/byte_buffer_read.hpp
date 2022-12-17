#pragma once

#include <wite/env/features.hpp>

#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/concepts.hpp>
#include <wite/io/byte_utilities.hpp>

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
#error "C++20 concepts are required, but the compiler doesn't support them"
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, byte_iterator_like ByteIter_T>
  requires is_buffer_readable<Value_T>
auto unchecked_read(ByteIter_T buffer_iterator) noexcept
{
  if constexpr (io::is_encoded<Value_T>) {
    using Byte_t = std::remove_const_t<std::decay_t<decltype(*buffer_iterator)>>;
    auto out = Value_T{};
    std::copy_n(buffer_iterator, out.byte_count(), out.template byte_begin<Byte_t>());
    return std::pair<typename Value_T::value_type, decltype(buffer_iterator)>{out.value, std::next(buffer_iterator, out.byte_count())};

  } else {
    auto out = Value_T{};
    std::copy_n(buffer_iterator, value_size<Value_T>(), reinterpret_cast<std::decay_t<decltype(*buffer_iterator)>*>(&out));
    return std::pair<Value_T, decltype(buffer_iterator)>{out, std::next(buffer_iterator, value_size(out))};
  }
}

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer::read {

  template <byte_range_like ByteRange_T, typename FirstValue_T, typename... OtherValue_Ts>
  auto _recursive_read(ByteRange_T&& buffer) noexcept {
    auto [first_value, next_pos] = io::unchecked_read<FirstValue_T>(buffer.begin());

    if constexpr (sizeof...(OtherValue_Ts) > 0) {
      auto other_values =
          _recursive_read<std::span<std::remove_reference_t<decltype(*buffer.begin())>>, OtherValue_Ts...>(std::span{next_pos, buffer.end()});

      return std::tuple_cat(std::tuple{first_value}, std::tuple{other_values});
    } else {
      return first_value;
    }
  }
}  // namespace detail::buffer::read

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_NO_EXCEPTIONS

template <typename... Value_Ts, byte_range_like ByteRange_T>
auto read(ByteRange_T&& buffer) {
  if (byte_count<Value_Ts...>() > buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return detail::buffer::read::_recursive_read<ByteRange_T, Value_Ts...>(std::forward<ByteRange_T>(buffer));
}

///////////////////////////////////////////////////////////////////////////////

template <typename Range_T, byte_range_like ByteRange_T>
  requires common::is_sized_range_v<Range_T>
auto read_range(ByteRange_T&& buffer, Range_T&& range) {
  if (byte_count(range) > buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  auto read_pos = buffer.begin();
  std::ranges::generate(range, [&read_pos]() {
    using Value_t = typename std::decay_t<Range_T>::value_type;

    auto value                = Value_t{};
    std::tie(value, read_pos) = unchecked_read<Value_t>(read_pos);

    return value;
  });

  return std::move(range);
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
auto read_at(size_t position, const std::span<const io::byte>& buffer) {
  if (position + byte_count<Value_Ts...>() < position) {
    throw std::invalid_argument{"Buffer read position exceeds allowed value"};
  }

  if (position > buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return read<Value_Ts...>(std::span<const io::byte>{std::next(buffer.begin(), position), buffer.end()});
}

///////////////////////////////////////////////////////////////////////////////

template <typename Range_T>
  requires common::is_sized_range_v<Range_T>
auto read_range_at(size_t position, const std::span<const io::byte>& buffer, Range_T&& range) {
  if (position + byte_count(range) < position) {
    throw std::invalid_argument{"Buffer read position exceeds allowed value"};
  }

  if (position > buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }
  
  return read_range(std::span<const io::byte>{std::next(buffer.begin(), position), buffer.end()}, std::forward<Range_T>(range));
}

#endif

///////////////////////////////////////////////////////////////////////////////

template<typename Value_T>
requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>)
read_result_t<Value_T> try_read(const std::span<const io::byte>& buffer) noexcept {
  if (buffer.size() < value_size<Value_T>()) {
    return read_error::insufficient_buffer;
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Range_T>
  requires common::is_sized_range_v<Range_T>
read_result_t<std::decay_t<Range_T>> try_read_range(const std::span<const io::byte>& buffer, Range_T&& range) noexcept {
  if (buffer.size() < range.size() * byte_count<typename Range_T::value_type>()) {
    return read_error::insufficient_buffer;
  }

  auto read_pos = buffer.begin();
  std::ranges::generate(range, [&read_pos]() {
    using Value_t = typename std::decay_t<Range_T>::value_type;

    auto value                = Value_t{};
    std::tie(value, read_pos) = unchecked_read<Value_t>(read_pos);

    return value;
  });

  return {std::move(range)};
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T> and is_encoded<Value_T>
read_result_t<typename Value_T::value_type> try_read(const std::span<const io::byte>& buffer) noexcept {
  if (buffer.size() < value_size<Value_T>()) {
    return read_error::insufficient_buffer;
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T> and is_encoded<Value_T>
read_result_t<typename Value_T::value_type> try_read_at(size_t position, const std::span<const io::byte>& buffer) noexcept {
  if (position + value_size<Value_T>() < position) {
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
  if (position + value_size<Value_T>() < position) {
    return read_error::invalid_position_offset;
  }

  if (position >= buffer.size()) {
    return read_error::insufficient_buffer;
  }

  return try_read<Value_T>(std::span<const io::byte>{std::next(buffer.begin(), position), buffer.end()});
}

///////////////////////////////////////////////////////////////////////////////

template <typename Range_T>
  requires common::is_sized_range_v<Range_T>
read_result_t<Range_T> try_read_range_at(size_t position, const std::span<const io::byte>& buffer, Range_T&& range) noexcept {
  if (position + byte_count(range) < position) {
    return read_error::invalid_position_offset;
  }

  if (position >= buffer.size()) {
    return read_error::insufficient_buffer;
  }

  return try_read_range({std::next(buffer.begin(), position), buffer.end()}, std::forward<Range_T>(range));
}

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer::read {

  template <typename FirstValue_T, typename... OtherValue_Ts>
  auto _recursive_try_read(const std::span<const io::byte>& buffer) noexcept {
    auto first_value = std::make_tuple(io::try_read<FirstValue_T>(buffer));

    if constexpr (sizeof...(OtherValue_Ts) > 0) {
      constexpr auto increment = value_size<FirstValue_T>();

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
      constexpr auto increment = value_size<FirstValue_T>();

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

#ifndef WITE_NO_EXCEPTIONS

template <typename Value_T>
requires is_buffer_readable<Value_T>
Value_T read_with_endian(const std::span<const io::byte>& buffer, endian endianness) {
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

#endif

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>)
read_result_t<Value_T> try_from_bytes(const std::span<const io::byte>& buffer) noexcept {
  return try_read<Value_T>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
