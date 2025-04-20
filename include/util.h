/*
 *  File: util.h
 *  Description:
 *      Provides helper functions and macros for the intEx8 library.
 *      This file includes utility functions for memory size calculations, and digit conversions.
 *
 *  Author: Moham KazemiMoghaddam
 *  Created: 10-Jan-2025
 *  License: GNU General Public License v3.0 (GPL-3.0)
 */

#ifndef __intex8_UTIL_H__
#define __intex8_UTIL_H__

#include "intEx8.h"

#define _min(a, b)	((a) <= (b) ? (a) : (b))
#define _max(a, b)	((a) >= (b) ? (a) : (b))
#define _abs(a)		((a) >= 0   ? (a) : -(a))
#define _sgn(a)		((a) >  0   ?  1  : ((a) < 0 ? -1 : 0))

/*
 * Computes the number of `dig_t` digits required to store value of an int64_t.
 *
 * Parameters:
 *   - x: Given int64_t.
 *
 * Returns:
 *   - The number of `dig_t` digits required to store value of x.
 */
#define _required_digits_for_int64(x)	(x == 0 ? 0 : _abs(x) <= INTEX8_DIGIT_MAX_VALUE ? 1 : INTEX8_DIGIT_COUNT_IN_64BITS)

/*
 * Computes the number of `dig_t` digits required to store sum of two  big integers.
 *
 * Parameters:
 *   - x: First big integer.
 *   - y: Second big integer.
 *
 * Returns:
 *   - The number of `dig_t` digits required to store sum of parameters.
 */
cntx_t _required_digits_for_sum(intx_t x, intx_t y);

/*
 * Computes the number of `dig_t` digits required to store quotient of a division.
 *
 * Parameters:
 *   - m: Digit count of dividend.
 *   - n: Digit count of divisor.
 *
 * Returns:
 *   - The number of `dig_t` digits required to store quotient of a division.
 */
#define _get_quotient_size(m, n)	((m) + 1 < (n) ? 0 : (m) + 1 - (n))

/*
 * Computes the number of `dig_t` digits required to represent a given number of decimal digits.
 *
 * Parameters:
 *   - decimal_digit_count: The number of decimal digits.
 *
 * Returns:
 *   - The minimum number of `dig_t` digits required to store a number with `decimal_digit_count` decimal digits.
 *
 * Note:
 *   - This function may count 1 digit more than strictly necessary due to rounding up in floating-point calculations.
 */
#define _required_digit_count(decimal_digit_count)	((decimal_digit_count) / (2.4 * sizeof(dig_t)) + 1)

/*
 * Computes the number of decimal digits required to represent a given big integer `x`.
 *
 * Parameters:
 *   - digit_count: The number of `dig_t` digits in `x`.
 *   - negate: `true` if `x` is negative, `false` otherwise.
 *
 * Returns:
 *   - Number of bytes required to store the decimal representation of `x`.
 *
 * Note:
 *   - This function may return 1, 2,.. bytes more than strictly necessary due to rounding up in floating-point calculations.
 */
#define _required_decimal_count(digit_count, negate)	((negate ? 1 : 0) + (size_t)((digit_count) * sizeof(dig_t) * (8 * 0.301029995664) + 1) + 1)

#endif	// __intex8_UTIL_H__