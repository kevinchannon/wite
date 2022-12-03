#pragma once

#include <wite/core/id.hpp>
#include <wite/core/index.hpp>
#include <wite/core/uuid.hpp>

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

#if _WITE_HAS_CONCEPTS
template<wite::uuid_like Uuid_T>
std::ostream& operator<<(std::ostream& os, const Uuid_T& id) {
#else
inline std::ostream& operator<<(std::ostream& os, const wite::uuid& id) {
#endif
  const auto s = wite::to_string(id); 
  os << s;
  return os;
}

///////////////////////////////////////////////////////////////////////////////
