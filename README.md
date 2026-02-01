# scm754

`scm754` is a Scheme implemented on top of IEEE-754.

In Scheme, code is data, and in scm754, data is IEEE-754.

A minimal, efficient Scheme with a small read, a tight eval/apply core, and no runtime allocation.
All Scheme objects are represented as 64-bit values, leveraging unused NaN space to implement the Scheme programming language.

## Attention

`scm754` is currently in the development phase and not suitable for production use.

## Correctness

`scm754` emphasizes correctness using:

- Test cases derived from the scheme specification
- White-box and black-box fuzzing
- Static and dynamic analysis (including AddressSanitizer and UndefinedBehaviorSanitizera)
- Reader invariant check

## Features

- tail call optimization
- mark and sweep garbage collector
- no third-party dependencies

## Standards

- [R5RS](https://standards.scheme.org/official/r5rs.pdf) — The language implemented
- [IEEE 754‑2008](https://standards.ieee.org/ieee/754/993) — Representation used for code and data
- [C99 (ISO/IEC 9899:1999)](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf) — The implementation language

