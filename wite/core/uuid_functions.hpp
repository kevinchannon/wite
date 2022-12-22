#pragma once

#include <wite/binascii/hexlify.hpp>
#include <wite/collections/static_lookup.hpp>
#include <wite/common/constructor_macros.hpp>
#include <wite/core/basic_uuid.hpp>
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
#include <concepts>

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

struct uuid;

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T, wite::uuid_like Uuid_T>
_WITE_NODISCARD bool to_c_str(const Uuid_T& id, Char_T* buffer, size_t max_buffer_length, char format = default_uuid_format);

///////////////////////////////////////////////////////////////////////////////

template <wite::uuid_like Uuid_T>
_WITE_NODISCARD std::string to_string(const Uuid_T& id, char format = default_uuid_format);

///////////////////////////////////////////////////////////////////////////////

template <wite::uuid_like Uuid_T>
_WITE_NODISCARD std::wstring to_wstring(const Uuid_T& id, char format = default_uuid_format);

///////////////////////////////////////////////////////////////////////////////

namespace detail {

  template <typename Char_T, Char_T OPENING = Char_T{}, Char_T INTERNAL = Char_T{}, Char_T CLOSING = Char_T{}>
  struct uuid_format_delimiters {
    const Char_T opening{OPENING};
    const Char_T closing{CLOSING};
    const Char_T internal{INTERNAL};
  };

  template <size_t SIZE, typename NarrowDelimiters_T, typename WideDelimiters_T>
  struct uuid_format_type {
    const size_t size           = SIZE;

    const NarrowDelimiters_T narrow_delimiters{};
    const WideDelimiters_T wide_delimiters{};

    constexpr explicit uuid_format_type(const char* format,
                                   const wchar_t* wide_format)
        : format_string{format}
        , wide_format_string{wide_format} {}

    template <typename C>
    constexpr bool is_opening(C c) const {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<C>>) {
        return narrow_delimiters.opening == c;
      } else {
        return wide_delimiters.opening == c;
      }
    }

    template <typename C>
    constexpr bool is_closing(C c) const {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<C>>) {
        return narrow_delimiters.closing == c;
      } else {
        return wide_delimiters.closing == c;
      }
    }

    template <typename C>
    constexpr bool is_internal_separator(C c) const {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<C>>) {
        return narrow_delimiters.internal == c;
      } else {
        return wide_delimiters.internal == c;
      }
    }

    template <typename C>
    constexpr const C* format() const {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<C>>) {
        return format_string;
      } else {
        return wide_format_string;
      }
    }

    const char* format_string{};
    const wchar_t* wide_format_string{};
  };
}  // namespace detail

namespace uuid_format {
  static constexpr auto N =
      detail::uuid_format_type<32, detail::uuid_format_delimiters<char>, detail::uuid_format_delimiters<wchar_t>>{
          "%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
          L"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X"};

  static constexpr auto n =
      detail::uuid_format_type<32, detail::uuid_format_delimiters<char>, detail::uuid_format_delimiters<wchar_t>>{
          "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
          L"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x"};

  static constexpr auto D = detail::uuid_format_type<36,
                                                detail::uuid_format_delimiters<char, char{}, '-', char{}>,
                                                detail::uuid_format_delimiters<wchar_t, wchar_t{}, L'-', wchar_t{}>>{
      "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
      L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"};

  static constexpr auto d = detail::uuid_format_type<36,
                                                detail::uuid_format_delimiters<char, char{}, '-', char{}>,
                                                detail::uuid_format_delimiters<wchar_t, wchar_t{}, L'-', wchar_t{}>>{
      "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x"};

  static constexpr auto B = detail::uuid_format_type<38,
                                                detail::uuid_format_delimiters<char, '{', '-', '}'>,
                                                detail::uuid_format_delimiters<wchar_t, L'{', L'-', L'}'>>{
      "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
      L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"};

  static constexpr auto b = detail::uuid_format_type<38,
                                                detail::uuid_format_delimiters<char, '{', '-', '}'>,
                                                detail::uuid_format_delimiters<wchar_t, L'{', L'-', L'}'>>{
      "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
      L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}"};

  static constexpr auto P = detail::uuid_format_type<38,
                                                detail::uuid_format_delimiters<char, '(', '-', ')'>,
                                                detail::uuid_format_delimiters<wchar_t, L'(', L'-', L')'>>{
      "(%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X)",
      L"(%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X)"};

  static constexpr auto p = detail::uuid_format_type<38,
                                                detail::uuid_format_delimiters<char, '(', '-', ')'>,
                                                detail::uuid_format_delimiters<wchar_t, L'(', L'-', L')'>>{
      "(%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x)",
      L"(%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x)"};

  static constexpr auto X = detail::uuid_format_type<68,
                                                detail::uuid_format_delimiters<char, '{', char{}, '}'>,
                                                detail::uuid_format_delimiters<wchar_t, L'{', wchar_t{}, L'}'>>{
      "{0x%08X,0x%04X,0x%04X,{0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}}",
      L"{0x%08X,0x%04X,0x%04X,{0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}}"};

  static constexpr auto x = detail::uuid_format_type<68,
                                                detail::uuid_format_delimiters<char, '{', char{}, '}'>,
                                                detail::uuid_format_delimiters<wchar_t, L'{', wchar_t{}, L'}'>>{
      "{0x%08x,0x%04x,0x%04x,{0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x}}",
      L"{0x%08x,0x%04x,0x%04x,{0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x}}"};
}  // namespace uuid_format

///////////////////////////////////////////////////////////////////////////////

namespace detail {

  ///////////////////////////////////////////////////////////////////////////////

  template <typename Char_T>
  using _uuid_sprintf_t = int (*)(Char_T* const buffer, size_t const count, Char_T const* const fmt, ...);

  template <typename Char_T>
  constexpr _uuid_sprintf_t<Char_T> _uuid_sprintf() {
    if constexpr (std::is_same_v<Char_T, char>) {
      return ::snprintf;
    } else {
      return ::swprintf;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <typename Char_T, wite::uuid_like Uuid_T, typename Format_T>
  _WITE_NODISCARD bool _to_c_str(const Uuid_T& id, Format_T&& fmt, Char_T* buffer, size_t max_buffer_length)
  {
    if (max_buffer_length < fmt.size + 1) {
      return false;
    }

    auto read_pos = reinterpret_cast<const uint8_t*>(&id);
    auto data_1   = uint32_t{};
    auto data_2   = uint16_t{};
    auto data_3   = uint16_t{};

    std::tie(data_1, read_pos) = io::unchecked_read<io::little_endian<uint32_t>>(read_pos);
    std::tie(data_2, read_pos) = io::unchecked_read<io::little_endian<uint16_t>>(read_pos);
    std::tie(data_3, read_pos) = io::unchecked_read<io::little_endian<uint16_t>>(read_pos);
    const uint8_t* data_4      = reinterpret_cast<const uint8_t*>(&id) + 8;

    std::ignore = detail::_uuid_sprintf<Char_T>()(buffer,
                                                  max_buffer_length,
                                                  fmt.template format<Char_T>(),
                                                  data_1,
                                                  data_2,
                                                  data_3,
                                                  data_4[0],
                                                  data_4[1],
                                                  data_4[2],
                                                  data_4[3],
                                                  data_4[4],
                                                  data_4[5],
                                                  data_4[6],
                                                  data_4[7]);

    return true;
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <typename Char_T, wite::uuid_like Uuid_T, typename Format_T>
  _WITE_NODISCARD std::basic_string<Char_T> _to_string(const Uuid_T& id, Format_T&& fmt) {
    constexpr auto buffer_length = 1 + uuid_format::X.size;  // X is the biggest format
    Char_T buffer[buffer_length] = {};

    std::ignore = _to_c_str<Char_T, Uuid_T>(id, std::forward<Format_T>(fmt),buffer, buffer_length);
    return {buffer};
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <typename Char_T, wite::uuid_like Uuid_T>
  _WITE_NODISCARD std::basic_string<Char_T> _to_string(const Uuid_T& id, char format) {
    using namespace uuid_format;

    switch (format) {
      case 'D':
        return _to_string<Char_T, Uuid_T>(id, D);
      case 'N':
        return _to_string<Char_T, Uuid_T>(id, N);
      case 'B':
        return _to_string<Char_T, Uuid_T>(id, B);
      case 'P':
        return _to_string<Char_T, Uuid_T>(id, P);
      case 'X':
        return _to_string<Char_T, Uuid_T>(id, X);
      case 'd':
        return _to_string<Char_T, Uuid_T>(id, d);
      case 'n':
        return _to_string<Char_T, Uuid_T>(id, n);
      case 'b':
        return _to_string<Char_T, Uuid_T>(id, b);
      case 'p':
        return _to_string<Char_T, Uuid_T>(id, p);
      case 'x':
        return _to_string<Char_T, Uuid_T>(id, x);
      default:;
    }

    throw std::invalid_argument{"Invalid UUID format type"};
  }

  ///////////////////////////////////////////////////////////////////////////////

}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename Char_T, wite::uuid_like Uuid_T>
_WITE_NODISCARD bool to_c_str(const Uuid_T& id, Char_T* buffer, size_t max_buffer_length, char format) {
  using namespace uuid_format;

  switch (format) {
    case 'D':
      return detail::_to_c_str<Char_T, Uuid_T>(id, D, buffer, max_buffer_length);
    case 'N':
      return detail::_to_c_str<Char_T, Uuid_T>(id, N, buffer, max_buffer_length);
    case 'B':
      return detail::_to_c_str<Char_T, Uuid_T>(id, B, buffer, max_buffer_length);
    case 'P':
      return detail::_to_c_str<Char_T, Uuid_T>(id, P, buffer, max_buffer_length);
    case 'X':
      return detail::_to_c_str<Char_T, Uuid_T>(id, X, buffer, max_buffer_length);
    case 'd':
      return detail::_to_c_str<Char_T, Uuid_T>(id, d, buffer, max_buffer_length);
    case 'n':
      return detail::_to_c_str<Char_T, Uuid_T>(id, n, buffer, max_buffer_length);
    case 'b':
      return detail::_to_c_str<Char_T, Uuid_T>(id, b, buffer, max_buffer_length);
    case 'p':
      return detail::_to_c_str<Char_T, Uuid_T>(id, p, buffer, max_buffer_length);
    case 'x':
      return detail::_to_c_str<Char_T, Uuid_T>(id, x, buffer, max_buffer_length);
    default:
      return false;
  }
}

///////////////////////////////////////////////////////////////////////////////

template <wite::uuid_like Uuid_T>
_WITE_NODISCARD std::string to_string(const Uuid_T& id, char format) {
  return detail::_to_string<char, Uuid_T>(id, format);
}

///////////////////////////////////////////////////////////////////////////////

template <wite::uuid_like Uuid_T>
_WITE_NODISCARD std::wstring to_wstring(const Uuid_T& id, char format) {
  return detail::_to_string<wchar_t, Uuid_T>(id, format);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
