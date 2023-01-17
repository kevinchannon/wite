/*
* SPDX-License-Identifier: MIT
*
* Copyright (c) 2023 Kevin J Channon
*
*/

#include <wite/maths/value_range.hpp>
#include <wite/maths/io.hpp>

#include <iostream>

///////////////////////////////////////////////////////////////////////////////

namespace wm = wite::maths;

///////////////////////////////////////////////////////////////////////////////

void simple_example() {

  std::cout << "=======================================================" << std::endl;
  std::cout << "Simple value range example" << std::endl;
  std::cout << "=======================================================" << std::endl;

  auto r = wm::value_range{0, 10};

  std::cout << "Created value range " << r << std::endl;
  std::cout << "\tRange min: " << r.min() << std::endl;
  std::cout << "\tRange max: " << r.max() << std::endl;
  std::cout << "\tRange width: " << r.width() << std::endl;
}

///////////////////////////////////////////////////////////////////////////////

/// <summary>
///  A simple class that uses a value_range to define a transition zone between two values.
/// </summary>
class BlendingModel
{
 public:
  using TransitionZone_t = wm::value_range<double, wm::range_boundary::closed, wm::range_boundary::open>;

  constexpr BlendingModel(double value_1, double value_2, TransitionZone_t transition_zone)
      : _value_1{value_1}, _value_2{value_2}, _transition_zone{transition_zone} {}

  double get(double position) const noexcept {
    return _blend(_transition_zone.clamp(position));
  }

  constexpr const TransitionZone_t& extent() const noexcept { return _transition_zone; };

private:
  double _blend(double position) const noexcept{
   return _value_1 * (1 - _transition_zone.fraction(position)) + _value_2 * _transition_zone.fraction(position);
  }

  double _value_1{};
  double _value_2{};
  TransitionZone_t _transition_zone{};
};

///////////////////////////////////////////////////////////////////////////////

void more_complex_example() {
  std::cout << "=======================================================" << std::endl;
  std::cout << "Blending model value range example" << std::endl;
  std::cout << "=======================================================" << std::endl;

  constexpr auto model       = BlendingModel{123.45, 678.9, {-5.0, 5.0}};
  constexpr auto model_width = model.extent().size();

  for (auto x = model.extent().min() - model_width; x < model.extent().max() + model_width; x += 3 * model_width / 30) {
   std::cout << x << "\t" << model.get(x) << std::endl;
  }
}

///////////////////////////////////////////////////////////////////////////////

int main() {

  simple_example();

  std::cout << std::endl;

  more_complex_example();

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
