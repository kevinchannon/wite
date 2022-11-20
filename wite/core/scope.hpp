#pragma once

#include <wite/env/environment.hpp>

#include <exception>
#include <functional>
#include <optional>

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

template <typename Fn_T>
class scope_exit {
 public:
  using function_type = Fn_T;

  scope_exit()                                 = delete;
  scope_exit(const scope_exit&)                = delete;
  scope_exit& operator=(const scope_exit&)     = delete;
  scope_exit(scope_exit&&) noexcept            = default;
  scope_exit& operator=(scope_exit&&) noexcept = default;

  constexpr explicit scope_exit(Fn_T fn) : _fn{std::move(fn)} {}
  ~scope_exit() {
    if (_fn) {
      std::invoke(*_fn);
    }
  }

  void release() { _fn.reset(); }

 private:
  std::optional<function_type> _fn;
};

///////////////////////////////////////////////////////////////////////////////

#if _WITE_HAS_UNCAUGHT_EXCEPTION

template <typename Fn_T>
class scope_success {
 public:
  using function_type = Fn_T;

  scope_success()                                    = delete;
  scope_success(const scope_success&)                = delete;
  scope_success& operator=(const scope_success&)     = delete;
  scope_success(scope_success&&) noexcept            = default;
  scope_success& operator=(scope_success&&) noexcept = default;

  constexpr explicit scope_success(Fn_T fn) : _fn{std::move(fn)} {}
  ~scope_success() {
    if (_fn and std::uncaught_exceptions() == 0) {
      std::invoke(*_fn);
    }
  }

  void release() { _fn.reset(); }

 private:
  std::optional<function_type> _fn;
};

///////////////////////////////////////////////////////////////////////////////

#ifndef WITE_NO_EXCEPTIONS

template <typename Fn_T>
class scope_fail {
 public:
  using function_type = Fn_T;

  scope_fail()                                       = delete;
  scope_fail(const scope_fail&)                      = delete;
  scope_fail& operator=(const scope_fail&)           = delete;
  scope_fail(scope_fail&&) noexcept                  = default;
  scope_fail& operator=(scope_fail&&) noexcept       = default;

  constexpr explicit scope_fail(Fn_T fn) : _fn{std::move(fn)} {}
  ~scope_fail() {
    if (_fn and std::uncaught_exceptions() > 0) {
      std::invoke(*_fn);
    }
  }

  void release() { _fn.reset(); }

 private:
  std::optional<function_type> _fn;
};

#endif

#endif

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
