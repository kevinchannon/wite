#pragma once

#include <algorithm>
#include <array>
#include <compare>
#include <cstdint>
#include <random>

namespace wite {

struct uuid {
  uint32_t data_1{};
  uint16_t data_2{};
  uint16_t data_3{};
  std::array<uint8_t, 8> data_4{};

  constexpr auto operator<=>(const uuid&) const noexcept = default;
};

inline uuid make_uuid() {
  static auto random_engine  = std::mt19937_64(std::random_device{}());
  
  auto random_bits_8  = std::uniform_int_distribution<uint16_t>{0x00, 0xFF};
  auto random_bits_16 = std::uniform_int_distribution<uint16_t>{0x00, 0xFFFF};
  auto random_bits_32 = std::uniform_int_distribution<uint32_t>{0x00, 0xFFFFFFFF};

  return {random_bits_32(random_engine),
          static_cast<uint16_t>((random_bits_16(random_engine) & 0x0F) | 0x40), // Version 4 UUID
          static_cast<uint16_t>((random_bits_16(random_engine) & 0x3F) | 0x80), // Variant 1 UUID
          {static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine)),
           static_cast<uint8_t>(random_bits_8(random_engine))}};
}

}  // namespace wite
