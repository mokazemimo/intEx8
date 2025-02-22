/*
 *  File: ix8.c
 *  Description:
 *      Implements the `ix8` interface functions of the intEx8 library for big integers.
 *      Contains function definitions for arithmetic, comparison, and string conversions.
 *
 *  Notes:
 *      - `intex8` interface is the primary interface of the intEx8 library and is recommended for general use.
 *      - Caller is responsible for freeing NEW `intx_t` instances returned by functions using `ix8_free()`.
 *      - Caller must also free strings (`char *`) returned by `ix8_to_string()` using `ix8_free_string()`.
 *      - `ix8` functions internally call `i8` functions for computation.
 */

#include <string.h>	// strlen()
#include "util.h"
#include "ix8.h"

/* Internal function. Do NOT call this directly! */
static intx_t _call_ix8_1param_interface(intx_t x, intx_t(*ix8_func)(intx_t, dig_t*))
{
	if(x.size == 0)
		return intx_zero;

	dig_t* buf = (dig_t*)malloc(x.size * sizeof(dig_t));
	if (buf == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	return ix8_func(x, buf);
}

/* Internal function. Do NOT call this directly! */
static intx_t _call_ix8_1param_interface_special(intx_t x, intx_t(*ix8_func)(intx_t, dig_t*))
{
	if (i8_is_zero(x))
		return intx_zero;

	size_t size = x.size;
	if (x.ptr[x.size - 1] == INTEX8_DIGIT_SIGN_MASK())
		++size;

	dig_t* buf = (dig_t*)malloc(size * sizeof(dig_t));
	if (buf == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	return ix8_func(x, buf);
}

/* Internal function. Do NOT call this directly! */
static intx_t _call_ix8_2param_interface(intx_t x, intx_t y, size_t size, intx_t (*ix8_func)(intx_t, intx_t, dig_t *))
{
	if (size == 0)
		return intx_zero;

	x = ix8_trim(x);
	y = ix8_trim(y);
	dig_t* buf = (dig_t*)malloc(size * sizeof(dig_t));
	if (buf == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
			return intx_zero;
	}
	return ix8_func(x, y, buf);
}

/* Internal function. Do NOT call this directly! */
static intx_t _call_ix8_bitwise_interface(intx_t x, size_t bits, size_t size, intx_t(*ix8_func)(intx_t, size_t, dig_t*))
{
	dig_t* buf = (dig_t*)malloc(size * sizeof(dig_t));
	if (buf == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	return ix8_func(x, bits, buf);
}

/*
 * Initializes the intex8 interface.
 */
//int intex8_init()
//{
//	intEx8_errno = INTEX8_OK;
//	return intEx8_errno;
//}

/*
 * Creates a copy of x and returns the result.
 * Caller is responsible for freeing it using `ix8_free()`.
 */
intx_t ix8_copy(intx_t x)
{
	return _call_ix8_1param_interface(ix8_trim(x), i8_copy);
}

/*
 * Creates a big integer equal to an int64 value and returns the result.
 * Caller is responsible for freeing it using `ix8_free()`.
 */
intx_t ix8_from_int(int64_t x)
{
	dig_t* buf = (dig_t*)malloc(_get_int_size(x) * sizeof(dig_t));
	if (buf == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	return i8_from_int(x, buf);
}

//-------------------------------------------------------------------------------------------------------
// Arithmetic operations
/*
 * Adds two big integers `x`, `y` and returns the result (x + y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_x_add_x(intx_t x, intx_t y)
{
	return _call_ix8_2param_interface(x, y, _max(x.size, y.size) + 1, i8_x_add_x);
}

/*
 * Adds one big integer `x`, by an int64 `y` and returns the result (x + y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_x_add_i(intx_t x, int64_t y)
{
	return ix8_x_add_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS });
}

/*
 * Subtracts one big integer `y` from another `x` and returns the result (x - y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_x_sub_x(intx_t x, intx_t y)
{
	return _call_ix8_2param_interface(x, y, _max(x.size, y.size) + 1, i8_x_sub_x);
}

/*
 * Subtracts one int64 `y` from a big integer `x` and returns the result (x - y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_x_sub_i(intx_t x, int64_t y)
{
	return ix8_x_sub_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS });
}

/*
 * Subtracts one big integer `y` from an int64 `x` and returns the result (x - y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_i_sub_x(int64_t x, intx_t y)
{
	return ix8_x_sub_x((intx_t) { (dig_t*)&x, INTEX8_DIGIT_COUNT_IN_64BITS }, y);
}

/*
 * Multiplies two big integers `x`, `y` and returns the result (x * y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_x_mul_x(intx_t x, intx_t y)
{
	if (i8_is_zero(x) || i8_is_zero(y))
		return intx_zero;

	if (_max(x.size, y.size) > INTEX8_MAX_MULTIPLICATION_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_MULTIPLICATION_DIGITS_EXCEEDED;
		return intx_zero;
	}

	return _call_ix8_2param_interface(x, y, (x.size + y.size), i8_x_mul_x);
}

/*
 * Multiplies one big integer `x` by an int64 `y` and returns the result (x * y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_x_mul_i(intx_t x, int64_t y)
{
	return ix8_x_mul_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS });
}

/*
 * Devides one big integer `x` by another `y` and returns the quotient (x / y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_x_div_x(intx_t x, intx_t y)
{
	x = ix8_trim(x);
	y = ix8_trim(y);

	if (i8_is_zero(y)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return intx_zero;
	}
	if (i8_is_zero(x))
		return intx_zero;

	if (x.size > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
		return intx_zero;
	}
	size_t size = _get_quotient_size(x, y);
	return _call_ix8_2param_interface(x, y, size, i8_x_div_x);
}

/*
 * Devides one big integer `x` by a int64 `y` and returns the quotient (x / y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_x_div_i(intx_t x, int64_t y)
{
	return ix8_x_div_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS });
}

/*
 * Devides one int64 `x` by a big integer `y` and returns the quotient (x / y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_i_div_x(int64_t x, intx_t y)
{
	return ix8_x_div_x((intx_t) { (dig_t*)&x, INTEX8_DIGIT_COUNT_IN_64BITS }, y);
}

/*
 * Computes the remainder of the division of one big integer `x` by another `y` (x % y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_x_mod_x(intx_t x, intx_t y)
{
	if (i8_is_zero(y)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return intx_zero;
	}
	if (i8_is_zero(x))
		return intx_zero;

	return _call_ix8_2param_interface(x, y, y.size, i8_x_mod_x);
}

/*
 * Computes the remainder of the division of one big integer `x` by an int64 `y` (x % y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_x_mod_i(intx_t x, int64_t y)
{
	return ix8_x_mod_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS });
}

/*
 * Computes the remainder of the division of an int64 `x` by a big integer `y` (x % y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_i_mod_x(int64_t x, intx_t y)
{
	return ix8_x_mod_x((intx_t) { (dig_t*)&x, INTEX8_DIGIT_COUNT_IN_64BITS }, y);
}

/*
 * Computes the negation of a big integer `x` (-x).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_negate(intx_t x)
{
	return _call_ix8_1param_interface_special(ix8_trim(x), i8_negate);
}

/*
 * Computes the negation of a big integer `x` in place (x = -x).
 * If `x` represents an Extreme Negative value, reports INTEX8_ERR_CANNOT_NEGATE_EXTREME_NEGATIVE.
 */
intx_t ix8_negate_self(intx_t x)
{
	if (i8_is_min_negative(x))
	{
		intEx8_errno = INTEX8_ERR_CANNOT_NEGATE_EXTREME_NEGATIVE;
		return intx_zero;
	}
	return i8_negate_self(x);
}

/*
 * Computes the absolute value of a big integer `x` (|x|).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_abs(intx_t x)
{
	return _call_ix8_1param_interface_special(ix8_trim(x), i8_abs);
}

/*
 * Computes the absolute value of a big integer `x` in place (x = |x|).
 * If `x` represents an Extreme Negative value, reports INTEX8_ERR_CANNOT_NEGATE_EXTREME_NEGATIVE.
 */
intx_t ix8_abs_self(intx_t x)
{
	if (i8_is_min_negative(x))
	{
		intEx8_errno = INTEX8_ERR_CANNOT_NEGATE_EXTREME_NEGATIVE;
		return intx_zero;
	}
	return i8_abs_self(x);
}

/*
 * Performs bitwise AND operation on two big integers `x`, `y` and returns the result (x & y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_binary_and(intx_t x, intx_t y)
{
	return _call_ix8_2param_interface(x, y, x.size, i8_binary_and);
}

/*
 * Performs bitwise OR operation on two big integers `x`, `y` and returns the result (x | y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_binary_or(intx_t x, intx_t y)
{
	return _call_ix8_2param_interface(x, y, x.size, i8_binary_or);
}

/*
 * Performs bitwise XOR operation on two big integers `x`, `y` and returns the result (x ^ y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_binary_xor(intx_t x, intx_t y)
{
	return _call_ix8_2param_interface(x, y, x.size, i8_binary_xor);
}

/*
 * Computes the bitwise NOT of a big integer `x` (~x).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_binary_not(intx_t x)
{
	return _call_ix8_1param_interface(x, i8_binary_not);
}

/*
 * Computes the bitwise NOT of a big integer `x` in place (x = ~x).
 */
intx_t ix8_binary_not_self(intx_t x)
{
	return i8_binary_not_self(x);
}

/*
 * Performs a left bitwise shift operation on a big integer `x` and returns the result (x << bits).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_left_shift(intx_t x, size_t bits)
{
	return _call_ix8_bitwise_interface(x, bits, x.size, i8_left_shift);
}

/*
 * Performs a left bitwise shift operation on a big integer `x` in place (x <<= bits).
 */
intx_t ix8_left_shift_self(intx_t x, size_t bits)
{
	return i8_left_shift_self(x, bits);
}

/*
 * Performs a right bitwise shift operation on a big integer `x` and returns the result (x >> bits).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_right_shift(intx_t x, size_t bits)
{
	//if (x.size <= bits / INTEX8_DIGIT_BIT_WIDTH())
	//	return intx_zero;
	//return _call_ix8_bitwise_interface(x, bits, x.size - bits / INTEX8_DIGIT_BIT_WIDTH(), i8_right_shift);
	return _call_ix8_bitwise_interface(x, bits, x.size, i8_right_shift);
}

/*
 * Performs a right bitwise shift operation on a big integer `x` in place (x >>= bits).
 */
intx_t ix8_right_shift_self(intx_t x, size_t bits)
{
	return i8_right_shift_self(x, bits);
}

/*
 * Converts a big integer to a string representation and returns the result (intx_t to ascii).
 * Caller is responsible for freeing the result using `ix8_free_string()`.
 */
char* const ix8_to_string(intx_t x)
{
	x = ix8_trim(x);

	if (i8_is_zero(x)) {
		char* const z = (char *)malloc(2);
		if (z == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
			return NULL;
		}
		z[0] = '0';
		z[1] = '\0';
		return z;
	}

	int negate = 0;
	if(i8_is_min_negative(x))
		negate = 1;
	else if (i8_is_negative(x)) {
		negate = 2;
		i8_negate_self(x);
	}

	size_t out_size = _required_decimal_count(x.size, negate != 0);
	
	char* const str = (char*)calloc(out_size, sizeof(char));
	if (str == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return NULL;
	}

	char* ptr = i8_to_string(x, str, out_size);

	if (negate != 0) {
		if(negate == 2)
			i8_negate_self(x);
		*(--ptr) = '-';
	}
	if (ptr > str) {
		memcpy(str, ptr, out_size - (ptr - str));
	}

	return str;
}

/*
 * Parses a big integer from a string representation and returns the result (ascii to intx_t).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_from_string(const char* in_str)
{
	bool negate = false;
	if (*in_str == '-') {
		negate = true;
		++in_str;
	}
	else if (*in_str == '+')
		++in_str;
	while (*in_str == '0')
		++in_str;

	if (*in_str == '\0')
		return intx_zero;

	for (char* p = in_str; *p; ++p)
		if (*p < '0' || *p > '9') {
			intEx8_errno = INTEX8_ERR_INVALID_DECIMAL_STRING;
			return intx_zero;
		}

	size_t in_len = strlen(in_str);
	size_t size = _required_digit_count(in_len);
	dig_t* buf = (dig_t*)calloc(size, sizeof(dig_t));
	if (buf == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	intx_t x = i8_from_string(in_str, in_len, buf);

	x.size = size;
	if ((negate && i8_is_positive(x)) || (!negate && i8_is_negative(x)))
		return i8_negate_self(x);
	else
		return x;
}
