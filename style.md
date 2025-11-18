# C++ Style Guide

## Files
- Header files.  File names end in `h`.  If the file contains a class definition, it should contain only one principle class definition, but may contain closely-related structure type or class definitions.  The name of the file is the name of the principal type defined therein, which should be in UpperCamelCase:  `class Person` is defined in `Person.h`.  If a header file contains other global data, not a type definition, its name should be simple and in lower case: `constants.h` for symbolic constant definitions.  All such files should be placed in the top-level directory `header`.
- Implementation (source code) files.  Every type definition header file should hav ea corresponding source code file, with the same name, also in UpperCamelCase, but the `.cpp` file name extension.  All functions should be defined out of line, and the out-of-line definitions should be placed in the appropriate source code files.  All `.cpp` source code files should be placed in the `src` top-level directory. `main.cpp` contains the `main` function and belongs in the `src` directory.
- Makefile.  A project-level makefile should appear as the top level and should be named `makefile`.  Subdirectories may contain makefiles temporarily for development purposes, but they must eventually be removed.

## Order Within a File
The following sections must be separated from each other by single vertical spaces.
- `#pragma once` declaration (header only).
- `#include` directives for system (C++ library) headers, in alphabetical order. Include only the headers that the compiler needs for the symbols in the current file!
- `#include` directives for project headers, in alphabetical order.
- Symbolic constants defined through the `#define` directive.  These should be avoided, except (perhaps) for those used for conditional compilation.  Symbolic constants, in general, should be global ``variables'' qualifed as `const` and defined in `header/constants.h`. Include only the necessary headers!
- Type definitions. Accessory type definitions (such as structure type definitions of types used in class defiinitions) should come first.
  - Within a class definition, the `public` section comes before the `private` section. The `public` section should contain public (API) functions only, with constructors and destructor coming first:
    - Default constructor (if any)
    - Non-default constructors (if any)
    - Copy constructor (if any)
    - Destructor (if any)
    - Other API functions (if any).
  - Enumeration types, wherever possible, should be associated with specific classes and should be defined within their respective class definitions.  Typically, an enumeration type belongs in the `public` section.
  - The `private` section should list functions above fields.
  - Structure types should not use access specifiers at all — they should only be used for cases where everything is `public` (by default), and should be used for types that serve primarily as mere data containers.
- Function definitions. Overloaded functions should occur in sequence, with overloads using smaller (or empty) parameter lists coming first.

## Symbol Names
- Custom-defined types (class, structure, union, and enumeration types) are named in UpperCamelCase: `Person`, `PublicSector`.
- Functions (other than constructors and destructors), global and local variables (including function parameters), and local constants (but not global constants) are in lower\_snake\_case.
- Enumerators in enumeration types and global constants (defined in `constants.h`) are in ALL\_CAPS\_SNAKE\_CASE.
  
## Horizontal Spacing
- One space before and after the `*` used as part of a type designation in a declaration: `Person * person`.
- One space before and after a binary arithmetic, relational, or logical operator and before and after the assignment operator: `-`, `+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `<=`, `==`, `!=`, `>=`, `>`, `&&`, `||`, `<<`, `>>`, `&`, `|`
- No space around a member access operator: `person.get_age()`, `person->get_age()`
- No space between a unary operator and its operand: - `*deref`, `!value`, `++value`, `value++`, `--value`, `value--`, `~bit_flip`.
