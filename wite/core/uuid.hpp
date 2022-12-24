#pragma once

#include <wite/core/basic_uuid.hpp>
#include <wite/core/uuid_functions.hpp>
#include <wite/core/uuid_hash.hpp>
#include <wite/binascii/hexlify.hpp>
#include <wite/common/constructor_macros.hpp>
#include <wite/env/features.hpp>
#include <wite/io/byte_buffer.hpp>
#include <wite/core/result.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <compare>
#include <cstdint>
#include <random>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <tuple>
#ifndef WITE_NO_EXCEPTIONS
#include <stdexcept>
#endif
#if _WITE_HAS_CONCEPTS
#include <concepts>
#endif

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_DEFAULT_UUID_FMT
#define WITE_DEFAULT_UUID_FMT 'D'
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

struct uuid : public basic_uuid {

  WITE_DEFAULT_CONSTRUCTORS(uuid);

  explicit uuid(Storage_t data) : basic_uuid{data} {}

#ifndef WITE_NO_EXCEPTIONS
  uuid(uint32_t d1, uint16_t d2, uint16_t d3, std::array<uint8_t, 8> d4) : basic_uuid{} {
    io::write(data, io::little_endian{d1}, io::little_endian{d2}, io::little_endian{d3}, d4);
  }
#endif

#if _WITE_HAS_CONCEPTS
  template <std::invocable Engine_T>
  explicit uuid(Engine_T&& engine)
#else
  explicit uuid(std::default_random_engine& engine)
#endif
    : basic_uuid{}
  {
    auto random_bits = std::uniform_int_distribution<uint64_t>{0x00, 0xFFFFFFFFFFFFFFFF};

    *reinterpret_cast<uint64_t*>(&data) = random_bits(engine);
    data[6]                             = static_cast<uint8_t>((data[6] & 0b0000'1111) | 0b0100'0000);  // Version 4 UUID

    *(reinterpret_cast<uint64_t*>(&data) + 1) = random_bits(engine);
    data[8]                             = static_cast<uint8_t>((data[8] & 0b0011'1111) | 0b1000'0000);  // Variant 1 UUID
  }

#ifndef WITE_NO_EXCEPTIONS
  explicit uuid(const std::string_view s, char format = default_uuid_format) : uuid{} {
    _init_from_string<char>(s, format);
  }

  explicit uuid(const std::wstring_view s, char format = default_uuid_format) : uuid{} {
    _init_from_string<wchar_t>(s, format);
  }
#endif

  constexpr auto operator<=>(const uuid&) const noexcept = default;

  _WITE_NODISCARD bool into_c_str(char* out, size_t size, char format = default_uuid_format) const noexcept {
    return to_c_str(*this, out, size, format);
  }
  _WITE_NODISCARD bool into_c_str(wchar_t* out, size_t size, char format = default_uuid_format) const noexcept {
    return to_c_str(*this, out, size, format);
  }
  _WITE_NODISCARD std::string str(char format = default_uuid_format) const { return to_string(*this, format); };
  _WITE_NODISCARD std::wstring wstr(char format = default_uuid_format) const { return to_wstring(*this, format); };

 private:
#ifndef WITE_NO_EXCEPTIONS
  template<typename Char_T>
  void _init_from_string(std::basic_string_view<Char_T> s, char format) {
    switch (format) {
      case 'N':
      case 'n': {
        _init_from_wrapped_fmt_string<Char_T>(s, uuid_format::N);
        break;
      }
      case 'D':
      case 'd': {
        _init_from_wrapped_fmt_string<Char_T>(s, uuid_format::D);
        break;
      }
      case 'B':
      case 'b': {
        _init_from_wrapped_fmt_string<Char_T>(s, uuid_format::B);
        break;
      }
      case 'P':
      case 'p': {
        _init_from_wrapped_fmt_string<Char_T>(s, uuid_format::P);
        break;
      }
      case 'X':
      case 'x': {
        _init_from_wrapped_fmt_string<Char_T>(s, uuid_format::X);
        break;
      }
      default:
        throw std::invalid_argument{"Invalid UUID format type"};
    }
  }

  template<typename Char_T, typename Format_T>
  void _init_from_wrapped_fmt_string(std::basic_string_view<Char_T> s, Format_T&& fmt) {
    if (not fmt.template is_a_valid_uuid_string<Char_T>(s)) {
      throw std::invalid_argument{"Invalid UUID format"};
    }

    auto data_as_hex_chars = std::array<Char_T, 2 * std::tuple_size_v<Storage_t>>{};
    detail::uuid::strip_non_hex_characters<Char_T>(s, fmt.prefixed_values, data_as_hex_chars);

    if (std::distance(data_as_hex_chars.begin(), std::ranges::find_if(data_as_hex_chars | std::views::reverse, [](auto ch) {
                                                   return ch != Char_T{};
                                                 }).base()) != static_cast<int64_t>(data_as_hex_chars.size())) {
      throw std::invalid_argument{"Invalid UUID format"};
    }

    data = unsafe_basic_uuid_from_string<Char_T>(std::basic_string_view{data_as_hex_chars.begin(), data_as_hex_chars.end()}).data;
  }
#endif
};

///////////////////////////////////////////////////////////////////////////////

constexpr static auto nulluuid = uuid{};

///////////////////////////////////////////////////////////////////////////////

inline uuid make_uuid() {
  static thread_local auto random_engine = std::mt19937_64(std::random_device{}());
  return uuid{random_engine};
}

///////////////////////////////////////////////////////////////////////////////

enum class make_uuid_error {
  invalid_uuid_format,
  invalid_uuid_format_type
};

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T>
uuid unsafe_uuid_from_string(std::basic_string_view<Char_T> s) {
  auto out = uuid{};

  out = binascii::unsafe_unhexlify<16, uint8_t>(s.data());
  detail::uuid::format_as_uuid_data(out.data);

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template<typename Char_T, typename Format_T>
result<uuid, make_uuid_error> try_make_uuid(std::basic_string_view<Char_T> s, Format_T&& fmt) noexcept {
  if (not fmt.template is_a_valid_uuid_string(s)){
    return {make_uuid_error::invalid_uuid_format};
  }

  auto data_as_hex_chars = std::array<Char_T, 2 * std::tuple_size_v<uuid::Storage_t>>{};
  detail::uuid::strip_non_hex_characters(s, fmt.prefixed_values, data_as_hex_chars);

  if (std::distance(data_as_hex_chars.begin(), std::ranges::find_if(data_as_hex_chars | std::views::reverse, [](auto ch) {
                                                 return ch != Char_T{};
                                               }).base()) != static_cast<int64_t>(data_as_hex_chars.size())) {
    return {make_uuid_error::invalid_uuid_format};
  }

  auto out = uuid{};
  out.data = unsafe_basic_uuid_from_string<Char_T>(std::basic_string_view{data_as_hex_chars.begin(), data_as_hex_chars.end()}).data;

  return {out};
}

namespace detail::uuid {
  template <typename Char_T>
  result<wite::uuid, make_uuid_error> try_make_uuid(std::basic_string_view<Char_T> s, char format) noexcept {
    using namespace uuid_format;

    switch (format) {
      case 'N':
      case 'n': {
        return wite::try_make_uuid(s, uuid_format::N);
      }
      case 'D':
      case 'd': {
        return wite::try_make_uuid(s, uuid_format::D);
      }
      case 'B':
      case 'b': {
        return wite::try_make_uuid(s, uuid_format::B);
      }
      case 'P':
      case 'p': {
        return wite::try_make_uuid(s, uuid_format::P);
      }
      case 'X':
      case 'x': {
        return wite::try_make_uuid(s, uuid_format::X);
      }
      default:;
    }

    return {make_uuid_error::invalid_uuid_format_type};
  }
}

inline result<uuid, make_uuid_error> try_make_uuid(std::string_view s, char format = WITE_DEFAULT_UUID_FMT) noexcept {
  return detail::uuid::try_make_uuid<char>(s, format);
}

inline result<uuid, make_uuid_error> try_make_uuid(std::wstring_view s, char format = WITE_DEFAULT_UUID_FMT) noexcept {
  return detail::uuid::try_make_uuid<wchar_t>(s, format);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////

template <>
struct std::hash<wite::uuid> {
  std::size_t operator()(const wite::uuid& id) const noexcept {
    return std::hash<uint64_t>{}(*reinterpret_cast<const uint64_t*>(id.data.data())) ^
           std::hash<uint64_t>{}(*reinterpret_cast<const uint64_t*>(id.data.data() + sizeof(uint64_t)));
  }
};

///////////////////////////////////////////////////////////////////////////////
