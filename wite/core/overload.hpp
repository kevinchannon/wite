/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

// These are taken from https://en.cppreference.com/w/cpp/utility/variant/visit

#pragma once

#include <wite/common/concepts.hpp>

namespace wite {

template <typename... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace wite
