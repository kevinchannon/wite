#pragma once

#include <wite/common/constructor_macros.hpp>

#include <bit>
#include <type_traits>

namespace wite::io {

#if __cpp_lib_endian >= 201907
using endian = std::endian;
#else
enum class endian { little = 0, big = 1, native = little };
#endif

struct encoding {};

template <typename Value_T, endian ENDIANNESS>
#if __cpp_concepts >= 201907
requires std::is_integral_v<Value_T> and(sizeof(Value_T) > 1)
#endif
struct endianness : public encoding {
#if __cpp_concepts >= 201907
  using value_type = Value_T;
#else
  using value_type = std::enable_if_t<(std::is_integral_v<Value_T> && (sizeof(Value_T) != 1)), Value_T>;
#endif

  Value_T value;

  endianness(Value_T val) : value{val} {}

  DEFAULT_CONSTRUCTORS(endianness);
};

template <typename Value_T>
struct little_endian : public endianness<Value_T, endian::little> {
  little_endian(Value_T val) : endianness<Value_T, endian::little>{val} {}

  DEFAULT_CONSTRUCTORS(little_endian);
};

template <typename Value_T>
little_endian(Value_T) -> little_endian<Value_T>;

template <typename Value_T>
struct big_endian : public endianness<Value_T, endian::big> {
  big_endian(Value_T val) : endianness<Value_T, endian::big>{val} {}

  DEFAULT_CONSTRUCTORS(big_endian);
};

template <typename Value_T>
big_endian(Value_T) -> big_endian<Value_T>;

}  // namespace wite::io
