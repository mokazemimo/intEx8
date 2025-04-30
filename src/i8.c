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
static intx_t _get_pow2(size_t p, cntx_t sign, dig_t* dest)
{
	intx_t x = { dest, 0 };
	x.size = p / INTEX8_DIGIT_BIT_WIDTH;
	for (size_t i = 0; i < x.size; ++i)
		dest[i] = 0;
	dest[x.size++] = 1 << (p % INTEX8_DIGIT_BIT_WIDTH);
	x.size *= sign;
	return x;
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
static cntx_t _get_highest_bit_position(intx_t x)
{
	cntx_t index = x.size - 1;
	cntx_t highest_bit = INTEX8_DIGIT_BIT_WIDTH * index + (INTEX8_DIGIT_BIT_WIDTH - 1);
	while (index >= 0 && x.ptr[index] == 0) {
		--index;
		highest_bit -= INTEX8_DIGIT_BIT_WIDTH;
	}

	if (index >= 0 && x.ptr[index] != 0) {
		size_t p = INTEX8_DIGIT_SIGN_MASK;
		for (size_t i = 0; i < INTEX8_DIGIT_BIT_WIDTH; ++i) {
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
	uni64_t u = { 0 };

	if (shift % INTEX8_DIGIT_BIT_WIDTH == 0) {
		size_t bx_len = INTEX8_DIGIT_COUNT_IN_64BITS;
		while (bx_len > 0 && bx.b[bx_len - 1] == 0)
			--bx_len;
		size_t i = shift / INTEX8_DIGIT_BIT_WIDTH;
		for (size_t j = 0; j < bx_len || u.a != 0; ++i, ++j) {
			u.a += (i < a.size ? (uint64_t)a.ptr[i] : 0) + (j < bx_len ? (uint64_t)bx.b[j] : 0);
			a.ptr[i] = u.b[0];
			u.a >>= INTEX8_DIGIT_BIT_WIDTH;
		}
		if (a.size < i)
			a.size = i;
	}
	else {
		size_t m = shift % INTEX8_DIGIT_BIT_WIDTH, n = INTEX8_DIGIT_BIT_WIDTH - m;
		uint64_t yL = 0;
		size_t i = shift / INTEX8_DIGIT_BIT_WIDTH;
		for (size_t j = 0; j < INTEX8_DIGIT_COUNT_IN_64BITS || u.a != 0; ++i, ++j) {
			uint64_t y = (j < INTEX8_DIGIT_COUNT_IN_64BITS ? (((uint64_t)bx.b[j] << m) & INTEX8_DIGIT_MAX_VALUE) : 0) | yL;
			yL = (j < INTEX8_DIGIT_COUNT_IN_64BITS ? ((uint64_t)bx.b[j] >> n) : 0);
			u.a += (i < a.size ? (uint64_t)a.ptr[i] : 0) + y;
			a.ptr[i] = u.b[0];
			u.a >>= INTEX8_DIGIT_BIT_WIDTH;
		}
		if (a.size < i)
			a.size = i;
	}

	return a;
}

/* Internal function. Do NOT call this directly! */
static intx_t _subtract_shifted_multiple_from(intx_t a, intx_t b, uint64_t b_multiple, size_t b_shift)
{	// a >= 0, b >= 0 and a >= b * (b_multiple << b_shift)
	// a -= b * (b_multiple << b_shift)
	uni64_t u = { 0 };

	if (b_shift % INTEX8_DIGIT_BIT_WIDTH == 0) {
		uint64_t h = 0;
		for (size_t i = b_shift / INTEX8_DIGIT_BIT_WIDTH, j = 0; i < a.size || j < b.size || u.a != 0 || h != 0; ++i, ++j) {
			uint64_t x = a.ptr[i];
			u.a += (j < b.size ? b_multiple * (uint64_t)b.ptr[j] : 0);
			if (x >= u.b[0] + h) {
				a.ptr[i] = x - (u.b[0] + h);
				h = 0;
			}
			else {
				a.ptr[i] = (INTEX8_DIGIT_MAX_VALUE + 1) + x - (u.b[0] + h);
				h = 1;
			}
			u.a >>= INTEX8_DIGIT_BIT_WIDTH;
		}
	}
	else {
		size_t digit_shift = b_shift / INTEX8_DIGIT_BIT_WIDTH;
		size_t m = b_shift % INTEX8_DIGIT_BIT_WIDTH, n = INTEX8_DIGIT_BIT_WIDTH - m;
		uint64_t h = 0, yL = 0;
		for (size_t i = digit_shift, j = 0; i < a.size && (j <= b.size || u.a != 0 || h != 0); ++i, ++j) {
			uint64_t x = (i < a.size ? (uint64_t)a.ptr[i] : 0);
			uint64_t y = (j < b.size ? (((uint64_t)b.ptr[j] << m) & INTEX8_DIGIT_MAX_VALUE) : 0) | yL;
			yL = (j < b.size ? ((uint64_t)b.ptr[j] >> n) : 0);
			u.a += b_multiple * y;
			if (x >= u.b[0] + h) {
				a.ptr[i] = x - (u.b[0] + h);
				h = 0;
			}
			else {
				a.ptr[i] = (INTEX8_DIGIT_MAX_VALUE + 1) + x - (u.b[0] + h);
				h = 1;
			}
			u.a >>= INTEX8_DIGIT_BIT_WIDTH;
		}
	}
	while (a.size >= 1 && a.ptr[a.size - 1] == 0)
		--a.size;
	return a;
}

/* Internal function. Do NOT call this directly! */
static uint64_t _get_uint(intx_t x, cntx_t* pos, cntx_t posy)
{
	cntx_t index = *pos / INTEX8_DIGIT_BIT_WIDTH;
	while (index >= 0 && x.ptr[index] == 0)
		--index;

	cntx_t m = INTEX8_DIGIT_BIT_WIDTH;
	uint64_t p = INTEX8_DIGIT_SIGN_MASK;
	for (cntx_t i = 0; i < INTEX8_DIGIT_BIT_WIDTH && (x.ptr[index] & p) == 0; ++i) {
		--m;
		p >>= 1;
	}
	cntx_t n = INTEX8_DIGIT_BIT_WIDTH - m;
	*pos = index * INTEX8_DIGIT_BIT_WIDTH + m - 1;

	uni64_t u = { 0 };
	cntx_t y_index = posy / INTEX8_DIGIT_BIT_WIDTH;
	cntx_t i = index;
	for (int j = INTEX8_DIGIT_COUNT_IN_64BITS / 2 - 1; i >= y_index && j >= 0; --i, --j)
		u.b[j] = ((uint64_t)x.ptr[i] << n) | ((uint64_t)x.ptr[i - 1] >> m);
	if (*pos - posy < 32) {
		u.a >>= (32 - (*pos - posy));
	}
	return u.a;
}

/* Internal function. Do NOT call this directly! */
static cntx_t _bits_prior_to(intx_t x, cntx_t p)
{	// ASSUMES x.size > 0
	// if any 1-bit of position < p exist
	// returns maximum index for which x.ptr[index] has 1-bit whose position is < p
	cntx_t q = _min(p / INTEX8_DIGIT_BIT_WIDTH, x.size);
	if (q < x.size) {
		p %= INTEX8_DIGIT_BIT_WIDTH;
		dig_t r = 1;
		for (cntx_t i = 0; i < p; ++i) {
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

	cntx_t sign = _sgn(x.size);
	x.size = _abs(x.size);
	if (p >= x.size * INTEX8_DIGIT_BIT_WIDTH) {
		x.size *= sign;
		return x;
	}

	cntx_t q = _bits_prior_to(x, p);
	if (q == 0) {
		//return intx_zero;
		x.size = 0;
		return x;
	}
	--q;
	if (q < p / INTEX8_DIGIT_BIT_WIDTH)
		q = p / INTEX8_DIGIT_BIT_WIDTH;

	p %= INTEX8_DIGIT_BIT_WIDTH;
	dig_t r = 1 << p;
	for (; r; r <<= 1)
		x.ptr[q] &= ~r;
	x.size = sign * (q + 1);
	return x;
}

/* Internal function. Do NOT call this directly! */
static intx_t _right_shift(intx_t x, cntx_t bits, dig_t* dest, bool b_ext)
{
	if (i8_is_zero(x))
		return intx_zero;

	cntx_t x_size = _abs(x.size);
	intx_t y = { dest, 0 };
	bits %= x_size * INTEX8_DIGIT_BIT_WIDTH;

	const cntx_t bit_width = INTEX8_DIGIT_BIT_WIDTH;

	cntx_t d = 0;
	cntx_t s = d + bits / bit_width;

	if (bits % bit_width == 0) {
		while (s + INTEX8_DIGIT_COUNT_IN_64BITS <= x_size) {
			*(uint64_t*)(dest + d) = *(uint64_t*)(x.ptr + s);
			d += INTEX8_DIGIT_COUNT_IN_64BITS;
			s += INTEX8_DIGIT_COUNT_IN_64BITS;
		}
		while (s < x_size)
			dest[d++] = x.ptr[s++];
	}
	else {
		cntx_t m = bits % bit_width;
		cntx_t n = bit_width - m;

		while (s + INTEX8_DIGIT_COUNT_IN_64BITS + 1 <= x_size) {
			*(uint64_t*)(dest + d) = (*(uint64_t*)(x.ptr + s + 1) << n) | (x.ptr[s] >> m);
			d += INTEX8_DIGIT_COUNT_IN_64BITS;
			s += INTEX8_DIGIT_COUNT_IN_64BITS;
		}
		while (s + 1 < x_size) {
			dest[d] = (x.ptr[s + 1] << n) | (x.ptr[s] >> m);
			++d;
			++s;
		}
		dest[d++] = _right_shift_digit(x.ptr[x_size - 1], m);
	}
	if (b_ext) {
		while (d < x_size)
			dest[d++] = 0;
		y.size = x.size;
	}
	else {
		y.size = _sgn(x.size) * d;
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
		int64_t px = i8_is_pow2(x);
		if (px == 0) {
			if (i8_gt_zero(x)) {
				return _right_shift(x, p, dest, false);
			}
			else {
				x = i8_negate(x, NULL);
				return i8_negate(_right_shift(x, p, dest, false), NULL);
			}
		}
		else if (px - 1 >= p) {
			return _get_pow2((px - 1) - p, 1, dest);
		}
		else if (-(px + 1) >= p) {
			return _get_pow2(-(px + 1) - p, -1, dest);
		}
		else {
			intx_t z = { dest, 0 };
			return z;
		}
	}
	else {
		p = -p;
		int64_t px = i8_is_pow2(x);
		if (px == 0) {
			if (i8_gt_zero(x)) {
				intx_t y = _right_shift(x, p, dest, false);
				return i8_negate(y, dest);
			}
			else {
				x = i8_negate(x, NULL);
				return _right_shift(x, p, dest, false);
			}
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

/*
 * Creates a copy of x and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `x.size` digits space.
 */
intx_t i8_copy(const intx_t x, dig_t* dest)
{
	if (x.ptr == NULL)
		return intx_zero;
	intx_t y = { dest, x.size };
	const cntx_t y_size = _abs(y.size);
	const dig_t* ptr = x.ptr;
	for (cntx_t i = 0; i < y_size; ++i)
		*dest++ = *ptr++;
	return y;
}

/*
 * Creates a big integer equal to an int64 value and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `_required_digits_for_int64(x)` digits space.
 */
intx_t i8_copy_i(int64_t x, dig_t* dest)
{
	if (x == 0) {
		return intx_zero;
	}

	cntx_t sign = 1;
	if (x < 0) {
		x = -x;
		sign = -1;
	}

	cntx_t z_size = _required_digits_for_int64(x);
	intx_t z = { dest, z_size };

	uni_t ux = { x };
	if (z_size == 1) {
		*dest++ = ux.b[0];
	}
	else if (z_size == 2) {
		*dest++ = ux.b[0];
		*dest++ = ux.b[1];
	}
	z.size *= sign;
	return z;
}

/*
 *  Trims leading trivial digits from a big integer in place.
 */
intx_t i8_trim(const intx_t xi)
{
	intx_t x = { xi.ptr, _abs(xi.size) };
	while (x.size >= 1 && x.ptr[x.size - 1] == 0)
		--x.size;
	x.size *= _sgn(xi.size);
	return x;
}

//-------------------------------------------------------------------------------------------------------
// Arithmetic operations

/* Internal function. Do NOT call this directly! */
static intx_t _add(intx_t xi, intx_t yi, dig_t* dest)
{	// _sgn(xi) == _sgn(yi)

	intx_t x = xi;
	intx_t y = yi;

	x.size = _abs(x.size);
	y.size = _abs(y.size);
	if (x.size > y.size) {
		x = yi;
		y = xi;
		x.size = _abs(x.size);
		y.size = _abs(y.size);
	}

	intx_t z = { dest, 0 };
	uni_t u = { 0 };

	size_t s = 0;
	for (; s < x.size; ++s) {
		u.a += (uint64_t)(*x.ptr++) + (*y.ptr++);
		*dest++ = u.b[0];
		u.a = u.b[1];
	}
	for (; s < y.size; ++s) {
		u.a += *y.ptr++;
		*dest++ = u.b[0];
		u.a = u.b[1];
	}
	if (u.a != 0) {
		*dest++ = u.b[0];
		++s;
	}
	z.size = s * _sgn(xi.size);
	return z;
}

/* Internal function. Do NOT call this directly! */
static intx_t _sub(intx_t x, intx_t y, dig_t* dest)
{
	// |x| >= |y|  ==> 	//|x.size| >= |y.size|

	cntx_t sign = _sgn(x.size);
	x.size = _abs(x.size);
	y.size = _abs(y.size);

	intx_t z = { dest, 0 };

	uint64_t h = 0;
	cntx_t s = 0;
	for (; s < y.size; ++s) {
		if (*x.ptr >= h + *y.ptr) {
			*dest++ = *x.ptr++ - (h + *y.ptr++);
			h = 0;
		}
		else {
			*dest++ = (INTEX8_DIGIT_MAX_VALUE + 1) + *x.ptr++ - (h + *y.ptr++);
			h = 1;
		}
	}
	if (h == 1) {
		for (; s < x.size && *x.ptr++ == 0; ++s) {
			*dest++ = INTEX8_DIGIT_MAX_VALUE;
		}
		*dest++ = x.ptr[-1] - 1;
		++s;
	}
	for (; s < x.size; ++s) {
		*dest++ = *x.ptr++;
	}

	z.size = sign * s;
	return z;
}

/* Internal function. Do NOT call this directly! */
static int _cmp(intx_t x, intx_t y)
{
	cntx_t i = _abs(x.size);
	cntx_t j = _abs(y.size);

	if (i != j) {
		return i > j ? 1 : -1;
	}
	--i;
	for (; i >= 0 && (x.ptr[i] == y.ptr[i]); --i) {
	}
	return i < 0 ? 0 : (x.ptr[i] > y.ptr[i]) ? 1 : -1;
}

/*
 * Adds two big integers and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `_required_digits_for_sum(x, y)` digits space.
 */
intx_t i8_add(const intx_t x, const intx_t y, dig_t* dest)
{
	if (x.size == 0 && y.size == 0) {
		return intx_zero;
	}
	else if (x.size == 0) {
		return i8_copy(y, dest);	// dest == NULL ? y : i8_copy(y, dest);
	}
	else if (y.size == 0) {
		return i8_copy(x, dest);	// dest == NULL ? x : i8_copy(x, dest);
	}
	else if (x.size > 0 && y.size > 0) {
		return _add(x, y, dest);
	}
	else if (x.size < 0 && y.size < 0) {
		return _add(x, y, dest);
	}
	else if (_abs(x.size) > _abs(y.size) || _cmp(x, y) == 1) {
		return _sub(x, y, dest);
	}
	else if (_abs(y.size) > _abs(x.size) || _cmp(y, x) == 1) {
		return _sub(y, x, dest);
	}
	else {
		return intx_zero;
	}
}

/*
 * Adds an int64 to a big integer and returns the result stored in `dest`.
 * Caller must ensure `dest` has enough space (call _required_digits_for_sum(x, i8_copy_i(y,..))).
 */
intx_t i8_add_i(const intx_t x, int64_t y, dig_t *dest)
{
	if (y == 0)
		return i8_copy(x, dest);	// dest == NULL ? x : i8_copy(x, dest);
	else if (y > 0)
		return i8_add(x, (intx_t) { (dig_t*)&y, INTEX8_DIGIT_COUNT_IN_64BITS }, dest);
	else {
		y = -y;
		return i8_add(x, (intx_t) { (dig_t*)&y, -INTEX8_DIGIT_COUNT_IN_64BITS }, dest);
	}
}

/*
 * Subtracts two big integers and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `_required_digits_for_sum(x, -y)` digits space.
 */
intx_t i8_sub(const intx_t x, const intx_t y, dig_t* dest)
{
	return i8_add(x, (intx_t) { y.ptr, -y.size }, dest);
}

/*
 * Subtracts an int64 from a big integer and returns the result stored in `dest`.
 * Caller must ensure `dest` has enough space (call _required_digits_for_sum(x, i8_copy_i(-y,..))).
 */
intx_t i8_sub_i(const intx_t x, int64_t y, dig_t* dest)
{
	return y == 0 ? i8_copy(x, dest) : i8_add_i(x, -y, dest);
}

/*
 * Subtracts a big integer from an int64 and returns the result stored in `dest`.
 * Caller must ensure `dest` has enough space (call _required_digits_for_sum(i8_copy_i(x,..), -y)).
 */
intx_t i8_i_sub(int64_t x, const intx_t y, dig_t* dest)
{
	return i8_add_i((intx_t) {y.ptr, -y.size}, x, dest);
}

/*
 * Multiplies two big integers and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `|x.size| + |y.size|` digits space.
 */
intx_t i8_mul(const intx_t xi, const intx_t yi, dig_t* dest)
{
	int64_t px = i8_is_pow2(xi);
	if (px == 1) {
		return i8_copy(yi, dest);
	}
	else if (px == -1) {
		return i8_negate(yi, dest);
	}
	else if (px != 0) {
		return i8_mul_p2(yi, px > 0 ? px - 1 : px + 1, dest);
	}

	int64_t py = i8_is_pow2(yi);
	if (py == 1) {
		return i8_copy(xi, dest);
	}
	else if (py == -1) {
		return i8_negate(xi, dest);
	}
	else if (py != 0) {
		return i8_mul_p2(xi, py > 0 ? py - 1 : py + 1, dest);
	}

	intx_t x = xi;
	intx_t y = yi;

	cntx_t x_size = _abs(x.size);
	cntx_t y_size = _abs(y.size);
	cntx_t zero_count = 0, x_zeros = 0, y_zeros = 0;

	while (x_zeros < x_size && x.ptr[x_zeros] == 0)
		++x_zeros;
	if (x_zeros == x_size)
		return intx_zero;
	while (y_zeros < y_size && y.ptr[y_zeros] == 0)
		++y_zeros;
	if (y_zeros == y_size)
		return intx_zero;
	zero_count = x_zeros + y_zeros;

	x.ptr += x_zeros;
	x_size -= x_zeros;

	y.ptr += y_zeros;
	y_size -= y_zeros;

	dig_t* dest0 = dest;
	for (size_t i = 0; i < zero_count; ++i)
		*dest++ = 0;

	if (_max(x_size, y_size) > INTEX8_MAX_MULTIPLICATION_DIGITS) {
		intEx8_errno = INTEX8_ERR_MAX_MULTIPLICATION_DIGITS_EXCEEDED;
		return intx_zero;
	}

	cntx_t sign = _sgn(x.size) * _sgn(y.size);
	x.size = x_size;
	y.size = y_size;

	intx_t _toom3_multiply(intx_t x, intx_t y, dig_t * dest);
	intx_t z = _toom3_multiply(x, y, dest);

	z.ptr = dest0;
	z.size = sign * (z.size + zero_count);

	return z;
}

/*
 * Multiplies one big integers by an int64 and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `|x.size| + _required_digits_for_int64(yi)` digits space.
 */
intx_t i8_mul_i(const intx_t xi, int64_t yi, dig_t* dest)
{
	intx_t x = i8_trim(xi);
	cntx_t x_size = _abs(x.size);
	if (x_size == 0 || yi == 0) {
		return intx_zero;
	}
	int sign = 1;
	if (yi < 0) {
		yi = -yi;
		sign = -1;
	}

	uni_t uy = { yi };
	if (uy.b[1] == 0) {

		intx_t z = { dest, 0 };

		uint64_t y = yi;// uy.b[0];

		uy.a = 0;
		for (cntx_t i = 0; i < x_size; ++i) {
			uy.a += y * *x.ptr++;
			*dest++ = uy.b[0];
			uy.a = uy.b[1];
		}
		if (uy.a != 0) {
			*dest++ = uy.b[0];
			//++i;
		}
		z.size = sign * (dest - z.ptr);
		return z;
	}
	else {
		intx_t z = { dest, 0 };

		union {
			uint64_t a[2];
			dig_t b[4];
		} u = { 0, 0 }, v;

		int i = 0, j;

		u.a[0] = (uint64_t)uy.b[0] * x.ptr[i++];
		*dest++ = u.b[0];

		u.b[0] = u.b[1];
		u.b[1] = u.b[2];
		u.b[2] = u.b[3];

		for (; i < x_size; ++i) {
			uint64_t a = (uint64_t)uy.b[0] * x.ptr[i];
			uint64_t b = (uint64_t)uy.b[1] * x.ptr[i - 1];
			v.a[0] = a + b;
			v.a[1] = (v.a[0] < a) ? 1 : 0; // carry of a + b

			/// add {v.b, 3} to {u.b, 4}
			uni_t w = { 0 };
			for (j = 0; j < 3; ++j) {
				w.a += (uint64_t)u.b[j] + v.b[j];
				u.b[j] = w.b[0];
				w.a = w.b[1];
			}
			if (w.a != 0) {
				u.b[3] = w.b[0];
			}
			*dest++ = u.b[0];

			u.b[0] = u.b[1];
			u.b[1] = u.b[2];
			u.b[2] = u.b[3];
			u.b[3] = 0;
		}
		{
			v.a[0] = (uint64_t)uy.b[1] * x.ptr[x_size - 1];
			v.a[1] = 0;// (v.a[0] < a) ? 1 : 0; // carry of a + b

			/// add {v.b, 3} to {u.b, 4}
			uni_t w = { 0 };
			for (j = 0; j < 3; ++j) {
				w.a += (uint64_t)u.b[j] + v.b[j];
				u.b[j] = w.b[0];
				w.a = w.b[1];
			}
			if (w.a != 0) {
				u.b[3] = w.b[0];
			}
			*dest++ = u.b[0];
			//++i;

			u.b[0] = u.b[1];
			u.b[1] = u.b[2];
			u.b[2] = u.b[3];
			u.b[3] = 0;
		}
		if (u.b[2] != 0) {
			*dest++ = u.b[0];
			//++i;
			*dest++ = u.b[1];
			//++i;
			*dest++ = u.b[2];
			//++i;
		}
		else if (u.b[1] != 0) {
			*dest++ = u.b[0];
			//++i;
			*dest++ = u.b[1];
			//++i;
		}
		else if (u.b[0] != 0) {
			*dest++ = u.b[0];
			//++i;
		}
		z.size = sign * (dest - z.ptr);
		return z;
	}
}

/*
 * Multiplies one big integer `x` by `sgn(y) * 2^|y|` and returns the result stored in `dest`.
 * Caller must ensure `dest` has enough space for the result.
 */
intx_t i8_mul_p2(const intx_t x, int64_t p, dig_t* dest)
{
	if (p == 0) {
		return i8_copy(x, dest);
	}
	if (x.size == 0) {
		return x;
	}

	cntx_t sgn_p = _sgn(p);
	if (p < 0)
		p = -p;

	const int bit_count = INTEX8_DIGIT_BIT_WIDTH;

	cntx_t x_size = _abs(x.size);
	intx_t y = { dest, 0 };
	if (p % bit_count == 0) {
		p /= bit_count;
		for (cntx_t i = 0; i < p; ++i)
			dest[i] = 0;
		for (cntx_t j = 0; j < x_size; ++j)
			dest[p + j] = x.ptr[j];
		y.size = sgn_p * _sgn(x.size) * (x_size + p);
	}
	else {
		cntx_t m = p % bit_count;
		cntx_t n = bit_count - m;
		p /= bit_count;
		for (cntx_t i = 0; i < p; ++i)
			dest[i] = 0;

		uint64_t k = 0;
		for (cntx_t i = 0; i < x_size; ++i) {
			dest[p + i] = (x.ptr[i] << m) | (k >> n);
			k = x.ptr[i];
		}
		dest[p + x_size] = _right_shift_digit(x.ptr[x_size - 1], n);
		y.size = sgn_p * _sgn(x.size) * (p + x_size + 1);
	}
	return y;
}

/*
 * Divides `x` by `y` and returns the quotient stored in `dest`.
 * Caller must ensure `dest` has enough space for the result.
 */
intx_t i8_div(intx_t x, const intx_t yi, dig_t *dest)
{
	if (i8_is_zero(yi)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return intx_zero;
	}
	else if (i8_is_zero(x)) {
		return intx_zero;
	}

	int64_t py = i8_is_pow2(yi);
	if (py == 1) {
		return i8_copy(x, dest);
	}
	else if (py == -1) {
		return i8_negate(x, dest);
	}
	else if (py != 0) {
		return _divide_pow2(x, py > 0 ? py - 1 : py + 1, dest);
	}

	if (_cmp(x, yi) == -1) {
		return intx_zero;
	}

	intx_t y = yi;
	cntx_t sign = _sgn(x.size) * _sgn(y.size);
	x.size = _abs(x.size);
	y.size = _abs(y.size);

	size_t div_size = _get_quotient_size(_abs(x.size), _abs(y.size));
	if (div_size <= 0)
		return intx_zero;

	cntx_t posy = _get_highest_bit_position(y);
	cntx_t posx = x.size * INTEX8_DIGIT_BIT_WIDTH - 1;

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
	if (posx == posy && i8_le(i8_trim(y), i8_trim(x))) {
		z = _add_shifted_to(z, 1, 0);
	}

	z.size *= sign;
	return z;
}

/*
 * Devides one big integer `x` by `sgn(y) * 2^y` and returns the result stored in `dest`.
 * Caller must ensure `dest` has enough space for the result.
 */
intx_t i8_div_p2(intx_t x, int64_t p, dig_t* dest)
{
	return _divide_pow2(x, p, dest);
}

/*
 * Computes `x % y` and returns the result stored in `dest`.
 * Caller must ensure `dest` has enough space for the result.
 */
intx_t i8_mod(intx_t x, const intx_t yi, dig_t *dest)
{
	if (i8_is_zero(yi)) {
		intEx8_errno = INTEX8_ERR_DIVISION_BY_ZERO;
		return intx_zero;
	}
	else if (i8_is_zero(x)) {
		return intx_zero;
	}

	int64_t py = i8_is_pow2(yi);
	if (py != 0) {
		return _rightmost_bits(x, py > 0 ? (py - 1) : (-py - 1));
	}
	if (_cmp(x, yi) == -1) {
		return x;
	}

	intx_t y = yi;
	cntx_t sign = _sgn(x.size);
	x.size = _abs(x.size);
	y.size = _abs(y.size);

	cntx_t posy = _get_highest_bit_position(y);
	cntx_t posx = x.size * INTEX8_DIGIT_BIT_WIDTH - 1;

	while (posx > posy) {
		uint64_t u = _get_uint(x, &posx, posy);
		int shift = (posx - posy > 32 ? posx - posy - 32 : 0);
		x = _subtract_shifted_multiple_from(x, y, u, shift);
	}
	if (posx == posy && i8_le(i8_trim(y), i8_trim(x))) {
		x = _subtract_shifted_multiple_from(x, y, 1, 0);
	}
	x.size *= sign;
	return dest == NULL ? x : i8_copy(x, dest);
}

/*
 * Computes the remainder of the division of a big integer `x` by `2^|y|` (x % 2^|y|) and returns the result stored in `dest`.
 * Caller must ensure `dest` has enough space for the result.
 */
intx_t i8_mod_p2(intx_t x, uint64_t y, dig_t* dest)
{
	if (dest == NULL) {
		return _rightmost_bits(x, y);
	}
	else {
		return i8_copy(_rightmost_bits(x, y), dest);
	}
}

/*
 * Computes `-x` and returns the result stored in `dest`.
 * Caller must ensure `dest` is either NULL or has at least `x.size` digits.
 */
intx_t i8_negate(const intx_t x, dig_t* dest)
{
	if (dest == NULL) {
		return (intx_t) { x.ptr, -x.size };
	}
	else {
		return i8_copy((intx_t) { x.ptr, -x.size }, dest);
	}
}

void i8_negate_me(intx_t* x)
{
	x->size = -x->size;
}

/*
 * Computes `|x|` and returns the result stored in `dest`.
 * Caller must ensure `dest` has at least `x.size` digits.
 */
intx_t i8_abs(const intx_t x, dig_t *dest)
{
	if (dest == NULL) {
		return (intx_t) { x.ptr, _abs(x.size) };
	}
	else {
		return i8_copy((intx_t) { x.ptr, _abs(x.size) }, dest);
	}
}

void i8_abs_me(intx_t* x)
{
	x->size = _abs(x->size);
}

// Comparison operators
/*
 * Compares two big integers. Returns `true` if `x == y`, `false` otherwise.
 * Assumes x and y are trimmed.
 */
bool i8_eq(const intx_t x, const intx_t y)
{
	if (i8_is_zero(x) && i8_is_zero(y))
		return true;
	else if (i8_is_zero(x) || i8_is_zero(y))
		return false;
	else if (x.size != y.size)
		return false;
	else
		return _cmp(x, y) == 0;
}

/*
 * Compares two big integers. Returns `true` if `x <= y`, `false` otherwise.
 * Assumes x and y are trimmed.
 */
bool i8_le(const intx_t x, const intx_t y)
{
	if (x.size < y.size)
		return true;
	else if (x.size > y.size)
		return false;
	cntx_t s = _abs(x.size) - 1;
	while (s >= 0 && x.ptr[s] == y.ptr[s])
		--s;
	return s < 0 || (x.size > 0 ? x.ptr[s] <= y.ptr[s] : x.ptr[s] >= y.ptr[s]);
}

/*
 * Compares a big integer with 0. Returns `true` if `x == 0`, `false` otherwise.
 */
bool i8_is_zero(const intx_t x)
{
	cntx_t i = _abs(x.size) - 1;
	while (i >= 0 && x.ptr[i] == 0)
		--i;
	return i == -1;
}

/*
 * Compares a big integer with 0. Returns `true` if `x > 0`, `false` otherwise.
 */
bool i8_gt_zero(const intx_t x)
{
	return x.size > 0 && !i8_is_zero(x);
}

/*
 * Compares a big integer with 0. Returns `true` if `x < 0`, `false` otherwise.
 */
bool i8_lt_zero(const intx_t x)
{
	return x.size < 0 && !i8_is_zero(x);
}

/*
 * Compares a big integer by an int64. Returns `true` if `x == y`, `false` otherwise.
 * Assumes x is trimmed.
 */
bool i8_eq_i(const intx_t x, const int64_t y)
{
	if (x.size == 2) {
		return (y > 0 && *(int64_t *)x.ptr == y);
	}
	else if (x.size == 1) {
		return (y > 0 && y == x.ptr[0]);
	}
	else if (x.size == 0) {
		return y == 0;
	}
	else if (x.size == -1) {
		return (y < 0 && -y == x.ptr[0]);
	}
	else if (x.size == -2) {
		return (y < 0 && *(int64_t*)x.ptr == -y);
	}
	else {
		return false;
	}
}

/*
 * Compares a big integer by an int64. Returns `true` if `x <= y`, `false` otherwise.
 * Assumes x is trimmed.
 */
bool i8_le_i(const intx_t x, const int64_t y)
{
	if (x.size > 2) {
		return false;
	}
	else if (x.size == 2) {
		return y <= 0 ? false : *(uint64_t *)x.ptr <= y;
	}
	else if (x.size == 1) {
		return x.ptr[0] <= y;
	}
	else if (x.size == 0) {
		return y >= 0;
	}
	else if (x.size == -1) {
		return -(int64_t)x.ptr[0] <= y;
	}
	else if (x.size == -2) {
		return y >= 0 ? true : *(uint64_t*)x.ptr >= (uint64_t)(-y);
	}
	else {
		return true;
	}
}

/*
 * Compares a big integer by an int64. Returns `true` if `x < y`, `false` otherwise.
 * Assumes x is trimmed.
 */
bool i8_lt_i(const intx_t x, const int64_t y)
{
	if (x.size > 2) {
		return false;
	}
	else if (x.size == 2) {
		return y <= 0 ? false : *(uint64_t*)x.ptr < y;
	}
	else if (x.size == 1) {
		return x.ptr[0] < y;
	}
	else if (x.size == 0) {
		return y > 0;
	}
	else if (x.size == -1) {
		return -(int64_t)x.ptr[0] < y;
	}
	else if (x.size == -2) {
		return y >= 0 ? true : *(uint64_t*)x.ptr > (uint64_t)(-y);
	}
	else {
		return true;
	}
}

/*
 * Checks if `x` is of the form `pow(2, n)` or `-pow(2, n)`, for some non-negative integer `n`.
 * Returns `n + 1` if `x == pow(2, n)`; `-(n + 1)` if `x == -pow(2, n)`; `0` otherwise.
 */
int64_t i8_is_pow2(const intx_t x)
{
	cntx_t x_size = _abs(x.size);
	cntx_t i = 0;
	while (i < x_size && x.ptr[i] == 0)
		++i;
	if (i >= x_size)
		return 0;

	int64_t pos = i * INTEX8_DIGIT_BIT_WIDTH;
	dig_t p = 1;
	for (; p; p <<= 1) {
		if ((x.ptr[i] & p) == p)
			break;
		++pos;
	}
	// pos is the least-significant 1-bit
	if (pos + 1 == x_size * INTEX8_DIGIT_BIT_WIDTH)
		return _sgn(x.size) * (pos + 1);

	p <<= 1;
	// all higher bits MUST be 0
	for (; p; p <<= 1) {
		if ((x.ptr[i] & p) == p)
			return 0;
	}
	for (cntx_t j = i + 1; j < x_size; ++j) {
		if (x.ptr[j] != 0)
			return 0;
	}
	return _sgn(x.size) * (pos + 1);
}

//----------------------------------------------------------------------------------------------------------
// String conversion
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

/*
 * Converts a POSITIVE big integer to its string representation, storing the result in `str`.
 * Caller must fill (memset) `str` with '\0' for `size` bytes.
 * `size` specifies the allocated space (in bytes) for `str` and must be large enough to store the result.
 * Use `_required_decimal_count(x.size, false)` to determine the required size.
 */
char* i8_copy_to_s(const intx_t x, char* str, size_t size)
{
	if (size == 0)
		return str;

	char* ptr = str;
	const uint64_t p32 = 4294967296 /* 2^32 */;
	
	for (size_t i = x.size; i-- > 0;)
		ptr = _add_string_to(ptr, strlen(ptr), str, size - 1, p32, x.ptr[i]);

	return ptr;
}

/*
 * Parses a big integer from its decimal string representation and stores it in `dest`.
 * Caller must ensure `dest` has enough space for the result using `_required_digit_count(in_len)`.
 * Caller must ensure the memory allocated for `dest` is initialized (memset) with 0.
 * Caller must ensure `in_str` represents a POSITIVE big integer (ONLY the digits '0' to '9').
 */
intx_t i8_copy_s(const char* in_str, size_t in_len, dig_t *dest)
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
