#pragma once

#include <wite/env/environment.hpp>

#include <optional>

namespace wite::maths {

template <typename T>
struct value_range {
  using value_type = T;

  _WITE_NODISCARD constexpr bool operator==(const value_range& other) const noexcept {
    return min == other.min and max == other.max;
  }
  _WITE_NODISCARD constexpr bool operator!=(const value_range& other) const noexcept { return not(*this == other); }
  _WITE_NODISCARD constexpr bool operator<(const value_range& other) const noexcept { return max < other.min; }
  _WITE_NODISCARD constexpr bool operator>(const value_range& other) const noexcept { return min > other.max; }

  _WITE_NODISCARD constexpr value_type size() const noexcept { return max - min; }

  _WITE_NODISCARD constexpr std::optional<value_range> overlap(const value_range& other) const noexcept {
    if (min > other.max or max < other.min) {
      return {};
    }

    return value_range{std::max(min, other.min), std::min(max, other.max)};
  }

  value_type min;
  value_type max;
};

}  // namespace wite::maths
