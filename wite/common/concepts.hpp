#pragma once

#include <wite/env/environment.hpp>

#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::common {

///////////////////////////////////////////////////////////////////////////////

template <typename T>
_WITE_CONCEPT is_pod_like = std::is_standard_layout_v<T> && std::is_trivial_v<T>;

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

}  // namespace wite::common

///////////////////////////////////////////////////////////////////////////////
