#pragma once

#include <wite/env/environment.hpp>

#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::common {

///////////////////////////////////////////////////////////////////////////////

template <typename T>
_WITE_CONCEPT is_pod_like = std::is_standard_layout_v<T> && std::is_trivial_v<T>;


// Adapted from https://stackoverflow.com/a/31409532
template <typename T, typename = void>
struct is_input_iterator : std::false_type {};

template <typename T>
struct is_input_iterator<T,
                         ::std::void_t<decltype(++std::declval<T>()),                      // incrementable,
                                       decltype(*std::declval<T>()),                       // dereferencable,
                                       decltype(std::declval<T>() == std::declval<T>())>>  // comparable
    : std::true_type {};

template <typename T>
_WITE_CONCEPT is_iterator_v = is_input_iterator<T>::value;



template <typename T, typename = void>
struct is_sized_range : std::false_type {};

template <typename T>
struct is_sized_range<T,
                      ::std::void_t<decltype(std::declval<T>().begin()),  // has a beginning
                                    decltype(std::declval<T>().end()),    // ...and an end
                                    decltype(std::declval<T>().size())>>  // ...and the size can be known
    : std::true_type {};

template <typename T>
_WITE_CONCEPT is_sized_range_v = is_sized_range<T>::value;

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::common

///////////////////////////////////////////////////////////////////////////////
