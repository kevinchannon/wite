#pragma once

#include <optional>

///////////////////////////////////////////////////////////////////////////////

namespace wite {

///////////////////////////////////////////////////////////////////////////////

template <typename Fn_T>
class scope_exit {
 public:
  using function_type = Fn_T;

  scope_exit()                             = delete;
  scope_exit(const scope_exit&)            = delete;
  scope_exit& operator=(const scope_exit&) = delete;

  constexpr explicit scope_exit(Fn_T fn) : _fn{std::move(fn)} {}
  ~scope_exit() {
    if (_fn) {
      (*_fn)();
    }
  }

  void release() { _fn.reset(); }

 private:
  std::optional<function_type> _fn;
};

///////////////////////////////////////////////////////////////////////////////

}  // namespace wite

///////////////////////////////////////////////////////////////////////////////
