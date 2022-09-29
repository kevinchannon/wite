#pragma once

#include <wite/io/concepts.hpp>

#include <type_traits>

namespace wite::io {

template <typename Value_T>
  requires is_buffer_readable<Value_T>
_WITE_CONSTEVAL auto value_size() noexcept {
  if constexpr (is_encoded<Value_T>) {
    // This will fail to build if the type satisfies the requirements but doesn't have a value_type alias in it.
    // In that case, a new overload of this function will need to be added for the new type.
    return sizeof(typename Value_T::value_type);
  } else {
    return sizeof(Value_T);
  }
}

template <typename Value_T>
  requires(not std::is_pointer_v<std::remove_reference_t<Value_T>>)
constexpr auto value_size(Value_T&& value) noexcept {
  using DecayedValue_t = std::decay_t<Value_T>;

  if constexpr (common::is_sized_range_v<DecayedValue_t>) {
    return value.size() * sizeof(typename DecayedValue_t::value_type);
  }
  else {
    return value_size<DecayedValue_t>();
  }
}

template <typename T, typename... Ts>
constexpr auto byte_count(T first_value, Ts... other_values) noexcept {
  auto out = value_size(first_value);

  if constexpr (sizeof...(Ts) > 0) {
    out += byte_count(other_values...);
  }

  return out;
}

template<typename T, typename... Ts>
_WITE_CONSTEVAL auto byte_count() noexcept {
  auto out = value_size<T>();

  if constexpr (sizeof...(Ts) > 0) {
    out += byte_count<Ts...>();
  }

  return out;
}

}  // namespace wite::io
