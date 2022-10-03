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
A small collection of routines for doing IO to buffers and things like that. The type of "buffer" isn't defined, so you should be able to use vectors, arrays, pointers, whatever.  The main restriction is that a "buffer" is a thing of std::bytes.

## Buffer operations
To write to a buffer, then you can do:

```
auto buffer_data = std::vector<io::byte>(100, io::byte{0x00});

{
  const auto d = double{3.142};
  const auto i32 = int32_t{-1234};
  const auto u16 = uint16_t{100};

  // Write something to the buffer
  auto writer = io::bytes_write_buffer_view{buffer_data};

  writer.write(d);
  writer.write(i32);
  writer.write(u16);
}

// Read things from the buffer
{
  auto reader = io::byte_read_buffer_view{buffer_data};

  const auto d = reader.read<double>();
  const auto i32 = reader.read<int32_t>();
  const auto u16 = reader.read<uint16_t>();
}
```
This example is using `byte_read_buffer_view` and `byte_write_buffer_view` to manipulate the bytes in the buffer. This does things like increment the read/write position and things like that.

Because it's not specified above, then the write operations are using the native platform endianness.  The endianness may be specified using an "encoding adapter" (if you know it at compile time): 
```
const auto x = reader.read<io::big_endian<uint32_t>>();
```
or as an additional parameter if it's only known at runtime, for some reason.
```
const auto x = reader.read<uint32_t>(io::endian::big);
```

You can also write ranges of values (like vectors, arrays and lists):
```
const auto my_values = std::vector<double>{ 1.0, 2.0, 3.0 };
const auto bytes_written = writer.write(my_values);
```
Because we don't know how the allocator works for your chosen range, in general, the read interface for ranges is a little different. You call the `read_range` method and pass it a range to use for the output data. This needs to have the right size and type for the data you expect. It's moved in if it's an R-value, so you can do something like this:
```
const auto v = reader.read(std::vector<double>(3, 0.0));
```
This will read a vector from the buffer into `v`.

### Multiple values
Often, you will know what things you expect to read and write at compile time.  If so, then you can write/read a bunch of values in one operation, like this:
```
const auto a = uint32_t{0x12345678};
const auto b = uint16_t{0xABCD};
const auto c = true;
const auto d = uint32_t{0xFEDCBA98};

const auto bytes_written = writer.write(a, io::big_endian{b}, c, d);

...

const auto [ w, x, y, z ] =
    reader.read<uint32_t, io::big_endian<uint16_t>, bool, uint32_t>();
```
The advantage of this approach is that the various checks on the buffer capacity are only done once at the start, rather than for each value. Notice that you can use encoding adapters in the arguments. It's also possible to write ranges in this way too. So, you might serialise the size of a vector ahead of the vector itself. So, for a vector, `v`:
```
writer.write(uint64_t{v.size()}, v);
```

## Simple byte conversions
If you have a value and you want to get it as an array of `std::bytes`, then you can simply do:
```
// "bytes" <- wite::io::static_byte_buffer<sizeof(my_value)>
const auto bytes = wite::io::to_bytes(my_value);
```
of course, you can do the opposite too:
```
const auto bytes = std::array<io::byte, sizeof(uint32_t)>{
    io::byte{0x12}, io::byte{0x34}, io::byte{0x56}, io::byte{078}};

const auto i = wite::io::from_bytes<int>(bytes);
```

## Fancier usage
### Controlling edianness
Sometimes you want to write the bytes of a value with a particular endianness.  If you know the endianness that you're going to need at build time, then you can specify it using either of the two endian encoding adapters `wite::io::little_endian` or `wite::io::big_endian`. So, to write some `int` called `my_int` to a buffer as big endian, then:
```
wite::io::write(buffer, wite::io::big_endian{my_int});
```
You can also specify the endianness when reading:
```
const auto my_int = wite::io::read<wite::io::big_endian<int>>(buffer);
```

The endianness adapters also work in an equivalent way with the `to_bytes` and `from_bytes` functions.

If you only know the endianness at runtime, for some reason, then you can provide a final argument to `read` and `write` to specify the endianness:
```
const auto my_int = wite::io::read<int>(buffer, std::endian::little);
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
