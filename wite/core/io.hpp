#pragma once

#include <wite/core/index.hpp>

#include <iostream>

template<wite::index_like Idx_T>
std::ostream& operator<<(std::ostream& os, const Idx_T& idx) {
  os << idx.value();

  return os;
}
