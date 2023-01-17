/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

/// This macro implements the constructors "rule-of-five".
#define WITE_DEFAULT_CONSTRUCTORS(class_name)             \
  class_name()                                 = default; \
  class_name(const class_name&)                = default; \
  class_name(class_name&&) noexcept            = default; \
  class_name& operator=(const class_name&)     = default; \
  class_name& operator=(class_name&&) noexcept = default

#define WITE_MOVABLE_ONLY(class_name)                     \
  class_name()                                 = delete;  \
  class_name(const class_name&)                = delete;  \
  class_name(class_name&&) noexcept            = delete;  \
  class_name& operator=(const class_name&)     = default; \
  class_name& operator=(class_name&&) noexcept = default
