# TODO

## Core
* UUIDs should be formattable in `std::format` and `std::ostream`. So, you should be able to do something like `std::format("My ID: {:X}\n", id)` to format a `uuid` in X-format. Alternatively, `std::cout << wite::uuid_format::X << id << std::endl;` should do roughly the same thing.
* UUIDs should be natively serialisable and deserialisable via wite::io things
* Constructors of `result` should be conditionally explicit (i.e. in the same way as `std::expected`)
* If __cpp_lib_expected >= 202202L, then `wite::result` should be a typedef of `std::expected`?

## IO
* Simple CSV input?

## String

## Collections
* `identifiable_item_collection` needs an iterator type. This is basically an iterator that iterates over the ordered container and dereferences the pointer in its `operator*`.
* `identifiable_item_collection` should have `generate` method that inserts an object with a new ID and returns a reference to it (with accompanying `try_generate` version). Should be a variadic template that allows the user to pass in the constructor args for the item. Will need some way for the user to tell the collection what the generator for new object IDs is. Either a template argument, or pass a thing into the constructor, or some kind of "set_id_generator" method, or something?

## Geometry
* Add `line` class (from AdventOfCode repo), as `direction` template
* Add point arithmetic:
  * point - point = direction
  * point + direction = point
  * direction + direction = direction
  * direction - direction = direction

## Maths

## Build things