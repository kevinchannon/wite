#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>

#include <variant>
#include <functional>

namespace wite {

template <typename Value_T, typename Error_T>
class result : std::variant<Value_T, Error_T> {
  using _base_t = std::variant<Value_T, Error_T>;

 public:
  using value_type = Value_T;
  using error_type = Error_T;

  constexpr result(Value_T value) noexcept : _base_t{std::move(value)} {}
  constexpr result(Error_T error) noexcept : _base_t(std::move(error)) {}

  WITE_DEFAULT_CONSTRUCTORS(result);

  constexpr explicit operator bool() const noexcept { return ok(); }
  _WITE_NODISCARD constexpr bool has_value() const noexcept { return ok(); }

  _WITE_NODISCARD constexpr const value_type& operator*() const noexcept { return value(); }
  _WITE_NODISCARD constexpr value_type& operator*() noexcept { return const_cast<value_type&>(value()); }

  _WITE_NODISCARD constexpr const value_type* operator->() const noexcept { return &value(); }
  _WITE_NODISCARD constexpr value_type* operator->() noexcept { return const_cast<value_type*>(&value()); }

  _WITE_NODISCARD constexpr bool ok() const noexcept { return this->index() == 0; }
  _WITE_NODISCARD constexpr bool is_error() const noexcept { return false == ok(); }
  _WITE_NODISCARD constexpr const Value_T& value() const noexcept { return std::get<Value_T>(*this); }
  _WITE_NODISCARD constexpr const Error_T& error() const noexcept { return std::get<Error_T>(*this); }

  template <typename DefaultValue_T>
  _WITE_NODISCARD constexpr value_type value_or(DefaultValue_T&& default_value) const {
    if (has_value()) {
      return value();
    } else {
      return default_value;
    }
  }

  template<typename Fn_T>
  _WITE_NODISCARD constexpr result<value_type, error_type> and_then(Fn_T&& fn) const {
    if(has_value()) {
      return result<value_type, error_type>{std::invoke(std::forward<Fn_T>(fn), value())};
    } else {
      return *this;
    }
  }

  template<typename Fn_T>
  _WITE_NODISCARD constexpr result<value_type, error_type>& transform(Fn_T&& fn) {
    *this = this->and_then(std::forward<Fn_T>(fn));
    return *this;
  }
};

}  // namespace wite
