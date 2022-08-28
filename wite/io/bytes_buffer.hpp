#pragma once

#include <algorithm>
#include <cstddef>
#include <istream>
#include <iterator>
#include <span>
#include <stdexcept>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io::buffers {

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, std::endian ENDIANNESS = std::endian::native>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T> Value_T
read(std::span<const std::byte> buffer) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

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

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
Value_T read(std::span<const std::byte> buffer, std::endian endianness) {
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

  if constexpr (std::endian::little == ENDIANNESS) {
    std::copy_n(reinterpret_cast<std::byte*>(&value), sizeof(Value_T), buffer.begin());
  } else {
    std::copy_n(std::make_reverse_iterator(std::next(reinterpret_cast<std::byte*>(&value), sizeof(Value_T))),
                sizeof(Value_T),
                buffer.begin());
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
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T> Value_T
read(byte_read_buffer_view& buffer) {
  const auto out = read<Value_T, ENDIANNESS>({buffer.read_position, buffer.data.end()});
  std::advance(buffer.read_position, sizeof(Value_T));

  return out;
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

// TODO: This should be in its own "streams" io file.
template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
Value_T read(std::istream& stream) {
  auto out = Value_T{};
  stream.read(reinterpret_cast<char*>(&out), sizeof(Value_T));
  if (stream.eof()) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io::buffers

///////////////////////////////////////////////////////////////////////////////
