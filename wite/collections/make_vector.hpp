/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/core/overload.hpp>
#include <wite/env/environment.hpp>

#include <vector>

namespace wite {

namespace arg {
  struct reserve {
    size_t value{};
  };

  template<typename T>
  struct size {
    size_t value{};
    T initialise_to{};
  };

  template<typename T>
  size(size_t, T) -> size<T>;
}  // namespace arg

template <typename T, typename... Arg_Ts>
_WITE_NODISCARD std::vector<T> make_vector(Arg_Ts... args) {
  auto out = std::vector<T>{};

  (overloaded{[&out](arg::reserve arg) { out.reserve(arg.value); },
              [&out](arg::size<T> arg) { out.resize(arg.value, arg.initialise_to); },
              [](auto arg) { static_assert(common::always_false_v<decltype(arg)>, "Invalid make_vector arg type"); }}(
       std::forward<Arg_Ts>(args)),
   ...);

  return out;
}

}  // namespace wite
