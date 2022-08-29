#pragma once

#include <wite/common/constructor_macros.hpp>

#include <type_traits>
#include <bit>

namespace wite::io {

struct encoding {
};

template<typename Value_T, std::endian>
requires std::is_integral_v<Value_T> and (sizeof(Value_T) > 1)
struct endianness : public encoding {
  using value_type = Value_T;

  Value_T value;

  endianness(Value_T val) : value{val} {}

  DEFAULT_CONSTRUCTORS(endianness);
};

template<typename Value_T>
struct little_endian : public endianness<Value_T, std::endian::little> {
  little_endian(Value_T val) : endianness<Value_T, std::endian::little>{val} {}

  DEFAULT_CONSTRUCTORS(little_endian);
};

template <typename Value_T>
little_endian(Value_T) -> little_endian<Value_T>;

template <typename Value_T>
struct big_endian : public endianness<Value_T, std::endian::big> {
  big_endian(Value_T val) : endianness<Value_T, std::endian::big>{val} {}

  DEFAULT_CONSTRUCTORS(big_endian);
};

template <typename Value_T>
big_endian(Value_T) -> big_endian<Value_T>;

}
