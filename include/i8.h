/*
 *  File: i8.h
 *  Description:
 *      Declares the public API for the `i8` interface of the intEx8 library for big integers.
 *      Provides function declarations for arithmetic, comparison, bit-wise opertions, and string conversion.
 *
 *  Author: Moham KazemiMoghaddam
 *  Created: 11-Jan-2025
 *  License: GNU General Public License v3.0
 */

#ifndef __intex8_I8_H__
#define __intex8_I8_H__

#include "intx.h"

/*
 * Creates a copy of a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to copy.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for (x.size) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - A new `intx_t` instance equal to `x`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_copy(const intx_t x, dig_t* dest);

/*
 * Creates a big integer from an int64.
 *
 * Parameters:
 *   - x: int64 value.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for `_get_int_size(x)` digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - A new `intx_t` instance equal to `x`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_from_int(int64_t x, dig_t* dest);

/*
 *  Trims leading trivial digits from a big integer in-place.
 *
 *  Description:
 *      Removes unnecessary leading digits from `x` to minimize its storage size.
 *      - For positive numbers, leading zero digits (0x00000000) are removed.
 *      - For negative numbers, leading sign-extension digits (0xFFFFFFFF) are removed.
 *
 *  Parameters:
 *      - x: The big integer to be trimmed.
 *
 *  Returns:
 *      - `x`, with its `size` field updated.
 */
intx_t i8_trim(const intx_t x);

//----------------------------------------------------------------------------------------------------------
// Arithmetic operations:
// i8_x_add_x(), i8_x_add_i(), i8_x_add_eq_x(),
// i8_x_sub_x(), i8_x_sub_i(), i8_i_sub_x(),
// i8_x_mul_x(), i8_x_mul_i(),
// i8_x_div_x(), i8_x_div_i(), i8_i_div_x(),
// i8_x_mod_x(), i8_x_mod_i(), i8_i_mod_x(),
// i8_negate(), i8_negate_self(), i8_abs(), i8_abs_self()

/*
 * Adds two big integers (`intx_t` instances).
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (max(x.size, y.size) + 1) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x + y`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_x_add_x(const intx_t x, const intx_t y, dig_t* dest);

/*
 * Adds an int64 to a big integer.
 *
 * Parameters:
 *   - x: The big integer.
 *   - y: The int64.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (max(x.size, y.size) + 1) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x + y`.
 *   - Otherwise, `intx_zero`.
 */
intx_t i8_x_add_i(const intx_t x, int64_t y, dig_t* dest);

/*
 * Adds a big integer (`intx_t` instance) to another in place (`x += y`).
 *
 * Parameters:
 *   - x: Big integer to be modified.
 *   - y: Big integer to be added to `x`.
 *
 * Returns:
 *   - If `ix8_errno` is `IX8_OK`, `x` is updated with the sum `x + y` and returned.
 *   - Otherwise, returns `intx_zero`.
 *
 * Note:
 *   - Caller is responsible for ensuring `x` has at least max(x.size, y.size) digits space to store the result.
 *   - Caller is responsible for an additional digit (e.g., due to carry), if the result requires.
 */
intx_t i8_x_add_eq_x(intx_t x, const intx_t y);

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
intx_t i8_x_sub_x(const intx_t x, const intx_t y, dig_t* dest);	// -

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
intx_t i8_x_sub_i(const intx_t x, int64_t y, dig_t* dest);

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
intx_t i8_i_sub_x(int64_t x, const intx_t y, dig_t* dest);

/*
 * Subtracts a big integer (`intx_t` instance) from another in place (`x -= y`).
 *
 * Parameters:
 *   - x: Big integer to be modified.
 *   - y: Big integer to be subtracted from `x`.
 *
 * Returns:
 *   - If `ix8_errno` is `IX8_OK`, `x` is updated with the difference `x - y` and returned.
 *   - Otherwise, returns `intx_zero`.
 *
 * Note:
 *   - Caller is responsible for ensuring `x` has at least max(x.size, y.size) digits space to store the result.
 *   - If `x` becomes an Extreme Negative, an additional digit may be required.
 */
intx_t i8_x_sub_eq_x(intx_t x, intx_t y);

/*
 * Subtracts an int64 from big integer in place (`x -= y`).
 *
 * Parameters:
 *   - x: Big integer to be modified.
 *   - y: int64 to be subtracted from `x`.
 *
 * Returns:
 *   - If `ix8_errno` is `IX8_OK`, `x` is updated with the difference `x - y` and returned.
 *   - Otherwise, returns `intx_zero`.
 *
 * Note:
 *   - Caller is responsible for ensuring `x` has at least max(x.size, y.size) digits space to store the result.
 *   - If `x` becomes an Extreme Negative, an additional digit may be required.
 */
intx_t i8_x_sub_eq_i(intx_t x, int64_t y);

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
intx_t i8_x_mul_x(const intx_t x, const intx_t y, dig_t* dest);

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
intx_t i8_x_mul_i(const intx_t x, int64_t y, dig_t* dest);

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
 *
 * Notes:
 *   - If `x` and `y` are positive, the following holds:
 *       `(-x) / y == -(x / y) == -(x / (-y)) == (-x) / (-y)`.
 */
intx_t i8_x_div_x(const intx_t x, const intx_t y, dig_t* dest);

/*
 * Divides a big integer by an int64.
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (int64 to divide by).
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least `_get_quotient_size(x, y)` digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x / y` (quotient of the division).
 *   - Otherwise, `intx_zero`.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following holds:
 *       `(-x) / y == -(x / y) == -(x / (-y)) == (-x) / (-y)`.
 */
intx_t i8_x_div_i(const intx_t x, int64_t y, dig_t* dest);

/*
 * Divides an int64 by a big integer.
 *
 * Parameters:
 *   - x: The dividend (int64 to be divided).
 *   - y: The divisor (big integer to divide by).
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least `_get_quotient_size(x, y)` digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x / y` (quotient of the division).
 *   - Otherwise, `intx_zero`.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following holds:
 *       `(-x) / y == -(x / y) == -(x / (-y)) == (-x) / (-y)`.
 */
intx_t i8_i_div_x(int64_t x, const intx_t y, dig_t* dest);

/*
 * Computes the remainder of the division of one big integer by another (`intx_t` instances).
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (big integer to divide by).
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (x.size) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x % y` (the remainder of `x / y`).
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following identity holds:
 *       `(-x) % y == -(x % y) == -(x % (-y)) == (-x) % (-y)`.
 */
intx_t i8_x_mod_x(const intx_t x, const intx_t y, dig_t* dest);

/*
 * Computes the remainder of the division of a big integer by an int64.
 *
 * Parameters:
 *   - x: The dividend (big integer to be divided).
 *   - y: The divisor (int64 to divide by).
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (x.size) (?????????????) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x % y` (the remainder of `x / y`).
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following identity holds:
 *       `(-x) % y == -(x % y) == -(x % (-y)) == (-x) % (-y)`.
 */
intx_t i8_x_mod_i(const intx_t x, int64_t y, dig_t* dest);

/*
 * Computes the remainder of the division of an int64 by a big integer.
 *
 * Parameters:
 *   - x: The dividend (int64 to be divided).
 *   - y: The divisor (big integer to divide by).
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least (x.size) (?????????????) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x % y` (the remainder of `x / y`).
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - If `x` and `y` are positive, the following identity holds:
 *       `(-x) % y == -(x % y) == -(x % (-y)) == (-x) % (-y)`.
 */
intx_t i8_i_mod_x(int64_t x, const intx_t y, dig_t* dest);

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
intx_t i8_negate(const intx_t x, dig_t* dest);

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
 *   - If 'x' is an Extreme Negative (i.e., ONLY the highest bit is set),
 *     caller is responsible to provide space for an additional digit beyond digits of 'x'
 */
intx_t i8_negate_self(intx_t x);

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
 *   - If 'x' is an Extreme Negative (i.e., ONLY the highest bit is set),
 *     caller is responsible to provide space for an additional digit beyond digits of 'x'
 */
intx_t i8_abs_self(intx_t x);

//----------------------------------------------------------------------------------------------------------
// Comparison operators:
// i8_is_equal, i8_is_less_eq, i8_is_greater_eq, i8_is_less, i8_is_greater,
// i8_is_zero, i8_is_positive, i8_is_negative, i8_is_min_negative, i8_is_pow2

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
bool i8_is_equal(const intx_t x, const intx_t y);

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
bool i8_is_less_eq(const intx_t x, const intx_t y);

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
inline bool i8_is_greater_eq(const intx_t x, const intx_t y) { return i8_is_less_eq(y, x); }

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
inline bool i8_is_less(const intx_t x, const intx_t y) { return !i8_is_less_eq(y, x); }

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
inline bool i8_is_greater(const intx_t x, const intx_t y) { return !i8_is_less_eq(x, y); }

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
bool i8_is_positive(const intx_t x);

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
bool i8_is_negative(const intx_t x);

// NEW FUNCTION
/*
 * Checks if a big integer is an Extreme Positive (i.e. ALL bits are set, but highest one).
 *
 * Parameters:
 *   - x: Big integer to check.
 *
 * Returns:
 *   - `true` if `x` is an Extreme Positive
 *   - Otherwise `false`.
 */
bool i8_is_max_positive(const intx_t x);

/*
 * Checks if a big integer is an Extreme Negative (i.e. ONLY highest bit of x is set).
 *
 * Parameters:
 *   - x: Big integer to check.
 *
 * Returns:
 *   - `true` if `x` is an Extreme Negative
 *   - Otherwise `false`.
 */
bool i8_is_min_negative(const intx_t x);

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
// Binary operations:
// i8_binary_and, i8_binary_or, i8_binary_xor, i8_binary_not, i8_binary_not_self,
// i8_left_shift, i8_left_shift_self, i8_right_shift, i8_right_shift_self

/*
 * Performs bitwise AND operation on two big integers (`intx_t` instances).
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least max(x.size, y.size) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x & y`.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - The result will have a `size` equal to the maximum `size` of the input arguments.
 *   - As per standard behavior, positive integers are extended beyond the highest bit with `0` bits,
 *     while negative integers are extended with `1` bits.
 */
intx_t i8_binary_and(const intx_t x, const intx_t y, dig_t* dest);

/*
 * Performs bitwise OR operation on two big integers (`intx_t` instances).
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least max(x.size, y.size) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x | y`.
 *   - Otherwise, returns `intx_zero`.
 *
 * Notes:
 *   - The result will have a `size` equal to the maximum `size` of the input arguments.
 *   - As per standard behavior, positive integers are extended beyond the highest bit with `0` bits,
 *     while negative integers are extended with `1` bits.
 */
intx_t i8_binary_or(const intx_t x, const intx_t y, dig_t* dest);

/*
 * Performs bitwise XOR operation on two big integers (`intx_t` instances).
 *
 * Parameters:
 *   - x: The first big integer.
 *   - y: The second big integer.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `x ^ y`.
 *   - Otherwise, returns `intx_zero`.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for at least max(x.size, y.size) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Notes:
 *   - The result will have a `size` equal to the maximum `size` of the input arguments.
 *   - As per standard behavior, positive integers are extended beyond the highest bit with `0` bits,
 *     while negative integers are extended with `1` bits.
 */
intx_t i8_binary_xor(const intx_t x, const intx_t y, dig_t* dest);

/*
 * Computes the bitwise NOT of a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to be negated bitwise.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for (x.size) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing `~x`.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t i8_binary_not(const intx_t x, dig_t* dest);

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
intx_t i8_binary_not_self(intx_t x);

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
 *   - Otherwise, returns `intx_zero`.
 */
intx_t i8_left_shift(const intx_t x, size_t bits, dig_t* dest);

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
intx_t i8_left_shift_self(intx_t x, size_t bits);

/*
 * Performs a right bitwise shift operation on a big integer (`intx_t` instance).
 *
 * Parameters:
 *   - x: Big integer to be shifted.
 *   - bits: Number of bits to shift.
 *   - dest: Pointer to the memory segment where the result will be stored.
 *           Caller must allocate space for (x.size) digits to store the result.
 *           Caller is responsible for managing the memory for `dest`.
 *
 * Returns:
 *   - If `intEx8_errno` is `INTEX8_OK`, a new `intx_t` instance representing the right-shifted value of `x`.
 *   - Otherwise, returns `intx_zero`.
 */
intx_t i8_right_shift(const intx_t x, size_t bits, dig_t* dest);

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
intx_t i8_right_shift_self(intx_t x, size_t bits);

//----------------------------------------------------------------------------------------------------------
// String conversion:
// i8_to_string, i8_from_string

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
intx_t i8_from_string(const char* str, size_t len, dig_t* dest);

#endif // __intex8_I8_H__
