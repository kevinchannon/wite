#pragma once

#include <utility>
#include "wite/binascii/hexlify.hpp"
#include "wite/collections/static_lookup.hpp"
#include "wite/common/constructor_macros.hpp"
#include "wite/core/uuid/basic_uuid.hpp"
#include "wite/env/features.hpp"
#include "wite/io/byte_buffer.hpp"

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

namespace detail::uuid {

  ///////////////////////////////////////////////////////////////////////////////

  template <typename Char_T>
  void remove_0x_prefixes_and_nonhex_chars(const auto& in, const Char_T prefix_string[2], auto& out) {
    auto write_pos = out.begin();
    auto read_pos  = in.begin();
    while (in.end() != read_pos) {
      if (*read_pos == prefix_string[0] and *std::next(read_pos) == prefix_string[1]) {
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

  template <typename Char_T>
  void strip_non_hex_characters(std::basic_string_view<Char_T> s,
                                bool prefixed_values,
                                std::array<Char_T, 2 * std::tuple_size_v<basic_uuid::Storage_t>>& uuid_data_as_hex_chars) {
    if (prefixed_values) {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<Char_T>>) {
        char prefix[2] = {'0', 'x'};
        remove_0x_prefixes_and_nonhex_chars(s, prefix, uuid_data_as_hex_chars);
      } else {
        wchar_t prefix[2] = {L'0', L'x'};
        remove_0x_prefixes_and_nonhex_chars(s, prefix, uuid_data_as_hex_chars);
      }
    } else {
      std::ranges::copy_if(s, uuid_data_as_hex_chars.begin(), [](auto ch) {
        return 0 != std::isxdigit(static_cast<std::make_unsigned_t<Char_T>>(ch));
      });
    }
  }

  template <typename Storage_T>
  void format_as_uuid_data(Storage_T& arr) {
    std::reverse(arr.begin(), std::next(arr.begin(), 4));
    std::reverse(std::next(arr.begin(), 4), std::next(arr.begin(), 6));
    std::reverse(std::next(arr.begin(), 6), std::next(arr.begin(), 8));
  }

  ///////////////////////////////////////////////////////////////////////////////

}  // namespace detail::uuid

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

  template <typename Char_T>
  struct uuid_internal_delimiter {
    constexpr explicit uuid_internal_delimiter(Char_T v, size_t p) : value{v}, position{p} {}

    Char_T value;
    size_t position;
  };

  template <typename Char_T, size_t DELIMITER_COUNT = 0, Char_T OPENING = Char_T{}, Char_T CLOSING = Char_T{}>
  struct uuid_format_delimiters {
    static constexpr Char_T opening{OPENING};
    static constexpr Char_T closing{CLOSING};

    constexpr explicit uuid_format_delimiters(
        const std::array<uuid_internal_delimiter<Char_T>, DELIMITER_COUNT> delimiters) noexcept
        : internal_delimiters{delimiters} {}

    const std::array<uuid_internal_delimiter<Char_T>, DELIMITER_COUNT> internal_delimiters;
  };

  template <typename NarrowDelimiters_T, typename WideDelimiters_T>
  struct uuid_format_type {
    constexpr explicit uuid_format_type(size_t size,
                                        bool prefixed_values,
                                        const char* format,
                                        const wchar_t* wide_format,
                                        NarrowDelimiters_T narrow_delimiters,
                                        WideDelimiters_T wide_delimiters) noexcept
        : size{size}
        , prefixed_values{prefixed_values}
        , narrow_delimiters{std::move(narrow_delimiters)}
        , wide_delimiters{std::move(wide_delimiters)}
        , format_string{format}
        , wide_format_string{wide_format} {}

    template <typename C>
    _WITE_NODISCARD constexpr bool is_opening(C c) const noexcept {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<C>>) {
        return narrow_delimiters.opening == C{} or narrow_delimiters.opening == c;
      } else {
        return wide_delimiters.opening == C{} or wide_delimiters.opening == c;
      }
    }

    template <typename C>
    _WITE_NODISCARD constexpr bool is_closing(C c) const noexcept {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<C>>) {
        return narrow_delimiters.closing == C{} or narrow_delimiters.closing == c;
      } else {
        return wide_delimiters.closing == C{} or wide_delimiters.closing == c;
      }
    }

    template <typename C>
    _WITE_NODISCARD constexpr auto delimiters() const noexcept {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<C>>) {
        return &narrow_delimiters;
      } else {
        return &wide_delimiters;
      }
    }

    template <typename C>
    _WITE_NODISCARD constexpr const C* format() const noexcept {
      if constexpr (std::is_same_v<std::make_unsigned_t<char>, std::make_unsigned_t<C>>) {
        return format_string;
      } else {
        return wide_format_string;
      }
    }

    template<typename C>
    _WITE_NODISCARD bool is_a_valid_uuid_string(std::basic_string_view<C> s) const noexcept {
      if (s.length() != size){
        return false;
      }

      if (not is_opening(s.front()) or not is_closing(s.back())){
        return false;
      }

      const auto has_invalid_delimiter = [this](const auto& str) {
        return std::ranges::any_of(delimiters<C>()->internal_delimiters,
                                   [&str](const auto& x) {return x.value != str[x.position]; });
      };

      if (has_invalid_delimiter(s)) {
        return false;
      }

      return true;
    }

    const size_t size{};
    const bool prefixed_values{false};
    const NarrowDelimiters_T narrow_delimiters{};
    const WideDelimiters_T wide_delimiters{};
    const char* format_string{};
    const wchar_t* wide_format_string{};
  };
}  // namespace detail

namespace uuid_format {
  static constexpr auto N = detail::uuid_format_type{32,
                                                     false,
                                                     "%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
                                                     L"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
                                                     detail::uuid_format_delimiters<char>{{}},
                                                     detail::uuid_format_delimiters<wchar_t>{{}}};

  static constexpr auto n = detail::uuid_format_type{32,
                                                     false,
                                                     "%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
                                                     L"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
                                                     detail::uuid_format_delimiters<char>{{}},
                                                     detail::uuid_format_delimiters<wchar_t>{{}}};

  static constexpr auto D =
      detail::uuid_format_type{36,
                               false,
                               "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                               L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
                               detail::uuid_format_delimiters<char, 4>{std::array{detail::uuid_internal_delimiter{'-', 8},
                                                                                  detail::uuid_internal_delimiter{'-', 13},
                                                                                  detail::uuid_internal_delimiter{'-', 18},
                                                                                  detail::uuid_internal_delimiter{'-', 23}}},
                               detail::uuid_format_delimiters<wchar_t, 4>{std::array{detail::uuid_internal_delimiter{L'-', 8},
                                                                                     detail::uuid_internal_delimiter{L'-', 13},
                                                                                     detail::uuid_internal_delimiter{L'-', 18},
                                                                                     detail::uuid_internal_delimiter{L'-', 23}}}};

  static constexpr auto d =
      detail::uuid_format_type{36,
                               false,
                               "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                               L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                               detail::uuid_format_delimiters<char, 4>{std::array{detail::uuid_internal_delimiter{'-', 8},
                                                                                  detail::uuid_internal_delimiter{'-', 13},
                                                                                  detail::uuid_internal_delimiter{'-', 18},
                                                                                  detail::uuid_internal_delimiter{'-', 23}}},
                               detail::uuid_format_delimiters<wchar_t, 4>{std::array{detail::uuid_internal_delimiter{L'-', 8},
                                                                                     detail::uuid_internal_delimiter{L'-', 13},
                                                                                     detail::uuid_internal_delimiter{L'-', 18},
                                                                                     detail::uuid_internal_delimiter{L'-', 23}}}};

  static constexpr auto B = detail::uuid_format_type{
      38,
      false,
      "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
      L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
      detail::uuid_format_delimiters<char, 4, '{', '}'>{std::array{detail::uuid_internal_delimiter{'-', 9},
                                                                   detail::uuid_internal_delimiter{'-', 14},
                                                                   detail::uuid_internal_delimiter{'-', 19},
                                                                   detail::uuid_internal_delimiter{'-', 24}}},
      detail::uuid_format_delimiters<wchar_t, 4, L'{', L'}'>{std::array{detail::uuid_internal_delimiter{L'-', 9},
                                                                        detail::uuid_internal_delimiter{L'-', 14},
                                                                        detail::uuid_internal_delimiter{L'-', 19},
                                                                        detail::uuid_internal_delimiter{L'-', 24}}}};

  static constexpr auto b = detail::uuid_format_type{
      38,
      false,
      "{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
      L"{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}",
      detail::uuid_format_delimiters<char, 4, '{', '}'>{std::array{detail::uuid_internal_delimiter{'-', 9},
                                                                   detail::uuid_internal_delimiter{'-', 14},
                                                                   detail::uuid_internal_delimiter{'-', 19},
                                                                   detail::uuid_internal_delimiter{'-', 24}}},
      detail::uuid_format_delimiters<wchar_t, 4, L'{', L'}'>{std::array{detail::uuid_internal_delimiter{L'-', 9},
                                                                        detail::uuid_internal_delimiter{L'-', 14},
                                                                        detail::uuid_internal_delimiter{L'-', 19},
                                                                        detail::uuid_internal_delimiter{L'-', 24}}}};

  static constexpr auto P = detail::uuid_format_type{
      38,
      false,
      "(%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X)",
      L"(%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X)",
      detail::uuid_format_delimiters<char, 4, '(', ')'>{std::array{detail::uuid_internal_delimiter{'-', 9},
                                                                   detail::uuid_internal_delimiter{'-', 14},
                                                                   detail::uuid_internal_delimiter{'-', 19},
                                                                   detail::uuid_internal_delimiter{'-', 24}}},
      detail::uuid_format_delimiters<wchar_t, 4, L'(', L')'>{std::array{detail::uuid_internal_delimiter{L'-', 9},
                                                                        detail::uuid_internal_delimiter{L'-', 14},
                                                                        detail::uuid_internal_delimiter{L'-', 19},
                                                                        detail::uuid_internal_delimiter{L'-', 24}}}};

  static constexpr auto p = detail::uuid_format_type{
      38,
      false,
      "(%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x)",
      L"(%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x)",
      detail::uuid_format_delimiters<char, 4, '(', ')'>{std::array{detail::uuid_internal_delimiter{'-', 9},
                                                                   detail::uuid_internal_delimiter{'-', 14},
                                                                   detail::uuid_internal_delimiter{'-', 19},
                                                                   detail::uuid_internal_delimiter{'-', 24}}},
      detail::uuid_format_delimiters<wchar_t, 4, L'(', L')'>{std::array{detail::uuid_internal_delimiter{L'-', 9},
                                                                        detail::uuid_internal_delimiter{L'-', 14},
                                                                        detail::uuid_internal_delimiter{L'-', 19},
                                                                        detail::uuid_internal_delimiter{L'-', 24}}}};

  static constexpr auto X =
      detail::uuid_format_type{68,
                               true,
                               "{0x%08X,0x%04X,0x%04X,{0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}}",
                               L"{0x%08X,0x%04X,0x%04X,{0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}}",
                               detail::uuid_format_delimiters<char>{{}},
                               detail::uuid_format_delimiters<wchar_t>{{}}};

  static constexpr auto x =
      detail::uuid_format_type{68,
                               true,
                               "{0x%08x,0x%04x,0x%04x,{0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x}}",
                               L"{0x%08x,0x%04x,0x%04x,{0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x,0x%02x}}",
                               detail::uuid_format_delimiters<char, 0, '{', '}'>{{}},
                               detail::uuid_format_delimiters<wchar_t, 0, L'{', L'}'>{{}}};
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
  _WITE_NODISCARD bool _to_c_str(const Uuid_T& id, Format_T&& fmt, Char_T* buffer, size_t max_buffer_length) {
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

    std::ignore = _to_c_str<Char_T, Uuid_T>(id, std::forward<Format_T>(fmt), buffer, buffer_length);
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

template <typename Char_T>
_WITE_NODISCARD basic_uuid unsafe_basic_uuid_from_string(std::basic_string_view<Char_T> s) noexcept {
  auto out = basic_uuid{};

  out.data = binascii::unsafe_unhexlify<16, uint8_t>(s.data());
  detail::uuid::format_as_uuid_data(out.data);

  return out;
}

///////////////////////////////////////////////////////////////////////////////

_WITE_NODISCARD inline basic_uuid unsafe_basic_uuid_from_string(std::string_view s) noexcept {
  return unsafe_basic_uuid_from_string<char>(s);
}

///////////////////////////////////////////////////////////////////////////////

_WITE_NODISCARD inline basic_uuid unsafe_basic_uuid_from_string(std::wstring_view s) noexcept {
  return unsafe_basic_uuid_from_string<wchar_t>(s);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
