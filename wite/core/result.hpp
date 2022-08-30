#pragma once

#include <variant>

namespace wite {

template <typename Value_T, typename Error_T>
class result : public std::variant<Value_T, Error_T> {
  using base_t = std::variant<Value_T, Error_T>;

 public:
  result(Value_T value) : base_t{value} {}
  result(Error_T error) : base_t(error) {}

  [[nodiscard]] bool ok() const noexcept { return this->index() == 0; }
  [[nodiscard]] bool is_error() const noexcept { return not ok(); }
  [[nodiscard]] const Value_T& value() const noexcept { return std::get<Value_T>(*this); }
};

}  // namespace wite
