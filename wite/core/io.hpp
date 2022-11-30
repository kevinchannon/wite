#pragma once

#include <wite/core/id.hpp>
#include <wite/core/index.hpp>
#include <wite/core/guid.hpp>

#include <wite/env/environment.hpp>

#include <iostream>
#include <iomanip>

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
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
#endif

///////////////////////////////////////////////////////////////////////////////

inline std::ostream& operator<<(std::ostream& os, const wite::GUID& guid) {
  os << "{" << std::hex << std::uppercase << guid.Data1 << "-" << guid.Data2 << "-" << guid.Data3;
  for (auto i = 0u; i < 8; ++i) {
    os << guid.Data4[i];
  }

  os << "}";

  return os;
}

///////////////////////////////////////////////////////////////////////////////
