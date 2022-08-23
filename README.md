# wite.string

A small collection of string functions.  The aim is that the most common use case is the simplest thing to do and just does what you want without too much fuss. It might not be the most efficient way to do it, but it should be good enough for 99% of use cases. If you want to do something a bit more fancy, then there might be a way to do that using this library, or there might not.

## Basic usage

All of these examples assume that you've done something like `namespace ws = wite::string;` somewhere.
### `join`
Take a collection of strings and make them into a single string, separated by a specified character

```
const auto strings = {"One", "small", "step", "for", "a", "man..."};

// "sentence" <- std::string{ "One small step for a man..." }
const auto sentence = ws::join(string);
```

### `split`
The opposite of `join`.

```
const auto sentence = "Some long string";

// "words" <- std::vector<std::string>{ "Some", "long", "string" }
const auto words = ws::split(sentence);
```

### `trim_left`, `trim_right`
Remove white-space from either the left, or right, side of a string

```
const auto messy = "   \t\n <-- Messy bit here, and here -> \t\t\r\v\n  ";

// "tidier" <- std::string{ "<-- Messy bit here, and here -> \t\t\r\v\n  " }
const auto tidier = ws::trim_left(messy);

// "tidy" <- std::string{ "<-- Messy bit here, and here ->" }
const auto tidy = ws::trim_right(tidier);
```

### `strip`
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