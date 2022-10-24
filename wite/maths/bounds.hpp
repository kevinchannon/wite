#pragma once

namespace wite::maths {

template<typename T>
struct bounds {
  using value_type = T;

  value_type min;
  value_type max;
};

}
