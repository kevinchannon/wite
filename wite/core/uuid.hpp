#pragma once

#include <wite/core/basic_uuid.hpp>
#include <wite/core/uuid_functions.hpp>
#include <wite/binascii/hexlify.hpp>
#include <wite/common/constructor_macros.hpp>
#include <wite/env/features.hpp>
#include <wite/io/byte_buffer.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <compare>
#include <cstdint>
#include <random>
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
    data[5]                             = static_cast<uint8_t>((data[5] & 0x0F) | 0x40);  // Version 4 UUID
    data[6]                             = static_cast<uint8_t>((data[6] & 0x3F) | 0x80);  // Variant 1 UUID

    *(reinterpret_cast<uint64_t*>(&data) + 1) = random_bits(engine);
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
      default:;
    }
  }

  template<typename Char_T, typename Format_T>
  void _init_from_wrapped_fmt_string(std::basic_string_view<Char_T> s, Format_T&& fmt) {
    if (s.length() != fmt.size){
      throw std::invalid_argument{"Invalid UUID format"};
    }

    if (not fmt.is_opening(s.front()) or not fmt.is_closing(s.back())){
      throw std::invalid_argument{"Invalid UUID format"};
    }

    const auto has_invalid_delimiter = [&fmt](const auto& str) {
      return std::ranges::any_of(fmt.template delimiters<Char_T>()->internal_delimiters,
                          [&str](const auto& x) {return x.value != str[x.position]; });
    };

    if (has_invalid_delimiter(s)) {
      throw std::invalid_argument{"Invalid UUID format"};
    }

    try {
      _unsafe_generic_from_string<Char_T>(s, fmt.prefixed_values, data);
    } catch (const std::invalid_argument&) {
      throw std::invalid_argument{"Invalid UUID format"};
    }
  }

  template<typename Char_T>
  static void _unsafe_generic_from_string(std::basic_string_view<Char_T> s, bool prefixed_values, Storage_t& out) {
    auto c = std::array<Char_T, 70>{};

    if (prefixed_values) {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<Char_T>>) {
        char prefix[2] = {'0', 'x'};
        _remove_0x_prefixes_and_nonhex_chars(s, prefix, c);
      } else {
        wchar_t prefix[2] = {L'0', L'x'};
        _remove_0x_prefixes_and_nonhex_chars(s, prefix, c);
      }
    } else {
      std::ranges::copy_if(
          s, c.begin(), [](auto ch) { return 0 != std::isxdigit(static_cast<std::make_unsigned_t<Char_T>>(ch)); });
    }

    out = binascii::unhexlify<16, uint8_t>(c.data());
    _format_raw_array_as_data(out);
  }

  template<typename Char_T>
  static void _remove_0x_prefixes_and_nonhex_chars(const auto& in, const Char_T prefix_string[2], auto& out){
    auto write_pos = out.begin();
    auto read_pos  = in.begin();
    while (in.end() != read_pos) {
      if (*read_pos == prefix_string[0] and *std::next(read_pos) == prefix_string[1]){
        read_pos += 2;
      } else if (not std::isxdigit(static_cast<std::make_unsigned_t<Char_T>>(*read_pos))) {
        ++read_pos;
      } else {
        *write_pos = *read_pos;
        ++write_pos;
        ++read_pos;
      }
    }
  }

  static void _format_raw_array_as_data(Storage_t& arr) {
    std::reverse(arr.begin(), std::next(arr.begin(), 4));
    std::reverse(std::next(arr.begin(), 4), std::next(arr.begin(), 6));
    std::reverse(std::next(arr.begin(), 6), std::next(arr.begin(), 8));
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

inline uuid try_make_uuid(std::string_view s) noexcept {
  if (s.length() != 36) {
    return nulluuid;
  }

  const auto is_not_dash = [](auto c) { return '-' != c; };

  if (is_not_dash(s[8]) or is_not_dash(s[13]) or is_not_dash(s[18]) or is_not_dash(s[23])) {
    return nulluuid;
  }

  const auto data_1 = binascii::try_from_hex_chars<uint32_t>(s.substr(0, 8));
  if (data_1.is_error()) {
    return nulluuid;
  }

  const auto data_2 = binascii::try_from_hex_chars<uint16_t>(s.substr(9, 4));
  if (data_2.is_error()) {
    return nulluuid;
  }

  const auto data_3 = binascii::try_from_hex_chars<uint16_t>(s.substr(14, 4));
  if (data_3.is_error()) {
    return nulluuid;
  }

  const auto data_4a = binascii::try_unhexlify<2, uint8_t>(s.substr(19, 4));
  if (data_4a.is_error()) {
    return nulluuid;
  }

  const auto data_4b = binascii::try_unhexlify<6, uint8_t>(s.substr(24, 12));
  if (data_4b.is_error()) {
    return nulluuid;
  }

  auto data = uuid::Storage_t{};
  if (io::try_write(data,
                    io::little_endian{data_1.value()},
                    io::little_endian{data_2.value()},
                    io::little_endian{data_3.value()},
                    data_4a.value(),
                    data_4b.value())
          .is_error()) {
    return nulluuid;
  }

  return uuid{data};
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
