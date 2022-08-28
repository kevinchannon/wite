#pragma once

#include <array>
#include <vector>
#include <cstddef>

namespace wite::io {

template<size_t N>
using stack_byte_buffer = std::array<std::byte, N>;

using dynamic_byte_buffer = std::vector<std::byte>;
}
