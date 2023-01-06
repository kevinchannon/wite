#pragma once

#include <wite/env/platform.hpp>
#include <wite/env/compiler.hpp>

#undef _WITE_CONFIG_DEBUG

#if !defined(_WITE_COMPILER_GCC) 
#ifdef _DEBUG
#define _WITE_CONFIG_DEBUG
#endif
#else
#ifndef NDEBUG
#define _WITE_CONFIG_DEBUG
#endif
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

#ifndef _WITE_RELEASE_CONSTEXPR
#ifdef _WITE_CONFIG_DEBUG
#define _WITE_RELEASE_CONSTEXPR
#else
#define _WITE_RELEASE_CONSTEXPR constexpr
#endif  // _WITE_CONFIG_DEBUG
#endif  // _WITE_RELEASE_CONSTEXPR