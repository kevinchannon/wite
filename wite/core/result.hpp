/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#pragma once

#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>

#include <variant>
#include <functional>
#include <algorithm>

namespace wite {

template <typename Value_T, typename Error_T>
class result {
  using _storage_t = std::variant<Value_T, Error_T>;

 public:
  using value_type = Value_T;
  using error_type = Error_T;

  constexpr result(Value_T value) noexcept : _data{std::move(value)} {}
  constexpr result(Error_T error) noexcept : _data(std::move(error)) {}

  WITE_DEFAULT_CONSTRUCTORS(result);

  template<typename... Arg_Ts>
  constexpr value_type& emplace(Arg_Ts... args) noexcept {
    _data.template emplace<value_type>(std::forward<Arg_Ts>(args)...);
    return const_cast<value_type&>(value());
  }

  constexpr explicit operator bool() const noexcept { return ok(); }
  _WITE_NODISCARD constexpr bool has_value() const noexcept { return ok(); }

  _WITE_NODISCARD constexpr const value_type& operator*() const noexcept { return value(); }
  _WITE_NODISCARD constexpr value_type& operator*() noexcept { return const_cast<value_type&>(value()); }

  _WITE_NODISCARD constexpr const value_type* operator->() const noexcept { return &value(); }
  _WITE_NODISCARD constexpr value_type* operator->() noexcept { return const_cast<value_type*>(&value()); }

  _WITE_NODISCARD constexpr bool ok() const noexcept { return _data.index() == 0; }
  _WITE_NODISCARD constexpr bool is_error() const noexcept { return false == ok(); }
  _WITE_NODISCARD constexpr const Value_T& value() const noexcept { return std::get<Value_T>(_data); }
  _WITE_NODISCARD constexpr const Error_T& error() const noexcept { return std::get<Error_T>(_data); }

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

  template<typename Fn_T>
  _WITE_NODISCARD constexpr result<value_type, error_type> or_else(Fn_T&& fn) const {
    if(has_value()) {
      return *this;
    } else {
      return result<value_type, error_type>{std::invoke(std::forward<Fn_T>(fn), error())};
    }
  }

  template<typename Fn_T>
  _WITE_NODISCARD constexpr result<value_type, error_type>& transform_error(Fn_T&& fn) {
    *this = this->or_else(std::forward<Fn_T>(fn));
    return *this;
  }

  constexpr void swap(result<value_type, error_type>& other) noexcept {
    using std::swap;
    swap(_data, other._data);
  }

 private:
  _storage_t _data;
};

template<typename Value_T, typename Error_T>
void swap(result<Value_T, Error_T>& a, result<Value_T, Error_T>& b) noexcept {
  a.swap(b);
}

}  // namespace wite
