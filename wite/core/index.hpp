#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/core/assert.hpp>
#include <wite/env/environment.hpp>

#include <compare>
#include <limits>
#include <type_traits>

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

template <typename Obj_T>
class index {
 public:
  struct index_type_tag {
    static constexpr auto value = true;
  };

  using value_type      = size_t;
  using collection_type = Obj_T;

  constexpr explicit index(value_type idx) noexcept : _idx{idx} {}

  WITE_DEFAULT_CONSTRUCTORS(index);

  _WITE_NODISCARD constexpr auto operator<=>(const index&) const noexcept = default;

  index& operator++() _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_idx < std::numeric_limits<value_type>::max(), "Index overflow");
    ++_idx;
    return *this;
  }

  index operator++(int) _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_idx < std::numeric_limits<value_type>::max(), "Index overflow");
    auto temp = *this;
    ++_idx;
    return temp;
  }

  index& operator--() _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_idx > 0, "Index underflow");
    --_idx;
    return *this;
  }

  index operator--(int) _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(_idx > 0, "Index underflow");
    auto temp = *this;
    --_idx;
    return temp;
  }

  index& operator+=(std::make_signed_t<value_type> offset) _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(offset > 0 ? _idx <= std::numeric_limits<value_type>::max() - offset : _idx >= value_type(-offset),
                       offset > 0 ? "Index overflow" : "Index underflow");
    _idx += offset;
    return *this;
  }

  index& operator-=(std::make_signed_t<value_type> offset) _WITE_RELEASE_NOEXCEPT {
    _WITE_DEBUG_ASSERT(
        offset < 0 ? _idx <= std::numeric_limits<value_type>::max() - value_type(-offset) : _idx >= value_type(offset),
        offset > 0 ? "Index underflow" : "Index overflow");
    _idx -= offset;
    return *this;
  }

  _WITE_NODISCARD index operator+(std::make_signed_t<value_type> offset) const noexcept { return index{_idx + offset}; }
  _WITE_NODISCARD index operator-(std::make_signed_t<value_type> offset) const noexcept { return index{_idx - offset}; }

  _WITE_NODISCARD constexpr value_type value() const noexcept { return _idx; }

 private:
  value_type _idx;
};

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <typename T>
concept index_like = requires(T& t) { T::index_type_tag::value; };
#endif

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
