/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/env/environment.hpp>

#ifdef _WITE_CONFIG_DEBUG
#ifdef _WITE_ASSERT_WITH_EXCEPTION

#define _WITE_RELEASE_NOEXCEPT
#define _WITW_RELEASE_CONSTEXPR

#if _WITE_FEATURE_USE_STD_FORMAT
#include <format>
#else  // ^^^ _WITE_FEATURE_USE_STD_FORMAT / vvv ! _WITE_FEATURE_USE_STD_FORMAT
#include <string>
#endif  // _WITE_FEATURE_USE_STD_FORMAT
#include <stdexcept>

namespace wite {

struct assertion_error : public std::logic_error {
  explicit assertion_error(const char* msg) : std::logic_error{msg} {}
};

inline void _throw_assertion_error(const std::string& msg) noexcept(false) {
  throw assertion_error{msg.c_str()};
}
}  // namespace wite

#if _WITE_FEATURE_USE_STD_FORMAT
#define _WITE_ASSERT_MESSAGE(msg) std::format("Assertion error: {}", msg).c_str()
#else  // ^^^ _WITE_FEATURE_USE_STD_FORMAT / vvv ! _WITE_FEATURE_USE_STD_FORMAT
#define _WITE_ASSERT_MESSAGE(msg) (std::string("Assertion error: ") + msg).c_str()
#endif  // _WITE_FEATURE_USE_STD_FORMAT

#define _WITE_DEBUG_ASSERT(pred, msg)                            \
  do {                                                           \
    if (!(pred)) {                                               \
      ::wite::_throw_assertion_error(_WITE_ASSERT_MESSAGE(msg)); \
    }                                                            \
  } while (0)

#else  // ^^^ _WITE_ASSERT_WITH_EXCEPTION / vvv ! _WITE_ASSERT_WITH_EXCEPTION

#include <cassert>
#define _WITE_DEBUG_ASSERT(pred, msg) assert(pred)

#undef _WITE_RELEASE_NOEXCEPT
#define _WITE_RELEASE_NOEXCEPT noexcept

#undef _WITW_RELEASE_CONSTEXPR
#define _WITW_RELEASE_CONSTEXPR constexpr

#endif  // _WITE_ASSERT_WITH_EXCEPTION

#else  // ^^^ _WITE_CONFIG_DEBUG / vvv ! _WITE_CONFIG_DEBUG

#define _WITE_DEBUG_ASSERT(pred, msg)

#undef _WITE_RELEASE_NOEXCEPT
#define _WITE_RELEASE_NOEXCEPT noexcept

#undef _WITW_RELEASE_CONSTEXPR
#define _WITW_RELEASE_CONSTEXPR constexpr

#endif  // _WITE_CONFIG_DEBUG

#define _WITE_DEBUG_ASSERT_FALSE(pred, msg) _WITE_DEBUG_ASSERT(!(pred), msg)
