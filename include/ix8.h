/*
 *  File: ix8.h
 *  Description:
 *      Declares the public API for the `ix8` interface of the intEx8 library for big integers.
 *      Provides function declarations for big-integer arithmetic, comparisons, bit-wise operations, and string conversion.
 *
 *  Author: Moham KazemiMoghaddam
 *  Created: 13-Jan-2025
 *  License: GNU General Public License v3.0 (GPL-3.0)
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
intx_t ix8_copy_i(int64_t x);

//---------------------------------------------------------------------------------------------------
// Arithmetic
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
intx_t ix8_add(const intx_t x, const intx_t y);

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
intx_t ix8_add_i(const intx_t x, int64_t y);

/*
 * Adds a big integer (`intx_t` instance) to another in place (`x += y`).
 *
 * Parameters:
 *   - x: Big integer to be modified.
 *   - y: Big integer to be added to `x`.
 *
 * Returns:
 *   - If `ix8_errno` is `INTEX8_OK`, `x` is updated with the sum `x + y` and returned.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, x is not modified ?????????
 */
void ix8_addeq(intx_t *x, intx_t y);	// x += y

/*
 * Adds an int64 to a big integer in place (`x += y`).
 *
 * Parameters:
 *   - x: Big integer to be modified.
 *   - y: Int64 to be added to `x`.
 *
 * Returns:
 *   - If `ix8_errno` is `INTEX8_OK`, `x` is updated with the sum `x + y` and returned.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, x is not modified ?????????.
 */
void ix8_addeq_i(intx_t *x, int64_t y);

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
intx_t ix8_sub(const intx_t x, const intx_t y);

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
intx_t ix8_sub_i(const intx_t x, int64_t y);

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
intx_t ix8_i_sub(int64_t x, const intx_t y);

/*
 * Subtracts one big integer from another in place (`*x -= y`).
 *
 * Parameters:
 *   - x: Pointer to the minuend (big integer to subtract from) which will be updated if function is successful.
 *        If `intEx8_errno` is `INTEX8_OK`, *x will be updated to `*x - y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - y: The subtrahend (big integer to subtract).
 */
void ix8_subeq(intx_t *x, const intx_t y);

/*
 * Subtracts an int64 from a big integer in place (`*x -= y`).
 *
 * Parameters:
 *   - x: Pointer to the minuend (big integer to subtract from) which will be updated if function is successful.
 *        If `intEx8_errno` is `INTEX8_OK`, *x will be updated to `*x - y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - y: The subtrahend (big integer to subtract).
 */
void ix8_subeq_i(intx_t *x, int64_t y);

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
intx_t ix8_mul(const intx_t x, const intx_t y);

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
intx_t ix8_mul_i(const intx_t x, int64_t y);

/*
 * Multiplies one big integer to `sgn(y)*2^|y|`.
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
intx_t ix8_mul_p2(intx_t x, int64_t y);

/*
 * Multiplies a big integer by another big integer in place (`*x *= y`).
 *
 * Parameters:
 *   - x: Pointer to big integer which will be modified (multiplied by).
 *   - y: The big integer (multiplier).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, *x will be updated to `*x * y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
void ix8_muleq(intx_t *x, intx_t y);

/*
 * Multiplies a big integer by an int64 in place (`*x *= y`).
 *
 * Parameters:
 *   - x: Pointer to big integer which will be modified (multiplied by).
 *   - y: int64 multiplier.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, *x will be updated to `*x * y`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
void ix8_muleq_i(intx_t *x, int64_t y);

/*
 * Multiplies a big integer by a power-of-two in place (`*x *= sgn(y) * 2^|y|`).
 *
 * Parameters:
 *   - x: Pointer to big integer which will be modified (multiplied by).
 *   - y: int64 which contains sign and 2's power (multiplier).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, *x will be updated to `*x * (sgn(y) * 2^|y|)`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, `intx_zero`.
 */
void ix8_muleq_p2(intx_t *x, int64_t y);

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
 */
intx_t ix8_div(const intx_t x, const intx_t y);

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
 */
intx_t ix8_div_i(const intx_t x, int64_t y);

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
 */
int64_t ix8_i_div(int64_t x, const intx_t yi);

/*
 * Divides one big integer by a power-of-two sgn(y)*2^|y| (`*x / (sgn(y)*2^|y|)`).
 *
 * Parameters:
 *   - x: Pointer to dividend (big integer to be divided).
 *   - y: The power-of-two (to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `*x / (sgn(y)*2^|y|)` (quotient of the division).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, *x will not be modified.
 */
intx_t ix8_div_p2(intx_t x, int64_t y);

/*
 * Divides one big integer by another in place (`*x /= y`).
 *
 * Parameters:
 *   - x: Pointer to dividend (big integer to be divided).
 *   - y: The divisor (big integer to divide by).
 *
 * Notes:
 *   - If `intEx8_errno` is `INTEX8_OK`, updates *x to `*x / y` (quotient of the division).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, *x will not be modified.
 */
void ix8_diveq(intx_t *x, intx_t y);

/*
 * Divides one big integer by an int64 in place (`*x /= y`).
 *
 * Parameters:
 *   - x: Pointer to dividend (big integer to be divided).
 *   - y: The divisor (int64 to divide by).
 *
 * Notes:
 *   - If `intEx8_errno` is `INTEX8_OK`, updates *x to `*x / y` (quotient of the division).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, *x will not be modified.
 */
void ix8_diveq_i(intx_t *x, int64_t y);

/*
 * Divides an int64 by a big integer in place (`*x /= y`).
 *
 * Parameters:
 *   - x: Pointer to dividend (int64 to be divided).
 *   - y: The divisor (big integer to divide by).
 *
 * Notes:
 *   - If `intEx8_errno` is `INTEX8_OK`, updates *x to `*x / y` (quotient of the division).
 *   - Otherwise, *x will not be modified.
 */
void ix8_i_diveq(int64_t* x, const intx_t yi);

/*
 * Divides one big integer by a power-of-two sgn(y)*2^|y| in place (`*x /= (sgn(y)*2^|y|)`).
 *
 * Parameters:
 *   - x: Pointer to dividend (big integer to be divided).
 *   - y: The power-of-two (to divide by).
 *
 * Notes:
 *   - If `intEx8_errno` is `INTEX8_OK`, updates *x to `*x / (sgn(y)*2^|y|)` (quotient of the division).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, *x will not be modified.
 */
void ix8_diveq_p2(intx_t *x, int64_t y);

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
intx_t ix8_mod(const intx_t x, const intx_t y);

/*
 * Computes the remainder of the division of a big integer by an int64.
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (int64 to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, an int64 representing `x % y` (the remainder of `x / y`).
 *   - Otherwise, returns 0.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following identity holds:
 *       `(-x) % y == -(x % y) == -(x % (-y)) == (-x) % (-y)`.
 */
int64_t ix8_mod_i(const intx_t x, int64_t y);

/*
 * Computes the remainder of the division of an int64 by a big integer.
 *
 * Parameters:
 *   - x: The dividend (int64 to be divided).
 *   - y: The divisor (intx to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, an int64 representing `x % y` (the remainder of `x / y`).
 *   - Otherwise, returns 0.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following identity holds:
 *       `(-x) % y == -(x % y) == -(x % (-y)) == (-x) % (-y)`.
 */
int64_t ix8_i_mod(int64_t x, const intx_t y);

/*
 * Computes the remainder of the division of a big integer by 2^|y|.
 *
 * Parameters:
 *   - x: The dividend (int64 to be divided).
 *   - y: The power of two (to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a big integer representing `x % (2^|y|)` (the remainder of `x / (2^|y|)`).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - The following identities hold:
 *       ix8_mod_p2(x, -y) = ix8_mod_p2(x, y).
 *       ix8_mod_p2(-x, y) = -ix8_mod_p2(x, y).
 */
intx_t ix8_mod_p2(intx_t x, int64_t y);

/*
 * Computes the remainder of the division of one big integer by another in place (x %= y).
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (big integer to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, updated `intx_t` instance representing `x % y` (the remainder of `x / y`).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, updates x to `intx_zero`.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following identity holds:
 *       `(-x) % y == -(x % y) == -(x % (-y)) == (-x) % (-y)`.
 */
#define ix8_modeq(x, y)			(*(x) = i8_mod(i8_trim(*(x)), i8_trim(y), NULL))

/*
 * Computes the remainder of the division of a big integer by 2^|y| in place (`x %= 2^|y|`).
 *
 * Parameters:
 *   - x: The dividend (int64 to be divided).
 *   - y: The power of two (to divide by).
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, updates x to `x % (2^|y|)` (the remainder of `x / (2^|y|)`).
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - The following identities hold:
 *       ix8_mod_p2(x, -y) = ix8_mod_p2(x, y).
 *       ix8_mod_p2(-x, y) = -ix8_mod_p2(x, y).
 */
#define ix8_modeq_p2(x, y)		(*(x) = i8_mod_p2(i8_trim(*(x)), y, NULL))

/*
 * Computes the remainder of the division of a big integer by a 64-bit integer in place (`*x %= y`).
 *
 * Parameters:
 *   - x: Pointer to the dividend (big integer to be updated).
 *   - y: The divisor (int64 to divide by).
 *
 * Notes:
 *   - After execution, `*x` contains the result of `*x % y`.
 *   - The function ensures that `*x` is properly updated without memory leaks.
 *   - If `y` is positive, the following identity holds:
 *       `(-x) % y == -(x % y) == (-x) % (-y) == -(x % (-y))`
 */
void ix8_modeq_i(intx_t* xi, int64_t y);

/*
 * Computes the remainder of the division of an int64 by a big integer in place (`*x %= y`).
 *
 * Parameters:
 *   - x: Pointer to dividend (int64 to be updated).
 *   - y: The divisor (big integer to divide by).
 *
 * Notes:
 *   - After execution, `*x` contains the result of `*x % y`.
 *   - If `y` is positive, the following identity holds:
 *       `(-(*x)) % y == -((*x) % y)`
 */
void ix8_i_modeq(int64_t* x, intx_t y);

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

#define ix8_negate_me		i8_negate_me

/*
 * Computes the absolute value of a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to negate.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `|x|`.
 *     Caller is responsible for freeing it using `ix8_free()`.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t ix8_abs(const intx_t x);

#define ix8_abs_me		i8_abs_me

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
#define ix8_eq(x, y)		(true == i8_eq(i8_trim(x), i8_trim(y)))

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
#define ix8_le(x, y)		(true == i8_le(i8_trim(x), i8_trim(y)))

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
#define ix8_ge(x, y)		(true == ix8_le(y, x))

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
#define ix8_lt(x, y)		(false == ix8_le(y, x))

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
#define ix8_gt(x, y)		(false == ix8_le(x, y))

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
#define ix8_is_zero(x)		(true == i8_is_zero(i8_trim(x)))

/*
 * Checks if a big integer is positive.
 *
 * Parameters:
 *   - x: Big integer to check.
 *
 * Returns:
 *   - `true` if `x > 0`.
 *   - `false` otherwise.
 */
#define ix8_gt_zero(x)		(true == i8_gt_zero(i8_trim(x)))

/*
 * Checks if a big integer is negative.
 *
 * Parameters:
 *   - x: Big integer to check.
 *
 * Returns:
 *   - `true` if `x < 0`.
 *   - `false` otherwise.
 */
#define ix8_lt_zero(x)		(true == i8_lt_zero(i8_trim(x)))

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
 */
#define ix8_eq_i(x, y)		i8_eq_i(i8_trim(x), y)

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
 */
#define ix8_le_i(x, y)		i8_le_i(i8_trim(x), y)

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
 */
#define ix8_lt_i(x, y)		i8_lt_i(i8_trim(x), y)

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
 */
#define ix8_ge_i(x, y)		i8_ge_i(i8_trim(x), y)

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
 */
#define ix8_gt_i(x, y)		i8_gt_i(i8_trim(x), y)

/*
 * Frees the memory allocated for a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to be freed.
 *
 * Notes:
 *   - This function must only be used for instances returned by `ix8_*` functions
 *     that allocate memory (e.g., `ix8_add()`, `ix8_mul()`, etc.).
 *   - If `x` is `intx_zero` or has a `NULL` pointer, the function does nothing.
 */
#define ix8_free(x)		free((x).ptr)

//---------------------------------------------------------------------------------------------------
// String conversion
/*
 * Converts a big integer (`intx_t` instance) to a string representation.
 *
 * Parameters:
 *   - x: Big integer to convert.
 *   - fptr: if a non-NULL pointer is passed, returns the pointer which should be freed (!!!!!!!!!!!!!!! ???????)
 *
 * Returns:
 *   - A dynamically allocated null-terminated string representing `x` in decimal form.
 *     Caller is responsible for freeing the string using `ix8_free_string()`.
 *   - If an error occurs, `NULL` is returned.
 */
char* const ix8_to_string(const intx_t x, char** fptr);

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
intx_t ix8_copy_s(const char* str);

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
#define ix8_free_string(str)	free((void *)str)


#endif	// __intex8_IX8_H__