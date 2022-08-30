#pragma once

#include <variant>

namespace wite {

template<typename Value_T, typename Error_T>
class result : public std::variant<Value_T, Error_T> {
  using base_t = std::variant<Value_T, Error_T>;

 public:
  result(Value_T value) : base_t{value} {}
  result(Error_T error) : base_t(error) {}

  [[nodiscard]] bool ok() const noexcept { return this->index() == 0; }
};

}
