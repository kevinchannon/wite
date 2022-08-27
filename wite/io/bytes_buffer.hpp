#pragma once

#include <algorithm>
#include <cstddef>
#include <istream>
#include <iterator>
#include <span>

namespace wite::io::buffers {

template <typename Value_T>
Value_T read(std::span<const std::byte> buffer) {
  auto out = Value_T{};
  std::copy_n(buffer.begin(), sizeof(Value_T), reinterpret_cast<std::byte*>(&out));
  return out;
}

template <typename Value_T>
Value_T read(std::istream& stream) {
  auto out = Value_T{};
  stream.read(reinterpret_cast<char*>(&out), sizeof(Value_T));

  return out;
}

}  // namespace wite::io::buffers
