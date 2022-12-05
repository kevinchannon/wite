#pragma once

#include <wite/env/environment.hpp>
#include <wite/io/types.hpp>
#include <wite/core/result.hpp>

#ifdef WITE_NO_EXCEPTIONS
#error "Exceptions are required if binascii/hexlify.hpp is included"
#endif

#include <array>
#include <string>
#include <string_view>
#include <stdexcept>
#include <iostream>

namespace wite::binascii {

static constexpr auto valid_hex_chars = std::string_view{"0123456789ABCDEFabcdef"};

namespace detail {
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

  constexpr auto _invalid_nibble = std::byte{0xFF};

  inline io::byte low_nibble(char c){
    switch(c) {
      case '0': return io::byte(0x00);
      case '1': return io::byte(0x01);
      case '2': return io::byte(0x02);
      case '3': return io::byte(0x03);
      case '4': return io::byte(0x04);
      case '5': return io::byte(0x05);
      case '6': return io::byte(0x06);
      case '7': return io::byte(0x07);
      case '8': return io::byte(0x08);
      case '9': return io::byte(0x09);
      case 'A': case 'a': return io::byte(0x0A);
      case 'B': case 'b': return io::byte(0x0B);
      case 'C': case 'c': return io::byte(0x0C);
      case 'D': case 'd': return io::byte(0x0D);
      case 'E': case 'e': return io::byte(0x0E);
      case 'F': case 'f': return io::byte(0x0F);
      default:
        return _invalid_nibble;
    }
  }

  inline io::byte high_nibble(char c){
    switch(c) {
      case '0': return io::byte(0x00);
      case '1': return io::byte(0x10);
      case '2': return io::byte(0x20);
      case '3': return io::byte(0x30);
      case '4': return io::byte(0x40);
      case '5': return io::byte(0x50);
      case '6': return io::byte(0x60);
      case '7': return io::byte(0x70);
      case '8': return io::byte(0x80);
      case '9': return io::byte(0x90);
      case 'A': case 'a': return io::byte(0xA0);
      case 'B': case 'b': return io::byte(0xB0);
      case 'C': case 'c': return io::byte(0xC0);
      case 'D': case 'd': return io::byte(0xD0);
      case 'E': case 'e': return io::byte(0xE0);
      case 'F': case 'f': return io::byte(0xF0);
      default:
        return _invalid_nibble;
    }
  }
}  // namespace detail

template <typename Result_T>
_WITE_NODISCARD Result_T unsafe_from_hex_chars(const std::string_view str) noexcept {
  auto out = Result_T{};

  auto write_byte = reinterpret_cast<uint8_t*>(&out);
  const auto end  = write_byte + sizeof(Result_T);
  auto read_pos   = str.begin();

  for (auto b = write_byte; b != end; ++b, read_pos += 2) {
    *b = static_cast<uint8_t>(detail::high_nibble(*read_pos) | detail::low_nibble(*std::next(read_pos)));
  }

  return out;
}

template <typename Result_T>
_WITE_NODISCARD Result_T from_hex_chars(const std::string_view str) {
  if (str.length() != 2 * sizeof(Result_T)) {
    throw std::invalid_argument{"Invalid sequence length for type"};
  }

  if (std::string::npos != str.find_first_not_of(valid_hex_chars)) {
    throw std::invalid_argument{"Invalid hex char"};
  }

  return unsafe_from_hex_chars<Result_T>(str);
}

enum class from_hex_chars_error { invalid_sequence_length, invalid_hex_char };

template <typename Result_T>
using from_hex_chars_result_t = result<Result_T, from_hex_chars_error>;

template <typename Result_T>
_WITE_NODISCARD from_hex_chars_result_t<Result_T> try_from_hex_chars(const std::string_view str) noexcept {
  if (str.length() != 2 * sizeof(Result_T)) {
    return from_hex_chars_result_t<Result_T>{from_hex_chars_error::invalid_sequence_length};
  }

  if (std::string::npos != str.find_first_not_of(valid_hex_chars)) {
    return from_hex_chars_result_t<Result_T>{from_hex_chars_error::invalid_hex_char};
  }

  return unsafe_from_hex_chars<Result_T>(str);
}

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

_WITE_NODISCARD inline io::dynamic_byte_buffer unhexlify(const std::string_view str) {
  auto out = io::dynamic_byte_buffer(str.length() / 2, io::byte{});
  auto read_pos = str.begin();

  for (auto& b : out) {
    b = from_hex_chars<io::byte>(std::string_view(read_pos, std::next(read_pos, 2)));
    std::advance(read_pos, 2);
  }

  return out;
}

}  // namespace wite::binascii
