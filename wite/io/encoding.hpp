#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>
#include <wite/io/types.hpp>

#include <bit>
#include <type_traits>

#if !_WITE_HAS_CONCEPTS
#error "C++20 concepts are require, but the compiler doesn't support them"
#endif

namespace wite::io {

template <typename T>
concept byte_like = requires(T& t) { sizeof(t) == 1; };
 
#if _WITE_HAS_STD_ENDIAN
using endian = std::endian;
#else
enum class endian { little = 0, big = 1, native = little };
#endif

#ifdef WITE_LITTLE_ENDIAN
#ifdef WITE_BIG_ENDIAN
#error "Specify only one of WITE_LITTLE_ENDIAN, or WITE_BIG_ENDIAN"
#endif
constexpr auto system_native_endianness = endian::little;
#else
#ifdef WITE_BIG_ENDIAN
#ifdef WITE_LITTLE_ENDIAN
#error "Specify only one of WITE_LITTLE_ENDIAN, or WITE_BIG_ENDIAN"
#endif
constexpr auto system_native_endianness = endian::big;
#else
constexpr auto system_native_endianness = endian::native;
#endif// WITE_BIG_ENDIAN
#endif // WITE_LITTLE_ENDIAN

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
  using value_type = typename endianness<Value_T, endian::little>::value_type;

  little_endian(Value_T val) : endianness<Value_T, endian::little>{val} {}

  WITE_DEFAULT_CONSTRUCTORS(little_endian);

  template<byte_like Byte_T>
  _WITE_NODISCARD auto byte_begin() const noexcept {
    static_assert(endian::big == system_native_endianness or endian::little == system_native_endianness,
                  "Endianness should be either big or little");
    if constexpr (endian::little == system_native_endianness) {
      return reinterpret_cast<const Byte_T*>(&(this->value));
    } else {
      return std::make_reverse_iterator(std::next(reinterpret_cast<const Byte_T*>(&(this->value)), sizeof(value_type)));
    }
  }

  template <byte_like Byte_T>
  _WITE_NODISCARD auto byte_begin() noexcept {
    static_assert(endian::big == system_native_endianness or endian::little == system_native_endianness,
                  "Endianness should be either big or little");
    if constexpr (endian::little == system_native_endianness) {
      return reinterpret_cast<Byte_T*>(&(this->value));
    } else {
      return std::make_reverse_iterator(std::next(reinterpret_cast<Byte_T*>(&(this->value)), sizeof(value_type)));
    }
  }

  _WITE_NODISCARD constexpr auto byte_count() const noexcept { return sizeof(value_type); }
};

template <typename Value_T>
little_endian(Value_T) -> little_endian<Value_T>;

template <typename Value_T>
struct big_endian : public endianness<Value_T, endian::big> {
  using value_type = typename endianness<Value_T, endian::big>::value_type;

  big_endian(Value_T val) : endianness<Value_T, endian::big>{val} {}

  WITE_DEFAULT_CONSTRUCTORS(big_endian);

  template <byte_like Byte_T>
  _WITE_NODISCARD auto byte_begin() const noexcept {
    static_assert(endian::big == system_native_endianness or endian::little == system_native_endianness,
                  "Endianness should be either big or little");
    if constexpr (endian::little == system_native_endianness) {
      return std::make_reverse_iterator(std::next(reinterpret_cast<const Byte_T*>(&(this->value)), sizeof(value_type)));
    } else {
      return reinterpret_cast<const Byte_T*>(&(this->value));
    }
  }

  
  template <byte_like Byte_T>
  _WITE_NODISCARD auto byte_begin() noexcept {
    static_assert(endian::big == system_native_endianness or endian::little == system_native_endianness,
                  "Endianness should be either big or little");
    if constexpr (endian::little == system_native_endianness) {
      return std::make_reverse_iterator(std::next(reinterpret_cast<Byte_T*>(&(this->value)), sizeof(value_type)));
    } else {
      return reinterpret_cast<Byte_T*>(&(this->value));
    }
  }

  _WITE_NODISCARD constexpr auto byte_count() const noexcept { return sizeof(value_type); }
};

template <typename Value_T>
big_endian(Value_T) -> big_endian<Value_T>;

}  // namespace wite::io
