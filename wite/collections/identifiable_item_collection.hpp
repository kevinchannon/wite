#pragma once

#include <wite/env/environment.hpp>

#include <map>
#include <utility>
#include <memory>

namespace wite::collections {

template <typename T>
concept identifiable = requires(T& t) { t.id(); };

template <identifiable Item_T>
class identifiable_item_collection {
  using _item_map_type = std::map<typename Item_T::id_type, std::unique_ptr<Item_T>>;

 public:
  using size_type  = size_t;
  using value_type = Item_T;

  _WITE_NODISCARD constexpr size_type size() const noexcept { return 0; }
  _WITE_NODISCARD constexpr bool empty() const noexcept { return 0 == size(); }

//  std::pair<value_type&, bool> insert(Item_T&& item) {
//    auto p = std::unique_ptr<Item_T>(new Item_T{item});
//    auto id = p->id();
//    auto [iter, inserted] = _item_m.insert(
//        std::pair<typename _item_map_type::value_type::first_type, typename _item_map_type::value_type::second_type>{std::move(id), std::move(p)});
//    return std::pair<value_type&, bool>{iter->second, inserted};
//  }

 private:
  _item_map_type _item_m;
};
}  // namespace wite::collections