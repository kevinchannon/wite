#pragma once

#include <wite/env/environment.hpp>

#include <wite/io/types.hpp>

#include <cstdio>
#include <filesystem>
#include <optional>
#include <stdexcept>

namespace wite::io {

_WITE_NODISCARD inline dynamic_byte_buffer unsafe_read(FILE* file_pointer, size_t count) {
  auto out = dynamic_byte_buffer(count);
  std::fread(out.data(), 1, count, file_pointer);
  return out;
}

_WITE_NODISCARD auto file_size(FILE* file_pointer) {
  const auto initial_offset = std::ftell(file_pointer);

  std::fseek(file_pointer, 0, SEEK_END);
  auto file_size_in_bytes = static_cast<size_t>(std::ftell(file_pointer));
  std::fseek(file_pointer, initial_offset, SEEK_SET);

  return file_size_in_bytes;
}

_WITE_NODISCARD inline dynamic_byte_buffer read(const std::filesystem::path& path, std::optional<size_t> count = std::nullopt) {
#ifdef _WITE_COMPILER_MSVC
  FILE* file_pointer = nullptr;
  const auto ec = fopen_s(&file_pointer, path.string().c_str(), "rb");
  if (0 != ec) {
    throw std::invalid_argument{"cannot read invalid path"};
  }
#else
  auto file_pointer = std::fopen(path.string().c_str(), "rb");
  if (not file_pointer) {
    throw std::invalid_argument{"cannot read invalid path"};
  }
#endif

  auto out = unsafe_read(file_pointer, not count.has_value() ? file_size(file_pointer) : std::min(*count, file_size(file_pointer)));
  std::fclose(file_pointer);

  return out;
}

}  // namespace wite::io
