#pragma once

#include <istream>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io::stream {

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

}  // namespace wite::io::stream

///////////////////////////////////////////////////////////////////////////////
