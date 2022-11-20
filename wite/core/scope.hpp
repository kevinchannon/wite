#pragma once

#include <wite/common/concepts.hpp>
#include <wite/common/constructor_macros.hpp>
#include <wite/env/environment.hpp>

#include <exception>
#include <functional>
#include <optional>

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

namespace scope::detail {

  ///////////////////////////////////////////////////////////////////////////////

  enum class _exit_condition { all, success, fail };

  ///////////////////////////////////////////////////////////////////////////////

  template <_exit_condition CONDITION>
  constexpr bool _chech_exit_condition() {
    if constexpr (_exit_condition::all == CONDITION) {
      return true;
    }
#ifndef WITE_NO_EXCEPTIONS
#if _WITE_HAS_UNCAUGHT_EXCEPTION
    else if constexpr (_exit_condition::success == CONDITION) {
      return std::uncaught_exceptions() == 0;
    }
    else if constexpr (_exit_condition::fail == CONDITION) {
      return std::uncaught_exceptions() > 0;
    }
#endif  // _WITE_HAS_UNCAUGHT_EXCEPTION
#endif  // WITE_NO_EXCEPTIONS
    else {
      static_assert(_exit_condition::all == CONDITION, "Invalid exit condition");
    }
  }

  ///////////////////////////////////////////////////////////////////////////////

  template <typename Fn_T, _exit_condition EXIT_CONDITION>
  class _basic_scope_exit_runner {
   public:
    using function_type = Fn_T;

    WITE_MOVABLE_ONLY(_basic_scope_exit_runner);

    constexpr explicit _basic_scope_exit_runner(Fn_T fn) : _fn{std::move(fn)} {}
    ~_basic_scope_exit_runner() {
      if (_fn and _chech_exit_condition<EXIT_CONDITION>()) {
        std::invoke(*_fn);
      }
    }

    void release() { _fn.reset(); }

   private:
    std::optional<function_type> _fn;
  };

  ///////////////////////////////////////////////////////////////////////////////

}  // namespace scope::detail

///////////////////////////////////////////////////////////////////////////////

#define _WITE_DEFINE_SCOPE_EXIT_RUNNER(name, condition)                                                              \
  template <typename Fn_T>                                                                                           \
  class name : public scope::detail::_basic_scope_exit_runner<Fn_T, scope::detail::_exit_condition::condition> {     \
   public:                                                                                                           \
    explicit name(Fn_T fn)                                                                                           \
        : scope::detail::_basic_scope_exit_runner<Fn_T, scope::detail::_exit_condition::condition>{std::move(fn)} {} \
    WITE_MOVABLE_ONLY(name);                                                                                         \
  }

_WITE_DEFINE_SCOPE_EXIT_RUNNER(scope_exit, all);

#ifndef WITE_NO_EXCEPTIONS
#if _WITE_HAS_UNCAUGHT_EXCEPTION

_WITE_DEFINE_SCOPE_EXIT_RUNNER(scope_success, success);
_WITE_DEFINE_SCOPE_EXIT_RUNNER(scope_fail, fail);

#endif  // _WITE_HAS_UNCAUGHT_EXCEPTION
#endif  // WITE_NO_EXCEPTIONS

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
