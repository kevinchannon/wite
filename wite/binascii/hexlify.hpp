/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/core/result.hpp>
#include <wite/env/environment.hpp>
#include <wite/io/types.hpp>

#include <array>
#include <iostream>
#include <string>
#include <string_view>

#ifndef WITE_NO_EXCEPTIONS
#include<stdexcept>
#endif

///////////////////////////////////////////////////////////////////////////////

namespace wite::binascii {

///////////////////////////////////////////////////////////////////////////////

static constexpr auto valid_hex_chars = std::string_view{"0123456789ABCDEFabcdef"};
static constexpr auto valid_hex_wchars = std::wstring_view{L"0123456789ABCDEFabcdef"};

template<typename Char_T>
bool is_all_valid_hex_chars(std::basic_string_view<Char_T> str){
  if constexpr (std::is_same_v<std::make_signed_t<char>, std::make_signed_t<Char_T>>){
    return std::string::npos == str.find_first_not_of(valid_hex_chars);
  } else {
    return std::string::npos == str.find_first_not_of(valid_hex_wchars);
  }
}

enum class error { invalid_sequence_length, invalid_hex_char };

template <typename Result_T>
using result_t = result<Result_T, error>;

///////////////////////////////////////////////////////////////////////////////

namespace detail {

  ///////////////////////////////////////////////////////////////////////////////
  //
  // clang-format off
  constexpr auto upper_case_bytes = std::array{
      "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
      "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
      "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
      "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
      "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
      "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
      "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
      "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
      "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
      "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
      "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
      "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
      "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
      "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
      "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
      "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
  };
  // clang-format on
  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // clang-format off
  constexpr auto wide_upper_case_bytes = std::array{
      L"00", L"01", L"02", L"03", L"04", L"05", L"06", L"07", L"08", L"09", L"0A", L"0B", L"0C", L"0D", L"0E", L"0F",
      L"10", L"11", L"12", L"13", L"14", L"15", L"16", L"17", L"18", L"19", L"1A", L"1B", L"1C", L"1D", L"1E", L"1F",
      L"20", L"21", L"22", L"23", L"24", L"25", L"26", L"27", L"28", L"29", L"2A", L"2B", L"2C", L"2D", L"2E", L"2F",
      L"30", L"31", L"32", L"33", L"34", L"35", L"36", L"37", L"38", L"39", L"3A", L"3B", L"3C", L"3D", L"3E", L"3F",
      L"40", L"41", L"42", L"43", L"44", L"45", L"46", L"47", L"48", L"49", L"4A", L"4B", L"4C", L"4D", L"4E", L"4F",
      L"50", L"51", L"52", L"53", L"54", L"55", L"56", L"57", L"58", L"59", L"5A", L"5B", L"5C", L"5D", L"5E", L"5F",
      L"60", L"61", L"62", L"63", L"64", L"65", L"66", L"67", L"68", L"69", L"6A", L"6B", L"6C", L"6D", L"6E", L"6F",
      L"70", L"71", L"72", L"73", L"74", L"75", L"76", L"77", L"78", L"79", L"7A", L"7B", L"7C", L"7D", L"7E", L"7F",
      L"80", L"81", L"82", L"83", L"84", L"85", L"86", L"87", L"88", L"89", L"8A", L"8B", L"8C", L"8D", L"8E", L"8F",
      L"90", L"91", L"92", L"93", L"94", L"95", L"96", L"97", L"98", L"99", L"9A", L"9B", L"9C", L"9D", L"9E", L"9F",
      L"A0", L"A1", L"A2", L"A3", L"A4", L"A5", L"A6", L"A7", L"A8", L"A9", L"AA", L"AB", L"AC", L"AD", L"AE", L"AF",
      L"B0", L"B1", L"B2", L"B3", L"B4", L"B5", L"B6", L"B7", L"B8", L"B9", L"BA", L"BB", L"BC", L"BD", L"BE", L"BF",
      L"C0", L"C1", L"C2", L"C3", L"C4", L"C5", L"C6", L"C7", L"C8", L"C9", L"CA", L"CB", L"CC", L"CD", L"CE", L"CF",
      L"D0", L"D1", L"D2", L"D3", L"D4", L"D5", L"D6", L"D7", L"D8", L"D9", L"DA", L"DB", L"DC", L"DD", L"DE", L"DF",
      L"E0", L"E1", L"E2", L"E3", L"E4", L"E5", L"E6", L"E7", L"E8", L"E9", L"EA", L"EB", L"EC", L"ED", L"EE", L"EF",
      L"F0", L"F1", L"F2", L"F3", L"F4", L"F5", L"F6", L"F7", L"F8", L"F9", L"FA", L"FB", L"FC", L"FD", L"FE", L"FF"
  };
  // clang-format on

  ///////////////////////////////////////////////////////////////////////////////

  constexpr auto _invalid_nibble = io::byte{0xFF};

  ///////////////////////////////////////////////////////////////////////////////

  inline io::byte low_nibble(char c) {
    switch (c) {
      case '0':
        return io::byte(0x00);
      case '1':
        return io::byte(0x01);
      case '2':
        return io::byte(0x02);
      case '3':
        return io::byte(0x03);
      case '4':
        return io::byte(0x04);
      case '5':
        return io::byte(0x05);
      case '6':
        return io::byte(0x06);
      case '7':
        return io::byte(0x07);
      case '8':
        return io::byte(0x08);
      case '9':
        return io::byte(0x09);
      case 'A':
      case 'a':
        return io::byte(0x0A);
      case 'B':
      case 'b':
        return io::byte(0x0B);
      case 'C':
      case 'c':
        return io::byte(0x0C);
      case 'D':
      case 'd':
        return io::byte(0x0D);
      case 'E':
      case 'e':
        return io::byte(0x0E);
      case 'F':
      case 'f':
        return io::byte(0x0F);
      default:
        return _invalid_nibble;
    }
  }
  
  ///////////////////////////////////////////////////////////////////////////////

  inline io::byte low_nibble(wchar_t c) {
    switch (c) {
      case L'0':
        return io::byte(0x00);
      case L'1':
        return io::byte(0x01);
      case L'2':
        return io::byte(0x02);
      case L'3':
        return io::byte(0x03);
      case L'4':
        return io::byte(0x04);
      case L'5':
        return io::byte(0x05);
      case L'6':
        return io::byte(0x06);
      case L'7':
        return io::byte(0x07);
      case L'8':
        return io::byte(0x08);
      case L'9':
        return io::byte(0x09);
      case L'A':
      case L'a':
        return io::byte(0x0A);
      case L'B':
      case L'b':
        return io::byte(0x0B);
      case L'C':
      case L'c':
        return io::byte(0x0C);
      case L'D':
      case L'd':
        return io::byte(0x0D);
      case L'E':
      case L'e':
        return io::byte(0x0E);
      case L'F':
      case L'f':
        return io::byte(0x0F);
      default:
        return _invalid_nibble;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  inline io::byte high_nibble(char c) {
    switch (c) {
      case '0':
        return io::byte(0x00);
      case '1':
        return io::byte(0x10);
      case '2':
        return io::byte(0x20);
      case '3':
        return io::byte(0x30);
      case '4':
        return io::byte(0x40);
      case '5':
        return io::byte(0x50);
      case '6':
        return io::byte(0x60);
      case '7':
        return io::byte(0x70);
      case '8':
        return io::byte(0x80);
      case '9':
        return io::byte(0x90);
      case 'A':
      case 'a':
        return io::byte(0xA0);
      case 'B':
      case 'b':
        return io::byte(0xB0);
      case 'C':
      case 'c':
        return io::byte(0xC0);
      case 'D':
      case 'd':
        return io::byte(0xD0);
      case 'E':
      case 'e':
        return io::byte(0xE0);
      case 'F':
      case 'f':
        return io::byte(0xF0);
      default:
        return _invalid_nibble;
    }
  }
  
  ///////////////////////////////////////////////////////////////////////////////

  inline io::byte high_nibble(wchar_t c) {
    switch (c) {
      case L'0':
        return io::byte(0x00);
      case L'1':
        return io::byte(0x10);
      case L'2':
        return io::byte(0x20);
      case L'3':
        return io::byte(0x30);
      case L'4':
        return io::byte(0x40);
      case L'5':
        return io::byte(0x50);
      case L'6':
        return io::byte(0x60);
      case L'7':
        return io::byte(0x70);
      case L'8':
        return io::byte(0x80);
      case L'9':
        return io::byte(0x90);
      case L'A':
      case L'a':
        return io::byte(0xA0);
      case L'B':
      case L'b':
        return io::byte(0xB0);
      case L'C':
      case L'c':
        return io::byte(0xC0);
      case L'D':
      case L'd':
        return io::byte(0xD0);
      case L'E':
      case L'e':
        return io::byte(0xE0);
      case L'F':
      case L'f':
        return io::byte(0xF0);
      default:
        return _invalid_nibble;
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

}  // namespace detail

///////////////////////////////////////////////////////////////////////////////

template <typename Result_T, typename Char_T>
_WITE_NODISCARD Result_T unsafe_from_hex_chars(const std::basic_string_view<Char_T> str) noexcept {
  auto out = Result_T{};

  auto write_byte = reinterpret_cast<uint8_t*>(&out);
  const auto end  = write_byte + sizeof(Result_T);
  auto read_pos   = str.begin();

  for (auto b = write_byte; b != end; ++b, read_pos += 2) {
    *b = static_cast<uint8_t>(detail::high_nibble(*read_pos) | detail::low_nibble(*std::next(read_pos)));
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_NO_EXCEPTIONS
template <typename Result_T>
_WITE_NODISCARD Result_T from_hex_chars(const std::string_view str) {
  if (str.length() != 2 * sizeof(Result_T)) {
    throw std::invalid_argument{"Invalid sequence length for type"};
  }

  if (not is_all_valid_hex_chars(str)) {
    throw std::invalid_argument{"Invalid hex char"};
  }

  return unsafe_from_hex_chars<Result_T>(str);
}

template <typename Result_T>
_WITE_NODISCARD Result_T from_hex_chars(const std::wstring_view str) {
  if (str.length() != 2 * sizeof(Result_T)) {
    throw std::invalid_argument{"Invalid sequence length for type"};
  }

  if (not is_all_valid_hex_chars(str)) {
    throw std::invalid_argument{"Invalid hex char"};
  }

  return unsafe_from_hex_chars<Result_T>(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////

template <typename Result_T>
_WITE_NODISCARD result_t<Result_T> try_from_hex_chars(const std::string_view str) noexcept {
  if (str.length() != 2 * sizeof(Result_T)) {
    return result_t<Result_T>{error::invalid_sequence_length};
  }

  if (not is_all_valid_hex_chars(str)) {
    return result_t<Result_T>{error::invalid_hex_char};
  }

  return unsafe_from_hex_chars<Result_T, char>(str);
}

template <typename Result_T>
_WITE_NODISCARD result_t<Result_T> try_from_hex_chars(const std::wstring_view str) noexcept {
  if (str.length() != 2 * sizeof(Result_T)) {
    return result_t<Result_T>{error::invalid_sequence_length};
  }

  if (not is_all_valid_hex_chars(str)) {
    return result_t<Result_T>{error::invalid_hex_char};
  }

  return unsafe_from_hex_chars<Result_T, wchar_t>(str);
}

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_NO_EXCEPTIONS
template <typename Range_T>
_WITE_NODISCARD std::string hexlify(Range_T&& bytes) {
  auto out       = std::string(2 * bytes.size(), char{});
  auto write_pos = out.begin();

  for (io::byte b : bytes) {
    const auto chars = detail::upper_case_bytes[io::to_integer<size_t>(b)];
    *write_pos       = chars[0];
    ++write_pos;

    *write_pos = chars[1];
    ++write_pos;
  }

  return out;
}
#endif

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_NO_EXCEPTIONS
_WITE_NODISCARD inline io::dynamic_byte_buffer unhexlify(const std::string_view str) {
  auto out      = io::dynamic_byte_buffer(str.length() / 2, io::byte{});
  auto read_pos = str.begin();

  for (auto& b : out) {
    b = from_hex_chars<io::byte>(std::string_view(read_pos, std::next(read_pos, 2)));
    std::advance(read_pos, 2);
  }

  return out;
}

_WITE_NODISCARD inline io::dynamic_byte_buffer unhexlify(const std::wstring_view str) {
  auto out      = io::dynamic_byte_buffer(str.length() / 2, io::byte{});
  auto read_pos = str.begin();

  for (auto& b : out) {
    b = from_hex_chars<io::byte>(std::wstring_view(read_pos, std::next(read_pos, 2)));
    std::advance(read_pos, 2);
  }

  return out;
}
#endif

///////////////////////////////////////////////////////////////////////////////

template <size_t N, typename Value_T>
  requires(sizeof(Value_T) == 1)
_WITE_NODISCARD std::array<Value_T, N> unsafe_unhexlify(const std::string_view str) noexcept {
  auto read_pos = str.begin();

  std::array<Value_T, N> out;
  for (auto i = 0u; i < out.size(); ++i, std::advance(read_pos, 2)) {
    out[i] = unsafe_from_hex_chars<Value_T>(std::string_view(read_pos, std::next(read_pos, 2)));
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

template <size_t N, typename Value_T>
  requires(sizeof(Value_T) == 1)
_WITE_NODISCARD std::array<Value_T, N> unsafe_unhexlify(const std::wstring_view str) noexcept {
  auto read_pos = str.begin();

  std::array<Value_T, N> out;
  for (auto i = 0u; i < out.size(); ++i, std::advance(read_pos, 2)) {
    out[i] = unsafe_from_hex_chars<Value_T>(std::wstring_view(read_pos, std::next(read_pos, 2)));
  }

  return out;
}

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_NO_EXCEPTIONS
template <size_t N, typename Value_T>
  requires(sizeof(Value_T) == 1)
_WITE_NODISCARD std::array<Value_T, N> unhexlify(const std::string_view str) {
  if (str.length() != 2 * N) {
    throw std::invalid_argument{"Invalid sequence length"};
  }

  if (std::string::npos != str.find_first_not_of(valid_hex_chars)) {
    throw std::invalid_argument{"Invalid hex char"};
  }

  return unsafe_unhexlify<N, Value_T>(str);
}

template <size_t N, typename Value_T>
  requires(sizeof(Value_T) == 1)
_WITE_NODISCARD std::array<Value_T, N> unhexlify(const std::wstring_view str) {
  if (str.length() != 2 * N) {
    throw std::invalid_argument{"Invalid sequence length"};
  }

  if (not is_all_valid_hex_chars(str)) {
    throw std::invalid_argument{"Invalid hex char"};
  }

  return unsafe_unhexlify<N, Value_T>(str);
}
#endif

///////////////////////////////////////////////////////////////////////////////

template <size_t N, typename Value_T>
  requires(sizeof(Value_T) == 1)
_WITE_NODISCARD result_t<std::array<Value_T, N>> try_unhexlify(const std::string_view str) noexcept {
  if (str.length() != 2 * N) {
    return error::invalid_sequence_length;
  }

  if (not is_all_valid_hex_chars(str)) {
    return error::invalid_hex_char;
  }

  return unsafe_unhexlify<N, Value_T>(str);
}

///////////////////////////////////////////////////////////////////////////////

template <size_t N, typename Value_T>
  requires(sizeof(Value_T) == 1)
_WITE_NODISCARD result_t<std::array<Value_T, N>> try_unhexlify(const std::wstring_view str) noexcept {
  if (str.length() != 2 * N) {
    return error::invalid_sequence_length;
  }

  if (not is_all_valid_hex_chars(str)) {
    return error::invalid_hex_char;
  }

  return unsafe_unhexlify<N, Value_T>(str);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::binascii
