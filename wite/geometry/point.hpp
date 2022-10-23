#pragma once

#include <array>
#include <cstdint>

#ifndef WITE_DEFAULT_POINT_TYPE
#define WITE_DEFAULT_POINT_TYPE double
#endif

namespace wite::geometry {

template <size_t DIMENSION_COUNT, typename Value_T = WITE_DEFAULT_POINT_TYPE>
class point {
  using _storage_type = std::array<Value_T, DIMENSION_COUNT>;

 public:

   using value_type = Value_T;
   using size_type = typename _storage_type::size_type;

  [[nodiscard]] constexpr size_type dimensions() const noexcept { return _value.size(); }

 private:
  _storage_type _value;
};

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_2d = point<2, Value_T>;

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_3d = point<3, Value_T>;

template <typename Value_T = WITE_DEFAULT_POINT_TYPE>
using point_4d = point<4, Value_T>;

}
