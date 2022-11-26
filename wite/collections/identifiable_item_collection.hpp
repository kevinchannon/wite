#pragma once

#include <wite/env/environment.hpp>

namespace wite::collections {

template <typename T>
concept identifiable = requires(T& t) { t.id(); };

template <identifiable Item_T>
class identifiable_item_collection {
 public:
   using size_type = size_t;

   _WITE_NODISCARD constexpr size_type size() const noexcept { return 0; }
};
}  // namespace wite::collections