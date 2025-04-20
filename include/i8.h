/*
 *  File: i8.h
 *  Description:
 *      Declares the public API for the `i8` interface of the intEx8 library for big integers.
 *      Provides function declarations for arithmetic, comparison, bit-wise opertions, and string conversion.
 *
 *  Author: Moham KazemiMoghaddam
 *  Created: 11-Jan-2025
 *  License: GNU General Public License v3.0 (GPL-3.0)
 */

#ifndef __intex8_I8_H__
#define __intex8_I8_H__

#include "intx.h"

/* Internal macro. Do NOT call this directly! */
#define _right_shift_digit(x, n)	((x) >> (n))

/*
 * Creates a copy of a big integer and stores it in `dest`.
 *
 * Parameters:
 *   - x: Big integer to copy.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for `x.size` digits to store the result.
 *
 * Returns:
 *   - A new `intx_t` instance equal to `x`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_copy(const intx_t x, dig_t* dest);

/*
 * Creates a big integer equal to an int64 and stores it in `dest`.
 *
 * Parameters:
 *   - x: int64 value.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for `_required_digits_for_int64(x)` digits to store the result.
 *
 * Returns:
 *   - A new `intx_t` instance equal to `x`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_copy_i(int64_t x, dig_t* dest);

/*
 * Trims leading trivial digits of a big integer.
 *
 * Description:
 *     Removes unnecessary leading digits from `x` to minimize its storage size.
 *     - Leading zero digits (0x00000000) are removed.
 *
 * Parameters:
 *     - x: The big integer to be trimmed.
 *
 * Returns:
 *     - `x`, with its `size` field updated.
 */
intx_t i8_trim(const intx_t x);

//----------------------------------------------------------------------------------------------------------
// Arithmetic operations:
// i8_add(), i8_add_i(), i8_addeq(),
// i8_sub(), i8_sub_i(), i8_i_sub(),
// i8_mul(), i8_mul_i(),
// i8_div(), i8_div_i(),
// i8_mod(), i8_mod_i(), i8_i_mod(),
// i8_negate(), i8_abs(), i8_abs_me()

/*
 * Adds two big integers and stores the result in `dest`.
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least `_required_digits_for_sum(x, y)` digits to store the result.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x + y`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_add(const intx_t x, const intx_t y, dig_t* dest);

/*
 * Adds an int64 to a big integer and stores the result in `dest`.
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate enough space (call _required_digits_for_sum(x, i8_copy_i(y,..))) for `dest` to store the result.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x + y`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_add_i(const intx_t x, int64_t y, dig_t* dest);

/*
 * Subtracts one big integer from another (`intx_t` instances).
 *
 * Parameters:
 *   - x: The minuend (big integer to subtract from).
 *   - y: The subtrahend (big integer to subtract).
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (max(x.size, y.size) + 1) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x - y`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_sub(const intx_t x, const intx_t y, dig_t* dest);

/*
 * Subtracts an int64 from a big integer.
 *
 * Parameters:
 *   - x: The minuend (big integer to subtract from).
 *   - y: The subtrahend (int64 to subtract).
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (max(x.size, y.size) + 1) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x - y`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_sub_i(const intx_t x, int64_t y, dig_t* dest);

/*
 * Subtracts a big integer from an int64.
 *
 * Parameters:
 *   - x: The minuend (int64 to subtract from).
 *   - y: The subtrahend (big integer to subtract).
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (max(x.size, y.size) + 1) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x - y`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_i_sub(int64_t x, const intx_t y, dig_t* dest);

/*
 * Multiplies two big integers (`intx_t` instances).
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (x.size + y.size + 1) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x * y`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_mul(const intx_t x, const intx_t y, dig_t* dest);

/*
 * Multiplies a big integer by an int64.
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (x.size + y.size + 1) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x * y`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_mul_i(const intx_t x, int64_t y, dig_t* dest);

/*
 * Multiplies one big integer to a power-of-two sgn(y)*2^|y| (`x * (sgn(y)*2^|y|)`).
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x * (sgn(y)*2^|y|)`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_mul_p2(intx_t x, int64_t p, dig_t* dest);

/*
 * Divides one big integer by another (`intx_t` instances).
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (big integer to divide by).
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (x.size + 1 - y.size) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x / y` (quotient of the division).
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_div(intx_t x, const intx_t yi, dig_t* dest);

/*
 * Divides one big integer by a power-of-two sgn(y)*2^|y| (`x / (sgn(y)*2^|y|)`).
 *
 * Parameters:
 *   - x: Pointer to dividend (big integer to be divided).
 *   - y: The power-of-two (to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x / (sgn(y)*2^|y|)` (quotient of the division).
 *   - Otherwise, intx_zero.
 */
intx_t i8_div_p2(intx_t x, int64_t y, dig_t* dest);

/*
 * Computes the remainder of the division of one big integer by another and stores the result in `dest`.
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (big integer to divide by).
 *   - dest: Either NULL or pointer to the memory segment where the result will be stored.
 *           Caller must allocate enough space for `dest` to store the result.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x % y` (the remainder of `x / y`).
 *   - Otherwise, returns `intx_zero`.
 */
intx_t i8_mod(intx_t x, const intx_t y, dig_t*);

/*
 * Computes the remainder of the division of a big integer by 2^|y|.
 *
 * Parameters:
 *   - x: The dividend (int64 to be divided).
 *   - y: The power of two (to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a big integer representing `x % (2^|y|)` (the remainder of `x / (2^|y|)`).
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - The following identities hold:
 *       ix8_mod_p2(x, -y) = ix8_mod_p2(x, y).
 *       ix8_mod_p2(-x, y) = -ix8_mod_p2(x, y).
 */
intx_t i8_mod_p2(intx_t x, int64_t y, dig_t* dest);

/*
 * Computes the negation of a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to negate.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (x.size) digits to store the result.
 *           If x is an Extreme Negative (i.e., ONLY the highest bit is set), caller must allocate (x.size + 1) digits for the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `-x`.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t i8_negate(const intx_t x, dig_t *dest);

/*
 * Computes the absolute value of a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer whose absolute value is to be computed.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (x.size) digits to store the result.
 *           If x is an Extreme Negative (i.e., ONLY the highest bit is set), caller must allocate (x.size + 1) digits for the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `|x|` (absolute value of `x`).
 *   - Otherwise, returns `intx_zero`.
 */
intx_t i8_abs(const intx_t x, dig_t* dest);

void i8_abs_me(intx_t *x);

//----------------------------------------------------------------------------------------------------------
// Comparison operators:
// i8_eq, i8_le, i8_ge, i8_lt, i8_gt,
// i8_is_zero, i8_gt_zero, i8_lt_zero, i8_is_intx_min, i8_is_pow2

/*
 * Checks if two big integers (`intx_t` instances) are equal.
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *
 * Returns:
 *   - `true` if `x` is equal to `y`.
 *   - `false` otherwise.
 * 
 * Note:
 *   - Assumes x and y are trimmed
 */
bool i8_eq(const intx_t x, const intx_t y);

/*
 * Checks if a big integer (`intx_t` instance) is less than or equal to another.
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *
 * Returns:
 *   - `true` if `x <= y`.
 *   - `false` otherwise.
 * 
 * Note:
 *   - Assumes x and y are trimmed
 */
bool i8_le(const intx_t x, const intx_t y);

/*
 * Checks if a big integer (`intx_t` instance) is greater than or equal to another.
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *
 * Returns:
 *   - `true` if `x >= y`.
 *   - `false` otherwise.
 *
 * Note:
 *   - Assumes x and y are trimmed
 */
#define i8_ge(x, y)		(true == i8_le(y, x))

/*
 * Checks if a big integer (`intx_t` instance) is less than another.
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *
 * Returns:
 *   - `true` if `x < y`.
 *   - `false` otherwise.
 *
 * Note:
 *   - Assumes x and y are trimmed
 */
#define i8_lt(x, y)		(false == i8_le(y, x))

/*
 * Checks if a big integer (`intx_t` instance) is greater than another.
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *
 * Returns:
 *   - `true` if `x > y`.
 *   - `false` otherwise.
 *
 * Note:
 *   - Assumes x and y are trimmed
 */
#define i8_gt(x, y)		(false == i8_le(x, y))

/*
 * Checks if a big integer (`intx_t` instance) is zero.
 *
 * Parameters:
 *   - x: Big integer to check.
 *
 * Returns:
 *   - `true` if `x == 0`.
 *   - `false` otherwise.
 */
bool i8_is_zero(const intx_t x);

/*
 * Checks if a big integer (`intx_t` instance) is positive.
 *
 * Parameters:
 *   - x: Big integer to check.
 *
 * Returns:
 *   - `true` if `x > 0`.
 *   - `false` otherwise.
 */
bool i8_gt_zero(const intx_t x);

/*
 * Checks if a big integer (`intx_t` instance) is negative.
 *
 * Parameters:
 *   - x: Big integer to check.
 *
 * Returns:
 *   - `true` if `x < 0`.
 *   - `false` otherwise.
 */
bool i8_lt_zero(const intx_t x);

/*
 * Checks if a big integer is equal to an int64.
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *
 * Returns:
 *   - `true` if `x == y`.
 *   - `false` otherwise.
 *
 * Note:
 *   - Assumes x is trimmed
 */
bool i8_eq_i(const intx_t x, const int64_t y);

/*
 * Checks if a big integer is less than or equal to an int64.
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *
 * Returns:
 *   - `true` if `x <= y`.
 *   - `false` otherwise.
 *
 * Note:
 *   - Assumes x is trimmed
 */
bool i8_le_i(const intx_t x, const int64_t y);

/*
 * Checks if a big integer is less than an int64.
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *
 * Returns:
 *   - `true` if `x < y`.
 *   - `false` otherwise.
 *
 * Note:
 *   - Assumes x is trimmed
 */
bool i8_lt_i(const intx_t x, const int64_t y);

/*
 * Checks if a big integer is greater than or equal to an int64.
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *
 * Returns:
 *   - `true` if `x >= y`.
 *   - `false` otherwise.
 *
 * Note:
 *   - Assumes x is trimmed
 */
#define i8_ge_i(x, y)		(false == i8_lt_i(x, y))

/*
 * Checks if a big integer is greater than an int64.
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *
 * Returns:
 *   - `true` if `x > y`.
 *   - `false` otherwise.
 *
 * Note:
 *   - Assumes x is trimmed
 */
#define i8_gt_i(x, y)		(false == i8_le_i(x, y))

/*
 * Checks if a big integer is of the form `pow(2, n)` or `-pow(2, n)`, for some non-negative integer `n`.
 *
 * Parameters:
 *   - x: Big integer to check.
 *
 * Returns:
 *   - `n + 1` if `x == pow(2, n)`; `-(n + 1)` if `x == -pow(2, n)`;
 *   - Otherwise `0`.
 */
int64_t i8_is_pow2(const intx_t x);

//----------------------------------------------------------------------------------------------------------
// String conversion:
// i8_to_string, i8_copy_s

/*
 * Converts a positive big integer (`intx_t` instance) to its decimal string representation.
 *
 * Parameters:
 *   - x: A positive big integer to convert.
 *   - buf: Pointer to a memory segment where the result will be stored.
 *          Caller must allocate enough space using `_required_decimal_count(x.size, false)`.
 *          Caller is responsible for managing the memory for `buf`.
 *   - buf_size: Size (in bytes) of memory allocated for buf.
 * 
 * Returns:
 *   - buf + k (for some non-negative k < buf_size), where the null-terminated decimal string representation of `x` is stored.
 */
char* i8_to_string(const intx_t x, char* buf, size_t buf_size);

/*
 * Parses a big integer (`intx_t` instance) from its decimal string representation.
 *
 * Parameters:
 *   - str: A null-terminated decimal string representing a POSITIVE big integer (without a leading `-` or `+` sign).
 *          The string MUST contain only the digits '0' to '9'; otherwise, the behavior is undefined.
 *   - len: Nnumber of characters to be read from `str`.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate enough space using `_required_digit_count(len)`.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, parsed `intx_t` instance stored in dest.
 *   - Otherwise `intx_zero`.
 */
intx_t i8_copy_s(const char* str, size_t len, dig_t* dest);

#endif // __intex8_I8_H__
