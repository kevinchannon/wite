/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/core/id.hpp>
#include <wite/core/index.hpp>
#include <wite/core/uuid.hpp>

#include <wite/env/environment.hpp>

#include <iostream>

///////////////////////////////////////////////////////////////////////////////

template<wite::uuid_like Uuid_T>
std::ostream& operator<<(std::ostream& os, const Uuid_T& id) {
  os << wite::to_string(id);
  return os;
}

///////////////////////////////////////////////////////////////////////////////

template<wite::uuid_like Uuid_T>
std::wostream& operator<<(std::wostream& os, const Uuid_T& id) {
  os << wite::to_wstring(id);
  return os;
}

///////////////////////////////////////////////////////////////////////////////

template <wite::index_like Idx_T>
std::ostream& operator<<(std::ostream& os, const Idx_T& idx) {
  os << *idx;
  return os;
}

///////////////////////////////////////////////////////////////////////////////

template <wite::id_like Id_T>
std::ostream& operator<<(std::ostream& os, const Id_T& id) {
  os << *id;
  return os;
}

///////////////////////////////////////////////////////////////////////////////
