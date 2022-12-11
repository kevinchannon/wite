#pragma once

#include <wite/env/features.hpp>

#include <wite/io/concepts.hpp>
#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/byte_utilities.hpp>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>

#if !_WITE_HAS_CONCEPTS
#error "C++20 concepts are require, but the compiler doesn't support them"
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

namespace detail::buffer::write {

  template <byte_like Byte_T, typename Encoded_T>
    requires is_encoded<std::decay_t<Encoded_T>>
  size_t _write_single_encoded_value(auto buffer, Encoded_T&& value) noexcept {
    std::copy_n(value.template byte_begin<Byte_T>(), value.byte_count(), buffer);
    return value.byte_count();
  }

  template <byte_like Byte_T, typename Value_T>
    requires((not common::is_sized_range_v<std::decay_t<Value_T>>) and (not is_encoded<std::decay_t<Value_T>>))
  size_t _write_single_value(auto buffer, Value_T&& value) noexcept {
    std::copy_n(reinterpret_cast<const Byte_T*>(&value), sizeof(value), buffer);
    return sizeof(std::decay_t<Value_T>);
  }

  template <byte_like Byte_T, typename Range_T>
    requires common::is_sized_range_v<std::decay_t<Range_T>>
  size_t _write_single_range_value(auto buffer, Range_T&& values) noexcept {
    struct _write_info {
      size_t bytes_written{};
      decltype(buffer) next_write_position{};
    };

    return std::accumulate(values.begin(),
                           values.end(),
                           _write_info{0, buffer},
                           [](auto&& current, const auto& v) -> _write_info {
                             const auto byte_written_for_this_value = _write_single_value<Byte_T>(current.next_write_position, v);
                             return {current.bytes_written + byte_written_for_this_value,
                                     std::next(current.next_write_position, value_size(v))};
                           })
        .bytes_written;
  }

}  // namespace detail::buffer::write

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
size_t unchecked_write(auto buffer, Value_T&& value) {

  using DecayedValue_t = std::decay_t<Value_T>;
  using Byte_t = std::decay_t<decltype(*buffer)>;
  static_assert(is_byte_like_v<Byte_t>, "Buffer does not contain byte-like values");

  if constexpr (is_encoded<DecayedValue_t>) {
    return detail::buffer::write::_write_single_encoded_value<Byte_t>(buffer, std::forward<Value_T>(value));
  }
  else if constexpr (common::is_sized_range_v<DecayedValue_t>) {
    static_assert(is_buffer_writeable<typename DecayedValue_t::value_type>, "Only ranges of POD values can be directly written to a buffer");
    return detail::buffer::write::_write_single_range_value<Byte_t>(buffer, std::forward<Value_T>(value));
  }
  else if constexpr (is_buffer_writeable<DecayedValue_t>) {
    return detail::buffer::write::_write_single_value<Byte_t>(buffer, std::forward<Value_T>(value));
  }
  else {
    static_assert(is_buffer_writeable<DecayedValue_t>, "Type is not valid for writing to a buffer");
  }
}

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer::write {

  template <byte_iterator_like ByteIter_T, typename Value_T, typename... Value_Ts>
  size_t _recursive_unsafe_write(ByteIter_T begin, ByteIter_T end, Value_T&& first_value, Value_Ts&&... other_values) {
    auto out = io::unchecked_write(begin, std::forward<Value_T>(first_value));

    if constexpr (sizeof...(other_values) > 0) {
      out += _recursive_unsafe_write(std::next(begin, byte_count(first_value)), end, std::forward<Value_Ts>(other_values)...);
    }

    return out;
  }

  template <byte_range_like ByteRange_T, typename Value_T, typename... Value_Ts>
  size_t _recursive_unsafe_write(ByteRange_T&& buffer, Value_T&& first_value, Value_Ts&&... other_values) {
    return _recursive_unsafe_write(
        buffer.begin(), buffer.end(), std::forward<Value_T>(first_value), std::forward<Value_Ts>(other_values)...);
  }

}  // namespace detail::buffer::write

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_NO_EXCEPTIONS

template <byte_range_like ByteRange_T, typename... Value_Ts>
size_t write(ByteRange_T&& buffer, Value_Ts&&... values) {
  if (buffer.size() < byte_count(values...)) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  return detail::buffer::write::_recursive_unsafe_write(buffer, std::forward<Value_Ts>(values)...);
}

///////////////////////////////////////////////////////////////////////////////

template <byte_iterator_like ByteIter_T, typename... Value_Ts>
size_t write(ByteIter_T begin, ByteIter_T end, Value_Ts&&... values) {
  return write(std::span<std::decay_t<decltype(*begin)>>{begin, end}, std::forward<Value_Ts>(values)...);
}

#endif

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer::write {

  template <byte_range_like ByteRange_T, typename Value_T>
  write_result_t _try_write_single_value(ByteRange_T&& buffer, Value_T&& to_write) noexcept {
    if (buffer.size() < byte_count(to_write)) {
      return write_error::insufficient_buffer;
    }

    return unchecked_write(buffer.begin(), std::forward<Value_T>(to_write));
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <byte_range_like ByteRange_T, typename Range_T>
  write_result_t _try_write_range_value(ByteRange_T&& buffer, Range_T&& to_write) noexcept {
    if (buffer.size() < byte_count(to_write)) {
      return write_error::insufficient_buffer;
    }

    return unchecked_write(buffer.begin(), std::forward<Range_T>(to_write));
  }

}  // namespace detail::buffer::write

///////////////////////////////////////////////////////////////////////////////

template <byte_range_like ByteRange_T, typename Value_T>
write_result_t try_write(ByteRange_T&& buffer, Value_T&& to_write) noexcept {
  if constexpr (common::is_sized_range_v<std::decay_t<Value_T>>) {
    return detail::buffer::write::_try_write_range_value(std::forward<ByteRange_T>(buffer), std::forward<Value_T>(to_write));
  } else {
    return detail::buffer::write::_try_write_single_value(std::forward<ByteRange_T>(buffer), std::forward<Value_T>(to_write));
  }
}

///////////////////////////////////////////////////////////////////////////////

template <byte_iterator_like ByteIter_T, typename Value_T>
write_result_t try_write(ByteIter_T begin, ByteIter_T end, Value_T&& to_write) noexcept {
  if constexpr (common::is_sized_range_v<std::decay_t<Value_T>>) {
    return detail::buffer::write::_try_write_range_value(std::span<std::decay_t<decltype(*begin)>>{begin, end},
                                                         std::forward<Value_T>(to_write));
  } else {
    return detail::buffer::write::_try_write_single_value(std::span<std::decay_t<decltype(*begin)>>{begin, end},
                                                          std::forward<Value_T>(to_write));
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_writeable<Value_T>
write_result_t try_write_at(size_t position, std::span<io::byte> buffer, Value_T value) noexcept {
  if (position + byte_count(value) < position) {
    return write_error::invalid_position_offset;
  }

  if (position >= buffer.size()) {
    return write_error::insufficient_buffer;
  }

  const auto result = try_write(std::next(buffer.begin(), position), buffer.end(), value);
  if (result.is_error()) {
    return result;
  }

  return position + result.value();
}

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer::write {

  template <byte_iterator_like ByteIter_T, typename Value_T, typename... Value_Ts>
  write_result_t _recursive_try_write(ByteIter_T begin, ByteIter_T end, Value_T first_value, Value_Ts... other_values) {
    const auto first_result = try_write(begin, end, first_value);
    if (first_result.is_error()) {
      return first_result;
    }

    if constexpr (sizeof...(other_values) > 0) {
      const auto result =
          _recursive_try_write(std::next(begin, byte_count(first_value)), end, other_values...);
      if (result.is_error()) {
        return result;
      }

      return first_result.value() + result.value();
    } else {
      return first_result;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <byte_range_like ByteRange_T, typename Value_T, typename... Value_Ts>
  write_result_t _recursive_try_write(ByteRange_T&& buffer, Value_T first_value, Value_Ts... other_values) {
    const auto first_result = try_write(buffer, first_value);
    if (first_result.is_error()) {
      return first_result;
    }

    if constexpr (sizeof...(other_values) > 0) {
      const auto result = _recursive_try_write(std::next(buffer.begin(), byte_count(first_value)), buffer.end(), other_values...);
      if (result.is_error()) {
        return result;
      }

      return first_result.value() + result.value();
    } else {
      return first_result;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <typename Value_T, typename... Value_Ts>
  write_result_t _recursive_try_write_at(size_t position,
                                         std::span<io::byte> buffer,
                                         Value_T first_value,
                                         Value_Ts... other_values) {
    const auto first_result = try_write_at(position, buffer, first_value);
    if (first_result.is_error()) {
      return first_result;
    }

    if constexpr (sizeof...(other_values) > 0) {
      return _recursive_try_write_at(first_result.value(), buffer, other_values...);
    } else {
      return first_result;
    }
  }
}  // namespace detail::buffer::write

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_NO_EXCEPTIONS

template <byte_range_like ByteRange_T, typename... Value_Ts>
size_t write_at(size_t position, ByteRange_T&& buffer, Value_Ts&&... values) {
  const auto write_end_pos = position + byte_count(values...);
  if (write_end_pos < position) {
    throw std::invalid_argument{"Buffer read position exceeds allowed value"};
  }

  if (write_end_pos > buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  return position + detail::buffer::write::_recursive_unsafe_write(std::next(buffer.begin(), position), buffer.end(),
                                                                   std::forward<Value_Ts>(values)...);
}

#endif

///////////////////////////////////////////////////////////////////////////////

template <byte_range_like ByteRange_T, typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
write_result_t try_write(ByteRange_T&& buffer, Value_Ts&&... values) noexcept {
  return detail::buffer::write::_recursive_try_write(std::forward<ByteRange_T>(buffer), std::forward<Value_Ts>(values)...);
}

///////////////////////////////////////////////////////////////////////////////

template <byte_iterator_like ByteIter_T, typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
write_result_t try_write(ByteIter_T begin, ByteIter_T end, Value_Ts&&... values) noexcept {
  return detail::buffer::write::_recursive_try_write(begin, end, std::forward<Value_Ts>(values)...);
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
write_result_t try_write_at(size_t position, std::span<io::byte> buffer, Value_Ts&&... values) noexcept {
  return detail::buffer::write::_recursive_try_write_at(position, buffer, std::forward<Value_Ts>(values)...);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename Result_T = static_byte_buffer<sizeof(Value_T)>>
  requires is_buffer_writeable<Value_T>
result<Result_T, write_error> try_to_bytes(Value_T&& value) noexcept {
  auto out = Result_T{};

  const auto result = try_write(out, std::forward<Value_T>(value));
  if (result.ok()) {
    return {out};
  } else {
    return {result.error()};
  }
}

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_NO_EXCEPTIONS

template <typename Value_T, typename Result_T = static_byte_buffer<sizeof(std::decay_t<Value_T>)>>
  requires is_buffer_writeable<std::decay_t<Value_T>>
Result_T to_bytes(Value_T&& value) {
  auto out = Result_T{};

  write(out, std::forward<Value_T>(value));

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, byte_range_like ByteRange_T>
  requires is_buffer_writeable<Value_T>
size_t write_with_endian(ByteRange_T&& buffer, Value_T value, endian endianness) {
  if (endian::little == endianness) {
    return write(buffer, little_endian{value});
  } else {
    return write(buffer, big_endian{value});
  }
}

template <typename Value_T, byte_iterator_like ByteIter_T>
  requires is_buffer_writeable<Value_T>
size_t write_with_endian(ByteIter_T begin, ByteIter_T end, Value_T value, endian endianness) {
  return write_with_endian(std::span<std::decay_t<decltype(*begin)>>{begin, end}, value, endianness);
}

#endif

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
