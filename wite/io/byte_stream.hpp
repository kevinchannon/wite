/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/env/features.hpp>

#include <istream>
#include <ostream>

#if !_WITE_HAS_CONCEPTS
#error "C++20 concepts are require, but the compiler doesn't support them"
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires std::is_standard_layout_v<Value_T> and std::is_trivial_v<Value_T>
Value_T read(std::istream& stream) {
  auto out = Value_T{};
  stream.read(reinterpret_cast<char*>(&out), sizeof(Value_T));

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
