/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#undef _WITE_COMPILER_CLANG
#undef _WITE_COMPILER_GCC  
#undef _WITE_COMPILER_MSVC 

#if defined(__clang__)
#define _WITE_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#define _WITE_COMPILER_GCC
#elif defined(_MSC_VER)
#define _WITE_COMPILER_MSVC
#endif

#ifdef _WITE_COMPILER_MSVC
#define _WITE_NOINLINE __declspec(noinline)
#else
#define _WITE_NOINLINE __attribute__((noinline))
#endif
