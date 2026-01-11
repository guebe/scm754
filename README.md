# scm754

`scm754` is a Scheme implemented on top of IEEE-754.

The project focuses on a minimal, efficient reader and an `eval/apply` core with tight invariants, no malloc in the runtime, and simple, provable control flow.  

All Scheme objects are represented as 64-bit values, leveraging unused NaN space to implement the Scheme programming language.

This is an experimental, low-level implementation aimed at simplicity, correctness, and performance.

## Attention

`scm754` is currently in the development phase and not suitable for production use.

## Standards

- [R5RS](https://standards.scheme.org/official/r5rs.pdf) — The language implemented
- [IEEE 754‑2008](https://standards.ieee.org/ieee/754/993) — Representation used for code and data
- [C99 (ISO/IEC 9899:1999)](https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1256.pdf) — The implementation language

## Correctness

`scm754` enforces correctness through a combination of:

- Test cases derived from the scheme specification
- Whitebox fuzzing
- Static and dynamic analysis, including AddressSanitizer and UndefinedBehaviorSanitizer
- Reader invariant testing

