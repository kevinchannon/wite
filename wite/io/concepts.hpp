/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/common/concepts.hpp>
#include <wite/env/features.hpp>
#include <wite/io/encoding.hpp>

#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::io {

///////////////////////////////////////////////////////////////////////////////

template<typename T>
constexpr bool is_byte_like_v = sizeof(T) == 1;

template<typename T>
concept byte_range_like = requires(T& t) {
                            is_byte_like_v<typename std::decay_t<T>::value_type>;
                            t.begin();
                            t.rbegin();
                            t.end();
                            t.rend();
                          };

template <typename T>
concept byte_iterator_like = requires(T& t) {
                               ++t;
                               --t;
                               t += 1;
                               t -= 1;
                               t + 1;
                               t - 1;
                               sizeof(*t) == 1;
                          };

template <typename T>
_WITE_CONCEPT is_encoded = std::is_base_of_v<io::encoding, T>;

#if _WITE_HAS_CONCEPTS
template <typename T>
_WITE_CONCEPT is_buffer_readable = (not(std::is_array_v<T> or std::is_pointer_v<T>)) and
                                   (common::is_pod_like<T> or (is_encoded<T> and common::is_pod_like<typename T::value_type>));

template <typename T>
_WITE_CONCEPT is_buffer_writeable = (not(std::is_array_v<T> or std::is_pointer_v<T>)) and
                                    (common::is_pod_like<T> or (is_encoded<T> and common::is_pod_like<typename T::value_type>));
#else  // ^^^ _WITE_HAS_CONCEPTS / vvv !_WITE_HAS_CONCEPTS
namespace detail {
  template <typename T>
  _WITE_CONSTEVAL std::enable_if_t<is_encoded<T>, bool> _is_pod_like() {
    return common::is_pod_like<typename T::value_type>;
  }

  template <typename T>
  _WITE_CONSTEVAL std::enable_if_t<!is_encoded<T>, bool> _is_pod_like() {
    return common::is_pod_like<T>;
  }

}  // namespace detail

template <typename T>
_WITE_CONCEPT is_buffer_readable = (not(std::is_array_v<T> or std::is_pointer_v<T>)) and detail::_is_pod_like<T>();

template <typename T>
_WITE_CONCEPT is_buffer_writeable = (not(std::is_array_v<T> or std::is_pointer_v<T>)) and detail::_is_pod_like<T>();
#endif

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::io

///////////////////////////////////////////////////////////////////////////////
