#pragma once

#include <istream>
#include <ostream>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T> Value_T read(std::istream& stream) {
  auto out = Value_T{};
  stream.read(reinterpret_cast<char*>(&out), sizeof(Value_T));
  if (stream.eof()) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
void write(std::ostream& stream, Value_T value) {
  stream.write(reinterpret_cast<char*>(&value), sizeof(Value_T));
}

}  // namespace wite::io::stream

///////////////////////////////////////////////////////////////////////////////
