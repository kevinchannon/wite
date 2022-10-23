#pragma once

#include <wite/env/environment.hpp>

#ifdef _WITE_CONFIG_DEBUG
#ifdef _WITE_ASSERT_WITH_EXCEPTION

#define _WITE_RELEASE_NOEXCEPT

#if _WITE_FEATURE_USE_STD_FORMAT
#include <format>
#else
#include <string>
#endif
#include <stdexcept>

namespace wite {

struct assertion_error : public std::logic_error {
  assertion_error(const char* msg) : std::logic_error{msg} {}
};

}  // namespace wite

#if _WITE_FEATURE_USE_STD_FORMAT
#define _WITE_ASSERT_MESSAGE(msg) std::format("Assertion error: {}", msg).c_str()
#else
#define _WITE_ASSERT_MESSAGE(msg) (std::string("Assertion error: ") + msg).c_str()
#endif

#define _WITE_DEBUG_ASSERT(pred, msg)                            \
  do {                                                           \
    if (!(pred)) {                                               \
      throw ::wite::assertion_error{_WITE_ASSERT_MESSAGE(msg)};  \
    }                                                            \
  } while (0, 0)
#else
#include <cassert>
#define _WITE_DEBUG_ASSERT(pred, msg) assert(pred)

#undef _WITE_RELEASE_NOEXCEPT
#define _WITE_RELEASE_NOEXCEPT noexcept
#endif
#else
#define _WITE_DEBUG_ASSERT(pred, msg)

#undef _WITE_RELEASE_NOEXCEPT
#define _WITE_RELEASE_NOEXCEPT noexcept
#endif

#define _WITE_DEBUG_ASSERT_FALSE(pred, msg) _WITE_DEBUG_ASSERT(!(pred), msg)
