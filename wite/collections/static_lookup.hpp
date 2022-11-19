#pragma once

#include <wite/common/concepts.hpp>
#include <wite/env/environment.hpp>

#include <algorithm>
#include <array>
#include <cstring>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>

///////////////////////////////////////////////////////////////////////////////

namespace wite::collections {

///////////////////////////////////////////////////////////////////////////////

template <typename K, typename V, typename... Ts>
constexpr bool is_mapping_type_v = std::is_same_v<common::common_type_t<Ts...>, std::pair<K, V>>;

template <typename Key_T, typename Value_T, size_t ITEM_COUNT>
class static_lookup {
  enum class direction { forward, reverse };

  using _this_type = static_lookup<Key_T, Value_T, ITEM_COUNT>;

 public:
  using value_type   = Value_T;
  using key_type     = Key_T;
  using mapping_type = std::pair<key_type, value_type>;
  using size_type    = size_t;

  template <typename... Mapping_Ts>
    requires(is_mapping_type_v<key_type, value_type, Mapping_Ts...> && sizeof...(Mapping_Ts) == ITEM_COUNT)
  constexpr static_lookup(Mapping_Ts... mappings) noexcept {
    _init(mappings...);
  }

  _WITE_NODISCARD constexpr size_type size() const noexcept { return _items.size(); }
  _WITE_NODISCARD constexpr const value_type& at(const key_type& k) const {
    return get<key_type, value_type, direction::forward>(k);
  }
  _WITE_NODISCARD value_type& at(const key_type& k) { return get<key_type, value_type, direction::forward>(k); }
  _WITE_NODISCARD constexpr const key_type& with(const value_type& v) const {
    return get<value_type, key_type, direction::reverse>(v);
  }

 private:
  template <typename... Mapping_Ts>
  constexpr void _init(mapping_type element, Mapping_Ts... other_elements) noexcept {
    constexpr auto idx = ITEM_COUNT - sizeof...(Mapping_Ts) - 1;
    _items[idx]        = element;

    if constexpr (sizeof...(Mapping_Ts) > 0) {
      _init(other_elements...);
    }
  }

  template <typename Target_T, typename Result_T, direction DIRECTION>
  _WITE_NODISCARD constexpr const Result_T& get(const Target_T& t) const {
    const auto it = std::ranges::find_if(_items, [&t](const auto& x) { return std::get<static_cast<int>(DIRECTION)>(x) == t; });
    if (_items.cend() == it) {
      throw std::out_of_range("Invalid item");
    }

    return std::get<1 - static_cast<int>(DIRECTION)>(*it);
  }

  template <typename Target_T, typename Result_T, direction DIRECTION>
  _WITE_NODISCARD constexpr Result_T& get(const Target_T& t) {
    return const_cast<Result_T&>(const_cast<const _this_type*>(this)->get<Target_T, Result_T, DIRECTION>(t));
  }

  std::array<mapping_type, ITEM_COUNT> _items;
};

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CTAD
template <typename... Ts>
static_lookup(Ts... mappings) -> static_lookup<typename common::common_type_t<Ts...>::first_type,
                                               typename common::common_type_t<Ts...>::second_type,
                                               sizeof...(Ts)>;
#endif

///////////////////////////////////////////////////////////////////////////////

template <typename T, typename... Ts>
  requires(std::is_same_v<T, common::common_type_t<Ts...>>)
_WITE_NODISCARD constexpr static_lookup<typename T::first_type, typename T::second_type, 1 + sizeof...(Ts)> make_static_lookup(
    T first_mapping,
    Ts... other_mappings) noexcept {
  return static_lookup<typename T::first_type, typename T::second_type, 1 + sizeof...(Ts)>(first_mapping, other_mappings...);
}

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite::collections

///////////////////////////////////////////////////////////////////////////////
