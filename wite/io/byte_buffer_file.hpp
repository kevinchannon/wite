#pragma once

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

_WITE_NODISCARD inline dynamic_byte_buffer read(const std::filesystem::path& path, std::optional<size_t> count = std::nullopt) {
  auto file_pointer = std::fopen(path.c_str(), "rb");
  if (not file_pointer){
    throw std::invalid_argument{"cannot read invalid path"};
  }

  std::fseek(file_pointer, 0, SEEK_END);
  const auto file_size_in_bytes = static_cast<size_t>(std::ftell(file_pointer));
  std::rewind(file_pointer);

  if (not count.has_value()) {
    *count = file_size_in_bytes;
  } else {
    *count = std::min(*count, file_size_in_bytes);
  }

  auto out = unsafe_read(file_pointer, *count);

  std::fclose(file_pointer);

  return out;
}

}  // namespace wite::io
