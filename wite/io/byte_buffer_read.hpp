#pragma once

#include <wite/io/encoding.hpp>
#include <wite/io/types.hpp>
#include <wite/io/concepts.hpp>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <iterator>
#include <span>
#include <stdexcept>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T>
auto unchecked_read(auto buffer)
{
  if constexpr (std::is_base_of_v<io::encoding, Value_T>) {
    using OutputValue_t = typename Value_T::value_type;

    auto out = OutputValue_t{};

    if constexpr (std::is_same_v<io::little_endian<OutputValue_t>, Value_T>) {
      std::copy_n(buffer, sizeof(Value_T), reinterpret_cast<std::byte*>(&out));
    } else if constexpr (std::is_same_v<io::big_endian<OutputValue_t>, Value_T>) {
        std::copy_n(
            buffer, sizeof(Value_T), std::make_reverse_iterator(std::next(reinterpret_cast<std::byte*>(&out), sizeof(Value_T))));
    }
    else {
      static_assert(std::is_same_v<io::little_endian<OutputValue_t>, Value_T>, "Invalid encoding type");
    }

    return std::pair<OutputValue_t, decltype(buffer)>{out, std::next(buffer, sizeof(out))};
  } else {
    auto out = Value_T{};
    std::copy_n(buffer, sizeof(Value_T), reinterpret_cast<std::byte*>(&out));
    return std::pair<Value_T, decltype(buffer)>{out, std::next(buffer, sizeof(out))};
  }
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
  requires is_buffer_readable<Value_T>
auto read(const std::span<const std::byte>& buffer) {
  if (buffer.size() < sizeof(Value_T)) {
    throw std::out_of_range{"Insufficient buffer space for read"};
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, typename Value_Ts>
auto read(const std::span<const std::byte>& buffer) {
  return // TODO: Figure out how to do this. Fold expressions?
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_readable<Value_T>
auto from_bytes(const std::span<const std::byte>& buffer) {
  return read<Value_T>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>)
read_result_t<Value_T> try_read(const std::span<const std::byte>& buffer) {
  if (buffer.size() < sizeof(Value_T)) {
    return read_error::insufficient_buffer;
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_readable<Value_T> and (not is_encoded<Value_T>)
read_result_t<Value_T> try_from_bytes(const std::span<const std::byte>& buffer) {
  return try_read<Value_T>(buffer);
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_readable<Value_T> and is_encoded<Value_T>
read_result_t<typename Value_T::value_type> try_read( const std::span<const std::byte>& buffer) {
  if (buffer.size() < sizeof(typename Value_T::value_type)) {
    return read_error::insufficient_buffer;
  }

  return unchecked_read<Value_T>(buffer.begin()).first;
}

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T>
requires is_buffer_readable<Value_T>
Value_T read(const std::span<const std::byte>& buffer, std::endian endianness) {
  if (std::endian::little == endianness) {
    return read<io::little_endian<Value_T>>(buffer);
  } else {
    return read<io::big_endian<Value_T>>(buffer);
  }
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
