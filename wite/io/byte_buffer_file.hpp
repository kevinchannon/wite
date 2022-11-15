#pragma once

#include <wite/env/environment.hpp>

#include <wite/io/types.hpp>

#include <cstdio>
#include <filesystem>
#include <optional>
#include <stdexcept>

namespace wite::io {

namespace detail {
FILE* get_read_file_pointer(const std::filesystem::path& path) noexcept {
#ifdef _CRT_FUNCTIONS_REQUIRED
    FILE* file_pointer = nullptr;
    const auto ec      = fopen_s(&file_pointer, path.string().c_str(), "rb");
    return 0 == ec ? file_pointer : nullptr;
#else
    return std::fopen(path.string().c_str(), "rb");
#endif
  }
}

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
  auto file_pointer = detail::get_read_file_pointer(path.string().c_str());
  if (not file_pointer) {
    throw std::invalid_argument{"cannot read invalid path"};
  }

  auto out = unsafe_read(file_pointer, not count.has_value() ? file_size(file_pointer) : std::min(*count, file_size(file_pointer)));
  std::fclose(file_pointer);

  return out;
}

_WITE_NODISCARD inline read_result_t<dynamic_byte_buffer> try_read(const std::filesystem::path& path, std::optional<size_t> count = std::nullopt) {
  auto file_pointer = detail::get_read_file_pointer(path.string().c_str());
  if (not file_pointer) {
    return {read_error::file_not_found};
  }

  auto out =
      unsafe_read(file_pointer, not count.has_value() ? file_size(file_pointer) : std::min(*count, file_size(file_pointer)));
  std::fclose(file_pointer);

  return out;
}

}  // namespace wite::io
