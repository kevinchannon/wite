#pragma once

#include <wite/collections/make_vector.hpp>
#include <wite/common/concepts.hpp>
#include <wite/core/id.hpp>
#include <wite/core/index.hpp>
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

template <identifiable Item_T,
          typename AssocContainer_T = std::map<typename Item_T::id_type, std::unique_ptr<Item_T>>,
          typename OrderedCont_T    = std::vector<Item_T*>>
class identifiable_item_collection {
  using _associative_storage_type      = AssocContainer_T;
  using _ordered_storage_type = OrderedCont_T;

 public:
  using size_type  = typename _associative_storage_type::size_type;
  using value_type = Item_T;
  using id_type    = typename value_type::id_type;
  using index_type = index<identifiable_item_collection<value_type>>;

  constexpr identifiable_item_collection(std::initializer_list<Item_T> items) {
    insert(items);
  }

  constexpr identifiable_item_collection() = default;
  identifiable_item_collection(identifiable_item_collection&&) noexcept = default;
  identifiable_item_collection& operator=(identifiable_item_collection&&) noexcept = default;

  _WITE_NODISCARD constexpr size_type size() const noexcept { return _items.size(); }
  _WITE_NODISCARD constexpr bool empty() const noexcept { return _items.empty(); }

  constexpr void insert(Item_T item) {
    if (this->contains(item.id())) {
      throw std::logic_error{"Insertion of duplicate ID into identifiable_item_collection"};
    }

    _unchecked_insert(std::make_unique<Item_T>(std::move(item)));
  }

  template <std::ranges::forward_range Range_T>
    requires std::is_same_v<value_type, typename std::decay_t<Range_T>::value_type>
  constexpr void insert(Range_T&& values) {
    std::ranges::for_each(
        std::forward<Range_T>(values), [this](auto&& val) { this->try_insert(val); });
  }

  template <identifiable... Item_Ts>
  constexpr void insert(Item_Ts&&... items) {
    (...,
     [this](value_type item) { insert(std::move(item)); }(std::forward<Item_Ts>(items)));
  }

  constexpr bool try_insert(Item_T item) {
    auto p = std::make_unique<Item_T>(std::move(item));

    const auto [_, newly_inserted] = _try_unchecked_insert(std::move(p));
    return newly_inserted;
  }

  template <std::ranges::forward_range Range_T>
    requires std::is_same_v<value_type, typename std::decay_t<Range_T>::value_type>
  constexpr std::vector<bool> try_insert(Range_T&& values) {
    auto out = std::vector<bool>{};
    if constexpr (common::is_sized_range_v<Range_T>) {
      out.reserve(values.size());
    }

    std::ranges::transform(
        std::forward<Range_T>(values), std::back_inserter(out), [this](auto&& val) { return this->try_insert(val); });

    return out;
  }

  template <identifiable... Item_Ts>
  constexpr std::array<bool, sizeof...(Item_Ts)> try_insert(Item_Ts&&... items) {
    auto out     = std::array<bool, sizeof...(Item_Ts)>{};
    auto out_idx = size_t{0};

    (...,
     [this, &out, &out_idx](value_type item) { out[out_idx++] = try_insert(std::move(item)); }(std::forward<Item_Ts>(items)));

    return out;
  }

  constexpr bool erase(const id_type& id) {
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
  constexpr std::vector<bool> erase(Range_T&& ids) {
    auto out = std::vector<bool>{};
    if constexpr (common::is_sized_range_v<Range_T>) {
      out.reserve(ids.size());
    }

    std::ranges::transform(std::forward<Range_T>(ids), std::back_inserter(out), [this](auto&& id) { return this->erase(id); });

    return out;
  }

  template <id_like... Id_Ts>
  constexpr std::array<bool, sizeof...(Id_Ts)> erase(const Id_Ts&... ids) {
    auto out     = std::array<bool, sizeof...(Id_Ts)>{};
    auto out_idx = size_t{0};

    (..., [this, &out, &out_idx](const id_type& id) { out[out_idx++] = erase(id); }(ids));

    return out;
  }

  template <typename... Arg_Ts>
  constexpr value_type& emplace(Arg_Ts&&... args) {
    auto p                         = std::make_unique<value_type>(std::forward<Arg_Ts>(args)...);
    auto [out, inserted_new_value] = _try_unchecked_insert(std::move(p));
    if (not inserted_new_value) {
      throw std::logic_error{"identifiable_item_collection already contains this ID"};
    }

    return *(out->second);
  }

  std::unique_ptr<value_type> excise(const id_type& id) {
    auto id_and_item = _items.find(id);
    if (_items.end() == id_and_item) {
      return nullptr;
    }

    auto out = std::unique_ptr<value_type>{};
    std::swap(out, id_and_item->second);
    _items.erase(id_and_item);

    const auto to_erase = std::find(_ordered_items.begin(), _ordered_items.end(), out.get());
    _WITE_DEBUG_ASSERT(_ordered_items.end() != to_erase, "Failed to find item in orderer items that was found in associative items");

    _ordered_items.erase(to_erase);
    
    return out;
  }

  constexpr const value_type& at(const id_type& id) const {
    if (const auto item = _items.find(id); _items.end() != item) {
      return *(item->second);
    }

    throw std::out_of_range{"identifiable_item_collection failed to retreive item by ID"};
  }

  constexpr const value_type& at(const index_type& idx) const {
    if (*idx >= _ordered_items.size()) {
      throw std::out_of_range{"identifiable_item_collection failed to retreive item by index"};
    }

    return *_ordered_items[*idx];
  }

  constexpr void clear() {
    _items.clear();
    _ordered_items.clear();
  }

  _WITE_NODISCARD constexpr bool contains(const id_type& id) const noexcept { return _items.contains(id); }

 private:
  constexpr auto _try_unchecked_insert(std::unique_ptr<value_type> value) {
    auto id           = value->id();
    auto item_pointer = value.get();
    const auto out    = _items.insert(typename _associative_storage_type::value_type{std::move(id), std::move(value)});
    if (out.second) {
      _ordered_items.push_back(item_pointer);
    }

    _WITE_DEBUG_ASSERT(_items.size() == _ordered_items.size(), "item container size mismatch");
    return out;
  }

  constexpr void _unchecked_insert(std::unique_ptr<value_type> value) {
    auto id           = value->id();
    auto item_pointer = value.get();
    _items.insert(typename _associative_storage_type::value_type{std::move(id), std::move(value)});
    _ordered_items.push_back(item_pointer);

    _WITE_DEBUG_ASSERT(_items.size() == _ordered_items.size(), "item container size mismatch");
  }

  _associative_storage_type _items;
  _ordered_storage_type _ordered_items;
};
}  // namespace wite::collections
