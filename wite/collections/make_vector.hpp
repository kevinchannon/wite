#pragma once

#include <wite/core/overload.hpp>
#include <wite/env/environment.hpp>

#include <vector>

namespace wite {

struct reserve {
  size_t value{};
};

struct size {
  size_t value{};
};

template <typename T, typename... Param_Ts>
_WITE_NODISCARD std::vector<T> make_vector(Param_Ts... params) {
  auto out = std::vector<T>{};

  (overloaded{[&out](reserve param) { out.reserve(param.value); },
              [&out](size param) { out.resize(param.value); },
              [](auto param) { static_assert(always_false_v<decltype(param)>, "Invalid make_vector param type"); }}(
       std::forward<Param_Ts>(params)),
   ...);

  return out;
}

}  // namespace wite
