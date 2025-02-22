/*
 *  File: util.h
 *  Description:
 *      Provides helper functions and macros for the intEx8 library.
 *      This file includes utility functions for memory size calculations, and digit conversions.
 *
 *  Author: Moham KazemiMoghaddam
 *  Created: 10-Jan-2025
 *  License: GNU General Public License v3.0
 */

#ifndef __intex8_UTIL_H__
#define __intex8_UTIL_H__

#include "intEx8.h"

#define _min(a, b) ((a) <= (b) ? (a) : (b))
#define _max(a, b) ((a) >= (b) ? (a) : (b))

/*
 * Computes the number of `intx_t` digits required to store value of an int64_t.
 *
 * Parameters:
 *   - x: The given int64_t.
 *
 * Returns:
 *   - The number of `intx_t` digits required to store value of an int64_t.
 */
const inline size_t _get_int_size(int64_t x)
{
	uni_t ux = { x };
	if (ux.b[1] == 0 && !_has_sign_bit(ux.b[0])) {
		return 1;
	}
	else if (ux.b[1] == INTEX8_DIGIT_MAX_VALUE() && _has_sign_bit(ux.b[0])) {
		return 1;
	}
	else if (ux.b[0] == INTEX8_DIGIT_MAX_VALUE() && ux.b[1] == INTEX8_DIGIT_EXTREME_POSITIVE()) { // x is an Extreme Positive intx
		return 3;
	}
	else if (ux.b[0] == 0 && ux.b[1] == INTEX8_DIGIT_SIGN_MASK()) { // x is an Extreme Negative intx
		return 3;
	}
	else {
		return 2;
	}
}

/*
 * Computes the number of `intx_t` digits required to store quotient of a division.
 *
 * Parameters:
 *   - x: The number of decimal digits.
 *   - y: The number of decimal digits.
 *
 * Returns:
 *   - The number of `intx_t` digits required to store quotient of a division.
 */
const inline size_t _get_quotient_size(intx_t x, intx_t y)
{
	return (x.size + 2 < y.size ? 0 : x.size + 2 - y.size);
}

/*
 * Computes the number of `intx_t` digits required to represent a given number of decimal digits.
 *
 * Parameters:
 *   - decimal_digit_count: The number of decimal digits.
 *
 * Returns:
 *   - The minimum number of `intx_t` digits required to store a number with `decimal_digit_count` decimal digits.
 *
 * Note:
 *   - This function may count 1 or 2 digits more than strictly necessary due to:
 *     1. Rounding up when applying `ceil()` in floating-point calculations.
 *     2. Reserving an extra digit to handle potential Extreme Negative values after conversion.
 */
const inline size_t _required_digit_count(size_t decimal_digit_count)
{
	return decimal_digit_count / (2.4 * sizeof(dig_t)) +
		1 + // Round-up as calling ceil()
		1;  // To handle sign of potential Extreme Negative result
}

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
 *   - This function may return 1, 2,.. bytes more than strictly necessary due to rounding up
 *      when applying `ceil()` in floating-point calculations.
 */
const inline size_t _required_decimal_count(size_t digit_count, bool negate)
{
	return (negate ? 1 : 0) + // for '-' sign
		(size_t)(digit_count * sizeof(dig_t) * (8 * 0.301029995664) + 1) +   // 0.301029995664 = log(2)
		1;  // Round-up as calling ceil()
}

#endif	// __intex8_UTIL_H__