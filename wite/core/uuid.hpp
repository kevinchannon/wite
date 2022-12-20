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
  uuid(unsigned long d1, unsigned short d2, unsigned short d3, std::array<unsigned char, 8> d4) : basic_uuid{} {
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
    switch (format) {
      case 'D':
      case 'd': {
        _init_from_d_fmt_string(s);
        break;
      }
      case 'N':
      case 'n': {
        _init_from_n_fmt_string(s);
        break;
      }
      default:;
    }
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
  void _init_from_d_fmt_string(std::string_view s) {
    if (s.length() != detail::_uuid_strlen<'D'>() - 1) {
      throw std::invalid_argument{"Invalid UUID format"};
    }

    const auto is_not_dash = [](auto c) { return '-' != c; };

    if (is_not_dash(s[8]) or is_not_dash(s[13]) or is_not_dash(s[18]) or is_not_dash(s[23])) {
      throw std::invalid_argument{"Invalid UUID format"};
    }

    try {
      _unsafe_generic_from_string(s, data);
    } catch (const std::invalid_argument&) {
      throw std::invalid_argument{"Invalid UUID format"};
    }
  }

  void _init_from_n_fmt_string(std::string_view s) {
    if (s.length() != detail::_uuid_strlen<'N'>() - 1) {
      throw std::invalid_argument{"Invalid UUID format"};
    }
    try {
      _unsafe_generic_from_string(s, data);
    } catch (const std::invalid_argument&) {
      throw std::invalid_argument{"Invalid UUID format"};
    }
  }

  static void _unsafe_generic_from_string(std::string_view s, Storage_t& out) {
    auto c = std::array<char, 38>{};
    std::ranges::copy_if(s, c.begin(), [](auto ch) { return 0 != std::isxdigit(static_cast<unsigned char>(ch)); });
    out = binascii::unhexlify<16, uint8_t>(const_cast<const char*>(&c.front()));
    _format_raw_array_as_data(out);
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
