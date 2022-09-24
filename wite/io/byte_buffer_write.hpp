#pragma once

#include <wite/env/features.hpp>

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
#include <numeric>

#if !_WITE_HAS_CONCEPTS
#error "C++20 concepts are require, but the compiler doesn't support them"
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

namespace detail::buffer::write {

  template <typename Value_T>
    requires((not std::is_standard_layout_v<Value_T>) or (not std::is_trivial_v<Value_T>))
  constexpr auto value_size() noexcept {
    // This will fail to build if the type satisfies the reuirements but doesn't have a value_type alias in it.
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
}  // namespace detail::buffer::write

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
size_t unchecked_write(auto buffer, Value_T value) {
  if constexpr (is_encoded<Value_T>) {
    using RawValue_t = typename Value_T::value_type;

    if constexpr (std::is_same_v<little_endian<RawValue_t>, Value_T>) {
      std::copy_n(reinterpret_cast<io::byte*>(&value.value), sizeof(value.value), buffer);
    } else if constexpr (std::is_same_v<big_endian<RawValue_t>, Value_T>) {
      std::copy_n(std::make_reverse_iterator(std::next(reinterpret_cast<io::byte*>(&value.value), sizeof(value.value))),
                  sizeof(value.value),
                  buffer);
    }

    return sizeof(RawValue_t);
  } else {
    std::copy_n(reinterpret_cast<io::byte*>(&value), sizeof(value), buffer);

    return sizeof(Value_T);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
size_t write(std::span<io::byte> buffer, Value_T value) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  return unchecked_write<Value_T>(buffer.begin(), value);
}

///////////////////////////////////////////////////////////////////////////////


template <typename Range_T>
size_t write(std::span<io::byte> buffer, Range_T&& values) {
  using range_t = std::decay_t<Range_T>;
  
  if (buffer.size() < values.size() * sizeof(typename range_t::value_type)) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  struct _write_info {
    size_t bytes_written{};
    std::span<io::byte>::iterator next_write_position{};
  };

  return std::accumulate(
             values.begin(),
             values.end(),
             _write_info{0, buffer.begin()},
             [](auto&& current, const auto& value) -> _write_info{
               return {
                   current.bytes_written + unchecked_write(current.next_write_position, value),
                                   std::next(current.next_write_position,
                                             detail::buffer::write::template value_size<std::decay_t<decltype(value)>>())};
             })
      .bytes_written;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_writeable<Value_T>
size_t write_at(size_t position, std::span<io::byte> buffer, Value_T value) {
  if (position + sizeof(value) < position) {
    throw std::invalid_argument{"Buffer write position exceeds allowed value"};
  }

  if (position >= buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  return position + write({std::next(buffer.begin(), position), buffer.end()}, value);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename Result_T = static_byte_buffer<sizeof(Value_T)>>
requires is_buffer_writeable<Value_T> Result_T to_bytes(Value_T value) {
  auto out = Result_T{};

  write(out, value);

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_writeable<Value_T>
write_result_t try_write(std::span<io::byte> buffer, Value_T value) {
  if (buffer.size() < sizeof(Value_T)) {
    return write_error::insufficient_buffer;
  }

  return unchecked_write<Value_T>(buffer.begin(), value);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_writeable<Value_T>
write_result_t try_write_at(size_t position, std::span<io::byte> buffer, Value_T value) {
  if (position + sizeof(value) < position) {
    return write_error::invalid_position_offset;
  }

  if (position >= buffer.size()) {
    return write_error::insufficient_buffer;
  }

  const auto result = try_write({std::next(buffer.begin(), position), buffer.end()}, value);
  if (result.is_error()) {
    return result;
  }

  return position + result.value();
}

///////////////////////////////////////////////////////////////////////////////

namespace detail::buffer::write {

  template <typename Value_T, typename... Value_Ts>
  write_result_t _recursive_try_write(std::span<io::byte> buffer, Value_T first_value, Value_Ts... other_values) {
    const auto first_result = try_write(buffer, first_value);
    if (first_result.is_error()) {
      return first_result;
    }

    if constexpr (sizeof...(other_values) > 0) {
      const auto result = _recursive_try_write({std::next(buffer.begin(), value_size<Value_T>()), buffer.end()}, other_values...);
      if (result.is_error()) {
        return result;
      }

      return first_result.value() + result.value();
    } else {
      return first_result;
    }
  }

  template <typename Value_T, typename... Value_Ts>
  write_result_t _recursive_try_write_at(size_t position, std::span<io::byte> buffer, Value_T first_value, Value_Ts... other_values) {
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

  template <typename Value_T, typename... Value_Ts>
  size_t _recursive_write(std::span<io::byte> buffer, Value_T first_value, Value_Ts... other_values) {
    auto out = io::unchecked_write(buffer.begin(), first_value);

    if constexpr (sizeof...(other_values) > 0) {
      out +=
          _recursive_write(std::span<io::byte>{std::next(buffer.begin(), value_size<Value_T>()), buffer.end()}, other_values...);
    }

    return out;
  }

}  // namespace detail::buffer::write

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
size_t write(std::span<io::byte> buffer, Value_Ts... values) {
  if (detail::buffer::write::byte_count<Value_Ts...>() > buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  return detail::buffer::write::_recursive_write(buffer, values...);
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
size_t write_at(size_t position, std::span<io::byte> buffer, Value_Ts... values) {
  const auto write_end_pos = position + detail::buffer::write::byte_count<Value_Ts...>();
  if (write_end_pos < position) {
    throw std::invalid_argument{"Buffer read position exceeds allowed value"};
  }

  if (write_end_pos > buffer.size()) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  return position + detail::buffer::write::_recursive_write({std::next(buffer.begin(), position), buffer.end()}, values...);
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
write_result_t try_write(std::span<io::byte> buffer, Value_Ts... values) noexcept {
  return detail::buffer::write::_recursive_try_write(buffer, values...);
}

///////////////////////////////////////////////////////////////////////////////

template <typename... Value_Ts>
  requires(sizeof...(Value_Ts) > 1)
write_result_t try_write_at(size_t position, std::span<io::byte> buffer, Value_Ts... values) noexcept {
  return detail::buffer::write::_recursive_try_write_at(position, buffer, values...);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename Result_T = static_byte_buffer<sizeof(Value_T)>>
  requires is_buffer_writeable<Value_T>
result<Result_T, write_error> try_to_bytes(Value_T value) {
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
requires is_buffer_writeable<Value_T>
size_t write(std::span<io::byte> buffer, Value_T value, endian endianness) {
  if (endian::little == endianness) {
    return write(buffer, little_endian{value});
  } else {
    return write(buffer, big_endian{value});
  }
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
