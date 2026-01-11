# scm754

`scm754` is a Scheme implemented on top of IEEE-754.

The project focuses on a minimal, efficient reader and an `eval/apply` core with tight invariants, no malloc in the runtime, and simple, provable control flow.  

All Scheme objects are represented as 64-bit values, leveraging unused NaN space to implement the Scheme programming language.

This is an experimental, low-level implementation aimed at simplicity, correctness, and performance.

## Correctness

`scm754` enforces correctness through a combination of:

- Test cases derived from the R7RS specification
- Whitebox fuzzing
- Static and dynamic analysis, including AddressSanitizer and UndefinedBehaviorSanitizer
- Reader invariant testing

