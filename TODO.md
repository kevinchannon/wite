# TODO

## Core
* Make the interface of `result` match that of the upcoming `std::expected`

## IO
* Simple CSV input?

## String

## Collections
* Rename `stack_vector` -> `static_vector`
* `static_vector` should store values in `std::optional` to prevent requiring default constructor
  * `static_vector` will need a custom iterator to unwrap the optional

## Geometry
* Add `line` class (from AdventOfCode repo), as `direction` template
* Add point arithmetic:
  * point - point = direction
  * point + direction = point
  * direction + direction = direction
  * direction - direction = direction

## Maths

## Types
* Add `identifiable`
* Add `index`

## Build things