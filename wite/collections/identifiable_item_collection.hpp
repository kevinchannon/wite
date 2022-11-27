#pragma once

#include <wite/common/concepts.hpp>
#include <wite/core/index.hpp>
#include <wite/env/environment.hpp>

#include <map>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

namespace wite::collections {

template <typename T>
concept identifiable = requires(T& t) { t.id(); };

template <identifiable Item_T>
class identifiable_item_collection {
  using _item_map_type      = std::map<typename Item_T::id_type, std::unique_ptr<Item_T>>;
  using _ordered_items_type = std::vector<Item_T*>;

 public:
  using size_type  = typename _item_map_type::size_type;
  using value_type = Item_T;
  using id_type    = typename value_type::id_type;
  using index_type = index<identifiable_item_collection<value_type>>;

  _WITE_NODISCARD constexpr size_type size() const noexcept { return _items.size(); }
  _WITE_NODISCARD constexpr bool empty() const noexcept { return _items.empty(); }

  void insert(Item_T item) {
    auto p = std::make_unique<Item_T>(std::move(item));

    auto id                 = p->id();
    const auto item_pointer = p.get();

    const auto [_, newly_inserted] = _items.insert(typename _item_map_type::value_type{std::move(id), std::move(p)});
    if (newly_inserted) {
      _ordered_items.push_back(item_pointer);
    }
  }

  template <std::ranges::forward_range Range_T>
    requires std::is_same_v<value_type, typename std::decay_t<Range_T>::value_type>
  void insert(Range_T&& range) {
    for (const auto& value : range) {
      insert(value);
    }
  }

  const value_type& at(const id_type& id) const {
    if (const auto item = _items.find(id); _items.end() != item) {
      return *(item->second);
    }

    throw std::out_of_range{"identifiable_item_collection failed to retreive item"};
  }

  const value_type& at(const index_type& idx) const { return *_ordered_items[*idx]; }

 private:
  _item_map_type _items;
  _ordered_items_type _ordered_items;
};
}  // namespace wite::collections