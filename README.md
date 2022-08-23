# wite.string

A small collection of string functions.

## `join`
Take a collection of strings and make them into a sngle string, separated by a specified character

```
const auto strings = {"One", "small", "step", "for", "a", "man..."};

// "sentence" <- std::string{ "One small step for a man..." }
const auto sentence = wite::string::join(string);
```

## `split`
The opposite of `join`.

```
const auto sentence = "Some long string";

// "words" <- std::vector<std::string>{ "Some", "long", "string" }
const auto words = wite::string::split(sentence);
```

## `trim_left`, `trim_right`
Remove white-space from either the left, or right, side of a string

```
const auto messy = "   \t\n <-- Messy bit here, and here -> \t\t\r\v\n  ";

// "tidier" <- std::string{ "<-- Messy bit here, and here -> \t\t\r\v\n  " }
const auto tidier = wite::string::trim_left(messy);

// "tidy" <- std::string{ "<-- Messy bit here, and here ->" }
const auto tidy = wite::string::trim_right(tidier);
```

## `strip`
Remove all white space from both ends of a string.

```
const auto messy = "   \t\n <-- Messy bit here, and here -> \t\t\r\v\n  ";

// "tidy" <- std::string{ "<-- Messy bit here, and here ->" }
const auto tidy = wite::string::strip(messy);
```