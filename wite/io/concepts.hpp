#pragma once

#include <wite/env/features.hpp>
#include <wite/io/encoding.hpp>

#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

template <typename T>
_WITE_CONCEPT is_buffer_readable = std::is_standard_layout_v<T> && std::is_trivial_v<T>;

template <typename T>
_WITE_CONCEPT is_buffer_writeable = std::is_standard_layout_v<T> && std::is_trivial_v<T>;

template <typename T>
_WITE_CONCEPT is_encoded = std::is_base_of_v<io::encoding, T>;

#if !_WITE_HAS_CONCEPTS
// Adapted from https://stackoverflow.com/a/31409532
template <typename T, typename = void>
struct is_input_iterator : std::false_type {};

template <typename T>
struct is_input_iterator<T, ::std::void_t<decltype(++std::declval<T>()),                       // incrementable,
                                          decltype(*std::declval<T>()),                        // dereferencable,
                                          decltype(std::declval<T>() == std::declval<T>())>>   // comparable
    : std::true_type {};

template <typename T>
constexpr bool is_iterator_v = is_input_iterator<T>::value;
#endif

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
