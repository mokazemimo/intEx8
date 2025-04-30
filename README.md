![Build Status](https://github.com/mokazemimo/intEx8/actions/workflows/main.yml/badge.svg)

<p align="center">
  <img src="Logo.jpg" alt="intEx8 Logo" width="350">
</p>

# intEx8 (**Int**eger **Ex**tended Infinitely) - Arbitrary Precision Integer Library

![Version](https://img.shields.io/badge/version-2.0.1-blue.svg)

## Overview  
**intEx8** is a **C library** for handling **arbitrarily large integers**, supporting arithmetic, bitwise operations, and string conversion for numbers of **unlimited size** (unlike `int`, `long`, `int64_t`, etc.).  

## Key Features  
- **Arithmetic Operations**: Addition, subtraction, multiplication (**Toom-3 algorithm**), division, remainder, negation, absolute value.  
- **Decimal String Conversion**: Convert big integers to/from decimal (`ix8_copy_s()/i8_copy_s()`, `ix8_copy_to_s()/i8_copy_to_s()`).  
- **Two Interfaces**:  
  - **`ix8_`** – Memory-managed API (**caller must free results**).  
  - **`i8_`** – Low-level API (**caller provides memory**).  
## ix8_ Interface (Memory-Allocating)

| Name             | Operation             | C Syntax                           | Description                         |
|------------------|-----------------------|------------------------------------|-------------------------------------|
| `ix8_copy`       | `y = x`               | `y = ix8_copy(x);`                 | Deep copy                           |
| `ix8_copy_i`     | `x = i`               | `x = ix8_copy_i(i);`               | From `int64_t`                      |
| `ix8_copy_s`     | `x = s`               | `x = ix8_copy_s("1234");`          | From decimal string                 |
| `ix8_copy_to_s`  | `s = x`               | `s = ix8_copy_to_s(x);`            | To string; use `ix8_free_s()`       |
| `ix8_add`        | `z = x + y`           | `z = ix8_add(x, y);`               | Addition                            |
| `ix8_add_i`      | `z = x + i`           | `z = ix8_add_i(x, i);`             | Add with `int64_t`                  |
| `ix8_addeq`      | `x += y`              | `ix8_addeq(&x, y);`                | In-place addition                   |
| `ix8_addeq_i`    | `x += i`              | `ix8_addeq_i(&x, i);`              | In-place add `int64_t`              |
| `ix8_sub`        | `z = x - y`           | `z = ix8_sub(x, y);`               | Subtraction                         |
| `ix8_sub_i`      | `z = x - i`           | `z = ix8_sub_i(x, i);`             | Subtract `int64_t`                  |
| `ix8_i_sub`      | `z = i - x`           | `z = ix8_i_sub(i, x);`             | `int64_t` minus big int             |
| `ix8_subeq`      | `x -= y`              | `ix8_subeq(&x, y);`                | In-place subtraction                |
| `ix8_subeq_i`    | `x -= i`              | `ix8_subeq_i(&x, i);`              | In-place subtract `int64_t`         |
| `ix8_mul`        | `z = x * y`           | `z = ix8_mul(x, y);`               | Multiplication                      |
| `ix8_mul_i`      | `z = x * i`           | `z = ix8_mul_i(x, i);`             | Multiply by `int64_t`               |
| `ix8_mul_p2`     | `z = x*sgn(y)*2^abs(y)`  | `z = ix8_mul_p2(x, y);`            | Multiply by power of 2              |
| `ix8_muleq`      | `x *= y`              | `ix8_muleq(&x, y);`                | In-place multiplication             |
| `ix8_muleq_i`    | `x *= i`              | `ix8_muleq_i(&x, i);`              | In-place multiply `int64_t`         |
| `ix8_muleq_p2`   | `x *= sgn(y)*2^abs(y)`   | `ix8_muleq_p2(&x, y);`             | In-place multiply by power of 2     |
| `ix8_div`        | `z = x / y`           | `z = ix8_div(x, y);`               | Division                            |
| `ix8_div_i`      | `z = x / i`           | `z = ix8_div_i(x, i);`             | Divide by `int64_t`                 |
| `ix8_i_div`      | `z = i / x`           | `z = ix8_i_div(i, x);`             | `int64_t` divided by big int        |
| `ix8_div_p2`     | `z = x/(sgn(y)*2^abs(y))`| `z = ix8_div_p2(x, y);`            | Divide by power of 2                |
| `ix8_diveq`      | `x /= y`              | `ix8_diveq(&x, y);`                | In-place division                   |
| `ix8_diveq_i`    | `x /= i`              | `ix8_diveq_i(&x, i);`              | In-place divide `int64_t`           |
| `ix8_i_diveq`    | `i /= x`              | `ix8_i_diveq(&x, i);`              | `int64_t` divide by big int         |
| `ix8_diveq_p2`   | `x /= sgn(y)*2^abs(y)`   | `ix8_diveq_p2(&x, y);`             | In-place divide by power of 2       |
| `ix8_mod`        | `z = x % y`           | `z = ix8_mod(x, y);`               | Modulo                              |
| `ix8_mod_i`      | `z = x % i`           | `z = ix8_mod_i(x, i);`             | Modulo `int64_t`                    |
| `ix8_i_mod`      | `z = i % x`           | `z = ix8_i_mod(i, x);`             | `int64_t` mod big int               |
| `ix8_mod_p2`     | `z = x%(2^y)`         | `z = ix8_mod_p2(x, y);`            | Modulo power of 2                   |
| `ix8_modeq`      | `x %= y`              | `ix8_modeq(&x, y);`                | In-place modulo                     |
| `ix8_modeq_i`    | `x %= i`              | `ix8_modeq_i(&x, i);`              | In-place modulo `int64_t`           |
| `ix8_i_modeq`    | `i %= x`              | `ix8_i_modeq(&x, i);`              | In-place `int64_t` mod big int      |
| `ix8_modeq_p2`   | `x %= 2^y`            | `ix8_modeq_p2(&x, y);`             | In-place modulo power of 2          |
| `ix8_negate`     | `y = -x`              | `y = ix8_negate(x);`               | Negation                            |
| `ix8_negate_me`  | `x = -x`              | `ix8_negate_me(&x);`               | In-place negation                   |
| `ix8_abs`        | `y = abs(x)`          | `y = ix8_abs(x);`                  | Absolute value                      |
| `ix8_abs_me`     | `x = abs(x)`          | `ix8_abs_me(&x);`                  | In-place absolute value             |
| `ix8_eq`         | `x == y`              | `cmp = ix8_eq(x, y) {}`            | Equality                            |
| `ix8_le`         | `x <= y`              | `if( ix8_le(x, y) ) {}`            | Less than or equal to               |
| `ix8_ge`         | `x >= y`              | `if( ix8_ge(x, y) {}`              | Greater than or equal to            |
| `ix8_lt`         | `x < y`               | `if( ix8_lt(x, y) {}`              | Less than                           |
| `ix8_gt`         | `x > y`               | `if( ix8_gt(x, y) {}`              | Greater than                        |
| `ix8_is_zero`    | `x == 0`              | `if( ix8_is_zero(x) {}`            | Zero check                          |
| `ix8_gt_zero`    | `x > 0`               | `if( ix8_gt_zero(x) {}`            | Positive check                      |
| `ix8_lt_zero`    | `x < 0`               | `if( ix8_lt_zero(x) {}`            | Negative check                      |
| `ix8_eq_i`       | `x == y`              | `if( ix8_eq_i(x, y) {}`            | Equality to `int64_t`               |
| `ix8_le_i`       | `x <= y`              | `if( ix8_le_i(x, y) {}`            | Less than or equal to `int64_t`     |
| `ix8_ge_i`       | `x >= y`              | `if( ix8_ge_i(x, y) {}`            | Greater than or equal to `int64_t`  |
| `ix8_lt_i`       | `x < y`               | `if( ix8_lt_i(x, y) {}`            | Less than `int64_t`                 |
| `ix8_gt_i`       | `x > y`               | `if( ix8_gt_i(x, y) {}`            | Greater than `int64_t`              |


## i8_ Interface (Caller-Allocated)

| Name             | Operation             | C Syntax                           | Description                         |
|------------------|-----------------------|------------------------------------|-------------------------------------|
| `i8_copy`        | `y = x`               | `y = i8_copy(x, buf);`             | Copy big int to caller's buffer     |
| `i8_copy_i`      | `x = i`               | `x = i8_copy_i(i, buf);`           | From `int64_t`                      |
| `i8_copy_s`      | `x = s`               | `x = i8_copy_s("123", buf);`       | From decimal string                 |
| `i8_copy_to_s`   | `s = x`               | `s = i8_copy_to_s(x, buf);`        | Convert to string                   |
| `i8_add`         | `z = x + y`           | `z = i8_add(x, y, buf);`           | Addition                            |
| `i8_add_i`       | `z = x + i`           | `z = i8_add_i(x, i, buf);`         | Add with `int64_t`                  |
| `i8_sub`         | `z = x - y`           | `z = i8_sub(x, y, buf);`           | Subtraction                         |
| `i8_sub_i`       | `z = x - i`           | `z = i8_sub_i(x, i, buf);`         | Subtract `int64_t`                  |
| `i8_i_sub`       | `z = i - x`           | `z = i8_i_sub(i, x, buf);`         | `int64_t` minus big int             |
| `i8_mul`         | `z = x * y`           | `z = i8_mul(x, y, buf);`           | Multiplication                      |
| `i8_mul_i`       | `z = x * i`           | `z = i8_mul_i(x, i, buf);`         | Multiply with `int64_t`             |
| `i8_mul_p2`      | `z = x*sgn(y)*2^abs(y)`  | `z = i8_mul_p2(x, y, buf);`        | Multiply by power of 2              |
| `i8_div`         | `z = x / y`           | `z = i8_div(x, y, buf);`           | Division                            |
| `i8_div_p2`      | `z = x/(sgn(y)*2^abs(y))`| `z = i8_div_p2(x, y, buf);`        | Divide by power of 2                |
| `i8_mod`         | `z = x % y`           | `z = i8_mod(x, y, buf);`           | Modulo                              |
| `i8_mod_p2`      | `z = x%(2^y)`         | `z = i8_mod_p2(x, y, buf);`        | Modulo power of 2                   |
| `i8_negate`      | `y = -x`              | `y = i8_negate(x, buf);`           | Negation                            |
| `i8_negate_me`   | `x = -x`              | `i8_negate_me(&x);`                | In-place negation                   |
| `i8_abs`         | `y = abs(x)`          | `y = i8_abs(x, buf);`              | Absolute value                      |
| `i8_abs_me`      | `x = abs(x)`          | `i8_abs_me(&x);`                   | In-place absolute                   |
| `i8_eq`          | `x == y`              | `if( i8_eq(x, y) ) {}`             | Equality                            |
| `i8_le`          | `x <= y`              | `if( i8_le(x, y) ) {}`             | Less than or equal to               |
| `i8_ge`          | `x >= y`              | `if( i8_ge(x, y) {}`               | Greater than or equal to            |
| `i8_lt`          | `x < y`               | `if( i8_lt(x, y) {}`               | Less than                           |
| `i8_gt`          | `x > y`               | `if( i8_gt(x, y) {}`               | Greater than                        |
| `i8_is_zero`     | `x == 0`              | `if( i8_is_zero(x) {}`             | Zero check                          |
| `i8_gt_zero`     | `x > 0`               | `if( i8_gt_zero(x) {}`             | Positive check                      |
| `i8_lt_zero`     | `x < 0`               | `if( i8_lt_zero(x) {}`             | Negative check                      |
| `i8_eq_i`        | `x == y`              | `if( i8_eq_i(x, y) {}`             | Equality to `int64_t`               |
| `i8_le_i`        | `x <= y`              | `if( i8_le_i(x, y) {}`             | Less than or equal to `int64_t`     |
| `i8_ge_i`        | `x >= y`              | `if( i8_ge_i(x, y) {}`             | Greater than or equal to `int64_t`  |
| `i8_lt_i`        | `x < y`               | `if( i8_lt_i(x, y) {}`             | Less than `int64_t`                 |
| `i8_gt_i`        | `x > y`               | `if( i8_gt_i(x, y) {}`             | Greater than `int64_t`              |

## Installation  
Clone the repository and add the source files (*.c, *.h) to your project:  
```sh
git clone https://github.com/mokazemimo/intEx8.git
cd intEx8

# Copy source files
cp src/*.c <YOUR_PROJECT_DIRECTORY>/src/

# Copy header files
cp include/*.h <YOUR_PROJECT_DIRECTORY>/include/

## License  
This project is licensed under the **GNU General Public License v3 (GPL-3.0)**.  
You can redistribute and modify it under the terms of the license.  

For details, see the [GNU General Public License v3.0](LICENSE) file.  
