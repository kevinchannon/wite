/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/env/environment.hpp>

#include <wite/maths/value_range.hpp>

#include <limits>

///////////////////////////////////////////////////////////////////////////////

namespace wite::maths {

///////////////////////////////////////////////////////////////////////////////

template <typename Value_T, range_boundary LBOUND = range_boundary::closed, range_boundary HBOUND = range_boundary::closed>
struct bounded_value {
  using value_type = Value_T;
  using bound_type = value_range<value_type, LBOUND, HBOUND>;

  constexpr bounded_value(Value_T value,
                          bound_type bounds = bound_type{std::numeric_limits<value_type>::min(),
                                                         std::numeric_limits<value_type>::max()})
      : value{std::move(value)}, bounds{std::move(bounds)} {}

  _WITE_NODISCARD constexpr bool is_in_bounds() const noexcept { return bounds.contains(value); }

  bounded_value& clamp() noexcept {
    value = bounds.clamp(value);
    return *this;
  }

  value_type value{};
  bound_type bounds{};
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::maths

///////////////////////////////////////////////////////////////////////////////
