#pragma once

#include <wite/collections/make_vector.hpp>
#include <wite/common/concepts.hpp>
#include <wite/core/index.hpp>
#include <wite/core/id.hpp>
#include <wite/env/environment.hpp>

#include <algorithm>
#include <array>
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

  bool insert(Item_T item) {
    auto p = std::make_unique<Item_T>(std::move(item));

    auto id                 = p->id();
    const auto item_pointer = p.get();

    const auto [_, newly_inserted] = _items.insert(typename _item_map_type::value_type{std::move(id), std::move(p)});
    if (newly_inserted) {
      _ordered_items.push_back(item_pointer);
    }

    _WITE_DEBUG_ASSERT(_items.size() == _ordered_items.size(), "item container size mismatch");
    return newly_inserted;
  }

  template <std::ranges::forward_range Range_T>
    requires std::is_same_v<value_type, typename std::decay_t<Range_T>::value_type>
  std::vector<bool> insert(Range_T&& values) {
    auto out = std::vector<bool>{};
    if constexpr (common::is_sized_range_v<Range_T>) {
      out.reserve(values.size());
    }

    std::ranges::transform(
        std::forward<Range_T>(values), std::back_inserter(out), [this](auto&& val) { return this->insert(val); });

    return out;
  }

  template <identifiable... Item_Ts>
  std::array<bool, sizeof...(Item_Ts)> insert(Item_Ts&&... items) {
    auto out     = std::array<bool, sizeof...(Item_Ts)>{};
    auto out_idx = size_t{0};

    (..., [this, &out, &out_idx](value_type item) { out[out_idx++] = insert(std::move(item)); }(std::forward<Item_Ts>(items)));

    return out;
  }

  bool erase(const id_type& id) {
    const auto to_erase =
        std::find_if(_ordered_items.begin(), _ordered_items.end(), [&id](auto&& item) { return id == item->id(); });
    if (_ordered_items.end() == to_erase) {
      return false;
    }

    _ordered_items.erase(to_erase);
    _items.erase(id);

    return true;
  }

  template <std::ranges::forward_range Range_T>
    requires std::is_same_v<id_type, typename std::decay_t<Range_T>::value_type>
  std::vector<bool> erase(Range_T&& ids) {
    auto out = std::vector<bool>{};
    if constexpr (common::is_sized_range_v<Range_T>) {
      out.reserve(ids.size());
    }

    std::ranges::transform(std::forward<Range_T>(ids), std::back_inserter(out), [this](auto&& id) { return this->erase(id); });

    return out;
  }

  template <id_like... Id_Ts>
  std::array<bool, sizeof...(Id_Ts)> erase(const Id_Ts&... ids) {
    auto out     = std::array<bool, sizeof...(Id_Ts)>{};
    auto out_idx = size_t{0};

    (..., [this, &out, &out_idx](const id_type& id) { out[out_idx++] = erase(id); }(ids));

    return out;
  }

  template<typename... Arg_Ts>
  value_type& emplace(Arg_Ts&&... args) {
    auto p = std::make_unique<value_type>(std::forward<Arg_Ts>(args)...);
    const auto id = p->id();
    auto [out, inserted_new_item] = _items.insert(typename _item_map_type::value_type{std::move(id), std::move(p)});
    if (not inserted_new_item) {
      throw std::logic_error{"identifiable_item_collection already contains this ID"};
    }

    return *(out->second);
  }

  const value_type& at(const id_type& id) const {
    if (const auto item = _items.find(id); _items.end() != item) {
      return *(item->second);
    }

    throw std::out_of_range{"identifiable_item_collection failed to retreive item by ID"};
  }

  const value_type& at(const index_type& idx) const {
    if (*idx >= _ordered_items.size()) {
      throw std::out_of_range{"identifiable_item_collection failed to retreive item by index"};
    }

    return *_ordered_items[*idx];
  }

  void clear() {
    _items.clear();
    _ordered_items.clear();
  }

  _WITE_NODISCARD bool contains(const id_type& id) const noexcept { return _items.contains(id); }

 private:
  _item_map_type _items;
  _ordered_items_type _ordered_items;
};
}  // namespace wite::collections
