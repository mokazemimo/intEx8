/*
 *  File: intEx8.h
 *  Description:
 *      The main header file for the IntEx8 library, providing support for
 *      arbitrary-precision integer arithmetic.
 *
 *      This file includes:
 *      - `intex8.h`: Memory-managed interface for big integer operations.
 *      - `ix8.h`: Low-level interface where the caller provides memory.
 *      - Error handling definitions, including `intEx8_errno` and error codes.
 *
 *  Author: Moham KazemiMoghaddam
 *  Created: 13-Jan-2025
 *  License: GNU General Public License v3.0 (GPL-3.0)
 */

#ifndef __intex8_INTEX8_H__
#define __intex8_INTEX8_H__

#include "intx.h"

/* Internal function. Do NOT call this directly! */
inline const bool _has_sign_bit(dig_t a) {
	return (a & INTEX8_DIGIT_SIGN_MASK()) == INTEX8_DIGIT_SIGN_MASK();
}

/* Internal function. Do NOT call this directly! */
inline const dig_t _get_ext(intx_t a) {
	return (a.size > 0 && _has_sign_bit(a.ptr[a.size - 1])) ? INTEX8_DIGIT_MAX_VALUE() : 0;
}

/* Internal function. Do NOT call this directly! */
inline dig_t _right_shift_digit(dig_t x, size_t n) {
	return _has_sign_bit(x) ? ((INTEX8_DIGIT_HI_BITS_UINT64() | x) >> n) : (x >> n);
}

/*
 * Defines the maximum number of significant digits (excluding trailing zeros) supported for multiplication.
 *
 * Usage:
 *   - If the size of either operand in `ix8_x_mul_x` or `i8_x_mul_x` exceeds this limit, the function sets
 *     `INTEX8_ERR_MAX_MULTIPLICATION_DIGITS_EXCEEDED` and returns `intx_zero`.
 *   - Developers can increase this value to support larger integers if needed.
 *
 * Considerations:
 *   - Since `INTEX8_TOOM3_BUFFER_SIZE` depends on this value, increasing it will require a larger buffer
 *     for the Toom-3 multiplication algorithm (implemented in `_toom3_multiply`).
 *   - If modified, `INTEX8_TOOM3_MAX_RECURSION` MUST be updated accordingly.
 */
#define INTEX8_MAX_MULTIPLICATION_DIGITS	1024 // Supports multiplication of integers up to 32,768 bits (~9,864 decimal digits).

 /*
  * Defines the maximum number of digits allowed for the dividend.
  *
  * Usage:
  *   - If `ix8_x_div_x() or ix8_x_mod_x()`, `i8_x_div_x() or i8_x_mod_x()`, is called with a dividend exceeding this limit,
  *     the function reports `INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED` and returns intx_zero.
  *   - This constraint helps prevent unnecessary memory allocations and performance degradation.
  *
  * Note:
  *   - The actual value of this macro can be set based on system limitations and
  *     performance considerations.
  */
#define INTEX8_MAX_DIVIDEND_DIGITS		2048 // Supports division of integers up to 65,536 bits (~19,728 decimal digits).

  // Error codes
#define INTEX8_OK                  0
#define INTEX8_ERR_DIVISION_BY_ZERO						1
#define INTEX8_ERR_MEMORY_ALLOCATION_FAILED				2
#define INTEX8_ERR_INVALID_DECIMAL_STRING				3
#define INTEX8_ERR_CANNOT_NEGATE_EXTREME_NEGATIVE		4
#define INTEX8_ERR_MAX_MULTIPLICATION_DIGITS_EXCEEDED	5
#define INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED			6

#ifdef __cplusplus
extern "C" {
#endif
	extern int intEx8_errno;
	extern const intx_t intx_zero;
#ifdef __cplusplus
}
#endif

inline int intEx8_init()
{
	intEx8_errno = INTEX8_OK;
	return intEx8_errno;
}

#include "i8.h"
#include "ix8.h"

#endif	// __intex8_INTEX8_H__