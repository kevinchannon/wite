#pragma once

#include <wite/env/environment.hpp>

#include <variant>

namespace wite {

template <typename Value_T, typename Error_T>
class result : std::variant<Value_T, Error_T> {
  using _base_t = std::variant<Value_T, Error_T>;

 public:

  using value_type = Value_T;
  using error_type = Error_T;

  result(Value_T value) : _base_t{value} {}
  result(Error_T error) : _base_t(error) {}

  _WITE_NODISCARD bool ok() const noexcept { return this->index() == 0; }
  _WITE_NODISCARD bool is_error() const noexcept { return false == ok(); }
  _WITE_NODISCARD const Value_T& value() const noexcept { return std::get<Value_T>(*this); }
  _WITE_NODISCARD const Error_T& error() const noexcept { return std::get<Error_T>(*this); }
};

}  // namespace wite
