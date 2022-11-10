# wite
Wite stands for "Why isn't this easy!?". It's a collection of routines and classes that aims to make easy things that should be easy in C++ but are not, for one reason or another. The aim is to make a small library of things that can just be dropped into a project to make things a little easier. So, if you don't want the weight of introducing a dependency on Boost, or something, then maybe there's something here to help you. At the moment, Wite is header only, so good times! Just plop the files into your source tree and rock on.

## Prerequisites
Wite doesn't have any dependencies, so you don't have to worry about that sort of thing. If you're developing new features for  Wite, then the tests have a dependency on [Catch2](https://github.com/catchorg/Catch2), but that is handled by CMake [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html), so it should take care of itself (as long as you're connected to the internet).

## Download
If you're planning to just use Wite in your project, then you don't really need to manually download it; it should be acquired by some kind of package management solution. If you do want to download the source, then you can do by going to the [Releases](https://github.com/kevinchannon/wite/releases) section and downloading the "wite-src.zip" file for the version of your choice.

## Installation
Wite is header only, so you can do something as simple as downloading the code and copying the "wite" directory into your source tree.  The preferred way to consume Wite is _via_ CMake's FetchContent.  You can find an example project that does this in the [wite-cmake-example](https://github.com/kevinchannon/wite-cmake-example) repository

## Macros
Some of the features of Wite are tunable by the user at compilation time. For a list of the macros that you can define as command-line parameters to the compiler, see the Compiler Macros section at the end of this readme.

# Collections
### `stack_vector`
```c++
#include <wite/collections/stack_vector.hpp>
```

This is a vector with a compile-time capacity, but a run-time size. It stores it's data on the stack, so don't put too many huge things in it.  Other than not being able to call `reserve` on it, the interface is pretty much like that of `std::vector`. For example, you can do something like:

```c++
auto v = wite::collections::stack_vector<int, 20>{};

v.push_back(1);

```


# IO
```c++
#include <wite/io/bytes_buffer.hpp>
```
A small collection of routines for doing IO to buffers and things like that. The type of "buffer" isn't defined, so you should be able to use vectors, arrays, pointers, whatever.  The main restriction is that a "buffer" is a thing of std::bytes.

## Buffer operations
To write to a buffer, then you can do:

```c++
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
```c++
const auto x = reader.read<io::big_endian<uint32_t>>();
```
or as an additional parameter if it's only known at runtime, for some reason.
```c++
const auto x = reader.read<uint32_t>(io::endian::big);
```

You can also write ranges of values (like vectors, arrays and lists):
```c++
const auto my_values = std::vector<double>{ 1.0, 2.0, 3.0 };
const auto bytes_written = writer.write(my_values);
```
Because we don't know how the allocator works for your chosen range, in general, the read interface for ranges is a little different. You call the `read_range` method and pass it a range to use for the output data. This needs to have the right size and type for the data you expect. It's moved in if it's an R-value, so you can do something like this:
```c++
const auto v = reader.read_range(std::vector<double>(3, 0.0));
```
This will read a vector from the buffer into `v`.

### Multiple values
Often, you will know what things you expect to read and write at compile time.  If so, then you can write/read a bunch of values in one operation, like this:
```c++
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
```c++
writer.write(uint64_t{v.size()}, v);
```

## Simple byte conversions
If you have a value and you want to get it as an array of `std::bytes`, then you can simply do:
```c++
// "bytes" <- wite::io::static_byte_buffer<sizeof(my_value)>
const auto bytes = wite::io::to_bytes(my_value);
```
of course, you can do the opposite too:
```c++
const auto bytes = std::array<io::byte, sizeof(uint32_t)>{
    io::byte{0x12}, io::byte{0x34}, io::byte{0x56}, io::byte{078}};

const auto i = wite::io::from_bytes<int>(bytes);
```

## Fancier usage
### Controlling edianness
Sometimes you want to write the bytes of a value with a particular endianness.  If you know the endianness that you're going to need at build time, then you can specify it using either of the two endian encoding adapters `wite::io::little_endian` or `wite::io::big_endian`. So, to write some `int` called `my_int` to a buffer as big endian, then:
```c++
wite::io::write(buffer, wite::io::big_endian{my_int});
```
You can also specify the endianness when reading:
```c++
const auto my_int = wite::io::read<wite::io::big_endian<int>>(buffer);
```

The endianness adapters also work in an equivalent way with the `to_bytes` and `from_bytes` functions.

If you only know the endianness at runtime, for some reason, then you can provide a final argument to `read` and `write` to specify the endianness:
```c++
const auto my_int = wite::io::read<int>(buffer, std::endian::little);
```
# Binascii
```c++
#include <wite/binascii/hexlify.hpp>
```

This component is basically two functions, `hexlify` and `unhexlify` that are based on the [Python functions of the same name](https://docs.python.org/3/library/binascii.html). `hexlify` takes a range containing byte values and returns a `std::string` with the hexadecimal values of the bytes in it:
```c++
const auto bytes = std::vector<std::byte>{ ... };

const auto str = binascii::hexlify(bytes);
```
If you're `bytes` vector is `[ 0x01, 0x02, 0xFE, 0xFF ]`, then the resulting `str` would be "0102FEFF". The input just has to be a range containing bytes, so it could be a `std::list`, or `std::array`, or whatever you have.

`unhexlify` does the opposite of `hexlify`. `unhexlify` takes a `std::string_view` and returns a `wite::io::dynamic_byte_buffer`, which is basically a `std::vector` of `wite::io::byte` values.

# String
A small collection of string functions.  The aim is that the most common use case is the simplest thing to do and just does what you want without too much fuss. It might not be the most efficient way to do it, but it should be good enough for 99% of use cases. If you want to do something a bit more fancy, then there might be a way to do that using this library, or there might not.

## Basic usage

All of these examples assume that you've done something like `namespace ws = wite::string;` somewhere.
### `join`
```c++
#include <wite/string/join.hpp>
```
Take a collection of strings and make them into a single string, separated by a specified character

```c++
const auto strings = {"One", "small", "step", "for", "a", "man..."};

// "sentence" <- std::string{ "One small step for a man..." }
const auto sentence = ws::join(string);
```

### `split`
```c++
#include <wite/string/slit.hpp>
```
The opposite of `join`.

```c++
const auto sentence = "Some long string";

// "words" <- std::vector<std::string>{ "Some", "long", "string" }
const auto words = ws::split(sentence);
```

### `trim_left`, `trim_right`
```c++
#include <wite/string/trim.hpp>
```
Remove white-space from either the left, or right, side of a string

```c++
const auto messy = "   \t\n <-- Messy bit here, and here -> \t\t\r\v\n  ";

// "tidier" <- std::string{ "<-- Messy bit here, and here -> \t\t\r\v\n  " }
const auto tidier = ws::trim_left(messy);

// "tidy" <- std::string{ "<-- Messy bit here, and here ->" }
const auto tidy = ws::trim_right(tidier);
```

### `strip`
```c++
#include <wite/string/strip.hpp>
```
Remove all white space from both ends of a string.

```c++
const auto messy = "   \t\n <-- Messy bit here, and here -> \t\t\r\v\n  ";

// "tidy" <- std::string{ "<-- Messy bit here, and here ->" }
const auto tidy = ws::strip(messy);
```

## Fancier usage
### `join`
Join also accepts a second argument, which is the character that the strings will be joined with:

```c++
const auto values = { "1.618", "2.718", "3.142" };

const auto csv_row = ws::join(values, ',');
```

### `split`
You can tell `split` a specific character that you want to split on:

```c++
const auto csv_row = { "1.618,2.718,3.142" };

const auto values = ws::split(csv_row, ',');
```

You can also tell it to ignore empty items:
```c++
const auto csv_row = { "1.618,,,,2.718,3.142" };

const auto values = ws::split(csv_row, ',', ws::split_behaviour::drop_empty);
```

#### `split_to`
If you really want to control the thing, you can call `split_to`, which allows you to specify how the output is returned.  So, if you know that the string that you're splitting is going to out-live the result of the split and it's not going to change, then you can do something like:
```c++
const auto long_lived_values = { "1.618,2.718,3.142" };

const auto views_of_pieces = ws::split_to<std::vector<std::string_view>>(long_lived_values, ',');
```
In this case, no additional memory allocation happens for the strings in the split result, the string_views are just pointers into the original strings.

# fragment_string

```c++
#include <wite/string/fragment_string.hpp>
```

`wite::basic_fragment_string<Char_T, FRAGMENT_COUNT>` is a part of the wite strings library.  It allows you to compose literal strings into something that apppears to be a single string. The interface is pretty much the same as the non-mutating parts of std::string. Some examples:

```c++
const auto fs_1 = fragment_string{"Hello"};

const auto fs_2 = fs_1 + ", world!";

// Prints "Hello, World!"
std::cout << fs_2 << std::endl;
```

You can use `wite::fragment_wstring` for wide chars.

There is also a user-defined string literal in the `wite::string_literals` namespace:

```c++
using wite::string_literals;

const auto fs = "first fragment"_fs;
const auto wfs = L"wide fragment string"_wfs;
```

# Geometry

```c++
#include <wite/geometry.hpp>
```

Some basic geometry things; points, lines, shapes, that sort of thing.

## point

```c++
#include <wite/geometry/point.hpp>
```

`point` is a template that represents a point in an N-dimensional space:
```c++
using namespace wite::geometry;

const auto p = point<2, int>{1, 2};

// Access the dimensions of the point by templated 'get', or a square-bracket operator.
const auto x = p.get<dim::x>();
const auto y = p[1];
```

Compare points:
```c++
const auto p = point<3, double>{1.5, 2.0, 2.5};

const auto q = point<3, double>{1.5, 2.0, 2.5};
const auto r = point<3, double>{1.5, 2.0, 2.4};
const auto s = point<3, double>{1.5, 2.1, 2.5};

std::cout << "p == q: " << std::boolalpha << p == q << std::endl;
std::cout << "p != r: " << std::boolalpha << p != r << std::endl;
std::cout << "p  < r: " << std::boolalpha << p  < r << std::endl;
std::cout << "p  < s: " << std::boolalpha << p  < s << std::endl;
```
Initializer-lists:
```c++
auto p = point<4, uint8_t>;

...

p = {0x02, 0x03, 0x04};
```

### Specializations
Some kinds of point are very common, so there are specializations for those:
```c++
point_2d<T> ==> point<2, T>
point_3d<T> ==> point<3, T>
point_4d<T> ==> point<4, T>
```
So, you can do thins like:
```c++
const auto p = point_2d{10, 100};
```
which is nice and concise. This is using class type-deduction to work out the type of the values in the point.

### IO
```c++
#include <wite/geometry/io.hpp>
```

If you want to output a point to a stream easily, then include `geometry/io.hpp`:
```c++
const auto p = point_2d{2.718, 3.142};

...

std::cout << "p = " << p << std::endl;
```
will print `[ 2.718, 3.142 ]`.

# Maths
```c++
#include <wite/maths.hpp>
```

A bunch of basic maths-y routines and classes

## `next_value`, `prev_value`
```c++
#include <wite/maths/numeric.hpp
```

Use these to get at the next representable value after (`next_value`) or before (`prev_value`) the specified value:
```c++
const auto next = wite::maths::next_value(1.23e45);
const auto prev = wite::maths::prev_value(1.23e45);
```
These functions will take floating-point and integer types.

## Variadic `min`, `max` and `minmax`

These take arbitrary numbers of values and give you the min or max (or both) of them. Kind of obvious really.
```c++
// min_val <-- -1.23
const auto min_val = wite::maths::min(1.0, 10.0, -1.23, 5.0, 1000.0);

// max_val <-- 45
const auto max_val = wite::maths::max(21, 32, 1, 0, -10, 45, 19);

// a <-- -10, b <-- 45
const auto [a, b] = wite::maths::minmax(21, 32, 1, 0, -10, 45, 19);
```
These functions work with anything that is [`std::totally_ordered`](https://en.cppreference.com/w/cpp/concepts/totally_ordered).  If the input values are not trivially copyable, then they will be passed in by reference-to-const and the result will also be a reference-to-const, so watch out for that. It's a bit weird, but it mirrors the behaviour of `std::min` and `std::max`.

## `value_range`

```c++
#include <with/maths/value_range.hpp>
```

A small class that expresses an interval, in the mathematical sense. You give it the min and max of the range in its constructor and the you can test where values are inside or ourside the range, etc.

```c++
const auto val_rng = maths::value_range{-10, 10};

const auto a = val_rng.min();   // -10
const auto b = val_rng.max();   //  10

const auto width = val_rng.size();  // 20

const auto mid_point = val_rng.mid();   // 0
```
In this example, we're relying on class template deduction to guess the type for the `value_range`. So, it's deduced as `int` in this case. We could has used `const auto val_rng = maths::value_range<int>{-10, 10}` if we wanted to be explicit. Similarly, `maths::value_range{0.0, std::numbers::pi}` would return a `double` range.

To test whether points are in or out of the range, you can use `contains`:
```c++
if (val_rng.contains(5)){
    std::cout << "5 is in range" << std::endl;
}

if (not val_rng.contains(11)) {
    std::cout << "11 is not in range" << std::endl;
}
```

You can get the overlap of a range with another range by using `overlap`:
```c++
// *common_rng <-- [-10, -2]
const auto common_rng = val_rng.overlap(maths::value_range{-100, -2});
```
`overlap` returns an optional, because ranges aren't guaranteed to overlap. So, if there's no overlap, then the result of `overlap` is `std::nullopt`:
```c++
// no_overlap <-- std::nullopt
const auto no_overlap = val_rng.overlap(maths::value_range{20, 40});
```

## Open and closed ranges

By default, `value_range`s are closed (i.e. the min and max values are defined as being inside the range). If you need an open range, then you can specify it as a template parameter:
```c++
// These three value_ranges will all compare equal.
// [0.0, 1.0]
const auto closed_closed_1 = maths::value_range{0.0, 1.0};
const auto closed_closed_2 = maths::closed_value_range{0.0, 1.0};
const auto closed_closed_3 = maths::value_range<double, maths::range::boundary::closed, maths::range::boundary::closed>{0.0, 1.0};

// These two value_ranges will both compare equal.
// (0.0, 1.0)
const auto open_open_1 = maths::open_value_range{0.0, 1.0};
const auto open_open_2 = maths::value_range<double, maths::range::boundary::open, maths::range::boundary::open>{0.0, 1.0};

// [0.0, 1.0)
const auto closed_open = maths::value_range<double, maths::range::boundary::closed, maths::range::boundary::open>{0.0, 1.0};

// (0.0, 1.0]
const auto open_closed = maths::value_range<double, maths::range::boundary::open, maths::range::boundary::closed>{0.0, 1.0};
```
Note: there are specific classes for the common "all-open" and "all-closed" cases, `maths::open_value_range` and `maths::closed_value_range`, respectively.

### IO

```c++
#include <wite/maths/io.hpp>
```

You can insert a `value_range` into a `std::ostream` as expected:
```c++
// "My range = [0.1,2.0]"
std::cout << "My range = " << maths::value_range{0.1, 2.0} << std::endl;

const auto rng = maths::value_range<double, maths::range::boundary::closed, maths::range::boundary::open>{0.1, 2.0};

// "My range = [0.1,2.0)"
std::cout << "My range = " << rng << std::endl;
```

### Free functions

If you have a bunch of values and you want to get their range, you can use `envelope` for that:
```c++
// val_rng <-- [-4.0, 11.2]
const auto val_rng = maths::envelope(0.0, 0.1, -1.2, 10.0, -4.0, 11.2);

const auto values = std::vector<int>{0, 1, 2, 3, 4, 5, 10};

// val_rng_from_collection <-- [0, 10]
const auto val_rng_from_collection = maths::envelope(values);
```
`envelope` also works with value_ranges as inputs. It will return a new `value_range` the min of the range mins and the max of range maxes.

`maths::min` and `maths::max` are overloaded to take `value_range`. So, if you have a bunch of `value_range`s, then you can find the min of the mins, or the max of the maxes. 


# Compiler Macros
You might want to do things a bit differently, so here are some build-time specializations that you can make.

## Global

### `WITE_NO_NODISCARD`
Many of the functions in Wite are marked with the `[[nodiscard]]` attribute, if the compiler supports it, because it makes code safer.  If your compiler does support `[[nodiscard]]` but you don't want it, for some reason, then compile with `WITE_NO_NODISCARD` defined.

### `WITE_NO_EXCEPTIONS`
In general, things that go wrong throw exceptions in Wite. However, if you don't want exceptions being thrown from Wite code in your project, then define this command line variable in compilation. It doesn't guarantee that no exceptions will be emitted, but it does guarantee that Wite won't throw any. If you define this value, some features of the library will not be available to you.

## IO 
### `WITE_USER_DEFINED_BYTE_TYPE_`
By default, and if the compiler supports it, Wite will use `std::byte` as its native byte type for IO. If your compiler doesn't support `std::byte`, then WIte will detect this at build time and use `unsigned char` as its native byte type. If you have a bunch of pre-existing code that uses some other type for bytes, then you can choose some other options at compile-time.
* `WITE_USER_DEFINED_BYTE_TYPE_CHAR` - sets the byte type to `char`
* `WITE_USER_DEFINED_BYTE_TYPE_U8` - sets the byte type to `std::uint8_t`
* `WITE_USER_DEFINED_BYTE_TYPE_I8` - sets the byte type to `std::int8_t`3

It is an error to specify more than one of these options. 

### `WITE_LITTLE_ENDIAN` and `WITE_BIG_ENDIAN`
By default, Wite will try to use the system's native endianness (if `std::endian` is available in your compiler). If `std::endian` is not available, then the default endianness will default to little-endian.  If you want to override what Wite considers to be the native system endianness, then you can specify `WITE_BIG_ENDIAN`, or `WITE_LITTLE_ENDIAN` to the compiler.

It is an error to specify both of these options at the same time.

## Geommetry

### `WITE_DEFAULT_POINT_TYPE`
If you're using `wite::geometry::point`, then the default default value type for points is `double`.  If you're concerned mainly with pixel positions, or something, and you want the default type for points to be some integer type, then you can use `WITE_DEFAULT_POINT_TYPE` to define the default type.