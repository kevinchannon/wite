#pragma once

#include <wite/env/environment.hpp>

#include <vector>

namespace wite {

struct reserve {
  size_t value{};
};

template<typename T>
_WITE_NODISCARD std::vector<T> make_vector(reserve reserve_amount = reserve{}) {
  auto out = std::vector<T>{};
  out.reserve(reserve_amount.value);

  return out;
}

}  // namespace wite
