#pragma once

#include <wite/env/platform.hpp>

#undef _WITE_CONFIG_DEBUG

#ifdef _DEBUG
#define _WITE_CONFIG_DEBUG
#endif

#undef _WITE_CONFIG_WORD_SIZE_32
#undef _WITE_CONFIG_WORD_SIZE_64

#ifdef _WITE_PLATFORM_OS_WINDOWS
#ifdef _WIN64
#define _WITE_CONFIG_WORD_SIZE_64
#else
#define _WITE_CONFIG_WORD_SIZE_32
#endif  // _WIN64
#else
#define _WITE_CONFIG_WORD_SIZE_64
#endif

#ifndef _WITE_RELEASE_NOEXCEPT
#ifdef _WITE_CONFIG_DEBUG
#define _WITE_RELEASE_NOEXCEPT
#else
#define _WITE_RELEASE_NOEXCEPT noexcept
#endif  // _WITE_CONFIG_DEBUG
#endif  // _WITE_RELEASE_NOEXCEPT