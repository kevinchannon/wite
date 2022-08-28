# wite
Wite stands for "Why isn't this easy!?". It's a collection of routines and classes that aims to make easy things that should be easy in C++ but are not, for one reason or another. The aim is to make a small library of things that can just be dropped into a project to make things a little easier. So, if you don't want the weight of introducing a dependency on Boost, or something, then maybe there's something here to help you. At the moment, Wite is header only, so good times! Just plop the files into your source tree and rock on.
# Collections
### `stack_vector`
```
#include <wite/collections/stack_vector.hpp>
```

This is a vector with a compile-time capacity, but a run-time size. It stores it's data on the stack, so don't put too many huge things in it.  Other than not being able to call `reserve` on it, the interface is pretty much like that of `std::vector`. For example, you can do something like:

```
auto v = wite::collections::stack_vector<int, 20>{};

v.push_back(1);

```


# IO
```
#include <wite/io/bytes_buffer.hpp>
```
A small collection of routines for doing IO to buffers and things like that. The type of "buffer" isn't defined, so you should be able to use vectors, arrays, pointers, whatever.  The main restriction is that a "buffer" is a thing of std::bytes.  To write to a buffer, then you can do:

```
auto buffer_data = std::vector<std::byte>(100, std::byte{0x00});

{
  const auto d = double{3.142};
  const auto i32 = int32_t{-1234};
  const auto u16 = uint16_t{100};

  // Write something to the buffer
  auto writer = wite::io::buffer::bytes_write_buffer_view{my_buffer};

  wite::io::buffer::write(writer, d);
  wite::io::buffer::write(writer, i32);
  wite::io::buffer::write(writer, u16);
}

// Read things from the buffer
{
  auto reader = wite::io::buffer::byte_read_buffer_view{buffer_data};

  const auto d = wite::io::buffer::read<double>(reader);
  const auto i32 = wite::io::buffer::read<int32_t>(reader);
  const auto u16 = wite::io::buffer::read<uint16_t>(reader);
}
```
This example is using `byte_read_buffer_view` and `byte_write_buffer_view` to manipulate the bytes in the buffer. This does things like increment the read/write position and things like that.

Because it's not specified above, then the write operations are using the native platform endianness.  The endianness may be specified as a template parameter (if you know it at compile time), or as an additional parameter if it's only known at runtime, for some reason.

## Fancier usage
### Controlling edianness
Sometimes you want to write the bytes of a value with a particular endianness.  If you know the endianness that you're going to need at build time, then you can specify it as a template parameter. So, to write some `int` called `my_int` to a buffer then:
```
wite::io::buffer::write<std::endian::big>(buffer, my_int);
```
You can also specify the endianness when reading:
```
const auto my_int = wite::io::buffer::read<int, std::endian::big>(buffer);
```
If you only know the endianness at runtime, for some reason, then you can provide a final argument to `read` and `write` to specify the endianness:
```
const auto my_int = wite::io::buffer::read<int>(buffer, std::endian::little);
```

# String
A small collection of string functions.  The aim is that the most common use case is the simplest thing to do and just does what you want without too much fuss. It might not be the most efficient way to do it, but it should be good enough for 99% of use cases. If you want to do something a bit more fancy, then there might be a way to do that using this library, or there might not.

## Basic usage

All of these examples assume that you've done something like `namespace ws = wite::string;` somewhere.
### `join`
```
#include <wite/string/join.hpp>
```
Take a collection of strings and make them into a single string, separated by a specified character

```
const auto strings = {"One", "small", "step", "for", "a", "man..."};

// "sentence" <- std::string{ "One small step for a man..." }
const auto sentence = ws::join(string);
```

### `split`
```
#include <wite/string/slit.hpp>
```
The opposite of `join`.

```
const auto sentence = "Some long string";

// "words" <- std::vector<std::string>{ "Some", "long", "string" }
const auto words = ws::split(sentence);
```

### `trim_left`, `trim_right`
```
#include <wite/string/trim.hpp>
```
Remove white-space from either the left, or right, side of a string

```
const auto messy = "   \t\n <-- Messy bit here, and here -> \t\t\r\v\n  ";

// "tidier" <- std::string{ "<-- Messy bit here, and here -> \t\t\r\v\n  " }
const auto tidier = ws::trim_left(messy);

// "tidy" <- std::string{ "<-- Messy bit here, and here ->" }
const auto tidy = ws::trim_right(tidier);
```

### `strip`
```
#include <wite/string/strip.hpp>
```
Remove all white space from both ends of a string.

```
const auto messy = "   \t\n <-- Messy bit here, and here -> \t\t\r\v\n  ";

// "tidy" <- std::string{ "<-- Messy bit here, and here ->" }
const auto tidy = ws::strip(messy);
```

## Fancier usage
### `join`
Join also accepts a second argument, which is the character that the strings will be joined with:

```
const auto values = { "1.618", "2.718", "3.142" };

const auto csv_row = ws::join(values, ',');
```

### `split`
You can tell `split` a specific character that you want to split on:

```
const auto csv_row = { "1.618,2.718,3.142" };

const auto values = ws::split(csv_row, ',');
```

You can also tell it to ignore empty items:
```
const auto csv_row = { "1.618,,,,2.718,3.142" };

const auto values = ws::split(csv_row, ',', ws::split_behaviour::drop_empty);
```

#### `split_to`
If you really want to control the thing, you can call `split_to`, which allows you to specify how the output is returned.  So, if you know that the string that you're splitting is going to out-live the result of the split and it's not going to change, then you can do something like:
```
const auto long_lived_values = { "1.618,2.718,3.142" };

const auto views_of_pieces = ws::split_to<std::vector<std::string_view>>(long_lived_values, ',');
```
In this case, no additional memory allocation happens for the strings in the split result, the string_views are just pointers into the original strings.
