/*
 *  File: ix8.c
 *  Description:
 *      Implements the `ix8` interface functions of the intEx8 library for big integers.
 *      Contains function definitions for arithmetic, comparison, and string conversions.
 *
 *  Notes:
 *      - `ix8` interface is the primary interface of the intEx8 library and is recommended for general use.
 *      - Caller is responsible for freeing NEW `intx_t` instances returned by functions using `ix8_free()`.
 *      - Caller must also free strings (`char *`) returned by `ix8_copy_to_s()` using `ix8_free_s()`.
 *      - `ix8` functions internally call `i8` functions for computation.
 */

#include <string.h>	// strlen()
#include "util.h"
#include "ix8.h"

/* Internal function. Do NOT call this directly! */
static intx_t _call_i8_1param_interface(intx_t x, intx_t(*ix8_func)(intx_t, dig_t*))
{
	if(x.size == 0)
		return intx_zero;

	dig_t* buf = (dig_t*)malloc(_abs(x.size) * sizeof(dig_t));
	if (buf == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	return ix8_func(x, buf);
}

/* Internal function. Do NOT call this directly! */
static intx_t _call_i8_1param_interface_special(intx_t x, intx_t(*ix8_func)(intx_t, dig_t*))
{
	if (i8_is_zero(x))
		return intx_zero;

	cntx_t size = x.size;
	if (x.ptr[x.size - 1] == INTEX8_DIGIT_SIGN_MASK)
		++size;

	dig_t* buf = (dig_t*)malloc(size * sizeof(dig_t));
	if (buf == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	return ix8_func(x, buf);
}

/* Internal function. Do NOT call this directly! */
static intx_t _call_i8_2param_interface(intx_t x, intx_t y, cntx_t size, intx_t (*ix8_func)(intx_t, intx_t, dig_t *))
{
	//if (size == 0)
	//	return intx_zero;
	if (size == 0) {
		return ix8_func(x, y, NULL);
	}
	else {
		x = i8_trim(x);
		y = i8_trim(y);
		dig_t* buf = (dig_t*)malloc(size * sizeof(dig_t));
		if (buf == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
			return intx_zero;
		}
		return ix8_func(x, y, buf);
	}
}

/* Internal function. Do NOT call this directly! */
static intx_t _call_i8_int64_interface(intx_t x, int64_t bits, cntx_t size, intx_t(*ix8_func)(intx_t, int64_t, dig_t*))
{
	if (size == 0) {
		return ix8_func(x, bits, NULL);
	}
	else {
		dig_t* buf = (dig_t*)malloc(size * sizeof(dig_t));
		if (buf == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
			return intx_zero;
		}
		return ix8_func(x, bits, buf);
	}
}

/*
 * Creates a copy of x and returns the result.
 * Caller is responsible for freeing it using `ix8_free()`.
 */
intx_t ix8_copy(const intx_t x)
{
	return _call_i8_1param_interface(i8_trim(x), i8_copy);
}

/*
 * Creates a big integer equal to an integer value and returns the result.
 * Caller is responsible for freeing it using `ix8_free()`.
 */
intx_t ix8_copy_i(int64_t x)
{
	if (x == 0) {
		return intx_zero;
	}
	else {
		dig_t* buf = (dig_t*)malloc(_required_digits_for_int64(x) * sizeof(dig_t));
		if (buf == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
			return intx_zero;
		}
		return i8_copy_i(x, buf);
	}
}

//-------------------------------------------------------------------------------------------------------
// Arithmetic operations

/*
 * Adds two big integers `x`, `y` and returns the result (x + y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_add(const intx_t x, const intx_t y)
{
	return _call_i8_2param_interface(x, y, _required_digits_for_sum(i8_trim(x), i8_trim(y)), i8_add);
}

/*
 * Adds one big integer `x` by an integer `y` and returns the result (x + y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_add_i(const intx_t x, int64_t y)
{
	if (y < 0) {
		y = -y;
		return ix8_add(x, (intx_t) { (dig_t*)&y, -_required_digits_for_int64(y) });
	}
	else {
		return ix8_add(x, (intx_t) { (dig_t*)&y, _required_digits_for_int64(y) });
	}
}

/*
 * Adds one big integer `x` by a big integer `y` in place (x += y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
void ix8_addeq(intx_t *x, intx_t y)
{
	cntx_t size = _required_digits_for_sum(*x, y);
	if (size <= _abs(x->size)) {
		*x = i8_add(i8_trim(*x), y, x->ptr);
	}
	else {
		dig_t* ptr = (dig_t*)realloc(x->ptr, size * sizeof(dig_t));
		if (ptr == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		}
		else {
			*x = i8_add(i8_trim(*x), y, ptr);
		}
	}
}

/*
 * Adds one big integer `x` by an integer `y` in place (x += y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
void ix8_addeq_i(intx_t *x, int64_t y)
{
	if (y < 0) {
		y = -y;
		ix8_addeq(x, (intx_t) { (dig_t*)&y, -_required_digits_for_int64(y) });
	}
	else {
		ix8_addeq(x, (intx_t) { (dig_t*)&y, _required_digits_for_int64(y) });
	}
}

/*
 * Subtracts one big integer `y` from another `x` and returns the result (x - y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_sub(const intx_t xi, const intx_t yi)
{
	intx_t x = i8_trim(xi);
	intx_t y = i8_trim(yi);
	y.size = -y.size;
	return _call_i8_2param_interface(x, y, _required_digits_for_sum(x, y), i8_add);
}

/*
 * Subtracts one integer `y` from a big integer `x` and returns the result (x - y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_sub_i(const intx_t x, int64_t y)
{
	if (y < 0) {
		y = -y;
		return ix8_sub(x, (intx_t) { (dig_t*)&y, -_required_digits_for_int64(y) });
	}
	else {
		return ix8_sub(x, (intx_t) { (dig_t*)&y, _required_digits_for_int64(y) });
	}
}

/*
 * Subtracts one big integer `y` from an integer `x` and returns the result (x - y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_i_sub(int64_t x, const intx_t y)
{
	if (x < 0) {
		x = -x;
		return ix8_sub((intx_t) { (dig_t*)&x, -_required_digits_for_int64(x) }, y);
	}
	else {
		return ix8_sub((intx_t) { (dig_t*)&x, _required_digits_for_int64(x) }, y);
	}
}

/*
 * Subtracts a big integer `y` from a big integer `x` in place (x -= y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
void ix8_subeq(intx_t *x, intx_t y)
{
	y = i8_trim(y);
	y.size = -y.size;
	cntx_t size = _required_digits_for_sum(*x, y);

	if (size <= _abs(x->size)) {
		*x = i8_add(i8_trim(*x), y, x->ptr);
	}
	else {
		dig_t* ptr = (dig_t*)realloc(x->ptr, size * sizeof(dig_t));
		if (ptr == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		}
		else {
			*x = i8_add(i8_trim(*x), y, ptr);
		}
	}
}

/*
 * Subtracts an int64 `y` from a big integer `x` by in place (x -= y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
void ix8_subeq_i(intx_t *x, int64_t y)
{
	if (y < 0) {
		y = -y;
		ix8_addeq(x, (intx_t) { (dig_t*)&y, _required_digits_for_int64(y) });
	}
	else {
		ix8_addeq(x, (intx_t) { (dig_t*)&y, -_required_digits_for_int64(y) });
	}
}

/*
 * Multiplies two big integers `x`, `y` and returns the result (x * y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_mul(const intx_t x, const intx_t y)
{
	if (ix8_is_zero(x) || ix8_is_zero(y))
		return intx_zero;

	cntx_t x_size = _abs(x.size);
	cntx_t y_size = _abs(y.size);
	if (_max(x_size, y_size) > INTEX8_MAX_MULTIPLICATION_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_MULTIPLICATION_DIGITS_EXCEEDED;
		return intx_zero;
	}

	return _call_i8_2param_interface(x, y, (x_size + y_size), i8_mul);
}

/*
 * Multiplies one big integer `x` by an integer `y` and returns the result (x * y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_mul_i(const intx_t x, int64_t y)
{
	return _call_i8_int64_interface(x, y, _abs(x.size) + (_abs(y) <= INTEX8_DIGIT_MAX_VALUE ? 1 : INTEX8_DIGIT_COUNT_IN_64BITS), i8_mul_i);
}

/*
 * Multiplies one big integer `x` by `sgn(y) * 2^|y|` and returns the result.
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_mul_p2(intx_t x, int64_t y)
{
	return _call_i8_int64_interface(x, y, _abs(x.size) + (_abs(y) + INTEX8_DIGIT_BIT_WIDTH - 1) / INTEX8_DIGIT_BIT_WIDTH, i8_mul_p2);
}

/*
 * Multiplies one big integer `x` by a big integer `y` in place (x *= y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
void ix8_muleq(intx_t *x, intx_t y)
{
	cntx_t x_size = _abs(x->size);
	cntx_t y_size = _abs(y.size);
	if (_max(x_size, y_size) > INTEX8_MAX_MULTIPLICATION_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_MULTIPLICATION_DIGITS_EXCEEDED;
	}
	else {
		dig_t x_buf[INTEX8_MAX_MULTIPLICATION_DIGITS];
		intx_t x0 = i8_copy(*x, x_buf);
		dig_t* ptr = (dig_t*)realloc(x->ptr, (x_size + y_size) * sizeof(dig_t));
		if (ptr == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		}
		else {
			*x = i8_mul(x0, y, ptr);
		}
	}
}

/*
 * Multiplies one big integer `x` by an integer `y` in place (x *= y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
void ix8_muleq_i(intx_t *x, int64_t y)	// x *= y
{
	if (y < 0) {
		y = -y;
		ix8_muleq(x, (intx_t) { (dig_t*)&y, -_required_digits_for_int64(y) });
	}
	else {
		ix8_muleq(x, (intx_t) { (dig_t*)&y, _required_digits_for_int64(y) });
	}
}

/*
 * Multiplies one big integer `x` by `sgn(y) * 2^|y|` in place (x *= sgn(y) * 2^|y|).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
void ix8_muleq_p2(intx_t *xi, int64_t y)
{
	if (y == 0)
		return;

	intx_t x = i8_trim(*xi);
	
	cntx_t x_size = _abs(x.size);
	if (x_size > INTEX8_MAX_MULTIPLICATION_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_MULTIPLICATION_DIGITS_EXCEEDED;
	}
	else {
		dig_t x_buf[INTEX8_MAX_MULTIPLICATION_DIGITS];
		intx_t x0 = i8_copy(x, x_buf);
		dig_t* ptr = (dig_t*)realloc(x.ptr, (x_size + (_abs(y) + INTEX8_DIGIT_BIT_WIDTH - 1) / INTEX8_DIGIT_BIT_WIDTH) * sizeof(dig_t));
		if (ptr == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		}
		else {
			*xi = i8_mul_p2(x0, y, ptr);
		}
	}
}

/*
 * Devides one big integer `x` by another `y` and returns the quotient (x / y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_div(const intx_t xi, const intx_t yi)
{
	intx_t x = i8_trim(xi);
	intx_t y = i8_trim(yi);

	cntx_t size = _get_quotient_size(_abs(x.size), _abs(y.size));
	if (i8_is_zero(y)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return intx_zero;
	}
	else if (size == 0) { // (i8_is_zero(x)) {
		return intx_zero;
	}
	else if (_abs(x.size) > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
		return intx_zero;
	}
	else {
		dig_t dividend_buf[INTEX8_MAX_DIVIDEND_DIGITS];
		x = i8_copy(x, dividend_buf);
		return _call_i8_2param_interface(x, y, size, i8_div);
	}
}

/*
 * Devides one big integer `x` by an integer `y` and returns the quotient (x / y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_div_i(const intx_t xi, int64_t y)
{
	if (y == 0) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return intx_zero;
	}
	intx_t x = i8_trim(xi);
	if (i8_is_zero(x)) {
		return intx_zero;
	}
	else if (_abs(x.size) > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
		return intx_zero;
	}
	else {
		dig_t divisor_buf[INTEX8_DIGIT_COUNT_IN_64BITS];
		return ix8_div(x, i8_copy_i(y, divisor_buf));
	}
}

/*
 * Divides an integer by a big integer and returns the quotient (x / y).
 */
int64_t ix8_i_div(int64_t x, const intx_t yi)
{
	intx_t y = i8_trim(yi);
	cntx_t sign = _sgn(yi.size);
	y.size = _abs(y.size);
	if (y.size == 0) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return 0;
	}
	else if (y.size == 1) {
		return sign * x / (uint32_t)y.ptr[0];
	}
	else if (y.size == INTEX8_DIGIT_COUNT_IN_64BITS) {
		return sign * (x / *(uint64_t *)y.ptr);
	}
	else {
		return 0;
	}
}

/*
 * Devides one big integer `x` by `sgn(y) * 2^y` and returns the quotient (x / (sgn(y) * 2^y)).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_div_p2(intx_t x, int64_t y)
{
	if (y == 0) {
		return ix8_copy(x);
	}
	else if (_abs(x.size) * INTEX8_DIGIT_BIT_WIDTH <= _abs(y)) {
		return intx_zero;
	}
	else {
		return _call_i8_int64_interface(x, y,
			(_abs(x.size) * INTEX8_DIGIT_BIT_WIDTH - _abs(y) + INTEX8_DIGIT_BIT_WIDTH - 1) / INTEX8_DIGIT_BIT_WIDTH, i8_div_p2);
	}
}

/*
 * Devides one big integer `x` by a big integer `y` in place (x /= y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
void ix8_diveq(intx_t *xi, const intx_t yi)
{
	intx_t x = i8_trim(*xi);
	intx_t y = i8_trim(yi);

	cntx_t size = _get_quotient_size(_abs(x.size), _abs(y.size));
	if (i8_is_zero(y)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
	}
	else if (size == 0) {
		ix8_free(x);
		*xi = intx_zero;
	}
	else if (_abs(x.size) > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
	}
	else {
		dig_t dividend_buf[INTEX8_MAX_DIVIDEND_DIGITS];
		intx_t d = i8_copy(x, dividend_buf);
		dig_t *ptr = realloc(x.ptr, size * sizeof(dig_t));
		if (ptr == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		}
		else {
			*xi = i8_div(d, y, ptr);
		}
	}
}

/*
 * Devides one big integer `x` by an int64 `y` in place (x /= y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
void ix8_diveq_i(intx_t *xi, int64_t y)
{
	intx_t x = i8_trim(*xi);
	dig_t divisor_buf[INTEX8_DIGIT_COUNT_IN_64BITS + 1] = { 0 };
	intx_t e = i8_copy_i(y, divisor_buf);
	
	cntx_t size = _get_quotient_size(_abs(x.size), _abs(e.size));
	if (y == 0) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
	}
	else if (size == 0) {
		ix8_free(x);
		*xi = intx_zero;
	}
	else if (_abs(x.size) > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
	}
	else {
		dig_t dividend_buf[INTEX8_MAX_DIVIDEND_DIGITS];
		intx_t d = i8_copy(x, dividend_buf);
		dig_t* ptr = realloc(x.ptr, size * sizeof(dig_t));
		if (ptr == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		}
		else {
			*xi = i8_div(d, e, ptr);
		}
	}
}

/*
 * Divides an integer by a big integer in place.
 */
void ix8_i_diveq(int64_t* x, const intx_t yi)
{
	int64_t d = ix8_i_div(*x, yi);
	if(intEx8_errno == INTEX8_OK)
		*x = d;
}

/*
 * Devides one big integer `x` by `sgn(y) * (2^|y|)` in place.
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
void ix8_diveq_p2(intx_t *xi, int64_t y)
{
	if (y == 0) {	// division by 2^0 (=1)
		return;
	}
	else if (_abs(xi->size) > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
	}
	else if (_abs(xi->size) * INTEX8_DIGIT_BIT_WIDTH <= _abs(y)) {
		ix8_free(*xi);
		*xi = intx_zero;
	}
	else {
		dig_t xbuf[INTEX8_MAX_DIVIDEND_DIGITS], *ptr = NULL;

		intx_t m = i8_trim(i8_div_p2(i8_trim(*xi), y, xbuf));
		if (m.size == 0) {
			ix8_free(*xi);
			*xi = intx_zero;
		}
		else if ((ptr = (dig_t*)realloc(xi->ptr, _abs(m.size) * sizeof(dig_t))) == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		}
		else {
			xi->size = m.size;
			xi->ptr = ptr;
			memcpy(xi->ptr, xbuf, _abs(xi->size) * sizeof(dig_t));
		}
	}
}

/*
 * Computes the remainder of the division of one big integer `x` by another `y` (x % y).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_mod(const intx_t xi, const intx_t yi)
{
	intx_t x = i8_trim(xi);
	intx_t y = i8_trim(yi);

	if (i8_is_zero(y)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return intx_zero;
	}
	else if (i8_is_zero(x))
		return intx_zero;

	cntx_t sign = _sgn(x.size);
	x.size = _abs(x.size);

	if (x.size > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
		return intx_zero;
	}
	dig_t xbuf[INTEX8_MAX_DIVIDEND_DIGITS];
	intx_t ret = i8_mod(i8_copy(x, xbuf), y, NULL);
	ret.ptr = (dig_t*)malloc(ret.size * sizeof(dig_t));
	if (ret.ptr == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	else {
		memcpy(ret.ptr, xbuf, ret.size * sizeof(dig_t));
		ret.size *= sign;
		return ret;
	}
}

/*
 * Computes the remainder of the division of one big integer `x` by an int64 `y` (x % y).
 */
int64_t ix8_mod_i(const intx_t xi, int64_t y)
{
	if (y == 0) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return 0;
	}
	intx_t x = i8_trim(xi);
	cntx_t sign = _sgn(x.size);
	x.size = _abs(x.size);
	if (i8_is_zero(x)) {
		return 0;
	}
	else if (x.size == 1) {
		return sign * (int64_t)x.ptr[0] % y;
	}
	else if (x.size > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
		return 0;
	}
	else {
		dig_t dividend_buf[INTEX8_MAX_DIVIDEND_DIGITS];
		dig_t divisor_buf[INTEX8_DIGIT_COUNT_IN_64BITS];

		intx_t ret = i8_mod(i8_copy(x, dividend_buf), i8_copy_i(_abs(y), divisor_buf), NULL);
		if (ret.size == 0)
			return 0;
		else if (ret.size == 1)
			return sign * (int64_t)ret.ptr[0];
		else
			return sign * *(int64_t *)ret.ptr;
	}
}

/*
 * Computes the remainder of the division of an int64 `x` by a big integer `y` (x % y).
 */
int64_t ix8_i_mod(int64_t x, const intx_t yi)
{
	intx_t y = i8_trim(yi);
	y.size = _abs(y.size);
	if (i8_is_zero(y)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return 0;
	}
	else if (y.size == 1) {
		return x % (uint64_t)y.ptr[0];
	}
	else if (y.size == INTEX8_DIGIT_COUNT_IN_64BITS) {
		return x % *(uint64_t *)y.ptr;
	}
	else {
		return x;
	}
}

/*
 * Computes the remainder of the division of a big integer `x` by a power-of-two `2^y` (x % (2^y)).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_mod_p2(const intx_t xi, uint64_t y)
{
	if (y == 0)
		return intx_zero;

	intx_t x = i8_trim(xi);

	intx_t ret = { NULL, (y + INTEX8_DIGIT_BIT_WIDTH - 1) / INTEX8_DIGIT_BIT_WIDTH };

	ret.ptr = (dig_t*)calloc(ret.size, sizeof(dig_t));
	if (ret.ptr == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	if (_abs(x.size) >= ret.size) {
		memcpy(ret.ptr, x.ptr, ret.size * sizeof(dig_t));
		y %= INTEX8_DIGIT_BIT_WIDTH;
		if (y != 0) {
			dig_t r = 1 << y, *ptr = &ret.ptr[ret.size - 1];
			for (; r; r <<= 1)
				*ptr &= ~r;
		}
	}
	else {
		memcpy(ret.ptr, x.ptr, _abs(x.size) * sizeof(dig_t));
	}

	ret.size *= _sgn(x.size);
	return ret;
}

/*
 * Computes the remainder of the division of a big integer `x` by a big integer `y` in place (x %= y).
 * Caller is responsible for freeing `x` using `ix8_free()`.
 */
void ix8_modeq(intx_t* x, intx_t y)
{
	*x = i8_trim(*x);

	cntx_t sign = _sgn(x->size);
	x->size = _abs(x->size);

	if (ix8_is_zero(y)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
	}
	else if (x->size > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
	}
	else {
		dig_t xbuf[INTEX8_MAX_DIVIDEND_DIGITS];

		intx_t m = i8_trim(i8_mod(i8_copy(*x, xbuf), i8_trim(y), NULL));
		if (m.size == 0) {
			x->size = 0;
		}
		else {
			dig_t* ptr = (dig_t*)realloc(x->ptr, m.size * sizeof(dig_t));
			if (ptr == NULL) {
				intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
			}
			else {
				x->size = sign * m.size;
				x->ptr = ptr;
				memcpy(x->ptr, xbuf, m.size * sizeof(dig_t));
			}
		}
	}
}

/*
 * Computes the remainder of the division of a big integer `x` by a power-of-two `2^y` in place (x %= (2^y)).
 * Caller is responsible for freeing `x` using `ix8_free()`.
 */
void ix8_modeq_p2(intx_t* x, uint64_t y)
{
	if (y == 0) {
		x->size = 0;
		return;
	}

	cntx_t size = (y + INTEX8_DIGIT_BIT_WIDTH - 1) / INTEX8_DIGIT_BIT_WIDTH;

	if (_abs(x->size) >= size) {
		dig_t* ptr = (dig_t*)realloc(x->ptr, size * sizeof(dig_t));
		if (ptr == NULL) {
			intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
			return;
		}
		x->ptr = ptr;
		y %= INTEX8_DIGIT_BIT_WIDTH;
		if (y != 0) {
			dig_t r = 1 << y;
			ptr = &x->ptr[size - 1];
			for (; r; r <<= 1)
				*ptr &= ~r;
		}

		x->size = _sgn(x->size) * size;
	}
	else {
	}
}

/*
 * Computes the remainder of the division of a big integer `x` by an int64 `y` in place (x %= y).
 * Caller is responsible for freeing `x` using `ix8_free()`.
 */
void ix8_modeq_i(intx_t *x, int64_t y)
{
	*x = i8_trim(*x);

	cntx_t sign = _sgn(x->size);
	x->size = _abs(x->size);

	if (y == 0) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
	}
	else if (x->size > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
	}
	else {
		dig_t xbuf[INTEX8_MAX_DIVIDEND_DIGITS], ybuf[INTEX8_DIGIT_COUNT_IN_64BITS];

		intx_t m = i8_trim(i8_mod(i8_copy(*x, xbuf), i8_copy_i(y, ybuf), NULL));
		if (m.size == 0) {
			x->size = 0;
		}
		else {
			dig_t* ptr = (dig_t*)realloc(x->ptr, m.size * sizeof(dig_t));
			if (ptr == NULL) {
				intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
			}
			else {
				x->size = sign * m.size;
				x->ptr = ptr;
				memcpy(x->ptr, xbuf, m.size * sizeof(dig_t));
			}
		}
	}
}

/*
 * Computes the remainder of the division of an integer `x` by a big integer `y` in place (x %= y).
 */
void ix8_i_modeq(int64_t *x, const intx_t yi)
{
	intx_t y = i8_trim(yi);

	if (ix8_is_zero(y)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
	}
	else if (*x != 0) {
		dig_t div_buf[INTEX8_DIGIT_COUNT_IN_64BITS];
		intx_t m = i8_trim(i8_mod(i8_copy_i(*x, div_buf), y, NULL));
		if (m.size == 0) {
			*x = 0;
		}
		else if (m.size == 1) {
			*x = (int32_t)div_buf[0];
		}
		else if (m.size == INTEX8_DIGIT_COUNT_IN_64BITS) {
			*x = *(int64_t *)div_buf;
		}
	}
}

/*
 * Computes the negative of a big integer `x` (-x).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_negate(const intx_t x)
{
	return _call_i8_1param_interface_special(i8_trim(x), i8_negate);
}

/*
 * Computes the absolute value of a big integer `x` (|x|).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_abs(const intx_t x)
{
	return _call_i8_1param_interface_special(i8_trim(x), i8_abs);
}

/*
 * Converts a big integer to a string representation and returns the result (intx_t to ascii).
 * BE CAREFUL: if a non-NULL value is passed for fptr, CALLER MUST FREE returned pointer in fptr.
 * If fptr is passed a NULL pointer, caller is responsible for freeing the result using `ix8_free_s()`.
 */
char* const ix8_copy_to_s(const intx_t xi, char **fptr)
{
	intx_t x = i8_trim(xi);

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

	cntx_t sign = _sgn(x.size);
	x.size = _abs(x.size);
	cntx_t out_size = _required_decimal_count(x.size, sign == -1);
	
	char* const str = (char*)calloc(out_size, sizeof(char));
	if (str == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return NULL;
	}

	char* ptr = i8_copy_to_s(x, str, out_size);

	if (sign == -1) {
		*(--ptr) = '-';
	}
	if (fptr != NULL) {
		*fptr = str;
		return ptr;
	}
	else if (ptr > str) {
		memcpy(str, ptr, out_size - (ptr - str));
		return str;
	}
	else {
		return str;
	}
}

/*
 * Parses a big integer from a string representation and returns the result (ascii to intx_t).
 * Caller is responsible for freeing the result using `ix8_free()`.
 */
intx_t ix8_copy_s(const char* in_str)
{
	cntx_t sign = 1;
	if (*in_str == '-') {
		sign = -1;
		++in_str;
	}
	else if (*in_str == '+')
		++in_str;
	while (*in_str == '0')
		++in_str;

	if (*in_str == '\0')
		return intx_zero;

	for (const char* p = in_str; *p; ++p)
		if (*p < '0' || *p > '9') {
			intEx8_errno = INTEX8_ERR_INVALID_DECIMAL_STRING;
			return intx_zero;
		}

	cntx_t in_len = strlen(in_str);
	cntx_t size = _required_digit_count(in_len);
	dig_t* buf = (dig_t*)calloc(size, sizeof(dig_t));
	if (buf == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	intx_t x = i8_copy_s(in_str, in_len, buf);

	x.size = sign * size;
	return i8_trim(x);
}
