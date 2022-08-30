#pragma once

#include <wite/core/result.hpp>

#include <array>
#include <cstddef>
#include <vector>

namespace wite::io {

template <size_t N>
using stack_byte_buffer = std::array<std::byte, N>;

using dynamic_byte_buffer = std::vector<std::byte>;

enum class read_error {
  insufficient_buffer
};

template<typename Value_T>
using read_result_t = wite::result<Value_T, read_error>;

}  // namespace wite::io
