#pragma once

#include <wite/core/id.hpp>
#include <wite/core/index.hpp>
#include <wite/core/uuid.hpp>

#include <wite/env/environment.hpp>

#include <iomanip>
#include <iostream>

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

inline std::ostream& operator<<(std::ostream& os, const wite::uuid& guid) {
  os << "{" << std::hex << std::uppercase << std::setw(8) << std::setfill('0') << guid.data_1 << "-" << std::setw(4)
     << guid.data_2 << "-" << guid.data_3 << "-" << std::setw(2);
  for (auto i = 0u; i < 8; ++i) {
    os << static_cast<uint32_t>(guid.data_4[i]);
  }

  os << "}";

  return os;
}

///////////////////////////////////////////////////////////////////////////////
