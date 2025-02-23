/*
 *  File: ix8.h
 *  Description:
 *      Declares the public API for the `ix8` interface of the intEx8 library for big integers.
 *      Provides function declarations for big-integer arithmetic, comparisons, bit-wise operations, and string conversion.
 *
 *  Author: Moham KazemiMoghaddam
 *  Created: 13-Jan-2025
 *  License: GNU General Public License v3.0
 */

#ifndef __intex8_IX8_H__
#define __intex8_IX8_H__

#include <stdlib.h>	// malloc()
#include "util.h"
#include "i8.h"

/*
 * Creates a copy of a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to copy.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance equal to `x`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
intx_t ix8_copy(const intx_t x);

/*
 * Creates a big integer from an int64.
 *
 * Parameters:
 *   - x: int64 value.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance equal to `x`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
intx_t ix8_from_int(int64_t x);

/*
 * Adds two big integers (`intx_t` instances).
 * 
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 * 
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x + y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
intx_t ix8_x_add_x(const intx_t x, const intx_t y);

/*
 * Adds an int64 to a big integer.
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x + y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
intx_t ix8_x_add_i(const intx_t x, int64_t y);

/*
 * Subtracts one big integer from another (`intx_t` instances).
 *
 * Parameters:
 *   - x: The minuend (big integer to subtract from).
 *   - y: The subtrahend (big integer to subtract).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x - y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
intx_t ix8_x_sub_x(const intx_t x, const intx_t y);

/*
 * Subtracts an int64 from a big integer.
 *
 * Parameters:
 *   - x: The minuend (big integer to subtract from).
 *   - y: The subtrahend (int64 to subtract).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x - y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
intx_t ix8_x_sub_i(const intx_t x, int64_t y);

/*
 * Subtracts a big integer from an int64.
 *
 * Parameters:
 *   - x: The minuend (int64 to subtract from).
 *   - y: The subtrahend (big integer to subtract).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x - y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
intx_t ix8_i_sub_x(int64_t x, const intx_t y);

/*
 * Multiplies two big integers (`intx_t` instances).
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x * y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
intx_t ix8_x_mul_x(const intx_t x, const intx_t y);

/*
 * Multiplies one big integer to an int64.
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x * y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
intx_t ix8_x_mul_i(const intx_t x, int64_t y);

/*
 * Divides one big integer by another (`intx_t` instances).
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (big integer to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x / y` (quotient of the division).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following holds:
 *       `(-x) / y == -(x / y) == -(x / (-y)) == (-x) / (-y)`.
 */
intx_t ix8_x_div_x(const intx_t x, const intx_t y);

/*
 * Divides one big integer by an int64_t.
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (int64_t to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x / y` (quotient of the division).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following holds:
 *       `(-x) / y == -(x / y) == -(x / (-y)) == (-x) / (-y)`.
 */
intx_t ix8_x_div_i(const intx_t x, int64_t y);

/*
 * Divides an int64_t by a big integer.
 *
 * Parameters:
 *   - x: The dividend (int64 to be divided).
 *   - y: The divisor (big integer to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x / y` (quotient of the division).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following holds:
 *       `(-x) / y == -(x / y) == -(x / (-y)) == (-x) / (-y)`.
 */
intx_t ix8_i_div_x(int64_t x, const intx_t y);

/*
 * Computes the remainder of the division of one big integer by another (`intx_t` instances).
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (big integer to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x % y` (the remainder of `x / y`).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - Allocates y.size digits of memory for return value.
 *   - If `x` and `y` are positive, the following identity holds:
 *       `(-x) % y == -(x % y) == -(x % (-y)) == (-x) % (-y)`.
 */
intx_t ix8_x_mod_x(const intx_t x, const intx_t y);

/*
 * Computes the remainder of the division of a big integer by an int64.
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (int64 to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x % y` (the remainder of `x / y`).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - Allocates y.size digits of memory for return value.
 *   - If `x` and `y` are positive, the following identity holds:
 *       `(-x) % y == -(x % y) == -(x % (-y)) == (-x) % (-y)`.
 */
intx_t ix8_x_mod_i(const intx_t x, int64_t y);

/*
 * Computes the remainder of the division of an int64 by a big integer.
 *
 * Parameters:
 *   - x: The dividend (int64 to be divided).
 *   - y: The divisor (intx to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x % y` (the remainder of `x / y`).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - Allocates y.size digits of memory for return value.
 *   - If `x` and `y` are positive, the following identity holds:
 *       `(-x) % y == -(x % y) == -(x % (-y)) == (-x) % (-y)`.
 */
intx_t ix8_i_mod_x(int64_t x, const intx_t y);

/*
 * Computes the negation of a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to negate.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `-x`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t ix8_negate(const intx_t x);

/*
 * Negates a big integer (`intx_t` instance) in place.
 *
 * Parameters:
 *   - x: Big integer to be negated.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, the negated `x` in place.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - This function modifies `x` in place and does not create a new instance.
 *   - If `x` represents an Extreme Negative value (i.e., ONLY the highest bit is set),
 *     negation requires an additional digit to store the result. This case will result
 *     in `INTEX8_ERR_CANNOT_NEGATE_EXTREME_NEGATIVE`
 */
intx_t ix8_negate_self(intx_t x);

/*
 * Computes the absolute value of a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer whose absolute value is to be computed.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `|x|` (absolute value of `x`).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t ix8_abs(const intx_t x);

/*
 * Computes the absolute value of a big integer (`intx_t` instance) in place.
 *
 * Parameters:
 *   - x: Big integer to be modified to its absolute value.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, `x` converted to its absolute value (`|x|`).
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - This function modifies `x` in place and does not create a new instance.
 *   - If `x` represents an Extreme Negative value (i.e., ONLY the highest bit is set),
 *     negation requires an additional digit to store the result. This case will result
 *     in `INTEX8_ERR_CANNOT_NEGATE_EXTREME_NEGATIVE`.
 */
intx_t ix8_abs_self(intx_t x);

/*
 * Performs bitwise AND operation on two big integers (`intx_t` instances).
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x & y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - The result will have a `size` equal to the maximum `size` of the input arguments.
 *   - As per standard behavior, positive integers are extended beyond the highest digit with `0` bits,
 *     while negative integers are extended with `1` bits.
 */
intx_t ix8_binary_and(const intx_t x, const intx_t y);

/*
 * Performs bitwise OR operation on two big integers (`intx_t` instances).
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x | y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - The result will have a `size` equal to the maximum `size` of the input arguments.
 *   - As per standard behavior, positive integers are extended beyond the highest digit with `0` bits,
 *     while negative integers are extended with `1` bits.
 */
intx_t ix8_binary_or(const intx_t x, const intx_t y);

/*
 * Performs bitwise XOR operation on two big integers (`intx_t` instances).
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x ^ y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - The result will have a `size` equal to the maximum `size` of the input arguments.
 *   - As per standard behavior, positive integers are extended beyond the highest digit with `0` bits,
 *     while negative integers are extended with `1` bits.
 */
intx_t ix8_binary_xor(const intx_t x, const intx_t y);

/*
 * Computes the bitwise NOT of a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to be negated bitwise.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `~x`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t ix8_binary_not(const intx_t x);

/*
 * Computes the bitwise NOT of a big integer (`intx_t` instance) in place.
 *
 * Parameters:
 *   - x: Big integer to be modified to its bitwise complement.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, `x` after applying bitwise NOT (`~x`).
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - This function modifies `x` in place and does not create a new instance.
 */
intx_t ix8_binary_not_self(intx_t x);

/*
 * Performs a left bitwise shift operation on a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to be shifted.
 *   - bits: Number of bits to shift.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for (x.size) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing the left-shifted value of `x`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t ix8_left_shift(const intx_t x, size_t bits);

/*
 * Performs a left bitwise shift operation on a big integer (`intx_t` instance) in place.
 *
 * Parameters:
 *   - x: Big integer to be shifted.
 *   - bits: Number of bits to shift.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, the left-shifted value of `x` is stored in place.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t ix8_left_shift_self(intx_t x, size_t bits);

/*
 * Performs a right bitwise shift operation on a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to be shifted.
 *   - bits: Number of bits to shift.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for (x.size - bits / INTEX8_DIGIT_BIT_WIDTH()) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing the right-shifted value of `x`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t ix8_right_shift(const intx_t x, size_t bits);

/*
 * Performs a right bitwise shift operation on a big integer (`intx_t` instance) in place.
 *
 * Parameters:
 *   - x: Big integer to be shifted.
 *   - bits: Number of bits to shift.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, the right-shifted value of `x` is stored in place.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t ix8_right_shift_self(intx_t x, size_t bits);

// Comparison operators
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
 */
inline bool ix8_is_equal(const intx_t x, const intx_t y) { return i8_is_equal(i8_trim(x), i8_trim(y)); }

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
 */
inline bool ix8_is_less_eq(const intx_t x, const intx_t y) { return i8_is_less_eq(i8_trim(x), i8_trim(y)); }

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
 */
inline bool ix8_is_greater_eq(const intx_t x, const intx_t y) { return ix8_is_less_eq(y, x); }

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
 */
inline bool ix8_is_less(const intx_t x, const intx_t y) { return !ix8_is_less_eq(y, x); }

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
 */
inline bool ix8_is_greater(const intx_t x, const intx_t y) { return !ix8_is_less_eq(x, y); }

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
inline bool ix8_is_zero(const intx_t x) { return i8_is_zero(i8_trim(x)); }

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
inline bool ix8_is_positive(const intx_t x) { return i8_is_positive(i8_trim(x)); }

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
inline bool ix8_is_negative(const intx_t x) { return i8_is_negative(i8_trim(x)); }

/*
 * Frees the memory allocated for a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to be freed.
 *
 * Notes:
 *   - This function must only be used for instances returned by `intex8_*` functions
 *     that allocate memory (e.g., `ix8_x_add_x()`, `ix8_x_mul_x()`, etc.).
 *   - If `x` is `intx_zero` or has a `NULL` pointer, the function does nothing.
 */
inline void ix8_free(intx_t x) { free(x.ptr); }

// String conversion
/*
 * Converts a big integer (`intx_t` instance) to a string representation.
 *
 * Parameters:
 *   - x: Big integer to convert.
 *
 * Returns:
 *   - A dynamically allocated null-terminated string representing `x` in decimal form.
 *     Caller is responsible for freeing the string using `ix8_free_string()`.
 *   - If an error occurs, `NULL` is returned.
 */
char* const ix8_to_string(const intx_t x);

/*
 * Parses a big integer (`intx_t` instance) from a string representation.
 *
 * Parameters:
 *   - str: A null-terminated string representing a big integer in decimal form.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing the parsed value.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t ix8_from_string(const char* str);

/*
 * Frees a string allocated by `ix8_to_string()`.
 *
 * Parameters:
 *   - str: The null-terminated string to be freed.
 *
 * Notes:
 *   - This function must only be used for strings returned by `ix8_to_string()`.
 *   - If `str` is `NULL`, the function does nothing.
 */
inline void ix8_free_string(const char* str) { free((void *)str); }


#endif	// __intex8_IX8_H__