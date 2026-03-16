# Contributing

So you want to contribute to the project? Great! All contributions are welcome, no matter how small. Even if you are new to coding or just new to C++, you should feel free to contribute to the project.

# Terms Used

The standard terms defined in [RFC2119](https://datatracker.ietf.org/doc/html/rfc2119) are used in this document. For your convenience, a brief explanation of that RFC is repeated below for those unfamiliar with it.

1. **MUST**  The specification is an absolute requirement.
2. **MUST NOT***  The specification is an absolute prohibition.
3. **SHOULD** The specification is strongly recommended and should only be ignored if it is either more optimal or otherwise required for functionality.
4. **SHOULD NOT** The specification is strongly recommended against and should only be ignored if it is either more optimal or otherwise required for functionality.
5. **MAY**  The specification is optional.

# Styling

Code indentations MUST be 2 spaces.

A `switch` statement's `case` labels SHOULD use the same indentation as `switch`.

Statements and declarations with curly braces SHOULD place them on new lines.

Statements and declarations which are not immediately obvious MUST have at least a brief comment explaining what they do/are for.

Statements and declarations with curly braces SHOULD have at least one empty new line before the opening statement/brace and one empty new line after the closing brace, unless the next line is also the end of a statement or declaration.

Statement comments should go above or next to the statement, not on the line with the curly brace.

Local variable declarations SHOULD be at the start of a function's definition. Local variable declarations with expensive initilizers MAY be placed elsewhere.

Infinite loops SHOULD NOT be used where the exit condition is already being stored in a variable.

Infinite loops SHOULD use `do {} while (true)` instead of `while (true) {}`.

```cpp
// Comment explaining `someFunc()`
void someFunc(uint8_t someParam) // < Comments don't go here (or the next line) for functions/methods/classes/etc
{ // < Brace is on a new line
  uint8_t someVariable = someParam * 2; // < We declare this variable here...

  // ... < ...even if we don't use it here

  // We might want to explain what we are checking for, for example: Explain what a value of 0, 5, etc means.
  if (someVariable == 0) // A comment could go here instead
  {
    // ...
  }
  // You can put a comment here...
  else if (someVariable < 5) // ...or here if it's only 1 line...
  { // ...but do not put a comment here.
    // ...
  }
  else
  {
    switch (someVariable)
    {
    case 6:
      // ...
      break;

    case 7:
      // ...
      return;

    default:
      break;
    }

    // You might explain what this for loop is doing here...
    for (uint8_t i = 1; i < someVariable; ++i) // ...or here.
    {
      uint8_t someOffset = i * 2;

      // ...
    }
  }

  // This expensive initilizer is placed further down, just in case it isn't needed.
  some_class_t expensiveInitilizer(/* ... */);

  // Infinite loops use `do {} while (true)`. While it only saves one cycle on exit, the main benefit is it's typically much clearer where long statements end.
  do
  {
    // ...
  }
  while (true);
}

bool ready = false; // < `ready` is a global variable

void checkReady()
{
  while (!ready) // < using an infinite loop here would be bad
  {
    if (doStuff())
    {
      ready = true;
    }
  }
}
```

# Data Types

| Type       | Description                   | Minimum Value              | Maximum Value              | Min/Max Constants        | Format            |
| ---------- | ----------------------------- | -------------------------- | -------------------------- | ------------------------ | ----------------- |
|   `int8_t` | signed 8-bit integer          |                       -128 |                        127 | `INT8_MIN`, `INT8_MAX`   | `PRId8`           |
|  `uint8_t` | unsigned 8-bit integer        |                          0 |                        255 | `UINT8_MAX`              | `PRIu8`/`PRIX8`   |
|  `int16_t` | signed 16-bit integer         |                    -32,768 |                     32,767 | `INT16_MIN`, `INT16_MAX` | `PRId16`          |
| `uint16_t` | unsigned 16-bit integer       |                          0 |                     65,535 | `UINT16_MAX`             | `PRIu16`/`PRIX16` |
|  `int32_t` | signed 32-bit integer         |             -2,147,483,648 |              2,147,483,647 | `INT32_MIN`, `INT32_MAX` | `PRId32`          |
| `uint32_t` | unsigned 32-bit integer       |                          0 |              4,294,967,295 | `UINT32_MAX`             | `PRIu32`/`PRIX32` |
|  `int64_t` | signed 64-bit integer         | -9,223,372,036,854,775,808 |  9,223,372,036,854,775,807 | `INT64_MIN`, `INT64_MAX` | `PRId64`          |
| `uint64_t` | unsigned 64-bit integer       |                          0 | 18,446,744,073,709,551,615 | `UINT64_MAX`             | `PRIu64`/`PRIX64` |

Variables MUST NOT use `int`, `unsigned int`, `long`, etc. Use the appropriate fixed-width type from the above table instead.

Variables MUST NOT use `char`/`unsigned char` (or `byte`) for storage of non-printable character data -- use `uint8_t` instead.

## Project-Specific Types

* `crc32_t`
* `rgb_t`
* `bitset`

# Declarations

Global non-const variables SHOULD NOT be used unless needed. You need to balance the added RAM usage against the extra program size that avoiding them adds. Try it both ways to see which one appears less significant.

## Constant Variables
The keyword `const` MUST be after the type, even if it could be placed at the start.

***BAD***
```cpp
const uint8_t exampleVariable = 1;
const uint8_t * const exampleVariable = 1; // Confusing
```

***GOOD***
```cpp
uint8_t const * exampleVariable = 1;
uint8_t const * const exampleVariable = 1;
```

The reason for this is that `const` always applies to the type on the left unless it is before any types, in which case it would apply to the type on the right, but this could result in confusion, so we avoid it.

## Constant Expressions

A variable SHOULD use the keyword `constexpr` if the value of it can be calculated at compile-time.

The helper macro `__if_constexpr` MUST be used instead of `constexpr` in constant expression `if` statements.

```cpp
constexpr uint8_t const exampleVariable = 1;

void someFunc()
{
  if __if_constexpr (exampleVariable > 0) return;
}
```

## Constant Initializations

The helper macro `__constinit` MUST be used in place of `constinit` (to support older compilers).

```cpp
__constinit uint8_t exampleVariable = 1;
```

# Preprocessor

Preprocessor macros SHOULD NOT be used when a constant variable could be used instead.

***BAD***
```cpp
#define EXAMPLE_VARIABLE 1

void someFunc()
{
  uint8_t someVar = 1 + (1 * EXAMPLE_VARIABLE);
}
```

***GOOD***
```cpp
constexpr uint8_t const exampleVariable = 1;

void someFunc()
{
  uint8_t someVar = 1 + (1 * exampleVariable);
}
```
