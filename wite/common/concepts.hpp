#pragma once

#include <wite/env/environment.hpp>

#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::common {

///////////////////////////////////////////////////////////////////////////////

template <typename T>
concept byte_like = requires(T& t) { sizeof(t) == 1; };

///////////////////////////////////////////////////////////////////////////////

template <typename>
inline constexpr bool always_false_v = false;

///////////////////////////////////////////////////////////////////////////////

template <typename T>
_WITE_CONCEPT is_pod_like = std::is_standard_layout_v<T> && std::is_trivial_v<T>;

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <typename T>
concept contiguous_range_like = requires(T& t) {
                                  t.begin();
                                  t.end();
                                  t.data();
                                };
#endif

///////////////////////////////////////////////////////////////////////////////

template <typename T, typename = void>
struct is_range : std::false_type {};

template <typename T>
struct is_range<T,
                      ::std::void_t<decltype(std::declval<T>().begin()),  // has a beginning
                                    decltype(std::declval<T>().end())>>   // ...and an end 
    : std::true_type {};

template <typename T>
constexpr bool is_range_v = is_range<T>::value;

///////////////////////////////////////////////////////////////////////////////

template <typename T, typename = void>
struct is_sized_range : std::false_type {};

template <typename T>
struct is_sized_range<T,
                      ::std::void_t<decltype(std::declval<T>().begin()),  // has a beginning
                                    decltype(std::declval<T>().end()),    // ...and an end
                                    decltype(std::declval<T>().size())>>  // ...and the size can be known
    : std::true_type {};

template <typename T>
constexpr bool is_sized_range_v = is_sized_range<T>::value;

///////////////////////////////////////////////////////////////////////////////

// This and_type and the all_types_are_the_same type are based on this SO post: https://stackoverflow.com/a/29603857
template <typename... Condition_Ts>
struct and_type : std::true_type {};

template <typename Condition_T, typename... Condition_Ts>
struct and_type<Condition_T, Condition_Ts...>
    : std::conditional<Condition_T::value, and_type<Condition_Ts...>, std::false_type>::type {};

template <typename T, typename... Ts>
struct all_types_are_the_same : public and_type<std::is_same<Ts, T>...> {
  using type = std::remove_reference_t<T>;
};

template <typename... Ts>
_WITE_CONCEPT all_types_are_the_same_v = all_types_are_the_same<Ts...>::value;

template <typename... Ts>
using common_type_t = typename all_types_are_the_same<Ts...>::type;

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::common

///////////////////////////////////////////////////////////////////////////////
