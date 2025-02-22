/*
 *  File: intx.h
 *  Description:
 *      Declares `dig_t` and `intx_t` as the main data types for representing big integers.
 *      Provides type definitions and structures used throughout the intEx8 library.
 *
 *  Author: Moham KazemiMoghaddam
 *  Created: 07-Jan-2025
 *  License: GNU General Public License v3.0
 */

#ifndef __intex8__INTX_H__
#define __intex8__INTX_H__

#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <stdint.h>

/*
 * Defines the base unit (digit) used for representing large integers.
 *
 * Performance Considerations:
 *   - On most architectures, `uint32_t` ensures optimal performance for arithmetic operations.
 */
typedef uint32_t dig_t;

#define INTEX8_DIGIT_COUNT_IN_64BITS		2	// sizeof(uint64_t) / sizeof(dig_t)
inline const uint64_t INTEX8_DIGIT_HI_BITS_UINT64() { return 0xFFFFFFFF00000000LL; }
inline const uint64_t INTEX8_DIGIT_MAX_VALUE() { return 0xFFFFFFFFL; }
inline const uint64_t INTEX8_DIGIT_EXTREME_POSITIVE() { return 0x7FFFFFFFL; }
inline const uint64_t INTEX8_DIGIT_SIGN_MASK() { return 0x80000000L; }
inline const uint64_t INTEX8_DIGIT_BIT_WIDTH() { return 32; }

typedef union {
	uint64_t a;
	dig_t b[INTEX8_DIGIT_COUNT_IN_64BITS];
} uni_t;

typedef uni_t uni64_t;

/*
 * Represents a dynamically allocated arbitrary-precision integer.
 *
 * Structure Members:
 *   - ptr: Pointer to an array of digits (`dig_t`), representing the big integer in base 2^DIGIT_BITS.
 *          The least significant digit is stored at `ptr[0]`, and the most significant digit at `ptr[size - 1]`.
 *   - size: The number of digits currently used in `ptr`.
 *           A value of `0` indicates that the integer is ZERO.
 *
 * Sign Representation:
 *   - The most significant bit (MSB) of the most significant digit determines the sign:
 *       - 0: Positive integer
 *       - 1: Negative integer
 * 
 * Example:
 *   - If `x` is a big integer stored as `{{0xFFFFFFFE, 0x00000002}, 2}`, it represents:
 *     (2 * 2^32) + (0xFFFFFFFE) = 8589934590
 *
 */
typedef struct _intx_t {
	dig_t* ptr;
	union {
		size_t size;
		struct {
			uint32_t count;
			int32_t point;
		};
	};
} intx_t;

#endif	// __intex8__INTX_H__