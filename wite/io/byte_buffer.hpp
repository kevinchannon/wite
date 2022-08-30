#pragma once

#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <bit>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires(std::is_standard_layout_v<Value_T>and std::is_trivial_v<Value_T> and
         (not std::is_base_of_v<io::encoding, Value_T>)) 
Value_T unchecked_read(const std::span<const std::byte>& buffer) {
  auto out = Value_T{};

  if constexpr (std::endian::little == ENDIANNESS) {
    std::copy_n(buffer.begin(), sizeof(Value_T), reinterpret_cast<std::byte*>(&out));
  } else {
    std::copy_n(buffer.begin(),
                sizeof(Value_T),
                std::make_reverse_iterator(std::next(reinterpret_cast<std::byte*>(&out), sizeof(Value_T))));
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires(std::is_standard_layout_v<Value_T>and std::is_trivial_v<Value_T> and
         (not std::is_base_of_v<io::encoding, Value_T>))
Value_T read(const std::span<const std::byte>& buffer) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }
  
  return unchecked_read<Value_T, ENDIANNESS>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires(std::is_standard_layout_v<Value_T>and std::is_trivial_v<Value_T> and
         (not std::is_base_of_v<io::encoding, Value_T>))
read_result_t<Value_T> try_read(const std::span<const std::byte>& buffer) {
  if (buffer.size() < sizeof(Value_T)) {
    return read_error::insufficient_buffer;
  }

  return unchecked_read<Value_T, ENDIANNESS>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_base_of_v<io::encoding, Value_T>
typename Value_T::value_type read(const std::span<const std::byte>& buffer) {
  using OuptutValue_t = typename Value_T::value_type;

  if constexpr (std::is_same_v<io::little_endian<OuptutValue_t>, Value_T>) {
    return read<OuptutValue_t, std::endian::little>(buffer);
  } else if constexpr (std::is_same_v<io::big_endian<OuptutValue_t>, Value_T>) {
    return read<OuptutValue_t, std::endian::big>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_base_of_v<io::encoding, Value_T>
read_result_t<typename Value_T::value_type> try_read(const std::span<const std::byte>& buffer) {
  using OuptutValue_t = typename Value_T::value_type;

  if constexpr (std::is_same_v<io::little_endian<OuptutValue_t>, Value_T>) {
    return try_read<OuptutValue_t, std::endian::little>(buffer);
  } else if constexpr (std::is_same_v<io::big_endian<OuptutValue_t>, Value_T>) {
    return try_read<OuptutValue_t, std::endian::big>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
Value_T read(const std::span<const std::byte>& buffer, std::endian endianness) {
  if (std::endian::little == endianness) {
    return read<Value_T, std::endian::little>(buffer);
  } else {
    return read<Value_T, std::endian::big>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <std::endian ENDIANNESS = std::endian::native, typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
void write(std::span<std::byte> buffer, Value_T value) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  if constexpr (std::is_base_of_v<io::encoding, Value_T>) {
    if constexpr (std::is_same_v<io::little_endian<typename Value_T::value_type>, Value_T>) {
      write<std::endian::little>(buffer, value.value);
    } else if constexpr (std::is_same_v<io::big_endian<typename Value_T::value_type>, Value_T>) {
      write<std::endian::big>(buffer, value.value);
    }
  } else {
    if constexpr (std::endian::little == ENDIANNESS) {
      std::copy_n(reinterpret_cast<std::byte*>(&value), sizeof(Value_T), buffer.begin());
    } else {
      std::copy_n(std::make_reverse_iterator(std::next(reinterpret_cast<std::byte*>(&value), sizeof(Value_T))),
                  sizeof(Value_T),
                  buffer.begin());
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
void write(std::span<std::byte> buffer, Value_T value, std::endian endianness) {
  if (std::endian::little == endianness) {
    write<std::endian::little>(buffer, value);
  } else {
    write<std::endian::big>(buffer, value);
  }
}

///////////////////////////////////////////////////////////////////////////////

struct byte_read_buffer_view {
  explicit byte_read_buffer_view(std::span<const std::byte> buf) : data{std::move(buf)}, read_position{data.begin()} {}

  byte_read_buffer_view(std::span<const std::byte> buf, typename std::span<const std::byte>::size_type offset)
      : data{std::move(buf)}, read_position{std::next(data.begin(), offset)} {}

  std::span<const std::byte> data;
  std::span<const std::byte>::iterator read_position;
};

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T> and (not std::is_base_of_v<io::encoding, Value_T>) 
Value_T read(byte_read_buffer_view& buffer) {
  const auto out = read<Value_T, ENDIANNESS>({buffer.read_position, buffer.data.end()});
  std::advance(buffer.read_position, sizeof(Value_T));

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T> and
    (not std::is_base_of_v<io::encoding, Value_T>)
read_result_t<Value_T> try_read(byte_read_buffer_view& buffer) {
  const auto out = try_read<Value_T, ENDIANNESS>({buffer.read_position, buffer.data.end()});
  std::advance(buffer.read_position, sizeof(Value_T));

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_base_of_v<io::encoding, Value_T>
read_result_t<typename Value_T::value_type> try_read(byte_read_buffer_view& buffer) {
  using OuptutValue_t = typename Value_T::value_type;

  if constexpr (std::is_same_v<io::little_endian<OuptutValue_t>, Value_T>) {
    return try_read<OuptutValue_t, std::endian::little>(buffer);
  } else if constexpr (std::is_same_v<io::big_endian<OuptutValue_t>, Value_T>) {
    return try_read<OuptutValue_t, std::endian::big>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
Value_T read(byte_read_buffer_view& buffer, std::endian endienness) {
  const auto out = read<Value_T>({buffer.read_position, buffer.data.end()}, endienness);
  std::advance(buffer.read_position, sizeof(Value_T));

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_base_of_v<io::encoding, Value_T>
typename Value_T::value_type read(byte_read_buffer_view& buffer) {
  using OuptutValue_t = typename Value_T::value_type;

  if constexpr (std::is_same_v<io::little_endian<OuptutValue_t>, Value_T>) {
    return read<OuptutValue_t, std::endian::little>(buffer);
  } else if constexpr (std::is_same_v<io::big_endian<OuptutValue_t>, Value_T>) {
    return read<OuptutValue_t, std::endian::big>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

struct byte_write_buffer_view {
  explicit byte_write_buffer_view(std::span<std::byte> buf) : data{std::move(buf)}, write_position{data.begin()} {}

  byte_write_buffer_view(std::span<std::byte> buf, typename std::span<const std::byte>::size_type offset)
      : data{std::move(buf)}, write_position{std::next(data.begin(), offset)} {}

  std::span<std::byte> data;
  std::span<std::byte>::iterator write_position;
};

///////////////////////////////////////////////////////////////////////////////

template <std::endian ENDIANNESS = std::endian::native, typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
void write(byte_write_buffer_view& buffer, Value_T value) {
  write<ENDIANNESS, Value_T>({buffer.write_position, buffer.data.end()}, value);
  std::advance(buffer.write_position, sizeof(Value_T));
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
void write(byte_write_buffer_view& buffer, Value_T value, std::endian endianness) {
  write<Value_T>({buffer.write_position, buffer.data.end()}, value, endianness);
  std::advance(buffer.write_position, sizeof(Value_T));
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
