#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>

namespace wite {

template <typename Obj_T>
class index {
 public:
  using value_type      = size_t;
  using collection_type = Obj_T;

  constexpr explicit index(value_type idx) noexcept : _idx{idx} {}

  WITE_DEFAULT_CONSTRUCTORS(index);

  _WITE_NODISCARD constexpr value_type value() const noexcept { return _idx; }

 private:
  value_type _idx;
};

}  // namespace wite
