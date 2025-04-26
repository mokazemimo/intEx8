/*
 *  File: toom3_multiply.c
 *  Description:
 *      This file contains internal functions used for TOOM-COOK (TOOM-3) multiplication.
 *      These functions are not intended to be called directly by users.
 *      Only public API functions should be used externally.
 */

#include <memory.h>

#pragma pack(8)

#include "i8.h"
#include "util.h"

#define _swap(T, u, v)		{ T t=u; u=v; v=t; }

#define _CMP(x_ptr, x_size, y_ptr, y_size)	\
cntx_t x_cmp_y = 0;							\
{	\
	if (x_size != y_size) {					\
		x_cmp_y = x_size > y_size ? 1 : -1;	\
	}										\
	else if (x_size == 0)					\
		x_cmp_y = 0;						\
	else {									\
		dig_t* xp = x_ptr + x_size;			\
		dig_t* yp = y_ptr + y_size;			\
		for (; xp > x_ptr && *(--xp) == *(--yp); ) {	\
		}									\
		x_cmp_y = (xp < x_ptr ? 0 : (*xp > *yp ? 1 : *xp < *yp ? -1 : 0));	\
	}	\
}

/* Internal function. Do NOT call this directly! */
static void _remove_leading_trivial_digits(intx_t* a)
{
	dig_t* ptr = a->ptr + _abs(a->size);
	while (ptr > a->ptr && ptr[-1] == 0)
		--ptr;
	a->size = (ptr - a->ptr) * _sgn(a->size);
}

#define _shift_l1(a, i, size)	(dig_t)((i) == 0 ? (a.ptr[i] << 1) : ((i) == size ? 0 : (a.ptr[i] << 1)) | (a.ptr[(i) - 1] >> (INTEX8_DIGIT_BIT_WIDTH - 1)))
#define _shift_l2(a, i, size)	(dig_t)((i) == 0 ? (a.ptr[i] << 2) : ((i) == size ? 0 : (a.ptr[i] << 2)) | (a.ptr[(i) - 1] >> (INTEX8_DIGIT_BIT_WIDTH - 2)))

static bool _sum_gt(intx_t a, intx_t b, intx_t c)
{
	// returns true if |a| + |b| > |c|, false otherwise

	cntx_t a_size = _abs(a.size);
	cntx_t b_size = _abs(b.size);
	if (a_size > b_size) {
		_swap(intx_t, a, b);
		a_size = _abs(a.size);
		b_size = _abs(b.size);
	}
	// ASSERT (a_size <= b_size)
	cntx_t c_size = _abs(c.size);

	if (c_size < b_size) {
		return true;
	}
	else if (c_size > b_size + 1) {
		return false;
	}

	// ASSERT (c_size == b_size) || (c_size == b_size + 1)
	int debt = 0;
	cntx_t i;
	if (c_size == b_size + 1) {
		if (c.ptr[b_size] > 1)
			return false;
		debt = 1;
		i = b_size - 1;
	}
	else { // ASSERT (c_size == b_size)
		for (i = b_size - 1; i >= a_size && (b.ptr[i] == c.ptr[i]); --i) {
		}
		if (i >= a_size && (uint64_t)b.ptr[i] + 1 < c.ptr[i])
			return false;
		if (i >= a_size && (uint64_t)b.ptr[i] > c.ptr[i])
			return true;
		// ASSERT (i == a_size - 1) || (i >= a_size && (uint64_t)b.ptr[i] + 1 == c.ptr[i])
		if (i >= a_size && b.ptr[i] + 1 == c.ptr[i]) {
			debt = 1;
			--i;
		}
	}
	if (debt == 1) {
		for (; i >= a_size && ((uint64_t)b.ptr[i] == INTEX8_DIGIT_MAX_VALUE && c.ptr[i] == 0); --i) {
		}
		if (i >= a_size)
			return false;
	}

	uni_t u = { 0 };
	++i;
	for (; i >= 0;)
	{
		if (debt == 0) {
			for (--i; i >= 0 && ((uint64_t)a.ptr[i] + b.ptr[i] == c.ptr[i]); --i) {
			}
			if (i < 0)
				return false;
			if ((uint64_t)a.ptr[i] + b.ptr[i] + 1 < c.ptr[i])
				return false;
			else if ((uint64_t)a.ptr[i] + b.ptr[i] > c.ptr[i])
				return true;
			// ASSERT (uint64_t)a.ptr[i] + b.ptr[i] + 1 == c.ptr[i]
		}
		do
		{
			for (--i; i >= 0 && ((uint64_t)a.ptr[i] + b.ptr[i] == INTEX8_DIGIT_MAX_VALUE && c.ptr[i] == 0); --i) {
			}
			if (i < 0)
				return false;
			u.a = (uint64_t)a.ptr[i] + b.ptr[i];
			if (u.a <= INTEX8_DIGIT_MAX_VALUE || (u.b[1] == 1 && u.b[0] + 1 < c.ptr[i]))
				return false;
			else if (u.b[1] == 1 && u.b[0] > c.ptr[i])
				return true;
			// ASSERT u.b[1] == 1 && (u.b[0] == c.ptr[i] || u.b[0] + 1 == c.ptr[i])
		} while (u.b[0] + 1 == c.ptr[i]);
		debt = 0;
	}
	return false;
}

#define _shift_left(x, b)		((dig_t)(x << b))
#define _shift_right(x, b)		((dig_t)(x >> b))

/* Internal function. Do NOT call this directly! */
static bool _lt_2x(intx_t a, intx_t b)
{	// returns true if |a| < 2*|b|, false otherwise
	if (b.size == 0)
		return false;
	else if (a.size == 0)
		return true;

	int as = _abs(a.size);
	int bs = _abs(b.size);
	if (as < bs) {
		return true;
	}
	else if (as == bs) {
		dig_t* b_ptr = b.ptr + (bs - 1);
		if (*b_ptr & INTEX8_DIGIT_SIGN_MASK)
			return true;
		dig_t* a_ptr = a.ptr + (as - 1);
		if (*a_ptr < 2 * *b_ptr) {
			return true;
		}

		for (; a_ptr > a.ptr; ) {
			dig_t d = (_shift_left(*b_ptr, 1) | _shift_right(b_ptr[-1], INTEX8_DIGIT_BIT_WIDTH - 1));
			if (*a_ptr < d)
				return true;
			else if (*a_ptr > d)
				return false;
			--a_ptr;
			--b_ptr;
		}
		return (a.ptr[0] < _shift_left(b.ptr[0], 1));
	}
	else if (as == 1 && bs == 0) {
		return false;
	}
	else if (as == bs + 1) {
		dig_t* a_ptr = a.ptr + (as - 1);
		if (*a_ptr > 1) {
			return false;
		}
		dig_t* b_ptr = b.ptr + (bs - 1);
		if (*b_ptr < INTEX8_DIGIT_SIGN_MASK)
			return false;

		for (; a_ptr > a.ptr; ) {
			dig_t d = (_shift_left(*a_ptr, INTEX8_DIGIT_BIT_WIDTH - 1) | _shift_right(a_ptr[-1], 1));
			if (d < *b_ptr) {
				return true;
			}
			else if (d > *b_ptr) {
				return false;
			}
			--a_ptr;
			--b_ptr;
		}
		return (a.ptr[0] & 1) != 1;
	}
	else {
		return false;
	}
}

static bool _4a_plus_b_gt_2c(intx_t a, intx_t b, intx_t c)
{
	// returns true if 4|a| + |b| >= 2|c|, false otherwise

	if (a.size == 0) {
		return !_lt_2x(b, c);
	}
	else if (c.size == 0) {
		return true;
	}
	else if (b.size == 0) {
		return _lt_2x(c, a);
	}

	cntx_t a_size0 = _abs(a.size);
	cntx_t a_size = a_size0;
	dig_t amb = (a.ptr[a_size - 1] & 0xc0000000);
	if (a_size > 0 && amb != 0)
		++a_size;
	cntx_t b_size = _abs(b.size);
	cntx_t c_size0 = _abs(c.size);
	cntx_t c_size = c_size0;
	dig_t cmb = (c.ptr[c_size - 1] & INTEX8_DIGIT_SIGN_MASK);
	if (c_size > 0 && cmb != 0)
		++c_size;

	if (c_size < _max(a_size, b_size)) {
		return true;
	}
	else if (c_size > _max(a_size, b_size) + 1) {
		return false;
	}

	if (a_size < b_size) {
		int debt = 0;
		cntx_t i;
		if (c_size == b_size + 1) {
			if (_shift_l1(c, b_size, c_size0) > 1)
				return false;
			debt = 1;
			i = b_size - 1;
		}
		else { // ASSERT (c_size == b_size)
			for (i = b_size - 1; i >= a_size && (b.ptr[i] == _shift_l1(c, i, c_size0)); --i) {
			}
			if (i >= a_size && (uint64_t)b.ptr[i] + 1 < _shift_l1(c, i, c_size0))
				return false;
			if (i >= a_size && (uint64_t)b.ptr[i] > _shift_l1(c, i, c_size0))
				return true;
			// ASSERT (i == a_size - 1) || (i >= a_size && (uint64_t)b.ptr[i] + 1 == c.ptr[i])
			if (i >= a_size && b.ptr[i] + 1 == _shift_l1(c, i, c_size0)) {
				debt = 1;
				--i;
			}
		}
		if (debt == 1) {
			for (; i >= a_size && ((uint64_t)b.ptr[i] == INTEX8_DIGIT_MAX_VALUE && _shift_l1(c, i, c_size0) == 0); --i) {
			}
			if (i >= a_size)
				return false;
		}

		uni_t u = { 0 };
		++i;
		for (; i >= 0;)
		{
			if (debt == 0) {
				for (--i; i >= 0 && ((uint64_t)_shift_l2(a, i, a_size0) + b.ptr[i] == _shift_l1(c, i, c_size0)); --i) {
				}
				if (i < 0)
					return false;
				if ((uint64_t)_shift_l2(a, i, a_size0) + b.ptr[i] + 1 < _shift_l1(c, i, c_size0))
					return false;
				else if ((uint64_t)_shift_l2(a, i, a_size0) + b.ptr[i] > _shift_l1(c, i, c_size0))
					return true;
				// ASSERT ((uint64_t)a.ptr[i] + b.ptr[i] + 1 == c.ptr[i])
			}
			do
			{
				for (--i; i >= 0 && ((uint64_t)_shift_l2(a, i, a_size0) + b.ptr[i] == INTEX8_DIGIT_MAX_VALUE && _shift_l1(c, i, c_size0) == 0); --i) {
				}
				if (i < 0)
					return false;
				u.a = (uint64_t)_shift_l2(a, i, a_size0) + b.ptr[i];
				if (u.a <= INTEX8_DIGIT_MAX_VALUE || (u.b[1] == 1 && u.b[0] + 1 < _shift_l1(c, i, c_size0)))
					return false;
				else if (u.b[1] == 1 && u.b[0] > _shift_l1(c, i, c_size0))
					return true;
				// ASSERT (u.b[1] == 1 && (u.b[0] == c.ptr[i] || u.b[0] + 1 == c.ptr[i]))
			} while (u.b[0] + 1 == _shift_l1(c, i, c_size0));
			debt = 0;
		}
	}
	else {	// ASSERT (a_size >= b_size)
		int debt = 0;
		cntx_t i;
		if (c_size == a_size + 1) {
			if (_shift_l1(c, a_size, c_size0) > 1)
				return false;
			debt = 1;
			i = a_size - 1;
		}
		else { // ASSERT (c_size == a_size)
			for (i = a_size - 1; i >= b_size && (_shift_l2(a, i, a_size0) == _shift_l1(c, i, c_size0)); --i) {
			}
			if (i >= b_size && (uint64_t)_shift_l2(a, i, a_size0) + 1 < _shift_l1(c, i, c_size0))
				return false;
			if (i >= b_size && (uint64_t)_shift_l2(a, i, a_size0) > _shift_l1(c, i, c_size0))
				return true;
			// ASSERT (i == b_size - 1) || (i >= b_size && (uint64_t)_shift_l1(a, i, a_size) + 1 == c.ptr[i])
			if (i >= b_size && _shift_l2(a, i, a_size0) + 1 == _shift_l1(c, i, c_size0)) {
				debt = 1;
				--i;
			}
		}
		if (debt == 1) {
			for (; i >= b_size && ((uint64_t)_shift_l2(a, i, a_size0) == INTEX8_DIGIT_MAX_VALUE && _shift_l1(c, i, c_size0) == 0); --i) {
			}
			if (i >= b_size)
				return false;
		}

		uni_t u = { 0 };
		++i;
		for (; i >= 0;)
		{
			if (debt == 0) {
				for (--i; i >= 0 && ((uint64_t)b.ptr[i] + _shift_l2(a, i, a_size0) == _shift_l1(c, i, c_size0)); --i) {
				}
				if (i < 0)
					return false;
				if ((uint64_t)b.ptr[i] + _shift_l2(a, i, a_size0) + 1 < _shift_l1(c, i, c_size0))
					return false;
				else if ((uint64_t)b.ptr[i] + _shift_l2(a, i, a_size0) > _shift_l1(c, i, c_size0))
					return true;
				// ASSERT ((uint64_t)b.ptr[i] + _shift_l1(a, i, a_size) + 1 == c.ptr[i])
			}
			do
			{
				for (--i; i >= 0 && ((uint64_t)b.ptr[i] + _shift_l2(a, i, a_size0) == INTEX8_DIGIT_MAX_VALUE && _shift_l1(c, i, c_size0) == 0); --i) {
				}
				if (i < 0)
					return false;
				u.a = (uint64_t)b.ptr[i] + _shift_l2(a, i, a_size0);
				if (u.a <= INTEX8_DIGIT_MAX_VALUE || (u.b[1] == 1 && u.b[0] + 1 < _shift_l1(c, i, c_size0)))
					return false;
				else if (u.b[1] == 1 && u.b[0] > _shift_l1(c, i, c_size0))
					return true;
				// ASSERT (u.b[1] == 1 && (u.b[0] == c.ptr[i] || u.b[0] + 1 == c.ptr[i]))
			} while (u.b[0] + 1 == _shift_l1(c, i, c_size0));
			debt = 0;
		}
	}
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_subtract(intx_t x, intx_t y, dig_t* dest)
{
	if (y.size == 0) {
		return i8_copy(x, dest);
	}
	else if (x.size == 0) {
		y.size = -y.size;
		return i8_copy(y, dest);
	}

	cntx_t sgn_x = _sgn(x.size);
	cntx_t sgn_y = _sgn(y.size);

	if (sgn_x * sgn_y == -1) {
		cntx_t x_size = _abs(x.size);
		cntx_t y_size = _abs(y.size);
		cntx_t sign = _sgn(x.size);
		if (x_size > y_size) {
			_swap(intx_t, x, y);
			x_size = _abs(x.size);
			y_size = _abs(y.size);
		}

		intx_t z = { dest, 0 };
		uni_t u = { 0 };

		size_t s = 0;
		for (; s < x_size; ++s) {
			u.a += (uint64_t)(*x.ptr++) + (*y.ptr++);
			*dest++ = u.b[0];
			u.a = u.b[1];
		}
		for (; s < y_size; ++s) {
			u.a += *y.ptr++;
			*dest++ = u.b[0];
			u.a = u.b[1];
		}
		if (u.a != 0) {
			*dest++ = u.b[0];
			++s;
		}
		z.size = s * sign;
		_remove_leading_trivial_digits(&z);
		return z;
	}
	else {
		cntx_t x_size = _abs(x.size);
		cntx_t y_size = _abs(y.size);

		_CMP(x.ptr, x_size, y.ptr, y_size)
		cntx_t sign = _sgn(x.size);
		if (x_cmp_y < 0) {
			_swap(intx_t, x, y);
			x_size = _abs(x.size);
			y_size = _abs(y.size);
			sign = -sign;
		}

		intx_t z = { dest, 0 };

		uint64_t h = 0;
		cntx_t s = 0;
		for (; s < y_size; ++s) {
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
			for (; s < x_size && *x.ptr++ == 0; ++s) {
				*dest++ = INTEX8_DIGIT_MAX_VALUE;
			}
			*dest++ = x.ptr[-1] - 1;
			++s;
		}
		for (; s < x_size; ++s) {
			*dest++ = *x.ptr++;
		}

		z.size = sign * s;
		_remove_leading_trivial_digits(&z);
		return z;
	}
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_subtract_positive(intx_t x, intx_t y, dig_t* dest)
{	// x >= 0 and y >= 0

	cntx_t x_size = _abs(x.size);
	cntx_t y_size = _abs(y.size);

	_CMP(x.ptr, x_size, y.ptr, y_size)
	if (x_cmp_y >= 0) {	// x >= y
		intx_t z = { dest, 0 };
		uint64_t h = 0;
		cntx_t s = 0;
		for (; s < y_size; ++s) {
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
			for (; s < x_size && *x.ptr++ == 0; ++s) {
				*dest++ = INTEX8_DIGIT_MAX_VALUE;
			}
			*dest++ = x.ptr[-1] - 1;
			++s;
		}
		for (; s < x_size; ++s) {
			*dest++ = *x.ptr++;
		}
		z.size = s;
		_remove_leading_trivial_digits(&z);
		return z;
	}
	else {
		intx_t z = { dest, 0 };
		uint64_t h = 0;
		cntx_t s = 0;
		for (; s < x_size; ++s) {
			if (*y.ptr >= h + *x.ptr) {
				*dest++ = *y.ptr++ - (h + *x.ptr++);
				h = 0;
			}
			else {
				*dest++ = (INTEX8_DIGIT_MAX_VALUE + 1) + *y.ptr++ - (h + *x.ptr++);
				h = 1;
			}
		}
		if (h == 1) {
			for (; s < y_size && *y.ptr++ == 0; ++s) {
				*dest++ = INTEX8_DIGIT_MAX_VALUE;
			}
			*dest++ = y.ptr[-1] - 1;
			++s;
		}
		for (; s < y_size; ++s) {
			*dest++ = *y.ptr++;
		}
		z.size = -s;
		_remove_leading_trivial_digits(&z);
		return z;
	}
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_adivide_by_2(intx_t a)
{	// a is a multiple of 2
	if (a.size == 0)
		return a;

	dig_t* ptr = a.ptr;
	dig_t* pend = a.ptr + _abs(a.size) - 1;
	for (; ptr < pend;) {
		*ptr = (*ptr >> 1) | (ptr[1] << (INTEX8_DIGIT_BIT_WIDTH - 1));
		++ptr;
	}
	*ptr >>= 1;

	_remove_leading_trivial_digits(&a);
	return a;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_adivide_by_3(intx_t a)
{	// a is a multiple of 3
	if (a.size == 0)
		return a;

	const dig_t D = 3;

	uni_t x = { 0 }, y = { 0 };

	intx_t out = { a.ptr, a.size };

	x.b[1] = 0;

	a.size = _abs(a.size);
	cntx_t i = a.size;
	dig_t* ptr = a.ptr + i;
	for (; i > 0;) {
		x.b[0] = ptr[-1];
		--i;
		y.a = x.a / D;
		*(--ptr) = y.b[0];
		--a.size;
		x.a -= y.a * D;
		x.b[1] = x.b[0];
	}
	_remove_leading_trivial_digits(&out);
	return out;
}

/* Internal function. Do NOT call this directly! */
static intx_t _a_pluseq_2xb(intx_t a, intx_t b)
{	// _sgn(a) == _sgn(b)
	if (b.size == 0)
		return a;

	intx_t out = { a.ptr, 0 };

	int a_size = _abs(a.size), b_size = _abs(b.size);
	if (a_size > b_size) {
		uni_t u = { 0 };
		cntx_t i = 0;
		for (; i < b_size; ++i) {
			u.a += (uint64_t)(*a.ptr) + 2 * (uint64_t)(*b.ptr++);
			*a.ptr++ = u.b[0];
			u.a = u.b[1];
		}
		for (; i < a_size; ++i) {
			u.a += (uint64_t)(*a.ptr);
			*a.ptr++ = u.b[0];
			u.a = u.b[1];
		}
		if (u.a != 0) {
			*a.ptr++ = u.b[0];
			++i;
		}
		out.size = _sgn(a.size) * i;
	}
	else {
		uni_t u = { 0 };
		cntx_t i = 0;
		for (; i < a_size; ++i) {
			u.a += (uint64_t)(*a.ptr) + 2 * (uint64_t)(*b.ptr++);
			*a.ptr++ = u.b[0];
			u.a = u.b[1];
		}
		for (; i < b_size; ++i) {
			u.a += 2 * (uint64_t)(*b.ptr++);
			*a.ptr++ = u.b[0];
			u.a = u.b[1];
		}
		if (u.a != 0) {
			*a.ptr++ = u.b[0];
			++i;
		}
		out.size = _sgn(a.size) * i;
	}
	_remove_leading_trivial_digits(&out);
	return out;
}

/* Internal function. Do NOT call this directly! */
static intx_t _2xb_minus_a(intx_t a, intx_t b)
{	// 2*|b| >= |a|
	intx_t out = { a.ptr, 0 };

	int a_size = _abs(a.size), b_size = _abs(b.size);
	if (a_size > b_size) {
		uni_t u = { 0 };
		uint64_t h = 0;
		cntx_t i = 0;
		for (; i < b_size; ++i) {
			u.a += 2 * (uint64_t)*b.ptr++;
			if (u.b[0] >= *a.ptr + h) {
				*a.ptr = u.b[0] - (*a.ptr + h);
				++a.ptr;
				h = 0;
			}
			else {
				*a.ptr = (INTEX8_DIGIT_MAX_VALUE + 1) + u.b[0] - (*a.ptr + h);
				++a.ptr;
				h = 1;
			}
			u.a = u.b[1];
		}
		out.size = i * _sgn(b.size);
		_remove_leading_trivial_digits(&out);
		return out;
	}
	else {	// ASSERT (a_size <= b_size)
		uni_t u = { 0 };
		uint64_t h = 0;
		cntx_t i = 0;
		for (; i < a_size; ++i) {
			u.a += 2 * (uint64_t)*b.ptr++;
			if (u.b[0] >= *a.ptr + h) {
				*a.ptr = u.b[0] - (*a.ptr + h);
				++a.ptr;
				h = 0;
			}
			else {
				*a.ptr = (INTEX8_DIGIT_MAX_VALUE + 1) + u.b[0] - (*a.ptr + h);
				++a.ptr;
				h = 1;
			}
			u.a = u.b[1];
		}
		for (; i < b_size; ++i) {
			u.a += 2 * (uint64_t)*b.ptr++;
			if (u.b[0] >= h) {
				*a.ptr++ = u.b[0] - h;
				h = 0;
			}
			else {
				*a.ptr++ = (INTEX8_DIGIT_MAX_VALUE + 1) + u.b[0] - h;
				h = 1;
			}
			u.a = u.b[1];
		}
		if (u.a > h) {
			*a.ptr++ = u.b[0] - h;
			++i;
		}
		out.size = i * _sgn(b.size);
		_remove_leading_trivial_digits(&out);
		return out;
	}
}

/* Internal function. Do NOT call this directly! */
static intx_t _a_minus_2xb(intx_t a, intx_t b)
{	// returns a - 2*b
	// |a| >= |2*b|
	intx_t out = { a.ptr, 0 };

	int a_size = _abs(a.size), b_size = _abs(b.size);
	
	// ASSERT (a_size >= b_size)

	uni_t u = { 0 }, v = { 0 };
	uint64_t h = 0;
	int i = 0;
	for (; i < b_size; ++i) {
		u.a += *a.ptr;
		v.a += (2 * (uint64_t)*b.ptr++);
		if (u.b[0] >= v.b[0] + h) {
			*a.ptr++ = u.b[0] - (v.b[0] + h);
			h = 0;
		}
		else {
			*a.ptr++ = (INTEX8_DIGIT_MAX_VALUE + 1) + u.b[0] - (v.b[0] + h);
			h = 1;
		}
		u.a = u.b[1];
		v.a = v.b[1];
	}
	for (; i < a_size; ++i) {
		u.a += *a.ptr;
		if (u.b[0] >= v.b[0] + h) {
			*a.ptr++ = u.b[0] - (v.b[0] + h);
			h = 0;
		}
		else {
			*a.ptr++ = (INTEX8_DIGIT_MAX_VALUE + 1) + u.b[0] - (v.b[0] + h);
			h = 1;
		}
		u.a = u.b[1];
		v.a = v.b[1];
	}
	if (u.a) {
		*a.ptr++ = u.b[0];
		++i;
	}
	out.size = i * _sgn(a.size);
	_remove_leading_trivial_digits(&out);
	return out;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_add_twice_to(intx_t a, intx_t b)
{	// It seems a + 2*b >= 0
	// a += 2 * b
	if (_sgn(a.size) == _sgn(b.size)) {
		return _a_pluseq_2xb(a, b);
	}
	else if (_lt_2x(a, b)) {
		// |a| < 2*|b|, so evaluate: a = 2*b - a
		a.size = _abs((int)a.size) * _sgn(b.size);
		return _2xb_minus_a(a, b);
	}
	else {
		// |a| >= 2*|b|, so evaluate: a = a - 2*b
		return _a_minus_2xb(a, b);
	}
}

#define _shift_r1(a, i)		(dig_t)((i) == a.size - 1 ? (a.ptr[i] >> 1) : (a.ptr[i] >> 1) | (a.ptr[(i) + 1] << (INTEX8_DIGIT_BIT_WIDTH - 1)))

#define _INTEX8_TOOM3_LOOP(size, exp1, exp2, add_um1, exp3, exp4, exp_cmp1, exp_cmp2, exp_cmp3, exp_cmp4) \
for (; i < size; ++i) {	\
	m0_plus_m2.a += exp1;	\
	m0_plus_m1_plus_m2.a += exp2;	\
	*ptr1++ = m0_plus_m1_plus_m2.b[0];				\
	m0_plus_m1_plus_m2.a = m0_plus_m1_plus_m2.b[1];	\
	um1.a += add_um1;		\
	if (exp_cmp1 >= exp_cmp2) {			\
		*ptr_1++ = exp_cmp1 - (exp_cmp2);	\
		h1 = 0;				\
	}						\
	else {					\
		*ptr_1++ = (INTEX8_DIGIT_MAX_VALUE + 1) + exp_cmp1 - (exp_cmp2);	\
		h1 = 1;	\
	}	\
	m0_plus_m2.a = m0_plus_m2.b[1];	\
	um1.a = um1.b[1];	\
	\
	m0_plus_4m2.a += exp3;	\
	_2m1.a += exp4;	\
	if (exp_cmp3 >= exp_cmp4) {	\
		*ptr_2++ = exp_cmp3 - (exp_cmp4);	\
		h2 = 0;	\
	}	\
	else {	\
		*ptr_2++ = (INTEX8_DIGIT_MAX_VALUE + 1) + exp_cmp3 - (exp_cmp4);	\
		h2 = 1;	\
	}	\
	m0_plus_4m2.a = m0_plus_4m2.b[1];	\
	_2m1.a = _2m1.b[1];	\
}

/* Internal function. Do NOT call this directly! */
// p_1 >= 0 and p_2 >= 0
static intx_t _toom3_branch0_012(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;

	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, *m2.ptr, *m2.ptr, 0,
			(uint64_t)*m2.ptr++ * 4, 0, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (m0_plus_4m2.b[0] != 0 && m0_plus_4m2.b[0] > _2m1.b[0] + h2) {
		*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = (ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch0_021(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;

	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, 0, (uint64_t)*m1.ptr, *m1.ptr, 0,
		(uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	
	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (m0_plus_4m2.b[0] != 0 && m0_plus_4m2.b[0] > _2m1.b[0] + h2) {
		*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = (ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch0_102(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr, 0,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, 0, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr, 0,
		(uint64_t)*m2.ptr++ * 4, 0, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (m0_plus_4m2.b[0] != 0 && m0_plus_4m2.b[0] > _2m1.b[0] + h2) {
		*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = (ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch0_120(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr, 0,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, 0, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr, 0,
		(uint64_t)*m0.ptr++, 0, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	
	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (m0_plus_4m2.b[0] != 0 && m0_plus_4m2.b[0] > _2m1.b[0] + h2) {
		*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = (ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch0_201(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, 0, (uint64_t)*m1.ptr, *m1.ptr,
		0, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (m0_plus_4m2.b[0] != 0 && m0_plus_4m2.b[0] > _2m1.b[0] + h2) {
		*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = (ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch0_210(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr, 0,
		(uint64_t)*m0.ptr++, 0, m0_plus_m2.b[0], um1.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (m0_plus_4m2.b[0] != 0 && m0_plus_4m2.b[0] > _2m1.b[0] + h2) {
		*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = (ptr_2 - p_2->ptr);
	return p1;
}

// p_1 >= 0 and p_2 < 0
static intx_t _toom3_branch1_012(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr, 0,
		(uint64_t)*m2.ptr++ * 4, 0, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (_2m1.b[0] != 0 && _2m1.b[0] > m0_plus_4m2.b[0] + h2) {
		*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = -(ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch1_021(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, 0, (uint64_t)*m1.ptr, *m1.ptr,
		0, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (_2m1.b[0] != 0 && _2m1.b[0] > m0_plus_4m2.b[0] + h2) {
		*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = -(ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch1_120(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr, 0,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, 0, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr, 0,
		(uint64_t)*m0.ptr++, 0, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (_2m1.b[0] != 0 && _2m1.b[0] > m0_plus_4m2.b[0] + h2) {
		*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = -(ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch1_201(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, 0, (uint64_t)*m1.ptr, *m1.ptr,
		0, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (_2m1.b[0] != 0 && _2m1.b[0] > m0_plus_4m2.b[0] + h2) {
		*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = -(ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch1_210(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++, (uint64_t)*m1.ptr++ * 2, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr, 0,
		(uint64_t)*m0.ptr++, 0, m0_plus_m2.b[0], um1.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (m0_plus_m2.b[0] != 0 && m0_plus_m2.b[0] > um1.b[0] + h1) {
		*ptr_1++ = m0_plus_m2.b[0] - (um1.b[0] + h1);
	}
	if (_2m1.b[0] != 0 && _2m1.b[0] > m0_plus_4m2.b[0] + h2) {
		*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = (ptr_1 - p_1->ptr);
	p_2->size = -(ptr_2 - p_2->ptr);
	return p1;
}

// p_1 < 0 and p_2 >= 0
static intx_t _toom3_branch2_012(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;

	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr, 0,
		(uint64_t)*m2.ptr++ * 4, 0, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (um1.b[0] != 0 && um1.b[0] > m0_plus_m2.b[0] + h1) {
		*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
	}
	if (m0_plus_4m2.b[0] != 0 && m0_plus_4m2.b[0] > _2m1.b[0] + h2) {
		*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = -(ptr_1 - p_1->ptr);
	p_2->size = (ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch2_021(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;

	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, 0, (uint64_t)*m1.ptr, *m1.ptr,
		0, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (um1.b[0] != 0 && um1.b[0] > m0_plus_m2.b[0] + h1) {
		*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
	}
	if (m0_plus_4m2.b[0] != 0 && m0_plus_4m2.b[0] > _2m1.b[0] + h2) {
		*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = -(ptr_1 - p_1->ptr);
	p_2->size = (ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch2_120(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	//BRANCH2_1(size1)
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)

	/*
	for (; i < size1; ++i) {
		m0_plus_m2.a += (uint64_t)*m0.ptr + *m2.ptr;
		m0_plus_m1_plus_m2.a += (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr;
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
		m0_plus_m1_plus_m2.a = m0_plus_m1_plus_m2.b[1];

		um1.a += *m1.ptr;
		if (um1.b[0] >= m0_plus_m2.b[0] + h1) {
			*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 0;
		}
		else {
			*ptr_1++ = (INTEX8_DIGIT_MAX_VALUE + 1) + um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 1;
		}
		m0_plus_m2.a = m0_plus_m2.b[1];
		um1.a = um1.b[1];

		m0_plus_4m2.a += (uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4;
		_2m1.a += (uint64_t)*m1.ptr++ * 2;
		if (m0_plus_4m2.b[0] >= _2m1.b[0] + h2) {
			*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 0;
		}
		else {
			*ptr_2++ = (INTEX8_DIGIT_MAX_VALUE + 1) + m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 1;
		}
		m0_plus_4m2.a = m0_plus_4m2.b[1];
		_2m1.a = _2m1.b[1];
	}
	*/

	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr, 0,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, 0, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)

	/*
	for (; i < size2; ++i) {
		m0_plus_m2.a += (uint64_t)*m0.ptr + *m2.ptr;
		m0_plus_m1_plus_m2.a += (uint64_t)*m0.ptr + *m2.ptr;
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
		m0_plus_m1_plus_m2.a = m0_plus_m1_plus_m2.b[1];

		if (um1.b[0] >= m0_plus_m2.b[0] + h1) {
			*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 0;
		}
		else {
			*ptr_1++ = (INTEX8_DIGIT_MAX_VALUE + 1) + um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 1;
		}
		m0_plus_m2.a = m0_plus_m2.b[1];
		um1.a = um1.b[1];

		m0_plus_4m2.a += (uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4;
		if (m0_plus_4m2.b[0] >= _2m1.b[0] + h2) {
			*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 0;
		}
		else {
			*ptr_2++ = (INTEX8_DIGIT_MAX_VALUE + 1) + m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 1;
		}
		m0_plus_4m2.a = m0_plus_4m2.b[1];
		_2m1.a = _2m1.b[1];
	}
	*/

	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr, 0,
		(uint64_t)*m0.ptr++, 0, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)

	/*
	for (; i < size0; ++i) {
		m0_plus_m2.a += *m0.ptr;
		m0_plus_m1_plus_m2.a += (uint64_t)*m0.ptr;
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
		m0_plus_m1_plus_m2.a = m0_plus_m1_plus_m2.b[1];

		if (um1.b[0] >= m0_plus_m2.b[0] + h1) {
			*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 0;
		}
		else {
			*ptr_1++ = (INTEX8_DIGIT_MAX_VALUE + 1) + um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 1;
		}
		m0_plus_m2.a = m0_plus_m2.b[1];
		um1.a = um1.b[1];

		m0_plus_4m2.a += *m0.ptr++;
		if (m0_plus_4m2.b[0] >= _2m1.b[0] + h2) {
			*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 0;
		}
		else {
			*ptr_2++ = (INTEX8_DIGIT_MAX_VALUE + 1) + m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 1;
		}
		m0_plus_4m2.a = m0_plus_4m2.b[1];
		_2m1.a = _2m1.b[1];
	}
	*/
	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (um1.b[0] != 0 && um1.b[0] > m0_plus_m2.b[0] + h1) {
		*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
	}
	if (m0_plus_4m2.b[0] != 0 && m0_plus_4m2.b[0] > _2m1.b[0] + h2) {
		*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = -(ptr_1 - p_1->ptr);
	p_2->size = (ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch2_201(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	//BRANCH2_1(size2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)

	/*
	for (; i < size2; ++i) {
		m0_plus_m2.a += (uint64_t)*m0.ptr + *m2.ptr;
		m0_plus_m1_plus_m2.a += (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr;
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
		m0_plus_m1_plus_m2.a = m0_plus_m1_plus_m2.b[1];

		um1.a += *m1.ptr;
		if (um1.b[0] >= m0_plus_m2.b[0] + h1) {
			*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 0;
		}
		else {
			*ptr_1++ = (INTEX8_DIGIT_MAX_VALUE + 1) + um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 1;
		}
		m0_plus_m2.a = m0_plus_m2.b[1];
		um1.a = um1.b[1];

		m0_plus_4m2.a += (uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4;
		_2m1.a += (uint64_t)*m1.ptr++ * 2;
		if (m0_plus_4m2.b[0] >= _2m1.b[0] + h2) {
			*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 0;
		}
		else {
			*ptr_2++ = (INTEX8_DIGIT_MAX_VALUE + 1) + m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 1;
		}
		m0_plus_4m2.a = m0_plus_4m2.b[1];
		_2m1.a = _2m1.b[1];
	}
	*/

	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)

	/*
	for (; i < size0; ++i) {
		m0_plus_m2.a += *m0.ptr;
		m0_plus_m1_plus_m2.a += (uint64_t)*m0.ptr + *m1.ptr;
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
		m0_plus_m1_plus_m2.a = m0_plus_m1_plus_m2.b[1];

		um1.a += *m1.ptr;
		if (um1.b[0] >= m0_plus_m2.b[0] + h1) {
			*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 0;
		}
		else {
			*ptr_1++ = (INTEX8_DIGIT_MAX_VALUE + 1) + um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 1;
		}
		m0_plus_m2.a = m0_plus_m2.b[1];
		um1.a = um1.b[1];

		m0_plus_4m2.a += *m0.ptr++;
		_2m1.a += (uint64_t)*m1.ptr++ * 2;
		if (m0_plus_4m2.b[0] >= _2m1.b[0] + h2) {
			*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 0;
		}
		else {
			*ptr_2++ = (INTEX8_DIGIT_MAX_VALUE + 1) + m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 1;
		}
		m0_plus_4m2.a = m0_plus_4m2.b[1];
		_2m1.a = _2m1.b[1];
	}
	*/

	_INTEX8_TOOM3_LOOP(size1, 0, (uint64_t)*m1.ptr, *m1.ptr,
		0, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, m0_plus_4m2.b[0], _2m1.b[0] + h2)
	/*
	for (; i < size1; ++i) {
		m0_plus_m1_plus_m2.a += *m1.ptr;
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
		m0_plus_m1_plus_m2.a = m0_plus_m1_plus_m2.b[1];

		um1.a += *m1.ptr;
		if (um1.b[0] >= m0_plus_m2.b[0] + h1) {
			*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 0;
		}
		else {
			*ptr_1++ = (INTEX8_DIGIT_MAX_VALUE + 1) + um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 1;
		}
		m0_plus_m2.a = m0_plus_m2.b[1];
		um1.a = um1.b[1];

		_2m1.a += (uint64_t)*m1.ptr++ * 2;
		if (m0_plus_4m2.b[0] >= _2m1.b[0] + h2) {
			*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 0;
		}
		else {
			*ptr_2++ = (INTEX8_DIGIT_MAX_VALUE + 1) + m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
			h2 = 1;
		}
		m0_plus_4m2.a = m0_plus_4m2.b[1];
		_2m1.a = _2m1.b[1];
	}
	*/
	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (um1.b[0] != 0 && um1.b[0] > m0_plus_m2.b[0] + h1) {
		*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
	}
	if (m0_plus_4m2.b[0] != 0 && m0_plus_4m2.b[0] > _2m1.b[0] + h2) {
		*ptr_2++ = m0_plus_4m2.b[0] - (_2m1.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = -(ptr_1 - p_1->ptr);
	p_2->size = (ptr_2 - p_2->ptr);
	return p1;
}

// p_1 < 0 and p_2 < 0
static intx_t _toom3_branch3_012(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;

	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	//BRANCH3_1(size0)
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)

	/*
	for (; i < size0; ++i) {
		m0_plus_m2.a += (uint64_t)*m0.ptr + *m2.ptr;
		m0_plus_m1_plus_m2.a += (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr;
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
		m0_plus_m1_plus_m2.a = m0_plus_m1_plus_m2.b[1];

		um1.a += *m1.ptr;
		if (um1.b[0] >= m0_plus_m2.b[0] + h1) {
			*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 0;
		}
		else {
			*ptr_1++ = (INTEX8_DIGIT_MAX_VALUE + 1) + um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 1;
		}
		m0_plus_m2.a = m0_plus_m2.b[1];
		um1.a = um1.b[1];

		m0_plus_4m2.a += (uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4;
		_2m1.a += (uint64_t)*m1.ptr++ * 2;
		if (_2m1.b[0] >= m0_plus_4m2.b[0] + h2) {
			*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
			h2 = 0;
		}
		else {
			*ptr_2++ = (INTEX8_DIGIT_MAX_VALUE + 1) + _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
			h2 = 1;
		}
		m0_plus_4m2.a = m0_plus_4m2.b[1];
		_2m1.a = _2m1.b[1];
	}
	*/
	
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	/*
	for (; i < size1; ++i) {
		m0_plus_m2.a += (uint64_t)*m2.ptr;
		m0_plus_m1_plus_m2.a += (uint64_t)*m2.ptr + *m1.ptr;
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
		m0_plus_m1_plus_m2.a = m0_plus_m1_plus_m2.b[1];

		um1.a += *m1.ptr;
		if (um1.b[0] >= m0_plus_m2.b[0] + h1) {
			*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 0;
		}
		else {
			*ptr_1++ = (INTEX8_DIGIT_MAX_VALUE + 1) + um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 1;
		}
		m0_plus_m2.a = m0_plus_m2.b[1];
		um1.a = um1.b[1];

		m0_plus_4m2.a += (uint64_t)*m2.ptr++ * 4;
		_2m1.a += (uint64_t)*m1.ptr++ * 2;
		if (_2m1.b[0] >= m0_plus_4m2.b[0] + h2) {
			*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
			h2 = 0;
		}
		else {
			*ptr_2++ = (INTEX8_DIGIT_MAX_VALUE + 1) + _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
			h2 = 1;
		}
		m0_plus_4m2.a = m0_plus_4m2.b[1];
		_2m1.a = _2m1.b[1];
	}
	*/

	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr, 0,
		(uint64_t)*m2.ptr++ * 4, 0, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	/*
	for (; i < size2; ++i) {
		m0_plus_m2.a += *m2.ptr;
		m0_plus_m1_plus_m2.a += *m2.ptr;
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
		m0_plus_m1_plus_m2.a = m0_plus_m1_plus_m2.b[1];

		if (um1.b[0] >= m0_plus_m2.b[0] + h1) {
			*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 0;
		}
		else {
			*ptr_1++ = (INTEX8_DIGIT_MAX_VALUE + 1) + um1.b[0] - (m0_plus_m2.b[0] + h1);
			h1 = 1;
		}
		m0_plus_m2.a = m0_plus_m2.b[1];
		um1.a = um1.b[1];

		m0_plus_4m2.a += (uint64_t)*m2.ptr++ * 4;
		if (_2m1.b[0] >= m0_plus_4m2.b[0] + h2) {
			*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
			h2 = 0;
		}
		else {
			*ptr_2++ = (INTEX8_DIGIT_MAX_VALUE + 1) + _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
			h2 = 1;
		}
		m0_plus_4m2.a = m0_plus_4m2.b[1];
		_2m1.a = _2m1.b[1];
	}
	*/
	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (um1.b[0] != 0 && um1.b[0] > m0_plus_m2.b[0] + h1) {
		*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
	}
	if (_2m1.b[0] != 0 && _2m1.b[0] > m0_plus_4m2.b[0] + h2) {
		*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = -(ptr_1 - p_1->ptr);
	p_2->size = -(ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch3_021(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;

	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m2.ptr, (uint64_t)*m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, 0, (uint64_t)*m1.ptr, *m1.ptr,
		0, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (um1.b[0] != 0 && um1.b[0] > m0_plus_m2.b[0] + h1) {
		*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
	}
	if (_2m1.b[0] != 0 && _2m1.b[0] > m0_plus_4m2.b[0] + h2) {
		*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = -(ptr_1 - p_1->ptr);
	p_2->size = -(ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch3_120(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size1, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr, 0,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, 0, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr, 0,
		(uint64_t)*m0.ptr++, 0, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (um1.b[0] != 0 && um1.b[0] > m0_plus_m2.b[0] + h1) {
		*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
	}
	if (_2m1.b[0] != 0 && _2m1.b[0] > m0_plus_4m2.b[0] + h2) {
		*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = -(ptr_1 - p_1->ptr);
	p_2->size = -(ptr_2 - p_2->ptr);
	return p1;
}

static intx_t _toom3_branch3_201(intx_t m0, intx_t m1, intx_t m2, dig_t* ptr1, intx_t* p_1, dig_t* ptr_1, intx_t* p_2, dig_t* ptr_2)
{
	intx_t p1 = { ptr1, 0 };
	p_1->ptr = ptr_1;
	p_2->ptr = ptr_2;
	cntx_t size0 = _abs(m0.size), size1 = _abs(m1.size), size2 = _abs(m2.size);

	uni_t m0_plus_m2 = { 0 }, m0_plus_m1_plus_m2 = { 0 }, um1 = { 0 }, _2m1 = { 0 }, m0_plus_4m2 = { 0 };
	uint64_t h1 = 0, h2 = 0;
	cntx_t i = 0;
	_INTEX8_TOOM3_LOOP(size2, (uint64_t)*m0.ptr + *m2.ptr, (uint64_t)*m0.ptr + *m2.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++ + (uint64_t)*m2.ptr++ * 4, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size0, (uint64_t)*m0.ptr, (uint64_t)*m0.ptr + *m1.ptr, *m1.ptr,
		(uint64_t)*m0.ptr++, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)
	_INTEX8_TOOM3_LOOP(size1, 0, (uint64_t)*m1.ptr, *m1.ptr,
		0, (uint64_t)*m1.ptr++ * 2, um1.b[0], m0_plus_m2.b[0] + h1, _2m1.b[0], m0_plus_4m2.b[0] + h2)

	if (m0_plus_m1_plus_m2.b[0] != 0) {
		*ptr1++ = m0_plus_m1_plus_m2.b[0];
	}
	if (um1.b[0] != 0 && um1.b[0] > m0_plus_m2.b[0] + h1) {
		*ptr_1++ = um1.b[0] - (m0_plus_m2.b[0] + h1);
	}
	if (_2m1.b[0] != 0 && _2m1.b[0] > m0_plus_4m2.b[0] + h2) {
		*ptr_2++ = _2m1.b[0] - (m0_plus_4m2.b[0] + h2);
	}
	p1.size = (ptr1 - p1.ptr);
	p_1->size = -(ptr_1 - p_1->ptr);
	p_2->size = -(ptr_2 - p_2->ptr);
	return p1;
}

/* Internal function. Do NOT call this directly! */
static intx_t _add_3(intx_t a, intx_t b, intx_t c, dig_t* dest, cntx_t sign)
{
	dig_t* p[3] = { a.ptr, b.ptr, c.ptr };

	int n[3] = { _abs(a.size), _abs(b.size) , _abs(c.size) };
	if (n[0] > n[1]) {
		_swap(int, n[0], n[1]);
		_swap(dig_t*, p[0], p[1]);
	}
	if (n[1] > n[2]) {
		_swap(int, n[1], n[2]);
		_swap(dig_t*, p[1], p[2]);
	}
	if (n[0] > n[1]) {
		_swap(int, n[0], n[1]);
		_swap(dig_t*, p[0], p[1]);
	}

	uni_t u = { 0 };
	int i = 0;
	for (; i < n[0]; ++i) {
		u.a += (uint64_t)(*p[0]++) + (*p[1]++) + (*p[2]++);
		*dest++ = u.b[0];
		u.a = u.b[1];
	}
	for (; i < n[1]; ++i) {
		u.a += (uint64_t)(*p[1]++) + (*p[2]++);
		*dest++ = u.b[0];
		u.a = u.b[1];
	}
	for (; i < n[2]; ++i) {
		u.a += (*p[2]++);
		*dest++ = u.b[0];
		u.a = u.b[1];
	}
	if (u.a != 0) {
		*dest++ = u.b[0];
		++i;
	}
	a.size = sign * i;
	_remove_leading_trivial_digits(&a);
	return a;
}

/* Internal function. Do NOT call this directly! */
static intx_t _add_2_sub(intx_t a, intx_t b, intx_t c, dig_t* dest, cntx_t sign)
{	// returns a + b - c
	// |a| + |b| >= |c|

	cntx_t a_size = _abs(a.size), b_size = _abs(b.size);
	if (a_size > b_size) {
		_swap(intx_t, a, b);
		a_size = _abs(a.size);
		b_size = _abs(b.size);
	}
	cntx_t c_size = _abs(c.size);

	uni_t u = { 0 };

	intx_t z = { dest, 0 };
	if (c_size < a_size) {
		int64_t h = 0;
		cntx_t i = 0;
		for (; i < c_size; ++i) {
			u.a += (uint64_t)(*b.ptr++) + (*a.ptr++);
			if (u.a >= *c.ptr + h) {
				u.a -= (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 0;
			}
			else {
				u.a += (INTEX8_DIGIT_MAX_VALUE + 1) - (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 1;
			}
			u.a = u.b[1];
		}
		for (; i < a_size; ++i) {
			u.a += (uint64_t)(*b.ptr++) + (*a.ptr++);
			if (u.a >= h) {
				u.a -= h;
				*dest++ = u.b[0];
				h = 0;
			}
			else {
				u.a += (INTEX8_DIGIT_MAX_VALUE + 1) - h;
				*dest++ = u.b[0];
				h = 1;
			}
			u.a = u.b[1];
		}
		for (; i < b_size; ++i) {
			u.a += (*b.ptr++);
			*dest++ = u.b[0];
			u.a = u.b[1];
		}
		if (u.a > 0) {
			*dest++ = u.b[0];
			++i;
		}
		z.size = sign * i;
		_remove_leading_trivial_digits(&z);
		return z;
	}
	else if (c_size < b_size) {
		uint64_t h = 0;
		cntx_t i = 0;
		for (; i < a_size; ++i) {
			u.a += (uint64_t)(*b.ptr++) + (*a.ptr++);

			if (u.a >= *c.ptr + h) {
				u.a -= (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 0;
			}
			else {
				u.a += (INTEX8_DIGIT_MAX_VALUE + 1) - (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 1;
			}
			u.a = u.b[1];
		}
		for (; i < c_size; ++i) {
			u.a += (*b.ptr++);
			if (u.a >= *c.ptr + h) {
				u.a -= (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 0;
			}
			else {
				u.a += (INTEX8_DIGIT_MAX_VALUE + 1) - (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 1;
			}
			u.a = u.b[1];
		}
		for (; i < b_size; ++i) {
			u.a += (*b.ptr++);
			if (u.a >= h) {
				u.a -= h;
				*dest++ = u.b[0];
				h = 0;
			}
			else {
				u.a += (INTEX8_DIGIT_MAX_VALUE + 1) - h;
				*dest++ = u.b[0];
				h = 1;
			}
			u.a = u.b[1];
		}
		if (u.a != 0) {
			*dest++ = u.b[0];
			++i;
		}
		z.size = sign * i;
		_remove_leading_trivial_digits(&z);
		return z;
	}
	else {	// ASSERT (c_size >= b_size >= a_size)
		uint64_t h = 0;
		cntx_t i = 0;
		for (; i < a_size; ++i) {
			u.a += (uint64_t)(*b.ptr++) + (*a.ptr++);
			if (u.a >= *c.ptr + h) {
				u.a -= (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 0;
			}
			else {
				u.a += (INTEX8_DIGIT_MAX_VALUE + 1) - (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 1;
			}
			u.a = u.b[1];
		}
		for (; i < b_size; ++i) {
			u.a += (*b.ptr++);
			if (u.a >= *c.ptr + h) {
				u.a -= (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 0;
			}
			else {
				u.a += (INTEX8_DIGIT_MAX_VALUE + 1) - (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 1;
			}
			u.a = u.b[1];
		}
		for (; i < c_size; ++i) {
			if (u.a >= *c.ptr + h) {
				u.a -= (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 0;
			}
			else {
				u.a += (INTEX8_DIGIT_MAX_VALUE + 1) - (*c.ptr++ + h);
				*dest++ = u.b[0];
				h = 1;
			}
			u.a = u.b[1];
		}
		if (u.a != 0) {
			*dest++ = u.b[0];
			++i;
		}

		z.size = sign * i;
		_remove_leading_trivial_digits(&z);
		return z;
	}
}

/* Internal function. Do NOT call this directly! */
static intx_t _add_sub_2(intx_t a, intx_t b, intx_t c, dig_t* dest, cntx_t sign)
{	// returns a - b - c
	// |a| >= |b| + |c|

	cntx_t a_size = _abs(a.size);

	cntx_t b_size = _abs(b.size), c_size = _abs(c.size);
	if (b_size < c_size) {
		_swap(intx_t, b, c);
		b_size = _abs(b.size);
		c_size = _abs(c.size);
	}
	// ASSERT (a_size >= b_size >= c_size)

	uni_t u = { 0 };

	intx_t z = { dest, 0 };

	int64_t h = 0;
	cntx_t i = 0;
	for (; i < c_size; ++i) {
		u.a += (uint64_t)(*b.ptr++) + (*c.ptr++);
		if (*a.ptr >= u.b[0] + h) {
			*dest++ = *a.ptr++ - (u.b[0] + h);
			h = 0;
		}
		else {
			*dest++ = (INTEX8_DIGIT_MAX_VALUE + 1) + *a.ptr++ - (u.b[0] + h);
			h = 1;
		}
		u.a = u.b[1];
	}
	for (; i < b_size; ++i) {
		u.a += (uint64_t)(*b.ptr++);
		if (*a.ptr >= u.b[0] + h) {
			*dest++ = *a.ptr++ - (u.b[0] + h);
			h = 0;
		}
		else {
			*dest++ = (INTEX8_DIGIT_MAX_VALUE + 1) + *a.ptr++ - (u.b[0] + h);
			h = 1;
		}
		u.a = u.b[1];
	}
	for (; i < a_size; ++i) {
		if (*a.ptr >= u.b[0] + h) {
			*dest++ = *a.ptr++ - (u.b[0] + h);
			h = 0;
		}
		else {
			*dest++ = (INTEX8_DIGIT_MAX_VALUE + 1) + *a.ptr++ - (u.b[0] + h);
			h = 1;
		}
		u.a = u.b[1];
	}
	if (u.a > h) {
		*dest++ = u.b[0] - h;
		++i;
	}

	z.size = sign * i;
	_remove_leading_trivial_digits(&z);
	return z;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_add_subtraction_to(intx_t r2, intx_t r1, intx_t r4)
{   // r2 += r1 - r4   OR: r2 = r2 + r1 - r4
	// r4 >= 0
	if (r2.size == 0 && r1.size == 0 && r4.size == 0) {
		return r2;
	}
	else if (r2.size == 0 && r1.size == 0) {
		r4.size = -r4.size;
		return i8_copy(r4, r2.ptr);
	}
	else if (r2.size == 0 && r4.size == 0) {
		return i8_copy(r1, r2.ptr);
	}
	else if (r1.size == 0 && r4.size == 0) {
		return r2;
	}
	else if (r2.size == 0) {
		return _toom3_subtract(r1, r4, r2.ptr);
	}
	else if (r1.size == 0) {
		return _toom3_subtract(r2, r4, r2.ptr);
	}

	r4.size = -r4.size;
	// Now, evaluate r2 = r2 + r1 + r4

	// ASSERT (r2.size != 0 && r1.size != 0 && r4.size <= 0)

	uint8_t signs = (_sgn(r2.size) == -1 ? 1 : 0) | (_sgn(r1.size) == -1 ? 2 : 0) | (_sgn(r4.size) == -1 ? 4 : 0);

	if (signs == 0) {
		// (r2 > 0, r1 > 0, r4 == 0)
		return _add_3(r2, r1, r4, r2.ptr, 1);
	}
	else if (signs == 7) {
		// (r2 < 0, r1 < 0, r4 < 0)
		return _add_3(r2, r1, r4, r2.ptr, -1);
	}
	else if (signs == 1) {
		// r2 < 0; r1 >= 0; r4 == 0
		if (_sum_gt(r1, r4, r2))	// |r1| + |r4| > |r2|
			return _add_2_sub(r1, r4, r2, r2.ptr, 1);
		else
			return _add_sub_2(r2, r1, r4, r2.ptr, -1);
	}
	else if (signs == 6) {
		// r2 >= 0; r1 < 0; r4 < 0
		if (_sum_gt(r1, r4, r2))	// |r1| + |r4| > |r2|
			return _add_2_sub(r1, r4, r2, r2.ptr, -1);	// _sgn(result) == -1
		else
			return _add_sub_2(r2, r1, r4, r2.ptr, 1);	// _sgn(result) == 1
	}
	else if (signs == 2) {
		// r2 > 0; r1 < 0; r4 == 0
		if (_sum_gt(r2, r4, r1))	// |r2| + |r4| > |r1|
			return _add_2_sub(r2, r4, r1, r2.ptr, 1);
		else
			return _add_sub_2(r1, r2, r4, r2.ptr, -1);
	}
	else if (signs == 5) {
		// r2 < 0; r1 > 0; r4 < 0
		if (_sum_gt(r2, r4, r1))	// |r2| + |r4| > |r1|
			return _add_2_sub(r2, r4, r1, r2.ptr, -1);
		else
			return _add_sub_2(r1, r2, r4, r2.ptr, 1);
	}
	else if (signs == 3) {
		// r2 < 0; r1 < 0; r4 == 0
		if (_sum_gt(r1, r2, r4))	// |r1| + |r2| > |r4|
			return _add_2_sub(r1, r2, r4, r2.ptr, -1);
		else
			return _add_sub_2(r4, r1, r2, r2.ptr, 1);
	}
	else { // sign == 4
		// r2 >= 0; r1 >= 0; r4 < 0
		if (_sum_gt(r1, r2, r4))	// |r1| + |r2| > |r4|
			return _add_2_sub(r1, r2, r4, r2.ptr, 1);
		else
			return _add_sub_2(r4, r1, r2, r2.ptr, -1);
	}
}

/* Internal function. Do NOT call this directly! */
static size_t _toom3_add_to_result(const intx_t a, dig_t* ptr_result, size_t result_size)
{
	uint64_t* ptr = (uint64_t *)a.ptr;
	uint64_t* ptr_res = (uint64_t*)ptr_result;

	uint64_t h = 0;
	cntx_t start = 0, a_size = _abs(a.size) / 2;
	for (; start < a_size; ++start)
	{
		uint64_t h1 = 0;
		uint64_t s = *ptr_res + *ptr++;
		h1 = (s < *ptr_res) ? 1 : 0;
		if (h > 0) {
			s += h;
			if (s < h)
				++h1;
		}
		h = h1;
		*ptr_res++ = s;
	}

	uni_t x = { h };
	start *= 2;
	if (start < _abs(a.size)) {
		x.a += (uint64_t)ptr_result[start] + a.ptr[start];
		ptr_result[start++] = x.b[0];
		x.a = x.b[1];
	}
	for (; start < result_size && x.a != 0; ++start) {
		x.a += (uint64_t)(ptr_result[start]);
		ptr_result[start] = x.b[0];
		x.a = x.b[1];
	}
	return start;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_basic_multiply(const intx_t a, const intx_t b, dig_t *ptr)
{	// a >= 0 and b >= 0
	dig_t* a_ptr = a.ptr;
	cntx_t a_size = _abs(a.size);
	dig_t* b_ptr = b.ptr;
	cntx_t b_size = _abs(b.size);

	intx_t out = { ptr, 0 };
	if (a_size == 0 || b_size == 0) {
		return out;
	}
	union {
		uint64_t a[2];
		dig_t b[4];
	} x = { 0 };

	cntx_t n = (a_size - 1) + (b_size - 1);
	for (cntx_t i = 0; i <= n; ++i) {
		cntx_t j0 = (i > b_size - 1) ? i - (b_size - 1) : 0;
		cntx_t j1 = _min(i, a_size - 1);
		for (size_t j = j0; j <= j1; ++j) {
			uint64_t s = (uint64_t)a_ptr[j] * b_ptr[i - j];
			x.a[0] += s;
			if (x.a[0] < s)
				++x.a[1];
		}
		*ptr++ = x.b[0];

		x.b[0] = x.b[1];
		x.b[1] = x.b[2];
		x.b[2] = x.b[3];
		x.b[3] = 0;
	}

	for (; x.a[0] != 0 || x.a[1] != 0; ) {
		*ptr++ = x.b[0];
		x.b[0] = x.b[1];
		x.b[1] = x.b[2];
		x.b[2] = x.b[3];
		x.b[3] = 0;
	}

	out.size = (ptr - out.ptr);
	while (out.size > 1 && ptr[-1] == 0) {
		--out.size;
		--ptr;
	}
	_remove_leading_trivial_digits(&out);
	return out;
}

/*
 * Defines the size of the internal static buffer used for Toom-3 multiplication.
 */
#define INTEX8_TOOM3_BUFFER_SIZE	(21 * INTEX8_MAX_MULTIPLICATION_DIGITS)

/*
 * Internal constant that defines the maximum recursion depth for Toom-3 multiplication.
 * MUST be set to at least `log3(INTEX8_MAX_MULTIPLICATION_DIGITS)`.
 */
#define INTEX8_TOOM3_MAX_RECURSION		11  // > log3(INTEX8_MAX_MULTIPLICATION_DIGITS) + 2

/* Internal function. Do NOT call this directly! */
static dig_t* _toom3_evaluate(intx_t a, cntx_t b_size, intx_t* p1, intx_t* p_1, intx_t* p_2, dig_t* ptr)
{
	cntx_t a_size = _abs(a.size);

	cntx_t split = (_max(a_size, b_size) + 2) / 3;

	intx_t m0 = { a.ptr, _min(split, a_size) };
	_remove_leading_trivial_digits(&m0);

	intx_t m1 = { 0, 0 };
	if (a_size > split) {
		m1.ptr = a.ptr + split;
		m1.size = _min(split, a_size - split);
		_remove_leading_trivial_digits(&m1);
	}

	intx_t m2 = { 0, 0 };
	if (a_size > 2 * split) {
		m2.ptr = a.ptr + 2 * split;
		m2.size = a_size - 2 * split;
		_remove_leading_trivial_digits(&m2);
	}

	++split;
	dig_t* ptr1 = ptr, * ptr_1 = ptr + split, * ptr_2 = ptr + 2 * split;

	if (m0.size < m1.size) {
		if (m1.size < m2.size) {
			// 0 < 1 < 2	--->	p_1 >= 0 && p_2 >= 0
			*p1 = _toom3_branch0_012(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
		}
		else if (m1.size == m2.size) {
			// 0 < 1==2
			if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0
				if (false == _4a_plus_b_gt_2c(m2, m0, m1)) { // p_2 < 0
					*p1 = _toom3_branch3_012(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
				else { // p_2 >= 0
					*p1 = _toom3_branch2_012(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
			}
			else { // p_1 >= 0 ---> p_2 > 0
				*p1 = _toom3_branch0_012(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
		}
		else if (m0.size <= m2.size) {	// m1.size > m2.size
			// 0 <= 2 < 1
			if (m1.ptr[m1.size - 1] >= 4) { //	---> p_1 < 0 and p_2 < 0
				*p1 = _toom3_branch3_021(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
			else if (m1.ptr[m1.size - 1] >= 2) { //	---> p_1 < 0
				if (false == _4a_plus_b_gt_2c(m2, m0, m1)) { // ---> p_2 < 0
					*p1 = _toom3_branch3_021(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
				else { // ---> p_2 >= 0
					*p1 = _toom3_branch2_021(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
			}
			else if (false == _4a_plus_b_gt_2c(m2, m0, m1)) { // ---> p_2 < 0
				if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0 and p_2 < 0
					*p1 = _toom3_branch3_021(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
				else { // p_1 >= 0 and p_2 < 0
					*p1 = _toom3_branch1_021(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
			}
			else if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0 and p_2 >= 0
				*p1 = _toom3_branch2_021(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
			else { // p_1 >= 0 and p_2 >= 0
				*p1 = _toom3_branch0_021(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
		}
		else {	// m0.size > m2.size && m2.size < m1.size
			// 2 < 0 < 1
			if (false == _4a_plus_b_gt_2c(m2, m0, m1)) { // ---> p_2 < 0
				if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0 and p_2 < 0
					*p1 = _toom3_branch3_201(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
				else { // p_1 >= 0 and p_2 < 0
					*p1 = _toom3_branch1_201(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
			}
			else if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0 and p_2 >= 0
				*p1 = _toom3_branch2_201(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
			else { // p_1 >= 0 and p_2 >= 0
				*p1 = _toom3_branch0_201(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
		}
	}
	else if (m0.size == m1.size) {
		if (m1.size < m2.size) {
			// 0==1 < 2		---> p_1 >= 0 and p_2 >= 0
			*p1 = _toom3_branch0_012(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
		}
		else if (m1.size == m2.size) {
			// 0==1==2
			if (false == _4a_plus_b_gt_2c(m2, m0, m1)) { // ---> p_2 < 0
				if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0 and p_2 < 0
					*p1 = _toom3_branch3_012(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
				else { // p_1 >= 0 and p_2 < 0
					*p1 = _toom3_branch1_012(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
			}
			else if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0 and p_2 >= 0
				*p1 = _toom3_branch2_012(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
			else { // p_1 >= 0 and p_2 >= 0
				*p1 = _toom3_branch0_012(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
		}
		else {	// m0.size==m1.size > m2.size
			// 2 < 0==1
			if (false == _4a_plus_b_gt_2c(m2, m0, m1)) { // ---> p_2 < 0
				if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0 and p_2 < 0
					*p1 = _toom3_branch3_201(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
				else { // p_1 >= 0 and p_2 < 0
					*p1 = _toom3_branch1_201(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
			}
			else if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0 and p_2 >= 0
				*p1 = _toom3_branch2_201(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
			else { // p_1 >= 0 and p_2 >= 0
				*p1 = _toom3_branch0_201(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
		}
	}
	else { // m1.size < m0.size
		if (m0.size <= m2.size) {
			// 1 < 0 <= 2	--->	p_1 >= 0 && p_2 >= 0
			*p1 = _toom3_branch0_102(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
		}
		else if (m1.size < m2.size) {
			// 1 < 2 < 0	---> p_1 >= 0 and p_2 >= 0
			*p1 = _toom3_branch0_120(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
		}
		else if (m1.size == m2.size) {
			// 1==2 < 0
			if (false == _4a_plus_b_gt_2c(m2, m0, m1)) { // ---> p_2 < 0
				if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0 and p_2 < 0
					*p1 = _toom3_branch3_120(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
				else { // p_1 >= 0 and p_2 < 0
					*p1 = _toom3_branch1_120(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
				}
			}
			else if (false == _sum_gt(m0, m2, m1)) { // p_1 < 0 and p_2 >= 0
				*p1 = _toom3_branch2_120(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
			else { // p_1 >= 0 and p_2 >= 0
				*p1 = _toom3_branch0_120(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
		}
		else {	// m1.size > m2.size
			// 2 < 1 < 0	---> p_1 >= 0
			if (false == _4a_plus_b_gt_2c(m2, m0, m1)) { // ---> p_2 < 0
				*p1 = _toom3_branch1_210(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
			else { // p_2 >= 0
				*p1 = _toom3_branch0_210(m0, m1, m2, ptr1, p_1, ptr_1, p_2, ptr_2);
			}
		}
	}
	return p_2->ptr + _abs(p_2->size);
}

/* Internal function. Do NOT call this directly! */
intx_t _toom3_multiply(intx_t x, intx_t y, dig_t* dest)
{
	enum Toom3Mult {
		Toom3Mult_Start = 0,
		Toom3Mult_Multiply_P_0_by_Q_0,
		Toom3Mult_Multiply_P_Minus1_by_Q_Minus1,
		Toom3Mult_Multiply_P_1_by_Q_1,
		Toom3Mult_Multiply_P_Minus2_by_Q_Minus2,
		Toom3Mult_Multiply_P_Inf_by_Q_Inf,
		Toom3Mult_Calculate_R1_R2_R3,
		Toom3Mult_Done,
		Toom3Mult_Finished
	};

	typedef struct {
		enum Toom3Mult state;
		cntx_t split;
		intx_t a;
		cntx_t a_size;
		intx_t b;
		cntx_t b_size;
		intx_t out;
		intx_t p1, q1;
		intx_t p_1, q_1;
		intx_t p_2, q_2;
		intx_t r1;
		intx_t r_1;
		intx_t r_2;
		intx_t _r0;
		intx_t _r4;
	} toom3_workspace_t;

#ifdef INTEX8_TOOM3_DYNAMIC_BUF
	dig_t* toom3_buf = (dig_T*)malloc(21 * _max(x.size, y.size) * sizeof(dig_t));
	if (toom3_buf == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
#else
	static dig_t toom3_buf[INTEX8_TOOM3_BUFFER_SIZE];
#endif

	dig_t* ptrBuffer = toom3_buf;

	static toom3_workspace_t proVecx[INTEX8_TOOM3_MAX_RECURSION];

	int proVecCounter = 0;

	toom3_workspace_t beg;
	beg.split = 0;
	beg.a = x;
	beg.a_size = beg.a.size;
	beg.b = y;
	beg.b_size = beg.b.size;
	beg.state = Toom3Mult_Start;
	beg.out.ptr = dest;
	beg.out.size = x.size + y.size;
	memset(beg.out.ptr, 0, beg.out.size * sizeof(dig_t));

	proVecx[proVecCounter++] = beg;

	const int _basic_multiply_threshold = 50;
	// 60: 3.97046 + 4.01573 + 3.99316 + 4.01123 + 3.92687
	// 50: 3.52541 + 3.55416 + 3.49638 + 3.62553 + 3.5843
	// 48: 3.55018 + 3.551   + 3.51455 + 3.52027 + 3.57545
	// 45: 3.53347 + 3.58367 + 3.54709 + 3.5053  + 3.55781
	// 40: 3.52415 + 3.47127 + 3.50908 + 3.64114 + 3.51563
	// 30: 3.47955 + 3.55046 + 3.55074 + 3.66696 + 3.44119

	intx_t ret = { 0, 0 }, zero = { 0, 0 };
	while (proVecCounter > 0) {
		toom3_workspace_t* pro = &proVecx[proVecCounter - 1];
		pro->state++;
		if (pro->state == Toom3Mult_Multiply_P_0_by_Q_0) {	// do multiplication r0 = m0 * n0
			//multiply_r0(pro);
			{
				if (pro->a_size <= _basic_multiply_threshold || pro->b_size <= _basic_multiply_threshold) {
					pro->out = _toom3_basic_multiply(pro->a, pro->b, pro->out.ptr);
					pro->out.size *= _sgn(pro->a.size) * _sgn(pro->b.size);
					pro->state = Toom3Mult_Done;
				}
				else {
					pro->split = (_max(pro->a_size, pro->b_size) + 2) / 3;

					toom3_workspace_t r = { 0 };
					//r.split = 0;
					r.a.ptr = pro->a.ptr;
					r.a.size = _min(pro->split, pro->a_size);
					_remove_leading_trivial_digits(&r.a);
					r.a_size = r.a.size;
					
					r.b.ptr = pro->b.ptr;
					r.b.size = _min(pro->split, pro->b_size);
					_remove_leading_trivial_digits(&r.b);
					r.b_size = r.b.size;

					r.out.ptr = pro->out.ptr;
					//r.state = Toom3Mult_Start;
					proVecx[proVecCounter++] = r;
				}
			}
		}
		else if (pro->state == Toom3Mult_Multiply_P_Minus1_by_Q_Minus1) {	// do multiplication r_1 = p_1 * q_1
			pro->_r0 = ret;
			ret = zero;
			//multiply_r_1(pro);
			{
				ptrBuffer = _toom3_evaluate(pro->a, pro->b_size, &pro->p1, &pro->p_1, &pro->p_2, ptrBuffer);
				ptrBuffer = _toom3_evaluate(pro->b, pro->a_size, &pro->q1, &pro->q_1, &pro->q_2, ptrBuffer);

				toom3_workspace_t r = { 0 };
				//r.split = 0;

				r.a = pro->p_1;
				r.a_size = _abs(r.a.size);
				r.b = pro->q_1;
				r.b_size = _abs(r.b.size);

				r.out.ptr = ptrBuffer;
				memset(ptrBuffer, 0, (r.a_size + r.b_size) * sizeof(dig_t));
				ptrBuffer += (r.a_size + r.b_size);

				//r.state = Toom3Mult_Start;
				proVecx[proVecCounter++] = r;
			}
		}
		else if (pro->state == Toom3Mult_Multiply_P_1_by_Q_1) {	// do multiplication r1 = p1 * q1
			pro->r_1 = ret;
			ptrBuffer = pro->r_1.ptr + _abs(pro->r_1.size);
			ret = zero;
			//multiply_r1(pro);
			{
				toom3_workspace_t r = { 0 };
				//r.split = 0;
				r.a = pro->p1;
				r.a_size = r.a.size;
				r.b = pro->q1;
				r.b_size = r.b.size;

				r.out.ptr = ptrBuffer;
				memset(ptrBuffer, 0, (r.a_size + r.b_size) * sizeof(dig_t));
				ptrBuffer += (r.a_size + r.b_size);

				//r.state = Toom3Mult_Start;
				proVecx[proVecCounter++] = r;
			}
		}
		else if (pro->state == Toom3Mult_Multiply_P_Minus2_by_Q_Minus2) {	// do multiplication r_2 = p_2 * q_2
			pro->r1 = ret;
			ptrBuffer = pro->r1.ptr + pro->r1.size;
			ret = zero;
			//multiply_r_2(pro);
			{
				toom3_workspace_t r = { 0 };
				//r.split = 0;
				r.a = pro->p_2;
				r.a_size = _abs(r.a.size);
				r.b = pro->q_2;
				r.b_size = _abs(r.b.size);

				r.out.ptr = ptrBuffer;
				memset(ptrBuffer, 0, (r.a_size + r.b_size) * sizeof(dig_t));
				ptrBuffer += (r.a_size + r.b_size);

				//r.state = Toom3Mult_Start;
				proVecx[proVecCounter++] = r;
			}
		}
		else if (pro->state == Toom3Mult_Multiply_P_Inf_by_Q_Inf) {	// do multiplication r4 = m2 * n2
			pro->r_2 = ret;
			ptrBuffer = pro->r_2.ptr + _abs(pro->r_2.size);
			ret = zero;
			//multiply_r4(pro);
			{
				if (pro->a_size <= _basic_multiply_threshold || pro->b_size <= _basic_multiply_threshold) {
					pro->out = _toom3_basic_multiply(pro->a, pro->b, pro->out.ptr);
					pro->state = Toom3Mult_Done;// done();
				}
				else if (pro->a_size > 2 * pro->split && pro->b_size > 2 * pro->split) {
					// append a new item to proVec
					toom3_workspace_t r = { 0 };
					//r.split = 0;
					if (pro->a_size > 2 * pro->split) {
						r.a.ptr = pro->a.ptr + 2 * pro->split;
						r.a.size = pro->a_size - 2 * pro->split;
						_remove_leading_trivial_digits(&r.a);
						r.a_size = r.a.size;
					}
					if (pro->b_size > 2 * pro->split) {
						r.b.ptr = pro->b.ptr + 2 * pro->split;
						r.b.size = pro->b_size - 2 * pro->split;
						_remove_leading_trivial_digits(&r.b);
						r.b_size = r.b.size;
					}
					r.out.ptr = pro->out.ptr + 4 * pro->split;
					//r.state = Toom3Mult_Start;
					proVecx[proVecCounter++] = r;
				}
			}
		}
		else if (pro->state == Toom3Mult_Calculate_R1_R2_R3) {
			// do evaluate R1, R2, R3
			pro->_r4 = ret;
			ret = zero;
			//calculate_C1_C2_C3(pro);
			{
				//****** _r3 = ( r(-2) - r(1) ) / 3
				// r_2 -= r1
				// r_2 /= 3
				dig_t* bp = ptrBuffer;
				intx_t _r3_1 = _toom3_adivide_by_3(_toom3_subtract(pro->r_2, pro->r1, ptrBuffer));

				//****** _r1 = ( r(1) - r(-1) ) / 2
				// r1 -= r_1
				// r1 /= 2
				intx_t _r1 = _toom3_adivide_by_2(_toom3_subtract(pro->r1, pro->r_1, _r3_1.ptr + 2 * pro->split + 2));

				//****** r2 = ( r(-1) - r(0) )
				// r_1 -= _r0
				intx_t _r2 = _toom3_subtract(pro->r_1, pro->_r0, _r1.ptr + 2 * pro->split + 2);

				//****** r3 = (r2 - r3) / 2 + 2 * r(inf)
				// _r3  = (_r2 - _r3) / 2
				// _r3 += 2 * r_inf;
				intx_t _r3_2 = _toom3_add_twice_to(_toom3_adivide_by_2(_toom3_subtract(_r2, _r3_1, _r2.ptr + 2 * pro->split + 2)), pro->_r4);

				//****** r2 = r2 + r1 - r4
				// _r2 += _r1 - _r4
				_r2 = _toom3_add_subtraction_to(_r2, _r1, pro->_r4);
				//_r2 = _toom3_subtract(_r1, pro->_r4, ptrBuffer);

				//****** r1 = r1 - r3
				// _r1 -= _r3
				_r1 = _toom3_subtract(_r1, _r3_2, _r1.ptr);

				///// Finalize
				_toom3_add_to_result(_r1, pro->out.ptr + 1 * pro->split, pro->a_size + pro->b_size);
				_toom3_add_to_result(_r2, pro->out.ptr + 2 * pro->split, pro->a_size + pro->b_size);
				_toom3_add_to_result(_r3_2, pro->out.ptr + 3 * pro->split, pro->a_size + pro->b_size);

				cntx_t size = pro->a_size + pro->b_size;
				while (size >= 1 && pro->out.ptr[size - 1] == 0)
					--size;
				pro->out.size = size * _sgn(pro->a.size) * _sgn(pro->b.size);
				ptrBuffer = bp;
			}
			pro->state = Toom3Mult_Done;
		}
		else if (pro->state == Toom3Mult_Finished) {
			ret = pro->out;
			proVecCounter--;
		}
		else {
		}
	}
#ifdef INTEX8_TOOM3_DYNAMIC_BUF
	free(toom3_buf);
#endif

	return proVecx[0].out;
}
