#pragma once

#include <wite/env/environment.hpp>

#include <map>
#include <memory>
#include <utility>

namespace wite::collections {

template <typename T>
concept identifiable = requires(T& t) { t.id(); };

template <identifiable Item_T>
class identifiable_item_collection {
  using _item_map_type = std::map<typename Item_T::id_type, std::unique_ptr<Item_T>>;

 public:
  using size_type  = typename _item_map_type::size_type;
  using value_type = Item_T;
  using id_type    = typename value_type::id_type;

  _WITE_NODISCARD constexpr size_type size() const noexcept { return _items.size(); }
  _WITE_NODISCARD constexpr bool empty() const noexcept { return _items.empty(); }

  void insert(Item_T item) {
    auto p  = std::make_unique<Item_T>(std::move(item));
    auto id = p->id();
    _items.insert(typename _item_map_type::value_type{std::move(id), std::move(p)});
  }

  const value_type& at(const id_type& id) const {
    const auto item = _items.find(id);
    return *(item->second);
  }

 private:
  _item_map_type _items;
};
}  // namespace wite::collections