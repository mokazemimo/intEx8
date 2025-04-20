/*
 *  File: intx.h
 *  Description:
 *      Declares `dig_t` and `intx_t` as the main data types for representing big integers.
 *      Provides type definitions and structures used in the intEx8 library.
 *
 *  Author: Moham KazemiMoghaddam
 *  Created: 07-Jan-2025
 *  License: GNU General Public License v3.0 (GPL-3.0)
 */

#ifndef __intex8__INTX_H__
#define __intex8__INTX_H__

#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <stdint.h>

#pragma pack(8)

/*
 * Defines the base unit (digit) used for representing big integers.
 *
 * Performance Considerations:
 *   - On most architectures, `uint32_t` ensures optimal performance for arithmetic operations.
 */
typedef uint32_t dig_t;

#define INTEX8_DIGIT_COUNT_IN_64BITS	2	// = sizeof(uint64_t) / sizeof(dig_t)
#define INTEX8_DIGIT_HI_BITS_UINT64		0xFFFFFFFF00000000LL
#define INTEX8_DIGIT_MAX_VALUE			0xFFFFFFFFLL
#define INTEX8_DIGIT_MAX_POSITIVE		0x7FFFFFFFLL
#define INTEX8_DIGIT_SIGN_MASK			0x80000000LL
#define INTEX8_DIGIT_BIT_WIDTH			32LL

typedef union {
	uint64_t a;
	dig_t b[INTEX8_DIGIT_COUNT_IN_64BITS];
} uni_t;

typedef uni_t uni64_t;

/*
 * Represents a big (arbitrary-precision) integer.
 *
 * Structure Members:
 *   - ptr: Pointer to an array of digits (`dig_t`), representing the big integer in base BASE=`2^INTEX8_DIGIT_BIT_WIDTH` (=4'294'967'296).
 *          The least significant digit is stored at `ptr[0]`, and the most significant digit at `ptr[size - 1]`.
 *   - size: Number of digits currently used in `ptr`.
 *           A value of `0` indicates that the integer is ZERO.
 *
 * Sign Representation:
 *   - The sign of `size` determines the sign:
 *       - size > 0 : Positive integer
 *       - size < 0 : Negative integer
 * 
 * Examples:
 *   - 1. A big integer stored as `{{27, 2}, 2}`, represents: 27 + 2*BASE = 8'589'934'619
 *   - 2. A big integer stored as `{{671, 1290}, -2}`, represents: -(671 + 1290*BASE) = -5'540'507'812'511
 *   - 3. A big integer stored as `{{7, 4, 9}, 3}`, represents: 7 + 4*BASE + 9*(BASE^2) = 166'020'696'680'565'833'735
 */
typedef int64_t cntx_t;
typedef struct _intx_t {
	dig_t* ptr;
	cntx_t size;
} intx_t;

#endif	// __intex8__INTX_H__