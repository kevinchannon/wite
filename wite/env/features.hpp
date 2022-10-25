#pragma once

#include <bit>

#if __cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
#define _WITE_CPP20
#elif __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#define _WITE_CPP17
#elif __cplusplus >= 201402L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)
#define _WITE_CPP14
#endif

#if __cpp_concepts >= 201907
#define _WITE_HAS_CONCEPTS 1
#define _WITE_CONCEPT      concept
#else
#define _WITE_HAS_CONCEPTS 0
#define _WITE_CONCEPT      constexpr bool
#endif

#ifdef WITE_NO_NODISCARD
#define _WITE_NODISCARD
#else
#ifndef _WITE_FEATURE_USE_NODISCARD
#ifndef __has_cpp_attribute
#define _WITE_FEATURE_USE_NODISCARD 0
#define _WITE_NODISCARD
#elif __has_cpp_attribute(nodiscard) >= 201603L
#define _WITE_FEATURE_USE_NODISCARD 1
#define _WITE_NODISCARD             [[nodiscard]]
#else
#define _WITE_FEATURE_USE_NODISCARD 0
#endif
#endif  // _WITE_FEATURE_USE_NODISCARD
#endif  // WITE_NO_NODISCARD

#ifndef _WITE_FEATURE_USE_STD_FORMAT
#ifdef __cpp_lib_format
#define _WITE_FEATURE_USE_STD_FORMAT 1
#else
#define _WITE_FEATURE_USE_STD_FORMAT 0
#endif  // __cpp_lib_format
#endif // _WITE_FEATURE_USE_STD_FORMAT

#ifndef _WITE_FEATURE_USE_NORETURN
#ifndef __has_cpp_attribute
#define _WITE_FEATURE_USE_NORETURN 0
#define _WITE_NORETURN
#elif __has_cpp_attribute(noreturn) >= 200809L
#define _WITE_FEATURE_USE_NORETURN 1
#define _WITE_NORETURN             [[noreturn]]
#else
#define _WITE_FEATURE_USE_NORETURN 0
#endif
#endif  // _WITE_FEATURE_USE_NORETURN

#ifndef _WITE_FEATURE_USE_CONSTEVAL
#ifdef __cpp_consteval
#define _WITE_FEATURE_USE_CONSTEVAL 1
#define _WITE_CONSTEVAL             consteval
#else
#define _WITE_FEATURE_USE_CONSTEVAL 0
#define _WITE_CONSTEVAL             constexpr
#endif  // __cpp_consteval
#endif  // _WITE_FEATURE_USE_CONSTEVAL

#ifdef WITE_USER_DEFINED_BYTE_TYPE_CHAR
#ifdef WITE_USER_DEFINED_BYTE_TYPE_U8 || WITE_USER_DEFINED_BYTE_TYPE_I8
#error "It is an error to specify multiple byte types"
#endif
#define WITE_BYTE char
#elif WITE_USER_DEFINED_BYTE_TYPE_U8
#ifdef WITE_USER_DEFINED_BYTE_TYPE_CHAR || WITE_USER_DEFINED_BYTE_TYPE_I8
#error "It is an error to specify multiple byte types"
#endif
#define WITE_BYTE std::uint8_t
#elif WITE_USER_DEFINED_BYTE_TYPE_I8
#ifdef WITE_USER_DEFINED_BYTE_TYPE_U8 || WITE_USER_DEFINED_BYTE_TYPE_CHAR
#error "It is an error to specify multiple byte types"
#endif
#define WITE_BYTE std::int8_t
#else
#ifndef _WITE_FEATURE_USE_STD_BYTE
#ifdef __cpp_lib_byte
#define _WITE_FEATURE_USE_STD_BYTE 1
#define WITE_BYTE                 std::byte
#else
#define _WITE_FEATURE_USE_STD_BYTE 0
#define WITE_BYTE                 unsigned char
#endif  // __cpp_lib_byte
#endif  // _WITE_FEATURE_USE_STD_BYTE
#endif  // WITE_USER_DEFINED_BYTE_TYPE

#ifndef _WITE_FEATURE_HAS_THREE_WAY_COMPARE
#if __cpp_impl_three_way_comparison >= 201711
#define _WITE_FEATURE_HAS_THREE_WAY_COMPARE 1
#else
#define _WITE_FEATURE_HAS_THREE_WAY_COMPARE 0
#endif  // __cpp_impl_three_way_comparison >= 201711
#endif  // _WITE_FEATURE_HAS_THREE_WAY_COMPARE

#if __cpp_lib_ranges >= 201911
#define _WITE_HAS_RANGES 1
#else
#define _WITE_HAS_RANGES 0
#endif

#if __cpp_lib_endian >= 201907
#define _WITE_HAS_STD_ENDIAN 1
#else
#define _WITE_HAS_STD_ENDIAN 0
#endif
