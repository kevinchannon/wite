// These are taken from https://en.cppreference.com/w/cpp/utility/variant/visit

#pragma once

namespace wite {

template <typename>
inline constexpr bool always_false_v = false;

template <typename... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

template <typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

}  // namespace wite
