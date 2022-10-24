#pragma once

#include <wite/geometry/point.hpp>

#include <ostream>

template<size_t N_DIM, typename Value_T>
std::ostream& operator<<(std::ostream& os, const wite::geometry::point<N_DIM, Value_T>& p) {
  os << "[ " << p[0];

  for (auto i = 1u; i < p.dimensions(); ++i) {
    os << ", " << p[i];
  }

  os << " ]";

  return os;
}