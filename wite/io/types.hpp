#pragma once

#include <wite/core/result.hpp>
#include <wite/env/environment.hpp>

#include <array>
#include <cstddef>
#include <vector>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

using byte = WITE_BYTE;

template <typename Result_T>
_WITE_NODISCARD Result_T to_integer(wite::io::byte b) {
#if _WITE_FEATURE_USE_STD_BYTE
  return std::to_integer<Result_T>(b);
#else
  return static_cast<Result_T>(b);
#endif
}

///////////////////////////////////////////////////////////////////////////////

template <size_t N>
using static_byte_buffer = std::array<byte, N>;

using dynamic_byte_buffer = std::vector<byte>;

///////////////////////////////////////////////////////////////////////////////

enum class read_error { insufficient_buffer, invalid_position_offset };

template <typename Value_T>
using read_result_t = wite::result<Value_T, read_error>;

///////////////////////////////////////////////////////////////////////////////

enum class write_error { insufficient_buffer, invalid_position_offset };

using write_result_t = wite::result<size_t, write_error>;

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
