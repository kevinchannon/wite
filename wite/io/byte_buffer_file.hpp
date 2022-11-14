#pragma once

#include <wite/io/types.hpp>

#include <cstdio>
#include <filesystem>

namespace wite::io {

dynamic_byte_buffer read(const std::filesystem::path& path, size_t count) {
  auto out = dynamic_byte_buffer(count);

  auto file_pointer = std::fopen(path.c_str(), "r");
  std::fread(out.data(), 1, count, file_pointer);
  std::fclose(file_pointer);

  return out;
}

}  // namespace wite::io
