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

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
Value_T read(std::span<const std::byte> buffer) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  auto out = Value_T{};
  std::copy_n(buffer.begin(), sizeof(Value_T), reinterpret_cast<std::byte*>(&out));
  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
void write(std::span<std::byte> buffer, Value_T value) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for write"};
  }

  std::copy_n(reinterpret_cast<std::byte*>(&value), sizeof(Value_T), buffer.begin());
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

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
Value_T read(byte_read_buffer_view& buffer) {
  const auto out = read<Value_T>({buffer.read_position, buffer.data.end()});
  std::advance(buffer.read_position, sizeof(Value_T));

  return out;
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
