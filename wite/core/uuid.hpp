#pragma once

#include <wite/binascii/hexlify.hpp>
#include <wite/common/constructor_macros.hpp>
#include <wite/env/features.hpp>

#include <algorithm>
#include <array>
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
#include <charconv>

namespace wite {

#ifdef _WITE_HAS_CONCEPTS
template <typename T>
concept wite_uuid_like = requires(T& t) { t.data; };

template <typename T>
concept guid_like = requires(T& t) {
                      t.Data1;
                      t.Data2;
                      t.Data3;
                      t.Data4[0];
                      t.Data4[1];
                      t.Data4[2];
                      t.Data4[3];
                      t.Data4[4];
                      t.Data4[5];
                      t.Data4[6];
                      t.Data4[7];
                    };
template <typename T>
concept uuid_like = ((wite_uuid_like<T> or guid_like<T>) and sizeof(T) == 16);
#endif

struct uuid;

#if _WITE_HAS_CONCEPTS
template <wite::uuid_like Uuid_T>
_WITE_NODISCARD bool to_c_str(const Uuid_T& id, char* buffer, size_t max_buffer_length, char format = 'D');
#else
_WITE_NODISCARD inline bool to_c_str(const uuid& id, char* buffer, size_t max_buffer_length, char format = 'D');
#endif

#if _WITE_HAS_CONCEPTS
template <wite::uuid_like Uuid_T>
_WITE_NODISCARD std::string to_string(const Uuid_T& id, char format = 'D');
#else
_WITE_NODISCARD inline std::string to_string(const uuid& id, char format = 'D');
#endif

#if _WITE_HAS_CONCEPTS
template <wite::uuid_like Uuid_T>
_WITE_NODISCARD bool to_c_str(const Uuid_T& id, wchar_t* buffer, size_t max_buffer_length, char format = 'D');
#else
_WITE_NODISCARD inline bool to_c_str(const uuid& id, wchar_t* buffer, size_t max_buffer_length, char format = 'D');
#endif

#if _WITE_HAS_CONCEPTS
template <wite::uuid_like Uuid_T>
_WITE_NODISCARD std::wstring to_wstring(const Uuid_T& id, char format = 'D');
#else
_WITE_NODISCARD inline std::wstring to_wstring(const uuid& id, char format = 'D');
#endif

struct uuid {
  WITE_DEFAULT_CONSTRUCTORS(uuid);

  uuid(unsigned long d1, unsigned short d2, unsigned short d3, std::array<unsigned char, 8> d4)
      : data{*((uint8_t*)(&d1)),
             *((uint8_t*)(&d1) + 1),
             *((uint8_t*)(&d1) + 2),
             *((uint8_t*)(&d1) + 3),
             *((uint8_t*)(&d2)),
             *((uint8_t*)(&d2) + 1),
             *((uint8_t*)(&d3)),
             *((uint8_t*)(&d3) + 1),
             d4[0],
             d4[1],
             d4[2],
             d4[3],
             d4[4],
             d4[5],
             d4[6],
             d4[7]} {}

#if _WITE_HAS_CONCEPTS
  template <std::invocable Engine_T>
  explicit uuid(Engine_T&& engine)
#else
  explicit uuid(std::default_random_engine& engine)
#endif
  {
    auto random_bits = std::uniform_int_distribution<uint64_t>{0x00, 0xFFFFFFFFFFFFFFFF};

    *reinterpret_cast<uint64_t*>(&data) = random_bits(engine);
    data[5]                             = static_cast<uint8_t>((data[5] & 0x0F) | 0x40);  // Version 4 UUID
    data[6]                             = static_cast<uint8_t>((data[6] & 0x3F) | 0x80);  // Variant 1 UUID

    *(reinterpret_cast<uint64_t*>(&data) + 1) = random_bits(engine);
  }

#ifndef WITE_NO_EXCEPTIONS
  explicit uuid(const std::string_view s) : uuid{} {
    if (s.length() != 36) {
      throw std::invalid_argument{"Invalid UUID format"};
    }

    const auto is_not_dash = [](auto c) { return '-' != c; };

    if (is_not_dash(s[8]) or is_not_dash(s[13]) or is_not_dash(s[18]) or is_not_dash(s[23])) {
      throw std::invalid_argument{"Invalid UUID format"};
    }

    try {
      const auto data_4a = binascii::unhexlify<2, uint8_t>(s.substr(19, 4));
      const auto data_4b = binascii::unhexlify<6, uint8_t>(s.substr(24, 12));

      data = uuid{binascii::from_hex_chars<uint32_t>(s.substr(0, 8)),
                  binascii::from_hex_chars<uint16_t>(s.substr(9, 4)),
                  binascii::from_hex_chars<uint16_t>(s.substr(14, 4)),
                  {data_4a[0], data_4a[1], data_4b[0], data_4b[1], data_4b[2], data_4b[3], data_4b[4], data_4b[5]}}
                 .data;
    } catch (const std::invalid_argument&) {
      throw std::invalid_argument{"Invalid UUID format"};
    }
  }
#endif

  constexpr auto operator<=>(const uuid&) const noexcept = default;

  _WITE_NODISCARD bool into_c_str(char* out, size_t size, char format = 'D') const noexcept {
    return to_c_str(*this, out, size, format);
  }
  _WITE_NODISCARD bool into_c_str(wchar_t* out, size_t size, char format = 'D') const noexcept {
    return to_c_str(*this, out, size, format);
  }
  _WITE_NODISCARD std::string str(char format = 'D') const { return to_string(*this, format); };
  _WITE_NODISCARD std::wstring wstr(char format = 'D') const { return to_wstring(*this, format); };

  std::array<uint8_t, 16> data{};
};

constexpr static auto nulluuid = uuid{};

inline uuid make_uuid() {
  static thread_local auto random_engine = std::mt19937_64(std::random_device{}());
  return uuid{random_engine};
}

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

  return uuid{data_1.value(),
              data_2.value(),
              data_3.value(),
              {data_4a.value()[0],
               data_4a.value()[1],
               data_4b.value()[0],
               data_4b.value()[1],
               data_4b.value()[2],
               data_4b.value()[3],
               data_4b.value()[4],
               data_4b.value()[5]}};
}

namespace detail {
  template <typename Char_T, char FMT_TYPE>
  _WITE_CONSTEVAL auto _uuid_format() {
    if constexpr (std::is_same_v<Char_T, char>) {
      if constexpr ('D' == FMT_TYPE) {
        return "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";
      } else if constexpr ('N' == FMT_TYPE) {
        return "%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X";
      } else if constexpr ('B' == FMT_TYPE) {
        return "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}";
      } else if constexpr ('P' == FMT_TYPE) {
        return "(%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X)";
      } else if constexpr ('X' == FMT_TYPE) {
        return "{0x%08X,0x%04X,0x%04X,{0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}}";
      }
    } else {
      if constexpr ('D' == FMT_TYPE) {
        return L"%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";
      } else if constexpr ('N' == FMT_TYPE) {
        return L"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X";
      } else if constexpr ('B' == FMT_TYPE) {
        return L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}";
      } else if constexpr ('P' == FMT_TYPE) {
        return L"(%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X)";
      } else if constexpr ('X' == FMT_TYPE) {
        return L"{0x%08X,0x%04X,0x%04X,{0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X,0x%02X}}";
      }
    }
  }

  template <char FMT_TYPE>
  _WITE_CONSTEVAL size_t _uuid_strlen() {
    if constexpr ('D' == FMT_TYPE) {
      return 36 + 1;
    } else if constexpr ('N' == FMT_TYPE) {
      return 32 + 1;
    } else if constexpr ('B' == FMT_TYPE) {
      return 38 + 1;
    } else if constexpr ('P' == FMT_TYPE) {
      return 38 + 1;
    } else if constexpr ('X' == FMT_TYPE) {
      return 69 + 1;
    }
  }

  template <typename Char_T>
  using _uuid_sprintf_t = int (*)(Char_T* const _Buffer, size_t const _BufferCount, Char_T const* const _Format, ...);

  template <typename Char_T>
  constexpr _uuid_sprintf_t<Char_T> _uuid_sprintf() {
    if constexpr (std::is_same_v<Char_T, char>) {
      return ::snprintf;
    } else {
      return ::swprintf;
    }
  }

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

    const uint32_t& data_1 = *reinterpret_cast<const uint32_t*>(&id);
    const uint16_t& data_2 = *reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(&id) + 4);
    const uint16_t& data_3 = *reinterpret_cast<const uint16_t*>(reinterpret_cast<const uint8_t*>(&id) + 6);
    const uint8_t* data_4  = reinterpret_cast<const uint8_t*>(&id) + 8;

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
    }

    throw std::invalid_argument{"Invalid UUID format type"};
  }

}  // namespace detail

#if _WITE_HAS_CONCEPTS
template <wite::uuid_like Uuid_T>
_WITE_NODISCARD bool to_c_str(const Uuid_T& id, char* buffer, size_t max_buffer_length, char format) {
  using uuid_t = Uuid_T;
#else
_WITE_NODISCARD bool to_c_str(const uuid& id, char* buffer, size_t max_buffer_length, char format) {
  using uuid_t = uuid;
#endif
  switch (format) {
    case 'D':
      return detail::_to_c_str<char, uuid_t, 'D'>(id, buffer, max_buffer_length);
    case 'N':
      return detail::_to_c_str<char, uuid_t, 'N'>(id, buffer, max_buffer_length);
    case 'B':
      return detail::_to_c_str<char, uuid_t, 'B'>(id, buffer, max_buffer_length);
    case 'P':
      return detail::_to_c_str<char, uuid_t, 'P'>(id, buffer, max_buffer_length);
    case 'X':
      return detail::_to_c_str<char, uuid_t, 'X'>(id, buffer, max_buffer_length);
    default:
      return false;
  }
}

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

#if _WITE_HAS_CONCEPTS
template <wite::uuid_like Uuid_T>
_WITE_NODISCARD bool to_c_str(const Uuid_T& id, wchar_t* buffer, size_t max_buffer_length, char format) {
  using uuid_t = Uuid_T;
#else
_WITE_NODISCARD inline bool to_c_str(const uuid& id, wchar_t* buffer, size_t max_buffer_length, char format) {
  using uuid_t = uuid;
#endif
  switch (format) {
    case 'D':
      return detail::_to_c_str<wchar_t, uuid_t, 'D'>(id, buffer, max_buffer_length);
    case 'N':
      return detail::_to_c_str<wchar_t, uuid_t, 'N'>(id, buffer, max_buffer_length);
    case 'B':
      return detail::_to_c_str<wchar_t, uuid_t, 'B'>(id, buffer, max_buffer_length);
    case 'P':
      return detail::_to_c_str<wchar_t, uuid_t, 'P'>(id, buffer, max_buffer_length);
    case 'X':
      return detail::_to_c_str<wchar_t, uuid_t, 'X'>(id, buffer, max_buffer_length);
    default:
      return false;
  }
}

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

}  // namespace wite
