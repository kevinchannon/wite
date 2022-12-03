#pragma once

#include <wite/core/id.hpp>
#include <wite/core/index.hpp>

#include <wite/env/environment.hpp>

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
