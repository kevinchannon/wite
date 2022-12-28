# TODO

## Core
* Make the interface of `result` match that of the upcoming `std::expected`
* UUIDs should be formattable in `std::format` and `std::ostream`. So, you should be able to do something like `std::format("My ID: {:X}\n", id)` to format a uuid in X-format. Alternatively, `std::cout << wite::uuid_format::X << id << std::endl;` should do roughly the same thing.

## IO
* Simple CSV input?

## String

## Collections
* Rename `stack_vector` -> `static_vector`
* `static_vector` should store values in `std::optional` to prevent requiring default constructor
  * `static_vector` will need a custom iterator to unwrap the optional
* `identifiable_item_collection` needs an iterator type. This is basically an iterator that iterates over the ordered container and dereferences the pointer in its `operator*`.

## Geometry
* Add `line` class (from AdventOfCode repo), as `direction` template
* Add point arithmetic:
  * point - point = direction
  * point + direction = point
  * direction + direction = direction
  * direction - direction = direction

## Maths

## Build things