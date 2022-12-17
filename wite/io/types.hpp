#pragma once

#include <wite/core/result.hpp>
#include <wite/env/environment.hpp>
#include <wite/common/concepts.hpp>

#include <array>
#include <cstddef>
#include <vector>
#include <type_traits>
#include <limits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

using byte = WITE_BYTE;

template <typename Result_T, common::byte_like Value_T>
  requires std::is_unsigned_v<Result_T>
_WITE_NODISCARD Result_T to_integer(Value_T value) {
#if _WITE_FEATURE_USE_STD_BYTE
  if constexpr (std::is_same_v<wite::io::byte, std::decay_t<Value_T>>) {
    return std::to_integer<Result_T>(value);
  } else {
#endif
    if constexpr (std::is_unsigned_v<std::decay_t<Value_T>>) {
      return static_cast<Result_T>(value);
    } else {
      if (value >= 0) {
        return static_cast<Result_T>(value);
      } else {
        return static_cast<Result_T>(value) + std::numeric_limits<std::make_unsigned_t<std::decay_t<Value_T>>>::max() + 1;
      }
    }
#if _WITE_FEATURE_USE_STD_BYTE
  }
#endif
}

///////////////////////////////////////////////////////////////////////////////

template <size_t N>
using static_byte_buffer = std::array<byte, N>;

using dynamic_byte_buffer = std::vector<byte>;

///////////////////////////////////////////////////////////////////////////////

enum class read_error { insufficient_buffer, invalid_position_offset, file_not_found };

template <typename Value_T>
using read_result_t = wite::result<Value_T, read_error>;

///////////////////////////////////////////////////////////////////////////////

enum class write_error { insufficient_buffer, invalid_position_offset };

using write_result_t = wite::result<size_t, write_error>;

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
