#pragma once

#include <wite/env/environment.hpp>
#include <wite/common/constructor_macros.hpp>
#include <wite/io/types.hpp>

#include <bit>
#include <type_traits>

#if !_WITE_HAS_CONCEPTS
#error "C++20 concepts are require, but the compiler doesn't support them"
#endif

namespace wite::io {

#if _WITE_HAS_STD_ENDIAN
using endian = std::endian;
#else
enum class endian { little = 0, big = 1, native = little };
#endif

struct encoding {};

template <typename Value_T, endian ENDIANNESS>
  requires std::is_integral_v<Value_T> and (sizeof(Value_T) > 1)
struct endianness : public encoding {
  using value_type = Value_T;

  Value_T value;

  endianness(Value_T val) : value{val} {}

  WITE_DEFAULT_CONSTRUCTORS(endianness);
};

template <typename Value_T>
struct little_endian : public endianness<Value_T, endian::little> {
  little_endian(Value_T val) : endianness<Value_T, endian::little>{val} {}

  WITE_DEFAULT_CONSTRUCTORS(little_endian);

  _WITE_NODISCARD auto begin_byte() const noexcept { return reinterpret_cast<const io::byte*>(&(this->value)); }
  _WITE_NODISCARD auto begin_byte() noexcept { return const_cast<io::byte*>(const_cast<const little_endian*>(this)->begin_byte());
  }
};

template <typename Value_T>
little_endian(Value_T) -> little_endian<Value_T>;

template <typename Value_T>
struct big_endian : public endianness<Value_T, endian::big> {
  big_endian(Value_T val) : endianness<Value_T, endian::big>{val} {}

  WITE_DEFAULT_CONSTRUCTORS(big_endian);
};

template <typename Value_T>
big_endian(Value_T) -> big_endian<Value_T>;

}  // namespace wite::io
