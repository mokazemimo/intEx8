/*
 *  File: i8.c
 *  Description:
 *      Implements the `i8` interface functions of the intEx8 library for big integers.
 *      Contains function definitions for arithmetic, comparison, and string conversions.
 *
 *  Notes:
 *      - The `i8` interface is designed for advanced users seeking better performance.
 *      - The caller is responsible for allocating and managing memory.
 */

#include <stdlib.h>	// malloc()
#include <string.h>	// strlen()
#include "i8.h"
#include "util.h"

int intEx8_errno;

const intx_t intx_zero = { NULL, 0 };

/* Internal function. Do NOT call this directly! */
static intx_t _get_pow2(size_t p, int sign, dig_t* dest)
{
	intx_t x = { dest, 0 };
	x.size = p / INTEX8_DIGIT_BIT_WIDTH();
	for (size_t i = 0; i < x.size; ++i)
		dest[i] = 0;
	if (sign > 0) {
		dest[x.size++] = 1 << (p % INTEX8_DIGIT_BIT_WIDTH());
		if (p % INTEX8_DIGIT_BIT_WIDTH() == INTEX8_DIGIT_BIT_WIDTH() - 1)
			dest[x.size++] = 0;
	}
	else if (sign < 0) {
		dest[x.size++] = INTEX8_DIGIT_MAX_VALUE() << (p % INTEX8_DIGIT_BIT_WIDTH());
	}
	else {
	}
	return x;
}

/* Internal function. Do NOT call this directly! */
static char* _add_string_to(char* str, size_t len, char* out, size_t outlen, uint64_t multiply, uint64_t add)
{
	size_t j = outlen;
	for (size_t i = len; i-- > 0; ) {
		add += (str[i] - '0') * multiply;
		out[--j] = add % 10 + '0';
		add /= 10;
	}
	while (add > 0) {
		out[--j] = add % 10 + '0';
		add /= 10;
	}
	return out + j;
}

/* Internal function. Do NOT call this directly! */
static size_t _get_rightmost_num(const char* str, size_t count, size_t* pos)
{
	if (*pos == 0)
		return 0;

	size_t ret = 0, p = 1;
	for (int i = 0; i < count && *pos > 0; ++i) {
		--(*pos);
		ret += (str[*pos] - '0') * p;
		p *= 10;
	}
	return ret;
}

/* Internal function. Do NOT call this directly! */
static int _get_highest_bit_position(intx_t x)
{
	int index = x.size - 1;
	int highest_bit = INTEX8_DIGIT_BIT_WIDTH() * index + (INTEX8_DIGIT_BIT_WIDTH() - 1);
	while (index >= 0 && x.ptr[index] == 0) {
		--index;
		highest_bit -= INTEX8_DIGIT_BIT_WIDTH();
	}

	if (index >= 0 && x.ptr[index] != 0) {
		size_t p = INTEX8_DIGIT_SIGN_MASK();
		for (size_t i = 0; i < INTEX8_DIGIT_BIT_WIDTH(); ++i) {
			if ((x.ptr[index] & p) == p) {
				break;
			}
			--highest_bit;
			p >>= 1;
		}
	}

	return highest_bit;
}

/* Internal function. Do NOT call this directly! */
static intx_t _add_shifted_to(intx_t a, uint64_t b, size_t shift)
{	// a >= 0, b >= 0
	// a += (b << shift)

	const uni64_t bx = { b };
	size_t bx_len = INTEX8_DIGIT_COUNT_IN_64BITS;
	while (bx_len > 0 && bx.b[bx_len - 1] == 0)
		--bx_len;

	uni64_t u = { 0 };

	if (shift % INTEX8_DIGIT_BIT_WIDTH() == 0) {
		size_t i = shift / INTEX8_DIGIT_BIT_WIDTH();
		for (size_t j = 0; j < bx_len || u.a != 0; ++i, ++j) {
			u.a += (i < a.size ? (uint64_t)a.ptr[i] : 0) + (j < bx_len ? (uint64_t)bx.b[j] : 0);
			a.ptr[i] = u.b[0];
			u.a >>= INTEX8_DIGIT_BIT_WIDTH();
		}
		if (a.size < i)
			a.size = i;
		if (_has_sign_bit(a.ptr[a.size - 1]))
			a.ptr[a.size++] = 0;
	}
	else {
		size_t m = shift % INTEX8_DIGIT_BIT_WIDTH(), n = INTEX8_DIGIT_BIT_WIDTH() - m;
		uint64_t yL = 0;
		size_t i = shift / INTEX8_DIGIT_BIT_WIDTH();
		for (size_t j = 0; j < INTEX8_DIGIT_COUNT_IN_64BITS || u.a != 0; ++i, ++j) {
			uint64_t y = (j < INTEX8_DIGIT_COUNT_IN_64BITS ? (((uint64_t)bx.b[j] << m) & INTEX8_DIGIT_MAX_VALUE()) : 0) | yL;
			yL = (j < INTEX8_DIGIT_COUNT_IN_64BITS ? ((uint64_t)bx.b[j] >> n) : 0);
			u.a += (i < a.size ? (uint64_t)a.ptr[i] : 0) + y;
			a.ptr[i] = u.b[0];
			u.a >>= INTEX8_DIGIT_BIT_WIDTH();
		}
		if (a.size < i)
			a.size = i;
		// out wouldn't be negative
		if (_has_sign_bit(a.ptr[a.size - 1]))
			a.ptr[a.size++] = 0;
	}

	return a;
}

/* Internal function. Do NOT call this directly! */
static intx_t _subtract_shifted_multiple_from(intx_t a, intx_t b, uint64_t b_multiple, size_t b_shift)
{	// a >= 0, b >= 0 and a >= b * (b_multiple << b_shift)
	// a -= b * (b_multiple << b_shift)
	uni64_t u = { 0 };

	if (b_shift % INTEX8_DIGIT_BIT_WIDTH() == 0) {
		uint64_t h = 0;
		for (size_t i = b_shift / INTEX8_DIGIT_BIT_WIDTH(), j = 0; i < a.size || j < b.size || u.a != 0 || h != 0; ++i, ++j) {
			uint64_t x = a.ptr[i];
			u.a += (j < b.size ? b_multiple * (uint64_t)b.ptr[j] : 0);
			if (x >= u.b[0] + h) {
				a.ptr[i] = x - (u.b[0] + h);
				h = 0;
			}
			else {
				a.ptr[i] = (INTEX8_DIGIT_MAX_VALUE() + 1) + x - (u.b[0] + h);
				h = 1;
			}
			u.a >>= INTEX8_DIGIT_BIT_WIDTH();
		}
		// output wouldn't be negative
		if (_has_sign_bit(a.ptr[a.size - 1]))
			a.ptr[a.size++] = 0;
	}
	else {
		size_t digit_shift = b_shift / INTEX8_DIGIT_BIT_WIDTH();
		size_t m = b_shift % INTEX8_DIGIT_BIT_WIDTH(), n = INTEX8_DIGIT_BIT_WIDTH() - m;
		uint64_t h = 0, yL = 0;
		for (size_t i = digit_shift, j = 0; j <= b.size || u.a != 0 || h != 0; ++i, ++j) {
			uint64_t x = (i < a.size ? (uint64_t)a.ptr[i] : 0);
			uint64_t y = (j < b.size ? (((uint64_t)b.ptr[j] << m) & INTEX8_DIGIT_MAX_VALUE()) : 0) | yL;
			yL = (j < b.size ? ((uint64_t)b.ptr[j] >> n) : 0);
			u.a += b_multiple * y;
			if (x >= u.b[0] + h) {
				a.ptr[i] = x - (u.b[0] + h);
				h = 0;
			}
			else {
				a.ptr[i] = (INTEX8_DIGIT_MAX_VALUE() + 1) + x - (u.b[0] + h);
				h = 1;
			}
			u.a >>= INTEX8_DIGIT_BIT_WIDTH();
		}
	}
	while (a.size >= 2 && a.ptr[a.size - 1] == 0 && !_has_sign_bit(a.ptr[a.size - 2]))
		--a.size;
	return a;
}

/* Internal function. Do NOT call this directly! */
static uint64_t _get_uint(intx_t x, int* pos, int posy)
{
	int index = *pos / INTEX8_DIGIT_BIT_WIDTH();
	while (index >= 0 && x.ptr[index] == 0)
		--index;

	int m = INTEX8_DIGIT_BIT_WIDTH();
	size_t p = INTEX8_DIGIT_SIGN_MASK();
	for (size_t i = 0; i < INTEX8_DIGIT_BIT_WIDTH() && (x.ptr[index] & p) == 0; ++i) {
		--m;
		p >>= 1;
	}
	int n = INTEX8_DIGIT_BIT_WIDTH() - m;
	*pos = index * INTEX8_DIGIT_BIT_WIDTH() + m - 1;

	uni64_t u = { 0 };
	int y_index = posy / INTEX8_DIGIT_BIT_WIDTH();
	int i = index;
	for (int j = INTEX8_DIGIT_COUNT_IN_64BITS / 2 - 1; i >= y_index && j >= 0; --i, --j)
		u.b[j] = ((uint64_t)x.ptr[i] << n) | ((uint64_t)x.ptr[i - 1] >> m);
	if (*pos - posy < 32) {
		u.a >>= (32 - (*pos - posy));
	}
	return u.a;
}

/* Internal function. Do NOT call this directly! */
static size_t _bits_prior_to(intx_t x, size_t p)
{
	// if any 1-bit of position < p exist
	// returns max-index for which x.ptr[index] has 1-bit whose position is < p
	size_t q = _min(p / INTEX8_DIGIT_BIT_WIDTH(), x.size);
	if (q < x.size) {
		p %= INTEX8_DIGIT_BIT_WIDTH();
		dig_t r = 1;
		for (size_t i = 0; i < p; ++i) {
			if (x.ptr[q] & r)
				return q + 1;
			r <<= 1;
		}
	}
	if (q == 0)
		return 0;
	for (; q-- > 0;)
		if (x.ptr[q] != 0)
			return q + 1;
	return 0;
}

/* Internal function. Do NOT call this directly! */
static intx_t _rightmost_bits(intx_t x, size_t p)
{
	if (x.size == 0)
		return x;
	if (p >= x.size * INTEX8_DIGIT_BIT_WIDTH())
		return x;

	size_t q = _bits_prior_to(x, p);
	if (q == 0) {
		x.ptr[0] = 0;
		x.size = 1;
		return x;
	}
	--q;
	if (q < p / INTEX8_DIGIT_BIT_WIDTH())
		q = p / INTEX8_DIGIT_BIT_WIDTH();

	p %= INTEX8_DIGIT_BIT_WIDTH();
	if (_has_sign_bit(x.ptr[x.size - 1])) {
		dig_t r = 1 << p;
		for (; r; r <<= 1)
			x.ptr[q] |= r;
		x.size = q + 1;
		return x;
	}
	else {
		dig_t r = 1 << p;
		for (; r; r <<= 1)
			x.ptr[q] &= ~r;
		x.size = q + 1;
		return x;
	}
}

/* Internal function. Do NOT call this directly! */
static intx_t _append_0_bits(intx_t x, size_t p, dig_t* dest)
{
	const int bit_count = INTEX8_DIGIT_BIT_WIDTH();

	intx_t y = { dest, 0 };
	if (p % bit_count == 0) {
		p /= bit_count;
		for (int i = 0; i < p; ++i)
			dest[i] = 0;
		for (int j = 0; j < x.size; ++j)
			dest[p + j] = x.ptr[j];
		y.size = x.size + p;
	}
	else {
		size_t m = p % bit_count;
		size_t n = bit_count - m;
		p /= bit_count;
		for (int i = 0; i < p; ++i)
			dest[i] = 0;

		uint64_t k = 0;
		for (int i = 0; i < x.size; ++i) {
			dest[p + i] = (x.ptr[i] << m) | (k >> n);
			k = x.ptr[i];
		}
		dest[p + x.size] = _right_shift_digit(x.ptr[x.size - 1], n);
		y.size = p + x.size + 1;
	}
	return y;
}

/* Internal function. Do NOT call this directly! */
static intx_t _multiply_pow2(intx_t x, int64_t p, dig_t* dest)
{
	if (p > 1) {
		return _append_0_bits(x, p - 1, dest);
	}
	else if (p == 1) {
		return i8_copy(x, dest);
	}
	else if (p == -1) {
		return i8_negate(x, dest);
	}
	else if (p < -1) {
		p = -p;
		int64_t px = i8_is_pow2(x);
		if (px == 0) {
			intx_t y = _append_0_bits(x, p - 1, dest);
			return i8_negate(y, dest);
		}
		else if (px > 0) {
			return _get_pow2((px - 1) + (p - 1), -1, dest);
		}
		else {
			return _get_pow2(-(px + 1) + (p - 1), 1, dest);
		}
	}
	else {
		return x;
	}
}

/* Internal function. Do NOT call this directly! */
static intx_t _right_shift(intx_t x, size_t bits, dig_t* dest, bool b_ext)
{
	if (i8_is_zero(x))
		return intx_zero;

	intx_t y = { dest, 0 };
	bits %= x.size * INTEX8_DIGIT_BIT_WIDTH();

	//const size_t dig_in_64 = INTEX8_DIGIT_COUNT_IN_64BITS;
	const size_t bit_width = INTEX8_DIGIT_BIT_WIDTH();
	const dig_t ext = _get_ext(x);

	size_t d = 0;
	size_t s = d + bits / bit_width;

	if (bits % bit_width == 0) {
		while (s + INTEX8_DIGIT_COUNT_IN_64BITS <= x.size) {
			*(uint64_t*)(dest + d) = *(uint64_t*)(x.ptr + s);
			d += INTEX8_DIGIT_COUNT_IN_64BITS;
			s += INTEX8_DIGIT_COUNT_IN_64BITS;
		}
		while (s < x.size)
			dest[d++] = x.ptr[s++];
	}
	else {
		size_t m = bits % (bit_width);
		size_t n = bit_width - m;

		while (s + INTEX8_DIGIT_COUNT_IN_64BITS + 1 <= x.size) {
			*(uint64_t*)(dest + d) = (*(uint64_t*)(x.ptr + s + 1) << n) | (x.ptr[s] >> m);
			d += INTEX8_DIGIT_COUNT_IN_64BITS;
			s += INTEX8_DIGIT_COUNT_IN_64BITS;
		}
		while (s + 1 < x.size) {
			dest[d] = (x.ptr[s + 1] << n) | (x.ptr[s] >> m);
			++d;
			++s;
		}
		dest[d++] = _right_shift_digit(x.ptr[x.size - 1], m);
	}
	if (b_ext) {
		while (d < x.size)
			dest[d++] = ext;
		y.size = x.size;
	}
	else {
		y.size = d;
	}
	return y;
}

/* Internal function. Do NOT call this directly! */
static intx_t _divide_pow2(intx_t x, int64_t p, dig_t* dest)
{
	if (p == 0) {
		return i8_copy(x, dest);
	}
	else if (p > 0) {
		return _right_shift(x, p - 1, dest, false);
	}
	else {
		p = -p - 1;
		int64_t px = i8_is_pow2(x);
		if (px == 0) {
			intx_t y = _right_shift(x, p, dest, false);
			return i8_negate(y, dest);
		}
		else if ((px - 1) - p >= 0) {
			return _get_pow2((px - 1) - p, -1, dest);
		}
		else if (-(px + 1) - p >= 0) {
			return _get_pow2(-(px + 1) - p, 1, dest);
		}
		else {
			intx_t z = { dest, 0 };
			return z;
		}
	}
}

#define IX8_BINARY_OPERATION(x, y, dest, opr)	\
	intx_t ret = {dest, 0};	\
	intx_t x1 = x;			\
	intx_t y1 = y;			\
	if(x.size > y.size) {	\
		x1 = y;				\
		y1 = x;				\
	}						\
	dig_t x_ext = _get_ext(x1);	\
	size_t i = 0;				\
	for (; i < x1.size; ++i)	\
	{							\
		ret.ptr[i] = x1.ptr[i] ##opr y1.ptr[i];	\
	}							\
	for(; i < y1.size; ++i)		\
	{							\
		ret.ptr[i] = x_ext ##opr y1.ptr[i];	\
	}							\
	ret.size = i;				\
	return ret;

/*
 * Creates a copy of x and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `x.size` digits space.
 */
intx_t i8_copy(const intx_t x, dig_t* dest)
{
	intx_t y = { dest, x.size };
	const dig_t* ptr = x.ptr;
	for (size_t i = 0; i < y.size; ++i)
		*dest++ = *ptr++;
	return y;
}

/*
 * Creates a big integer equal to an int64 value and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `_get_int_size(x)` digits space.
 */
intx_t i8_from_int(int64_t x, dig_t* dest)
{
	intx_t z = {dest, _get_int_size(x)};

	uni_t ux = {x};

	if (z.size < 3) {
		for (size_t i = 0; i < z.size; ++i)
			z.ptr[i] = ux.b[i];
	}
	else if (z.size == 3) {
		for (size_t i = 0; i < 2; ++i)
			z.ptr[i] = ux.b[i];
		if (ux.b[0] == INTEX8_DIGIT_MAX_VALUE() && ux.b[1] == INTEX8_DIGIT_EXTREME_POSITIVE()) {
			z.ptr[2] = 0;
		}
		else if (ux.b[0] == 0 && ux.b[1] == INTEX8_DIGIT_SIGN_MASK()) {
			z.ptr[2] = INTEX8_DIGIT_MAX_VALUE();
		}
	}
	return z;
}

/*
 *  Trims leading trivial digits from a big integer in place.
 */
intx_t i8_trim(const intx_t xi)
{
	intx_t x = xi;
	while (x.size >= 2 &&
		((x.ptr[x.size - 1] == 0 && !_has_sign_bit(x.ptr[x.size - 2])) ||
			(x.ptr[x.size - 1] == INTEX8_DIGIT_MAX_VALUE() && _has_sign_bit(x.ptr[x.size - 2])))
		)
		--x.size;
	if (x.size == 1 && x.ptr[0] == 0)
		x.size = 0;
	return x;
}

//-------------------------------------------------------------------------------------------------------
// Arithmetic operations

/*
 * Adds two big integers and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `max(x.size, y.size)` digits space.
 * In extreme situations, `dest` may require `max(x.size, y.size) + 1` digits space.
 */
intx_t i8_x_add_x(const intx_t x, const intx_t y, dig_t* dest)
{
	bool both_positive = i8_is_positive(x) && i8_is_positive(y);
	bool both_negative = i8_is_negative(x) && i8_is_negative(y);

	intx_t z = { dest, 0 };
	intx_t x1 = x;
	intx_t y1 = y;

	if (x.size > y.size) {
		x1 = y;
		y1 = x;
	}
	uni_t u = { 0 };

	size_t s = 0;
	for (; s < x1.size; ++s) {
		u.a += (uint64_t)x1.ptr[s] + y1.ptr[s];
		dest[s] = u.b[0];
		u.b[0] = u.b[1];
		u.b[1] = 0;
	}
	dig_t x_ext = _get_ext(x1);
	for (; s < y1.size; ++s) {
		u.a += (uint64_t)x_ext + y1.ptr[s];
		dest[s] = u.b[0];
		u.b[0] = u.b[1];
		u.b[1] = 0;
	}
	if (both_positive && _has_sign_bit(dest[s - 1]))
		dest[s++] = 0;
	else if (both_negative && !_has_sign_bit(dest[s - 1]))
		dest[s++] = INTEX8_DIGIT_MAX_VALUE();
	z.size = s;

	return z;
}

/*
 * Adds an int64 to a big integer and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `max(x.size, y.size)` digits space.
 * In extreme situations, `dest` may require `max(x.size, y.size) + 1` digits space.
 */
intx_t i8_x_add_i(const intx_t x, int64_t y, dig_t *dest)
{
	return i8_x_add_x(x, (intx_t){ (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS }, dest);
}

/*
 * Adds `y` to `x` in place (x += y).
 * Caller must ensure `x.ptr` has enough space for the result.
 */
intx_t i8_x_add_eq_x(intx_t x, const intx_t y)
{
	return i8_x_add_x(x, y, x.ptr);
}

/*
 * Adds an int64 to a big integer in place and returns the result.
 * Caller must ensure `dest` has at least `max(x.size, y.size)` digits space.
 * In extreme situations, `dest` may require `max(x.size, y.size) + 1` digits space.
 */
intx_t ix8_x_add_eq_i(intx_t x, int64_t y)
{
	return i8_x_add_eq_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS });
}

/*
 * Subtracts two big integers and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least max(x.size, y.size) digits space.
 * In extreme situations, `dest` may require (max(x.size, y.size) + 1) digits space.
 */
intx_t i8_x_sub_x(const intx_t x, const intx_t y, dig_t* dest)
{
	intx_t z = { dest, 0 };
	if (x.size == 0 && y.size == 0)
		return z;

	uint64_t x_ext = _get_ext(x);
	uint64_t y_ext = _get_ext(y);

	size_t s = 0;
	size_t m = _max(x.size, y.size);
	uint64_t h = 0;
	for (size_t i = 0; i < m; ++i) {
		uint64_t xx = (i < x.size) ? x.ptr[i] : x_ext;
		uint64_t yy = (i < y.size) ? y.ptr[i] : y_ext;

		if (xx >= yy + h) {
			z.ptr[s++] = xx - (yy + h);
			h = 0;
		}
		else {
			z.ptr[s++] = (INTEX8_DIGIT_MAX_VALUE() + 1) + xx - (yy + h);
			h = 1;
		}
	}
	if (h == 1) {
		if (x_ext >= y_ext + h)
			z.ptr[s++] = x_ext - (y_ext + h);
		else
			z.ptr[s++] = (INTEX8_DIGIT_MAX_VALUE() + 1) + x_ext - (y_ext + h);
	}
	if (x_ext == 0 && y_ext == INTEX8_DIGIT_MAX_VALUE()) {
		if (_has_sign_bit(z.ptr[s - 1]))
			z.ptr[s++] = 0;
	}
	else if (x_ext == INTEX8_DIGIT_MAX_VALUE() && y_ext == 0) {
		if (!_has_sign_bit(z.ptr[s - 1]))
			z.ptr[s++] = INTEX8_DIGIT_MAX_VALUE();
	}
	z.size = s;
	return z;
}

/*
 * Subtracts an int64 from a big integer and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least max(x.size, y.size) digits space.
 * In extreme situations, `dest` may require (max(x.size, y.size) + 1) digits space.
 */
intx_t i8_x_sub_i(const intx_t x, int64_t y, dig_t* dest)
{
	return i8_x_sub_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS }, dest);
}

/*
 * Subtracts a big integer from an int64 and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least max(x.size, y.size) digits space.
 * In extreme situations, `dest` may require (max(x.size, y.size) + 1) digits space.
 */
intx_t i8_i_sub_x(int64_t x, const intx_t y, dig_t* dest)
{
	return i8_x_sub_x((intx_t) { (dig_t*)&x, INTEX8_DIGIT_COUNT_IN_64BITS }, y, dest);
}

/*
 * Subtracts `y` from `x` in place (x -= y).
 * Caller must ensure `x.ptr` has enough space for the result.
 */
intx_t i8_x_sub_eq_x(intx_t x, const intx_t y)
{
	return i8_x_sub_x(x, y, x.ptr);
}

/*
 * Subtracts an int64 from a big integer in place and returns the result.
 * Caller must ensure `dest` has at least max(x.size, y.size) digits space.
 * In extreme situations, `dest` may require (max(x.size, y.size) + 1) digits space.
 */
intx_t i8_x_sub_eq_i(intx_t x, int64_t y)
{
	return i8_x_sub_eq_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS });
}

/*
 * Multiplies two big integers and stores the result in `dest`.
 * Caller must ensure `dest` has at least `x.size + y.size` digits space.
 */
intx_t i8_x_mul_x(const intx_t xi, const intx_t yi, dig_t* dest)
{
	size_t zero_count = 0, x_zeros = 0, y_zeros = 0;

	while (x_zeros < xi.size && xi.ptr[x_zeros] == 0)
		++x_zeros;
	if (x_zeros == xi.size)
		return intx_zero;

	while (y_zeros < yi.size && yi.ptr[y_zeros] == 0)
		++y_zeros;
	if (y_zeros == yi.size)
		return intx_zero;

	zero_count = x_zeros + y_zeros;

	intx_t x = xi;
	x.ptr += x_zeros;
	x.size -= x_zeros;

	intx_t y = yi;
	y.ptr += y_zeros;
	y.size -= y_zeros;

	dig_t* dest0 = dest;
	for (size_t i = 0; i < zero_count; ++i)
		*dest++ = 0;

	int64_t px = i8_is_pow2(x);
	if (px != 0) {
		intx_t z = _multiply_pow2(y, px, dest);
		z.ptr = dest0;
		z.size += zero_count;
		return z;
	}

	int64_t py = i8_is_pow2(y);
	if (py != 0) {
		intx_t z = _multiply_pow2(x, py, dest);
		z.ptr = dest0;
		z.size += zero_count;
		return z;
	}

	int neg_count = 0;

	bool negate_x = false;
	if (i8_is_negative(x)) {
		negate_x = true;
		x = i8_negate_self(x);
		++neg_count;
	}

	bool negate_y = false;
	if (i8_is_negative(y)) {
		negate_y = true;
		y = i8_negate_self(y);
		++neg_count;
	}

	if (_max(x.size, y.size) > INTEX8_MAX_MULTIPLICATION_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_MULTIPLICATION_DIGITS_EXCEEDED;
		return intx_zero;
	}

	intx_t _toom3_multiply(intx_t x, intx_t y, dig_t * dest);
	intx_t z = _toom3_multiply(x, y, dest);

	if (neg_count == 1)
		z = i8_negate_self(z);
	if (negate_x)
		x = i8_negate_self(x);
	if (negate_y)
		y = i8_negate_self(y);

	z.ptr = dest0;
	z.size += zero_count;

	return z;
}

/*
 * Multiplies a big integer by an int64 and stores the result in `dest`.
 * Caller must ensure `dest` has at least `x.size + y.size` digits space.
 * Note: y.size depends on the value of y (see _get_int_size(int64_t)).
 */
intx_t i8_x_mul_i(const intx_t x, int64_t y, dig_t* dest)
{
	return i8_x_mul_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS }, dest);
}

/*
 * Divides `x` by `y` and returns the quotient stored in `dest`.
 * Caller must ensure `dest` has enough space for the result.
 */
intx_t i8_x_div_x(const intx_t xi, const intx_t yi, dig_t* dest)
{
	if (i8_is_zero(yi)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return intx_zero;
	}
	if (i8_is_zero(xi)) {
		return intx_zero;
	}

	int64_t py = i8_is_pow2(yi);
	if (py != 0) {
		return _divide_pow2(xi, py, dest);
	}
	size_t div_size = _get_quotient_size(xi, yi);
	if (div_size <= 0)
		return intx_zero;

	if (xi.size > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
		return intx_zero;
	}

	dig_t dividend_buf[INTEX8_MAX_DIVIDEND_DIGITS];
	intx_t x = i8_copy(xi, dividend_buf);

	bool negate_x = false, negate_y = false;
	int sign = 0;
	if (i8_is_min_negative(x))
		++sign;
	else if (i8_is_negative(x)) {
		x = i8_negate_self(x);
		negate_x = true;
		++sign;
	}

	intx_t y = yi;
	// i8_is_min_negative(y) is already checked in i8_is_pow2(y) 
	if (i8_is_negative(y)) {
		y = i8_negate_self(y);
		negate_y = true;
		++sign;
	}

	int posy = _get_highest_bit_position(y);
	int posx = x.size * INTEX8_DIGIT_BIT_WIDTH() - 1;

	if (i8_is_less(i8_trim(x), i8_trim(y)))
		return intx_zero;

	intx_t z = { dest, 0 };
	// z may need (x.size - y.size) + 1 digits
	for (int i = 0; i < div_size; ++i)
		z.ptr[z.size++] = 0;

	while (posx > posy) {
		uint64_t u = _get_uint(x, &posx, posy);
		int shift = (posx - posy > 32 ? posx - posy - 32 : 0);
		z = _add_shifted_to(z, u, shift);
		x = _subtract_shifted_multiple_from(x, y, u, shift);
	}
	if (posx == posy && i8_is_less_eq(i8_trim(y), i8_trim(x))) {
		if (sign == 1 && i8_is_max_positive(z)) {
			memset(z.ptr, 0, z.size * sizeof(dig_t));
			z.ptr[z.size - 1] = INTEX8_DIGIT_MAX_VALUE();
			sign = 0;
		}
		else {
			z = _add_shifted_to(z, 1, 0);
		}
	}
	if (_has_sign_bit(dest[z.size - 1]))
		dest[z.size++] = 0;

	if (sign == 1)
		z = i8_negate_self(z);
	if (negate_y)
		y = i8_negate_self(y);

	return z;
}

/*
 * Divides `x` by `y` and returns the quotient stored in `dest`.
 * Caller must ensure `dest` has enough space for the result.
 */
intx_t i8_x_div_i(const intx_t x, int64_t y, dig_t* dest)
{
	return i8_x_div_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS }, dest);
}

/*
 * Divides `x` by `y` and returns the quotient stored in `dest`.
 * Caller must ensure `dest` has enough space for the result.
 */
intx_t i8_i_div_x(int64_t x, const intx_t y, dig_t* dest)
{
	return i8_x_div_x((intx_t) { (dig_t*)&x, INTEX8_DIGIT_COUNT_IN_64BITS }, y, dest);
}

/*
 * Computes `x % y` and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `x.size` (??????????????????) digits space.
 */
intx_t i8_x_mod_x(const intx_t xi, const intx_t yi, dig_t* dest)
{
	if (i8_is_zero(yi)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return intx_zero;
	}
	if (i8_is_zero(xi)) {
		return intx_zero;
	}

	if (xi.size > INTEX8_MAX_DIVIDEND_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_DIVIDEND_DIGITS_EXCEEDED;
		return intx_zero;
	}

	dig_t dividend_buf[INTEX8_MAX_DIVIDEND_DIGITS];
	intx_t x = i8_copy(xi, dividend_buf);

	int64_t py = i8_is_pow2(yi);
	if (py != 0) {
		return i8_copy(_rightmost_bits(x, py > 0 ? (py - 1) : (-py - 1)), dest);
	}

	bool negate_x = false, negate_y = false;
	if (i8_is_min_negative(x))
		negate_x = true;
	else if (i8_is_negative(x)) {
		x = i8_negate_self(x);
		negate_x = true;
	}

	intx_t y = yi;
	if (i8_is_negative(y)) {
		y = i8_negate_self(y);
		negate_y = true;
	}

	int posy = _get_highest_bit_position(y);
	int posx = x.size * INTEX8_DIGIT_BIT_WIDTH() - 1;

	while (posx > posy) {
		uint64_t u = _get_uint(x, &posx, posy);
		int shift = (posx - posy > 32 ? posx - posy - 32 : 0);
		x = _subtract_shifted_multiple_from(x, y, u, shift);
	}
	if (posx == posy && i8_is_less_eq(i8_trim(y), i8_trim(x))) {
		x = _subtract_shifted_multiple_from(x, y, 1, 0);
	}
	if (negate_x && i8_is_positive(x))
		x = i8_negate_self(x);
	if (negate_y && i8_is_positive(y))
		y = i8_negate_self(y);

	return i8_copy(x, dest);
}

/*
 * Computes `x % y` and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `x.size` (??????????????????) digits space.
 */
intx_t i8_x_mod_i(const intx_t x, int64_t y, dig_t* dest)
{
	return i8_x_mod_x(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS }, dest);
}

/*
 * Computes `x % y` and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `x.size` (??????????????????) digits space.
 */
intx_t i8_i_mod_x(int64_t x, const intx_t y, dig_t* dest)
{
	return i8_x_mod_x((intx_t) { (dig_t*)&x, INTEX8_DIGIT_COUNT_IN_64BITS }, y, dest);
}

/*
 * Computes `-x` and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `x.size` digits.
 * If `x` is an Extreme Negative, `x.size + 1` digits required for `dest`.
 */
intx_t i8_negate(const intx_t x, dig_t* dest)
{
	intx_t ret = { dest, x.size };
	if (x.size == 0)
		return ret;

	size_t i = 0;
	for (; i < x.size && x.ptr[i] == 0; ++i) {
		dest[i] = 0;
	}
	if (i == x.size)
		return ret;
	else if (i == x.size - 1 && (x.ptr[i] == INTEX8_DIGIT_SIGN_MASK())) {
		dest[i] = x.ptr[i];
		dest[ret.size++] = 0;
		return ret;
	}
	dest[i] = (~x.ptr[i]) + 1;
	++i;
	for (; i < x.size; ++i) {
		dest[i] = ~x.ptr[i];
	}

	return ret;
}

/*
 * Negates `x` in place (`x = -x`).
 * If `x` is an Extreme Negative, caller must ensure `x.ptr` has at least (x.size + 1) digits.
 */
intx_t i8_negate_self(intx_t x)
{
	return i8_negate(x, x.ptr);
}

/*
 * Computes `|x|` and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least (x.size) digits.
 * If `x` is an Extreme Negative, (x.size + 1) digits required for `dest`.
 */
intx_t i8_abs(const intx_t x, dig_t* dest)
{
	if (i8_is_negative(x)) {
		return i8_negate(x, dest);
	}
	else {
		return i8_copy(x, dest);
	}
}

/*
 * Computes `|x|` in place (`x = |x|`).
 * If `x` is an Extreme Negative, caller must ensure `x.ptr` has at least (x.size + 1) digits.
 */
intx_t i8_abs_self(intx_t x)
{
	return i8_abs(x, x.ptr);
}

// Comparison operators
/*
 * Compares two big integers. Returns `true` if `x == y`, `false` otherwise.
 * Assumes x and y are trimmed.
 */
bool i8_is_equal(const intx_t x, const intx_t y)
{
	if (i8_is_zero(x) && i8_is_zero(y))
		return true;
	else if (i8_is_zero(x) || i8_is_zero(y))
		return false;
	else if (x.size != y.size || _get_ext(x) != _get_ext(y))
		return false;

	size_t i = 0;
	for (; i < x.size; ++i)
		if (x.ptr[i] != y.ptr[i])
			return false;

	if (_has_sign_bit(x.ptr[x.size - 1])) {
		// x1 is negative
		for (; i < y.size; ++i)
			if (y.ptr[i] != INTEX8_DIGIT_MAX_VALUE())
				return false;
	}
	else {
		// x1 is positive
		for (; i < y.size; ++i)
			if (y.ptr[i] != 0)
				return false;
	}
	return true;
}

/*
 * Compares two big integers. Returns `true` if `x <= y`, `false` otherwise.
 * Assumes x and y are trimmed.
 */
bool i8_is_less_eq(const intx_t x, const intx_t y)
{
	bool x_is_positive = i8_is_positive(x);
	bool x_is_negative = i8_is_negative(x);
	bool x_is_zero = i8_is_zero(x);
	bool y_is_positive = i8_is_positive(y);
	bool y_is_negative = i8_is_negative(y);
	bool y_is_zero = i8_is_zero(y);

	if ((x_is_zero || x_is_negative) && (y_is_zero || y_is_positive))	// x <= 0 && 0 <= y
		return true;
	else if ((x_is_zero || x_is_positive) && y_is_negative)	// x >= 0 && y < 0
		return false;
	else if (x_is_positive && (y_is_zero || y_is_negative))	// x > 0 && y <= 0
		return false;

	if (x_is_positive) {	// x > 0 && y > 0
		if (x.size < y.size)
			return true;
		else if (x.size > y.size)
			return false;
		else {
			// x.size == y.size
			size_t s = x.size;
			while (s > 0 && x.ptr[s - 1] == y.ptr[s - 1])
				--s;
			return s == 0 || x.ptr[s - 1] <= y.ptr[s - 1];
		}
	}
	else {	// x < 0 && y < 0
		if (x.size < y.size)
			return false;
		else if (x.size > y.size)
			return true;
		else {
			// x.size == y.size
			size_t s = x.size;
			while (s > 0 && x.ptr[s - 1] == y.ptr[s - 1])
				--s;
			return s == 0 || x.ptr[s - 1] <= y.ptr[s - 1];
		}
	}
}

/*
 * Compares a big integer with 0. Returns `true` if `x == 0`, `false` otherwise.
 */
bool i8_is_zero(const intx_t x)
{
	size_t i = 0;
	while (i < x.size && x.ptr[i] == 0)
		++i;
	return (i == x.size);
}

/*
 * Compares a big integer with 0. Returns `true` if `x > 0`, `false` otherwise.
 */
bool i8_is_positive(const intx_t x)
{
	return !i8_is_zero(x) && !_has_sign_bit(x.ptr[x.size - 1]);
}

/*
 * Compares a big integer with 0. Returns `true` if `x < 0`, `false` otherwise.
 */
bool i8_is_negative(const intx_t x)	// x < 0
{
	return x.size > 0 && _has_sign_bit(x.ptr[x.size - 1]);
}


// NEW FUNCTION
/*
 * Checks if `x` is an Extreme Positive (i.e. ALL bits are set, but highest one).
 * Returns `true` if `x` is an Extreme Positive; `false` otherwise.
 */
bool i8_is_max_positive(const intx_t x)
{
	if (x.size == 0)
		return false;
	else if (x.ptr[x.size - 1] != INTEX8_DIGIT_EXTREME_POSITIVE())
		return false;
	else if (x.size == 1)
		return true;
	else {
		size_t i = 0;
		while (i < x.size - 1 && x.ptr[i] == INTEX8_DIGIT_MAX_VALUE())
			++i;
		return (i == x.size - 1);
	}
}

/*
 * Checks if `x` is an Extreme Negative (i.e. ONLY highest bit of x is set).
 * Returns `true` if `x` is an Extreme Negative; `false` otherwise.
 */
bool i8_is_min_negative(const intx_t x)
{
	if (x.size == 0)
		return false;
	else if (x.ptr[x.size - 1] != INTEX8_DIGIT_SIGN_MASK())
		return false;
	else if (x.size == 1)
		return true;
	else {
		size_t i = 0;
		while (i < x.size - 1 && x.ptr[i] == 0)
			++i;
		return (i == x.size - 1);
	}
}

/*
 * Checks if `x` is of the form `pow(2, n)` or `-pow(2, n)`, for some non-negative integer `n`.
 * Returns `n + 1` if `x == pow(2, n)`; `-(n + 1)` if `x == -pow(2, n)`; `0` otherwise.
 */
int64_t i8_is_pow2(const intx_t x)
{
	size_t i = 0;
	while (i < x.size && x.ptr[i] == 0)
		++i;
	if (i >= x.size)
		return 0;

	int64_t pos = (int)i * INTEX8_DIGIT_BIT_WIDTH();
	dig_t p = 1;
	for (; p; p <<= 1) {
		if ((x.ptr[i] & p) == p)
			break;
		++pos;
	}
	// pos is the least-significant 1-bit
	if (pos + 1 == (int)x.size * INTEX8_DIGIT_BIT_WIDTH())
		return -pos - 1;

	p <<= 1;
	if (x.ptr[(pos + 1) / INTEX8_DIGIT_BIT_WIDTH()] & (1 << ((pos + 1) % INTEX8_DIGIT_BIT_WIDTH()))) {
		// all higher bits MUST be 1
		for (; p; p <<= 1) {
			if ((x.ptr[i] & p) == 0)
				return 0;
		}
		for (++i; i < x.size; ++i) {
			if (x.ptr[i] != INTEX8_DIGIT_MAX_VALUE())
				return 0;
		}
		return -pos - 1;
	}
	else {
		// all higher bits MUST be 0
		for (; p; p <<= 1) {
			if ((x.ptr[i] & p) == p)
				return 0;
		}
		for (int j = i + 1; j < x.size; ++j) {
			if (x.ptr[j] != 0)
				return 0;
		}
		return pos + 1;
	}
}

//-------------------------------------------------------------------------------------------------------
// Binary operations
/*
 * Performs bitwise AND operation (&) on two big integers and returns the result (x & y) stored in `dest`.
 * Caller must ensure `dest` has at least max(x.size, y.size) digits.
 */
intx_t i8_binary_and(const intx_t x, const intx_t y, dig_t* dest)
{
	IX8_BINARY_OPERATION(x, y, dest, &)
}

/*
 * Performs bitwise OR operation (|) on two big integers and returns the result (x | y) stored in `dest`.
 * Caller must ensure `dest` has at least max(x.size, y.size) digits.
 */
intx_t i8_binary_or(const intx_t x, const intx_t y, dig_t* dest)
{
	IX8_BINARY_OPERATION(x, y, dest, |)
}

/*
 * Performs bitwise XOR operation (^) on two big integers and returns the result (x ^ y) stored in `dest`.
 * Caller must ensure `dest` has at least `max(x.size, y.size)` digits.
 */
intx_t i8_binary_xor(const intx_t x, const intx_t y, dig_t* dest)
{
	IX8_BINARY_OPERATION(x, y, dest, ^)
}

/*
 * Performs bitwise NOT operation (~) on a big integer and returns the result (~x) stored in `dest`.
 * Caller must ensure `dest` has at least `x.size` digits.
 */
intx_t i8_binary_not(const intx_t x, dig_t* dest)
{
	intx_t ret = { dest, x.size };
	const dig_t* ptr = x.ptr;
	for (size_t i = 0; i < x.size; ++i)
		*dest++ = ~(*ptr++);
	return ret;
}

/*
 * Performs bitwise NOT operation (~) in place (x = ~x).
 */
intx_t i8_binary_not_self(intx_t x)
{
	return i8_binary_not(x, x.ptr);
}

/*
 * Performs bitwise LEFT SHIFT operation (<<) and returns the result (x << bits) stored in `dest`.
 * Caller must ensure `dest` has at least (x.size) digits.
 */
intx_t i8_left_shift(const intx_t x, size_t bits, dig_t* dest)
{
	if (i8_is_zero(x))
		return intx_zero;

	intx_t y = { dest, x.size };
	bits %= x.size * INTEX8_DIGIT_BIT_WIDTH();

	const int dig_in_64 = INTEX8_DIGIT_COUNT_IN_64BITS;
	const int bit_count = INTEX8_DIGIT_BIT_WIDTH();
	if (bits % bit_count == 0) {
		int d = x.size - 1;
		int s = d - bits / bit_count;
		while (s - dig_in_64 >= 0) {
			d -= dig_in_64;
			s -= dig_in_64;
			*(uint64_t*)(dest + d) = *(uint64_t*)(x.ptr + s);
		}
		while (s >= 0)
			dest[d--] = x.ptr[s--];
		while (d >= 0)
			dest[d--] = 0;
	}
	else {
		size_t m = bits % (bit_count);
		size_t n = bit_count - m;

		int d = (int)x.size - dig_in_64;
		int s = d - bits / bit_count - 1;
		while (s >= 0) {
			*(uint64_t*)(dest + d) = (*(uint64_t*)(x.ptr + s + 1) << m) | (x.ptr[s] >> n);
			d -= dig_in_64;
			s -= dig_in_64;
		}
		d += dig_in_64 - 1;
		s += dig_in_64 - 1;
		while (s >= 0) {
			dest[d] = (x.ptr[s + 1] << m) | (x.ptr[s] >> n);
			--d;
			--s;
		}
		dest[d--] = (x.ptr[0] << m);
		while (d >= 0)
			dest[d--] = 0;
	}

	return y;
}

/*
 * Performs bitwise LEFT SHIFT operation (<<) in place (x <<= bits).
 */
intx_t i8_left_shift_self(intx_t x, size_t bits)
{
	return i8_left_shift(x, bits, x.ptr);
}

/*
 * Performs bitwise RIGHT SHIFT operation (>>) and returns the result (x >> bits) stored in `dest`.
 * Caller must ensure `dest` has at least `x.size - bits/INTEX8_DIGIT_BIT_WIDTH()` digits.
 */
intx_t i8_right_shift(const intx_t x, size_t bits, dig_t* dest)
{
	return _right_shift(x, bits, dest, false);
}

/*
 * Performs bitwise RIGHT SHIFT operation (>>) in place (x >>= bits).
 */
intx_t i8_right_shift_self(intx_t x, size_t bits)
{
	return _right_shift(x, bits, x.ptr, true);
}

//----------------------------------------------------------------------------------------------------------
// String conversion
/*
 * Converts a positive big integer to a string representation stored in `buf`.
 * Caller must ensure `buf` has enough space for the result ( call `_required_decimal_count(x.size, false)` )
 */
char* i8_to_string(const intx_t x, char* buf, size_t buf_size)
{
	if (buf_size == 0)
		return buf;

	char* str = buf;
	char* ptr = str;
	const uint64_t p32 = 4294967296 /* 2^32 */;
	
	for (size_t i = x.size; i-- > 0;)
		ptr = _add_string_to(ptr, strlen(ptr), str, buf_size - 1, p32, x.ptr[i]);

	return ptr;
}

/*
 * Parses a big integer (`intx_t` instance) from its decimal string representation stored in `dest`.
 * Caller must ensure `dest` has enough space for the result using `_required_digit_count(in_len)`.
 * Caller must ensure the memory allocated for `dest` is initialized with 0.
 * Caller must ensure `in_str` represents a POSITIVE big integer (ONLY the digits '0' to '9').
 */
intx_t i8_from_string(const char* in_str, size_t in_len, dig_t *dest)
{
	char *fstr[2] = { NULL, NULL };
	fstr[0] = (char *)malloc(in_len + 1);
	if (fstr[0] == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	fstr[1] = (char *)malloc(in_len + 1);
	if (fstr[1] == NULL) {
		free(fstr[0]);
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
	char *str[2] = { fstr[0], fstr[1] };
	memcpy(fstr[0], in_str, in_len + 1);
	str[1][in_len] = 0;

	intx_t x = { dest, 0 };

	uint8_t *ptr_out = (uint8_t *)x.ptr;

	const uint64_t count = 8, p5 = 390625/* 5 to-the-power-of count */;
	int i = 0;
	do {
		size_t len = strlen(str[i]);
		uint64_t a = _get_rightmost_num(str[i], count, &len);
		*ptr_out++ = (a & 0xff);
		a >>= count;
		str[1 - i] = _add_string_to(str[i], len, fstr[1 - i], in_len, p5, a);
		i = 1 - i;
	} while (*str[i]);

	free(fstr[0]);
	free(fstr[1]);

	return x;
}
