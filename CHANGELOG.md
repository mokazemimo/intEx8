# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/).

---

## [2.0.1] - 2025-04-27
### Fixed
- Corrected undefined behavior in shift operation due to improper pointer increment (`*ptr++` issue).
- `ix8_modeq` and `ix8_modeq_p2` macros are replaced with functions to avoid memory leaks!
- Logic of `ix8_mod_p2` and ix8_modeq_p2 are completely renewed.
- `ix8_to_string` renamed to `ix8_copy_to_s`; `i8_to_string` renamed to `i8_copy_to_s`; `ix8_free_string` renamed to `ix8_free_s`.

---

## [2.0.0] - 2025-04-25
### Added
- Introduced a family of in-place modification functions:
  `ix8_addeq`, `ix8_addeq_i`, `ix8_subeq`, `ix8_subeq_i`,
  `ix8_muleq`, `ix8_muleq_i`, `ix8_muleq_p2`,
  `ix8_diveq`, `ix8_diveq_i`, `ix8_diveq_p2`, 
  `ix8_modeq`, `ix8_modeq_i`, `ix8_modeq_p2`, `ix8_i_modeq`.

- Introduced comparison-to-`int64_t` functions:  
  `i8_eq_i`, `i8_le_i`, `i8_ge_i`, `i8_lt_i`, `i8_gt_i`.

- Introduced comparison-to-`int64_t` macros:  
  `ix8_eq_i`, `ix8_le_i`, `ix8_ge_i`, `ix8_lt_i`, `ix8_gt_i`.

### Changed
- Internal algorithm for sign handling was redesigned:  
  In version 1.0.0, the most significant bit (MSB) determined the sign (1 for negative, 0 for positive).  
  In version 2.0.0, the sign is now represented by the sign of the `size` field.  
  This major change resulted in significant performance improvements.

- Replaced 16-bit basic multiplication with 32-bit multiplication for numbers with fewer than 50 digits inside `_toom3_basic_multiply`,  
  leading to further optimization of multiplication speed.

### Other
- Various refactorings and additional optimizations across the internal codebase.---

## [1.0.0] - 2025-02-22
### Added
- Initial release of intEx8 library.
- Basic operations: addition, subtraction, multiplication, division, remainder.
- Memory management and conversion utilities.
- Bitwise operations: AND, OR, XOR, NOT, shifts.

---
