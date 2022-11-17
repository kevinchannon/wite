#pragma once

#include <wite/core/overload.hpp>
#include <wite/env/environment.hpp>

#include <vector>

namespace wite {

namespace arg {
  struct reserve {
    size_t value{};
  };

  struct size {
    size_t value{};
  };
}  // namespace arg

template <typename T, typename... Arg_Ts>
_WITE_NODISCARD std::vector<T> make_vector(Arg_Ts... args) {
  auto out = std::vector<T>{};

  (overloaded{[&out](arg::reserve arg) { out.reserve(arg.value); },
              [&out](arg::size arg) { out.resize(arg.value); },
              [](auto arg) { static_assert(always_false_v<decltype(arg)>, "Invalid make_vector arg type"); }}(
       std::forward<Arg_Ts>(args)),
   ...);

  return out;
}

}  // namespace wite
