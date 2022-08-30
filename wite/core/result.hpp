#pragma once

#include <variant>

namespace wite {

template<typename Happy_T, typename Error_T>
class result : public std::variant<Happy_T, Error_T> {
  using base_t = std::variant<Happy_T, Error_T>;

 public:
  result(Happy_T happy) : base_t{happy} {}
  result(Error_T error) : base_t(error) {}

  [[nodiscard]] bool ok() const noexcept { return this->index() == 0; }
};

}
