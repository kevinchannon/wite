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
#if _WITE_HAS_CONCEPTS
#include <concepts>
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

struct uuid;

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <typename Char_T, wite::uuid_like Uuid_T>
_WITE_NODISCARD bool to_c_str(const Uuid_T& id, Char_T* buffer, size_t max_buffer_length, char format = default_uuid_format);
#else
template <typename Char_T>
_WITE_NODISCARD bool to_c_str(const uuid& id, Char_T* buffer, size_t max_buffer_length, char format = default_uuid_format);
#endif

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <wite::uuid_like Uuid_T>
_WITE_NODISCARD std::string to_string(const Uuid_T& id, char format = default_uuid_format);
#else
_WITE_NODISCARD inline std::string to_string(const uuid& id, char format = default_uuid_format);
#endif

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <wite::uuid_like Uuid_T>
_WITE_NODISCARD std::wstring to_wstring(const Uuid_T& id, char format = default_uuid_format);
#else
_WITE_NODISCARD inline std::wstring to_wstring(const uuid& id, char format = default_uuid_format);
#endif

///////////////////////////////////////////////////////////////////////////////

namespace detail {

  template <typename Char_T, Char_T OPENING = Char_T{}, Char_T INTERNAL = Char_T{}, Char_T CLOSING = Char_T{}>
  struct uuid_format_delimiters {
    const Char_T opening{OPENING};
    const Char_T closing{CLOSING};
    const Char_T internal{INTERNAL};
  };

  template <char UPPER_FMT_SPEC, char LOWER_FMT_SPEC, size_t SIZE, typename NarrowDelims_T, typename WideDelims_T>
  struct format_type {
    const char uppercase_format_specifier = UPPER_FMT_SPEC;
    const char lowercase_format_specifier = LOWER_FMT_SPEC;
    const size_t size                     = SIZE;

    const NarrowDelims_T narrow_delimiters{};
    const WideDelims_T wide_delimiters{};

    constexpr explicit format_type(const char* lowercase_format,
                                   const char* uppercase_format,
                                   const wchar_t* lowercase_wide_format,
                                   const wchar_t* uppercase_wide_format)
        : lowercase_format_string{lowercase_format}
        , uppercase_format_string{uppercase_format}
        , lowercase_wide_format_string{lowercase_wide_format}
        , uppercase_wide_format_string{uppercase_wide_format} {}

    template <typename C>
    constexpr C opening() const {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<C>>) {
        return narrow_delimiters.opening;
      } else {
        return wide_delimiters.opening;
      }
    }

    template <typename C>
    constexpr C closing() const {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<C>>) {
        return narrow_delimiters.closing;
      } else {
        return wide_delimiters.closing;
      }
    }

    const char* lowercase_format_string{};
    const char* uppercase_format_string{};
    const wchar_t* lowercase_wide_format_string{};
    const wchar_t* uppercase_wide_format_string{};
  };
}  // namespace detail

namespace uuid_format {
  static constexpr auto N =
      detail::format_type<'N', 'n', 32 + 1, detail::uuid_format_delimiters<char>, detail::uuid_format_delimiters<wchar_t>>{
          "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
          "%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
          L"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
          L"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X"};

  static constexpr auto D = detail::format_type<'D',
                                                'd',
                                                36 + 1,
                                                detail::uuid_format_delimiters<char, char{}, '-', char{}>,
                                                detail::uuid_format_delimiters<wchar_t, wchar_t{}, L'-', wchar_t{}>>{
      "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
      L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X"};

  static constexpr auto B = detail::format_type<'B',
                                                'b',
                                                38 + 1,
                                                detail::uuid_format_delimiters<char, '{', '-', '}'>,
                                                detail::uuid_format_delimiters<wchar_t, L'{', L'-', L'}'>>{
      "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
      "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
      L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
      L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}"};

  static constexpr auto P = detail::format_type<'P',
                                                'p',
                                                38 + 1,
                                                detail::uuid_format_delimiters<char, '(', '-', ')'>,
                                                detail::uuid_format_delimiters<wchar_t, L'(', L'-', L')'>>{
      "(%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x)",
      "(%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X)",
      L"(%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x)",
      L"(%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X)"};

  static constexpr auto X = detail::format_type<'X',
                                                'x',
                                                69 + 1,
                                                detail::uuid_format_delimiters<char, '{', char{}, '}'>,
                                                detail::uuid_format_delimiters<wchar_t, L'{', wchar_t{}, L'}'>>{
      "{0x%08x,0x%04x,0x%04x,{0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x}}",
      "{0x%08X,0x%04X,0x%04X,{0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}}",
      L"{0x%08x,0x%04x,0x%04x,{0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x}}",
      L"{0x%08X,0x%04X,0x%04X,{0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}}"};
}  // namespace uuid_format

namespace detail {

  ///////////////////////////////////////////////////////////////////////////////

  template <typename Char_T, char FMT_TYPE>
  _WITE_CONSTEVAL auto _uuid_format() {
    using namespace uuid_format;

#define WITE_BEGIN_FORMAT_TYPES \
  if constexpr (false) {        \
  }
#define WITE_ADD_CASE_FOR_FORMAT_TYPE(t)                         \
  else if constexpr (t.uppercase_format_specifier == FMT_TYPE) { \
    if constexpr (std::is_same_v<Char_T, char>) {                \
      return t.uppercase_format_string;                          \
    } else {                                                     \
      return t.uppercase_wide_format_string;                     \
    }                                                            \
  }                                                              \
  else if constexpr (t.lowercase_format_specifier == FMT_TYPE) { \
    if constexpr (std::is_same_v<Char_T, char>) {                \
      return t.lowercase_format_string;                          \
    } else {                                                     \
      return t.lowercase_wide_format_string;                     \
    }                                                            \
  }
#define WITE_END_FORMAT_TYPES                                                     \
  else {                                                                          \
    static_assert('X' == FMT_TYPE, "Invalid UUID format specifier"); \
  }

    WITE_BEGIN_FORMAT_TYPES
    WITE_ADD_CASE_FOR_FORMAT_TYPE(N)
    WITE_ADD_CASE_FOR_FORMAT_TYPE(D)
    WITE_ADD_CASE_FOR_FORMAT_TYPE(B)
    WITE_ADD_CASE_FOR_FORMAT_TYPE(P)
    WITE_ADD_CASE_FOR_FORMAT_TYPE(X)
    WITE_END_FORMAT_TYPES

#undef WITE_BEGIN_FORMAT_TYPES
#undef WITE_ADD_CASE_FOR_FORMAT_TYPE
#undef WITE_END_FORMAT_TYPES
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <char FMT_TYPE>
  _WITE_CONSTEVAL size_t _uuid_strlen() {
    using namespace uuid_format;

#define WITE_BEGIN_FORMAT_TYPES \
  if constexpr (false) {        \
  }
#define WITE_ADD_CASE_FOR_FORMAT_TYPE(t)                                                                     \
  else if constexpr (t.uppercase_format_specifier == FMT_TYPE or t.lowercase_format_specifier == FMT_TYPE) { \
    return t.size;                                                                                           \
  }
#define WITE_END_FORMAT_TYPES                                                     \
  else {                                                                          \
    static_assert('X' == FMT_TYPE, "Invalid UUID format specifier"); \
  }

    WITE_BEGIN_FORMAT_TYPES
    WITE_ADD_CASE_FOR_FORMAT_TYPE(N)
    WITE_ADD_CASE_FOR_FORMAT_TYPE(D)
    WITE_ADD_CASE_FOR_FORMAT_TYPE(B)
    WITE_ADD_CASE_FOR_FORMAT_TYPE(P)
    WITE_ADD_CASE_FOR_FORMAT_TYPE(X)
    WITE_END_FORMAT_TYPES
  }

  ///////////////////////////////////////////////////////////////////////////////

}  // namespace detail

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

#if _WITE_HAS_CONCEPTS
  template <typename Char_T, wite::uuid_like Uuid_T, char FMT_TYPE>
  _WITE_NODISCARD bool _to_c_str(const Uuid_T& id, Char_T* buffer, size_t max_buffer_length)
#else
  template <typename Char_T>
  _WITE_NODISCARD bool _to_c_str(const uuid& id, Char_T* buffer, size_t max_buffer_length)
#endif
  {
    if (max_buffer_length < detail::_uuid_strlen<FMT_TYPE>()) {
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
                                                  detail::_uuid_format<Char_T, FMT_TYPE>(),
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

#if _WITE_HAS_CONCEPTS
  template <typename Char_T, wite::uuid_like Uuid_T, char FMT_TYPE>
  _WITE_NODISCARD std::basic_string<Char_T> _to_string(const Uuid_T& id) {
    using uuid_t = Uuid_T;
#else
  _WITE_NODISCARD inline std::string to_string(const uuid& id) {
    using uuid_t = uuid;
#endif
    constexpr auto buffer_length = _uuid_strlen<FMT_TYPE>();
    Char_T buffer[buffer_length] = {};

    std::ignore = _to_c_str<Char_T, uuid_t, FMT_TYPE>(id, buffer, buffer_length);
    return {buffer};
  }

  ///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
  template <typename Char_T, wite::uuid_like Uuid_T>
  _WITE_NODISCARD std::basic_string<Char_T> _to_string(const Uuid_T& id, char format) {
    using uuid_t = Uuid_T;
#else
  template <typename Char_T>
  _WITE_NODISCARD std::basic_string<Char_T> _to_string(const uuid& id, char format) {
    using uuid_t = uuid;
#endif
    switch (format) {
      case 'D':
        return _to_string<Char_T, uuid_t, 'D'>(id);
      case 'N':
        return _to_string<Char_T, uuid_t, 'N'>(id);
      case 'B':
        return _to_string<Char_T, uuid_t, 'B'>(id);
      case 'P':
        return _to_string<Char_T, uuid_t, 'P'>(id);
      case 'X':
        return _to_string<Char_T, uuid_t, 'X'>(id);
      case 'd':
        return _to_string<Char_T, uuid_t, 'd'>(id);
      case 'n':
        return _to_string<Char_T, uuid_t, 'n'>(id);
      case 'b':
        return _to_string<Char_T, uuid_t, 'b'>(id);
      case 'p':
        return _to_string<Char_T, uuid_t, 'p'>(id);
      case 'x':
        return _to_string<Char_T, uuid_t, 'x'>(id);
      default:;
    }

    throw std::invalid_argument{"Invalid UUID format type"};
  }

  ///////////////////////////////////////////////////////////////////////////////

}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <typename Char_T, wite::uuid_like Uuid_T>
_WITE_NODISCARD bool to_c_str(const Uuid_T& id, Char_T* buffer, size_t max_buffer_length, char format) {
  using uuid_t = Uuid_T;
#else
template <typename Char_T>
_WITE_NODISCARD bool to_c_str(const uuid& id, Char_T* buffer, size_t max_buffer_length, char format) {
  using uuid_t = uuid;
#endif
  switch (format) {
    case 'D':
      return detail::_to_c_str<Char_T, uuid_t, 'D'>(id, buffer, max_buffer_length);
    case 'N':
      return detail::_to_c_str<Char_T, uuid_t, 'N'>(id, buffer, max_buffer_length);
    case 'B':
      return detail::_to_c_str<Char_T, uuid_t, 'B'>(id, buffer, max_buffer_length);
    case 'P':
      return detail::_to_c_str<Char_T, uuid_t, 'P'>(id, buffer, max_buffer_length);
    case 'X':
      return detail::_to_c_str<Char_T, uuid_t, 'X'>(id, buffer, max_buffer_length);
    case 'd':
      return detail::_to_c_str<Char_T, uuid_t, 'd'>(id, buffer, max_buffer_length);
    case 'n':
      return detail::_to_c_str<Char_T, uuid_t, 'n'>(id, buffer, max_buffer_length);
    case 'b':
      return detail::_to_c_str<Char_T, uuid_t, 'b'>(id, buffer, max_buffer_length);
    case 'p':
      return detail::_to_c_str<Char_T, uuid_t, 'p'>(id, buffer, max_buffer_length);
    case 'x':
      return detail::_to_c_str<Char_T, uuid_t, 'x'>(id, buffer, max_buffer_length);
    default:
      return false;
  }
}

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <wite::uuid_like Uuid_T>
_WITE_NODISCARD std::string to_string(const Uuid_T& id, char format) {
  using uuid_t = Uuid_T;
#else
_WITE_NODISCARD inline std::string to_string(const uuid& id, char format) {
  using uuid_t = uuid;
#endif
  return detail::_to_string<char, uuid_t>(id, format);
}

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <wite::uuid_like Uuid_T>
_WITE_NODISCARD std::wstring to_wstring(const Uuid_T& id, char format) {
  using uuid_t = Uuid_T;
#else
_WITE_NODISCARD inline std::wstring to_wstring(const uuid& id, char format) {
  using uuid_t = uuid;
#endif
  return detail::_to_string<wchar_t, uuid_t>(id, format);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
