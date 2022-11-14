#pragma once

#include <wite/io/types.hpp>

#include <cstdio>
#include <filesystem>
#include <optional>

namespace wite::io {

_WITE_NODISCARD inline dynamic_byte_buffer read(const std::filesystem::path& path, std::optional<size_t> count = std::nullopt) {
  auto file_pointer = std::fopen(path.c_str(), "r");

  if (not count.has_value()) {
    std::fseek(file_pointer, 0, SEEK_END);
    *count = static_cast<size_t>(std::ftell(file_pointer));
    std::rewind(file_pointer);
  }

  auto out = dynamic_byte_buffer(*count);

  std::fread(out.data(), 1, *count, file_pointer);
  std::fclose(file_pointer);

  return out;
}

}  // namespace wite::io
