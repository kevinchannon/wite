/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include "wite/core/uuid/basic_uuid.hpp"

#include <functional>

///////////////////////////////////////////////////////////////////////////////

template <>
struct std::hash<wite::basic_uuid> {
  std::size_t operator()(const wite::basic_uuid& id) const noexcept {
    return std::hash<uint64_t>{}(*reinterpret_cast<const uint64_t*>(id.data.data())) ^
           std::hash<uint64_t>{}(*reinterpret_cast<const uint64_t*>(id.data.data() + sizeof(uint64_t)));
  }
};

///////////////////////////////////////////////////////////////////////////////
