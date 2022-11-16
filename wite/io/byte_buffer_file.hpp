#pragma once

#include <wite/env/environment.hpp>

#include <wite/io/types.hpp>
#include <wite/common/concepts.hpp>

#include <cstdio>
#include <filesystem>
#include <optional>
#include <stdexcept>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

namespace detail {

///////////////////////////////////////////////////////////////////////////////

_WITE_NODISCARD inline FILE* get_file_pointer(const std::filesystem::path& path, const char* mode) noexcept {
#ifdef _CRT_FUNCTIONS_REQUIRED
  FILE* file_pointer = nullptr;
  const auto ec      = fopen_s(&file_pointer, path.string().c_str(), mode);
  return 0 == ec ? file_pointer : nullptr;
#else
  return std::fopen(path.string().c_str(), mode);
#endif
}

///////////////////////////////////////////////////////////////////////////////

_WITE_NODISCARD inline FILE* get_read_file_pointer(const std::filesystem::path& path) noexcept {
  return get_file_pointer(path, "rb");
}

///////////////////////////////////////////////////////////////////////////////

_WITE_NODISCARD inline FILE* get_write_file_pointer(const std::filesystem::path& path) noexcept {
  return get_file_pointer(path, "wb");
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template<common::contiguous_range_type Range_T>
 inline void unsafe_read(FILE* file_pointer, Range_T& out) noexcept {
  std::fread(out.data(), 1, out.size(), file_pointer);
}

///////////////////////////////////////////////////////////////////////////////

template <common::contiguous_range_type Range_T>
inline void unsafe_write(FILE* file_pointer, size_t count, Range_T&& bytes) noexcept {
  std::fwrite(bytes.data(), 1, count, file_pointer);
}

///////////////////////////////////////////////////////////////////////////////

_WITE_NODISCARD inline auto file_size(FILE* file_pointer) noexcept {
  const auto initial_offset = std::ftell(file_pointer);

  std::fseek(file_pointer, 0, SEEK_END);
  auto file_size_in_bytes = static_cast<size_t>(std::ftell(file_pointer));
  std::fseek(file_pointer, initial_offset, SEEK_SET);

  return file_size_in_bytes;
}

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_NO_EXCEPTIONS
_WITE_NODISCARD inline dynamic_byte_buffer read(const std::filesystem::path& path, std::optional<size_t> count = std::nullopt) {
  auto file_pointer = detail::get_read_file_pointer(path.string().c_str());
  if (not file_pointer) {
    throw std::invalid_argument{"cannot read invalid path"};
  }

  auto out = dynamic_byte_buffer(not count.has_value() ? file_size(file_pointer) : std::min(*count, file_size(file_pointer)));
  unsafe_read(file_pointer, out);
  std::fclose(file_pointer);

  return out;
}
#endif

///////////////////////////////////////////////////////////////////////////////

template<common::contiguous_range_type Range_T>
_WITE_NODISCARD inline read_result_t<typename Range_T::size_type> try_read(const std::filesystem::path& path, Range_T& out) noexcept {
  auto file_pointer = detail::get_read_file_pointer(path.string().c_str());
  if (not file_pointer) {
    return {read_error::file_not_found};
  }

  if (const auto size = file_size(file_pointer); size < out.size()) {
    out.resize(size);
  }

  unsafe_read(file_pointer, out);
  std::fclose(file_pointer);

  return out.size();
}

///////////////////////////////////////////////////////////////////////////////

template<common::contiguous_range_type Range_T>
void write(const std::filesystem::path& path, size_t count, Range_T&& bytes) {
  auto file_pointer = detail::get_write_file_pointer(path);
  unsafe_write(file_pointer, count, std::forward<Range_T>(bytes));
  std::fclose(file_pointer);
}

///////////////////////////////////////////////////////////////////////////////

template <common::contiguous_range_type Range_T>
void write(const std::filesystem::path& path, Range_T&& bytes) {
  auto file_pointer = detail::get_write_file_pointer(path);
  const auto size   = std::distance(bytes.begin(), bytes.end());
  unsafe_write(file_pointer, size, std::forward<Range_T>(bytes));
  std::fclose(file_pointer);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
