# Style guide

## Background
C++ code in the Sigma project should use a consistent and well documented style
guide. Where possible, this should be enacted and enforced with tooling to avoid
toil both for authors of C++ code in the Sigma project and for code reviewers.

> :warning: **While we do suggest that you use the mentioned techniques the Sigma source code does not entirely abide by them yet.**

## General naming rules
-   Compile time constants and macros should use `UPPER_CASE` formatting.
-   Variable naming should always use `snake_case`
    -   Private or protected member variables should use the `m_` prefix.
    -   Global variables should use the `g_` prefix.
    -   Static variables should use the `s_` prefix.
    -   Local variables do not have a prefix and variables where multiple of these apply should use the last one from the list above which applies.
-   Classes and structs should always use `snake_case`.
-   Methods and functions should always use `snake_case`.
-   Template parameters should always use `snake_case`.
-   Namespaces should always use `snake_case`.

## File names
-   All file names should use `snake_case` (exceptions do however exist, ie. some markdown files).
-   C++ source files should use the `.cpp` file extension.
-   C++ header files should use the `.h` file extensions.

## Syntax and formatting
-   Always use trailing return type syntax for functions and methods.
-   Place the pointers `*` and references `&` after the relevant type (ie. `int*`, `my_type<int>&`).
-   Place `const` at the left-most side of the declaration.
-   Declare at most one variable at a time.
-   Namespace closures should always provide a comment specifying which namespace is ending.
-   Macro block closures should always provide a comment specifying which block is ending.
-   Your code should never exceed 100 characters/line, if this happens, its better to separate the statement into multiple sub statements, or space the entire statement into multiple lines.
-   Use the `using`-based type alias syntax instead of `typedef`.
-   Don't use `using` to support unqualified lookup on `std` types.
-   Always use braces for conditional, `switch`, and loop statements, even when the body is a single statement, the opening brace should always be on the same line as the closing parenthesis.

## Comments
-   Only use line comments (with `//`, not `/* ... */`).
    -   The only exceptions are documentation comments, where Javadoc should be used.
-   Comments should always be on a line by themselves. 
-   Comments should be descriptive and conscise. 
-   Short comments (non-documentation) should not be capitalized and shouldn't have a trailing dot character.
    -   Short comments may also provide a prefix tag, such as `NOTE:`, `TODO:`, or similar, these should always be capitalized and separated from the actual comment using the `:` character. 
-   Comments related to documentation should be properly capitalized and terminated.

## Other
-   Do not add other third-party library dependencies to any code that might conceivably be used as part of the compiler or runtime.
