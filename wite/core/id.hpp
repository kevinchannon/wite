#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>

#ifndef WITE_DEFAULT_ID_TYPE
#include <wite/core/uuid.hpp>

#define WITE_DEFAULT_ID_TYPE uuid
#endif

#include <compare>
#include <utility>

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

template <typename Obj_T, typename Id_T=WITE_DEFAULT_ID_TYPE>
struct id {
  struct id_type_tag {
    static constexpr auto value = true;
  };

  using value_type = Id_T;

  constexpr explicit id(value_type id) noexcept : _id{std::move(id)} {}

  WITE_DEFAULT_CONSTRUCTORS(id);

  _WITE_NODISCARD auto operator<=>(const id&) const noexcept = default;

  constexpr const value_type& value() const noexcept { return _id; }
  constexpr const value_type& operator*() const noexcept { return _id; }

  value_type _id;
};

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_CONCEPTS
template <typename T>
concept id_like = requires(T& t) { T::id_type_tag::value; };
#endif

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
