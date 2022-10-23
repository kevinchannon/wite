#pragma once

#include <array>

namespace wite::geometry {

template<typename Value_T, size_t DIMENSION_COUNT>
class point {
  using _storage_type = std::array<Value_T, DIMENSION_COUNT>;

 public:

   using value_type = Value_T;
   using size_type = typename _storage_type::size_type;

  [[nodiscard]] constexpr size_type dimensions() const noexcept { return _value.size(); }

 private:
  _storage_type _value;
};

}
