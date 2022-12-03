#pragma once

#include <wite/env/features.hpp>

#include <algorithm>
#include <array>
#include <compare>
#include <cstdint>
#include <random>
#include <string>
#include <tuple>

namespace wite {

struct uuid {
  uint32_t data_1{};
  uint16_t data_2{};
  uint16_t data_3{};
  std::array<uint8_t, 8> data_4{};

  constexpr auto operator<=>(const uuid&) const noexcept = default;

  _WITE_NODISCARD bool into_c_str(char* out, size_t size) const noexcept {
    if (size < 39) {
      return false;
    }

    std::ignore = ::snprintf(out,
               size,
               "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
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
    out[38]     = '\0';

    return true;
  }

  _WITE_NODISCARD std::string str() const {
    char buffer[39] = {};
    std::ignore = into_c_str(buffer, 39);
    return {buffer};
  }
};

#ifdef _WITE_HAS_CONCEPTS
template <typename T>
concept uuid_like = requires(T& t) {
                      std::is_same_v<std::decay_t<decltype(t.data_1)>, uint32_t>;
                      std::is_same_v<std::decay_t<decltype(t.data_2)>, uint16_t>;
                      std::is_same_v<std::decay_t<decltype(t.data_3)>, uint16_t>;
                      std::is_same_v<std::decay_t<decltype(t.data_4)>, std::array<uint8_t, 8>> or
                          std::is_same_v<std::decay_t<decltype(t.data_4)>, unsigned char[8]>;
                      sizeof(std::decay_t<decltype(t)>) == sizeof(uuid);
                    };
#endif

inline uuid make_uuid() {
  static auto random_engine = std::mt19937_64(std::random_device{}());

  auto random_bits_8  = std::uniform_int_distribution<uint16_t>{0x00, 0xFF};
  auto random_bits_16 = std::uniform_int_distribution<uint16_t>{0x00, 0xFFFF};
  auto random_bits_32 = std::uniform_int_distribution<uint32_t>{0x00, 0xFFFFFFFF};

  return {random_bits_32(random_engine),
          static_cast<uint16_t>((random_bits_16(random_engine) & 0x0F) | 0x40),  // Version 4 UUID
          static_cast<uint16_t>((random_bits_16(random_engine) & 0x3F) | 0x80),  // Variant 1 UUID
          {static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine))}};
}

_WITE_NODISCARD std::string to_string(const uuid& id) {
  return id.str();
}

}  // namespace wite
