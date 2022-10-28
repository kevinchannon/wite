#pragma once

#include <wite/maths/value_range.hpp>

#include <ostream>

template <typename Value_T, wite::maths::range_boundary LBOUND, wite::maths::range_boundary HBOUND>
std::ostream& operator<<(std::ostream& os, const wite::maths::value_range<Value_T, LBOUND, HBOUND>& rng) {
  if constexpr (wite::maths::range_boundary::closed == LBOUND) {
    os << "[ ";
  }
  else {
    os << "( ";
  }

  os << rng.min() << ", " << rng.max();

  if constexpr (wite::maths::range_boundary::closed == HBOUND) {
    os << " ]";
  } else {
    os << " )";
  }

  return os;
}

template <typename Value_T, wite::maths::range_boundary LBOUND, wite::maths::range_boundary HBOUND>
std::wostream& operator<<(std::wostream& os, const wite::maths::value_range<Value_T, LBOUND, HBOUND>& rng) {
  if constexpr (wite::maths::range_boundary::closed == LBOUND) {
    os << L"[ ";
  } else {
    os << L"( ";
  }

  os << rng.min() << L", " << rng.max();

  if constexpr (wite::maths::range_boundary::closed == HBOUND) {
    os << L" ]";
  } else {
    os << L" )";
  }

  return os;
}