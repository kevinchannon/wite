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

template<typename Value_T>
using point_2d = point<Value_T, 2>;

template<typename Value_T>
using point_3d = point<Value_T, 3>;

template<typename Value_T>
using point_4d = point<Value_T, 4>;

}
