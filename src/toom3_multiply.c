/*
 *  File: toom3_multiply.c
 *  Description:
 *      This file contains internal functions used within the ix8 library.
 *      These functions are not intended to be called directly by users.
 *      Only public API functions should be used externally.
 */

#include <memory.h>
#include "i8.h"
#include "util.h"

/* Internal function. Do NOT call this directly! */
static void _remove_leading_trivial_digits(intx_t* out)
{
	while (out->size >= 2 && (
		(out->ptr[out->size - 1] == 0 && !_has_sign_bit(out->ptr[out->size - 2])) ||
		(out->ptr[out->size - 1] == INTEX8_DIGIT_MAX_VALUE() && _has_sign_bit(out->ptr[out->size - 2])) )
		)
		out->ptr[--out->size] = 0;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_add_positive(const intx_t a1, const intx_t b1, dig_t* ptr)
{
	// a1 >= 0 and b1 >= 0

	intx_t out = { ptr, 0 };
	if (a1.size == 0 && b1.size == 0)
		return out;

	intx_t a = a1;
	intx_t b = b1;

	if (a.size > b.size) {
		a = b1;
		b = a1;
	}

	uni_t x = {0};

	size_t i = 0;
	for (; i < a.size; ++i) {
		x.a += (uint64_t)a.ptr[i] + (uint64_t)b.ptr[i];
		out.ptr[out.size++] = x.b[0];
		x.a = x.b[1];
	}
	for (; i < b.size; ++i) {
		x.a += (uint64_t)b.ptr[i];
		out.ptr[out.size++] = x.b[0];
		x.a = x.b[1];
	}
	if(x.b[0])
		out.ptr[out.size++] = x.b[0];
	if (_has_sign_bit(ptr[out.size - 1]))
		ptr[out.size++] = 0;
	_remove_leading_trivial_digits(&out);

	return out;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_subtract(const intx_t a, const intx_t b, dig_t* ptr, bool positive)
{   // returns a - b

	intx_t out = { ptr, 0 };
	if (a.size == 0 && b.size == 0)
		return out;

	uint64_t a_ext = (positive ? 0 : _get_ext(a));
	uint64_t b_ext = (positive ? 0 : _get_ext(b));

	size_t m = _max(a.size, b.size);
	uint64_t h = 0;
	for (size_t i = 0; i < m; ++i) {
		uint64_t x = (i < a.size) ? (uint64_t)a.ptr[i] : a_ext;
		uint64_t y = (i < b.size) ? (uint64_t)b.ptr[i] : b_ext;

		if (x >= y + h) {
			out.ptr[out.size++] = x - (y + h);
			h = 0;
		}
		else {
			out.ptr[out.size++] = (INTEX8_DIGIT_MAX_VALUE() + 1) + x - (y + h);
			h = 1;
		}
	}
	if (a_ext >= b_ext + h)
		out.ptr[out.size++] = a_ext - (b_ext + h);
	else
		out.ptr[out.size++] = (INTEX8_DIGIT_MAX_VALUE() + 1) + a_ext - (b_ext + h);

	if (a_ext == 0 && b_ext == INTEX8_DIGIT_MAX_VALUE()) {
		if (_has_sign_bit(out.ptr[out.size - 1]))
			out.ptr[out.size++] = 0;
	}
	else if (a_ext == INTEX8_DIGIT_MAX_VALUE() && b_ext == 0) {
		if (!_has_sign_bit(out.ptr[out.size - 1]))
			out.ptr[out.size++] = INTEX8_DIGIT_MAX_VALUE();
	}
	_remove_leading_trivial_digits(&out);
	return out;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_adivide_by_2(intx_t a)
{
	if (a.size == 0)
		return a;

	int sign = 1;
	if (i8_is_negative(a))
		sign = -1;

	uni_t x;
	x.a = 0;

	size_t i = 0;
	for (; i + 1 < a.size; i++) {
		x.b[0] = a.ptr[i];
		x.b[1] = a.ptr[i + 1];
		x.a >>= 1;
		a.ptr[i] = x.b[0];
	}
	a.ptr[i] >>= 1;
	if (sign == -1)
		a.ptr[i] |= INTEX8_DIGIT_SIGN_MASK();

	return a;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_adivide_by_3(intx_t a)
{	// a is a multiple of 3
	if (a.size == 0)
		return a;

	const dig_t D = 3;

	uni_t x, y;

	intx_t out = { a.ptr, a.size };

	if (i8_is_negative(a)) {
		// a is negative
		size_t f = 0;
		for (; f < a.size - 1; ++f) {
			if (a.ptr[f] > 0)
				break;
		}
		//// part 1: indices larger than f
		x.b[1] = 0;
		size_t i = a.size, s = out.size;
		for (; i > f + 1;)
		{
			x.b[0] = ~a.ptr[--i];
			y.a = x.a / D;
			a.ptr[--s] = ~y.b[0];
			x.a -= y.a * D;
			x.b[1] = x.b[0];
		}
		//// part 2: index equal to f
		--i;
		x.b[0] = (~a.ptr[f]) + 1;
		y.a = x.a / D;
		a.ptr[--s] = ~y.b[0];
		x.a -= y.a * D;
		x.b[1] = x.b[0];

		//// part 3: indices less than f
		for (; i > 0;)
		{
			--i;
			x.b[0] = 0;
			y.a = x.a / D;
			a.ptr[--s] = ~y.b[0];
			x.a -= y.a * D;
			x.b[1] = x.b[0];
		}
		//// part 4: add the result by 1
		f = 0;
		for (; f < out.size; ++f) {
			if (a.ptr[f] < INTEX8_DIGIT_MAX_VALUE()) {
				a.ptr[f]++;
				memset(a.ptr, 0, f * sizeof(dig_t));
				break;
			}
		}
		if (f >= out.size) {
			a.ptr[0] = 0;
			out.size = 1;
			return out;
		}
	}
	else {
		// a is not a negative intx
		x.b[1] = 0;

		size_t i = a.size;
		for (; i > 0;) {
			x.b[0] = a.ptr[--i];
			y.a = x.a / D;
			a.ptr[--a.size] = y.b[0];
			x.a -= y.a * D;
			x.b[1] = x.b[0];
		}
	}
	return out;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_add_twice_to(intx_t a, const intx_t b)
{	// a += 2 * b
	if (b.size == 0)
		return a;

	intx_t out = { a.ptr, 0 };

	uint64_t a_ext = _get_ext(a);

	uni_t x = {0}, y = {0}, b2 = {0};

	size_t m = _max(a.size, b.size);
	uint64_t aa, h = 0;
	for (size_t i = 0; i < m; ++i) {
		aa = (i < a.size) ? (uint64_t)a.ptr[i] : a_ext;

		if (i < b.size)
			b2.a += 2 * (uint64_t)b.ptr[i];

		y.a = (uint64_t)b2.b[0] + y.b[0];
		x.a += aa + y.b[0];
		out.ptr[out.size++] = x.b[0];
		x.a = x.b[1];
		y.a = y.b[1];
		b2.a = b2.b[1];
	}
	{
		y.a = (uint64_t)b2.b[0] + y.b[0];
		x.a += a_ext + y.b[0];
		out.ptr[out.size++] = x.b[0];
		x.a = x.b[1];
		y.a = y.b[1];
		b2.a = b2.b[1];
	}
	if (a_ext == 0) {
		if (_has_sign_bit(out.ptr[out.size - 1]))
			out.ptr[out.size++] = 0;
	}
	_remove_leading_trivial_digits(&out);
	return out;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_subtract_from_twice_sum(const intx_t p_1, const intx_t m2, const intx_t m0, dig_t *out_ptr)
{
	// m2 >= 0 and m0 >= 0
	// out = (p_1 + m2) * 2 - m0

	intx_t out = {out_ptr, 0};

	uint64_t p_1_ext = _get_ext(p_1);

	int m = _max(_max(p_1.size, m2.size), m0.size);

	uni_t x = {0};

	uint64_t h = 0;
	for (int i = 0; i < m; ++i) {
		uint64_t s = (i < p_1.size) ? (uint64_t)p_1.ptr[i] : p_1_ext;
		uint64_t t2 = (i < m2.size) ? (uint64_t)m2.ptr[i] : 0;
		uint64_t t0 = (i < m0.size) ? (uint64_t)m0.ptr[i] : 0;
		x.a += (s + t2) << 1;

		if (x.a >= t0 + h) {
			x.a -= (t0 + h);
			out.ptr[out.size++] = x.b[0];
			h = 0;
		}
		else {
			x.a += (INTEX8_DIGIT_MAX_VALUE() + 1) - (t0 + h);
			out.ptr[out.size++] = x.b[0];
			h = 1;
		}
		x.b[0] = x.b[1];
		x.b[1] = 0;
	}
	{
		x.a += (p_1_ext << 1);

		if (x.a >= h) {
			x.a -= h;
			out.ptr[out.size++] = x.b[0];
			h = 0;
		}
		else {
			x.a += (INTEX8_DIGIT_MAX_VALUE() + 1) - h;
			out.ptr[out.size++] = x.b[0];
			h = 1;
		}
		x.b[0] = x.b[1];
		x.b[1] = 0;
	}
	_remove_leading_trivial_digits(&out);
	return out;
}

/* Internal function. Do NOT call this directly! */
static size_t _toom3_add_to_result(const intx_t a, dig_t* ptr_result, size_t result_size)
{
	uni_t x = { 0 };
	size_t start = 0;
	for (size_t i = 0; i < a.size; ++i)
	{
		x.a += (uint64_t)ptr_result[start] + a.ptr[i];

		ptr_result[start] = x.b[0];
		start++;
		x.a = x.b[1];
	}
	for (; start < result_size && x.a != 0; ) {
		x.a += (uint64_t)ptr_result[start];
		ptr_result[start] = x.b[0];
		start++;
		x.a = x.b[1];
	}
	return start;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_basic_multiply(const intx_t a, const intx_t b, dig_t* ptr_out)
{	// a >= 0 and b >= 0
	intx_t out = { ptr_out, 0 };

	if (a.size == 0 || b.size == 0) {
		out.ptr[out.size++] = 0;
		return out;
	}
	union {
		uint64_t a;
		uint16_t b[4];
	} x = { 0 };

	size_t out_size = 0;
	uint16_t *out_ptr = (uint16_t *)out.ptr;

	size_t a_size = 2 * a.size;
	size_t b_size = 2 * b.size;
	uint16_t *a_ptr = (uint16_t *)a.ptr;
	uint16_t *b_ptr = (uint16_t *)b.ptr;

	size_t n = (a_size - 1) + (b_size - 1);
	for (size_t i = 0; i <= n; ++i) {
		size_t j0 = (i > b_size - 1) ? i - (b_size - 1) : 0;
		size_t j1 = _min(i, a_size - 1);
		for (size_t j = j0; j <= j1; ++j) {
			x.a += (uint64_t)a_ptr[j] * b_ptr[i - j];
		}
		out_ptr[out_size++] = x.b[0];
		x.b[0] = x.b[1];
		x.b[1] = x.b[2];
		x.b[2] = x.b[3];
		x.b[3] = 0;
	}
	for (size_t i = 0; i < 4 && x.a != 0; ++i) {
		out_ptr[out_size++] = x.b[i];
		x.b[i] = 0;
	}
	while (out_size > 1 && out_ptr[out_size - 1] == 0) {
		--out_size;
	}
	if (out_size % 2 == 1)
		out_ptr[out_size++] = 0;

	out.size = out_size / 2;
	if (i8_is_negative(out))
		out.ptr[out.size++] = 0;
	return out;
}

/* Internal function. Do NOT call this directly! */
static intx_t _toom3_add_subtraction_to(intx_t r2, const intx_t r1, const intx_t r4)
{   // r2 += r1 - r4   OR: r2 = r2 + r1 - r4
	// r4 >= 0

	uint64_t r2_ext = _get_ext(r2);
	uint64_t r1_ext = _get_ext(r1);

	int m = _max(_max(r1.size, r4.size), r2.size);

	dig_t *to = r2.ptr;

	uni_t x;
	x.a = 0;

	uint64_t s1 = 0, s2, s4, h = 0;
	for (int i = 0; i < m; ++i) {
		s4 = (i < r4.size) ? (uint64_t)r4.ptr[i] : 0;
		x.a += ((i < r1.size) ? (uint64_t)r1.ptr[i] : r1_ext) + ((i < r2.size) ? (uint64_t)r2.ptr[i] : r2_ext);

		if (x.a >= s4 + h) {
			x.a -= (s4 + h);
			*to++ = x.b[0];
			h = 0;
		}
		else {
			x.a += (INTEX8_DIGIT_MAX_VALUE() + 1) - (s4 + h);
			*to++ = x.b[0];
			h = 1;
		}
		x.b[0] = x.b[1];
		x.b[1] = 0;
	}
	{
		x.a += r1_ext + r2_ext;
		if (x.a >= h)
			x.a -= h;
		else
			x.a += (INTEX8_DIGIT_MAX_VALUE() + 1) - h;
		*to++ = x.b[0];
	}
	r2.size = (size_t)(to - r2.ptr);

	// if original r2 and r1 were negative, the new r2 should also be negative 
	if (r2_ext == INTEX8_DIGIT_MAX_VALUE() && r1_ext == INTEX8_DIGIT_MAX_VALUE()) {
		if (!_has_sign_bit(r2.ptr[r2.size - 1]))
			r2.ptr[r2.size++] = INTEX8_DIGIT_MAX_VALUE();
	}
	_remove_leading_trivial_digits(&r2);

	return r2;
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
		size_t split;
		bool negate_out;
		intx_t a;
		intx_t b;
		intx_t out;
		intx_t p0, q0;
		intx_t p1, q1;
		intx_t p_1, q_1;
		intx_t p_2, q_2;
		intx_t r1;
		intx_t r_1;
		intx_t r_2;
		intx_t _r0;
		intx_t _r4;
	} toom3_workspace_t;

	const int safety = 2;

#ifdef INTEX8_TOOM3_DYNAMIC_BUF
	dig_t *toom3_buf = (dig_T *)malloc( 21 * _max(x.size, y.size) * sizeof(dig_t) );
	if (toom3_buf == NULL) {
		intEx8_errno = INTEX8_ERR_MEMORY_ALLOCATION_FAILED;
		return intx_zero;
	}
#else
	dig_t toom3_buf[INTEX8_TOOM3_BUFFER_SIZE];
#endif

	dig_t* ptrBuffer = toom3_buf;

	int proVecCounter;
	toom3_workspace_t proVecx[INTEX8_TOOM3_MAX_RECURSION];

	proVecCounter = 0;

	toom3_workspace_t beg;
	beg.split = 0;
	beg.negate_out = false;
	beg.a = x;
	beg.b = y;
	beg.state = Toom3Mult_Start;
	beg.out.ptr = dest;
	beg.out.size = x.size + y.size;
	memset(beg.out.ptr, 0, beg.out.size * sizeof(dig_t));

	proVecx[proVecCounter++] = beg;

	intx_t ret, zero = {0, 0};
	while (proVecCounter > 0) {
		toom3_workspace_t *pro = &proVecx[proVecCounter - 1];
		pro->state++;
		if (pro->state == Toom3Mult_Multiply_P_0_by_Q_0) {	// do multiplication r0 = m0 * n0
			//multiply_r0(pro);
			{
				if (pro->a.size <= 2 && pro->b.size <= 2) {
					pro->out = _toom3_basic_multiply(pro->a, pro->b, pro->out.ptr);
					if (pro->negate_out)
						pro->out = i8_negate_self(pro->out);
					pro->state = Toom3Mult_Done;
				}
				else {
					pro->split = (_max(pro->a.size, pro->b.size) + 2) / 3;
					toom3_workspace_t r;
					r.split = 0;
					r.negate_out = false;
					r.a.ptr = pro->a.ptr;
					r.a.size = _min(pro->split, pro->a.size);
					r.b.ptr = pro->b.ptr;
					r.b.size = _min(pro->split, pro->b.size);

					r.out.ptr = pro->out.ptr;
					r.state = Toom3Mult_Start;
					proVecx[proVecCounter++] = r;
				}
			}
		}
		else if (pro->state == Toom3Mult_Multiply_P_Minus1_by_Q_Minus1) {	// do multiplication r_1 = p_1 * q_1
			pro->_r0 = ret;
			ret = zero;
			//multiply_r_1(pro);
			{
				// 1. calculate p0
				intx_t m0 = { pro->a.ptr, _min(pro->split, pro->a.size) };
				intx_t m2 = { 0, 0 };
				if (pro->a.size > 2 * pro->split) {
					m2.ptr = pro->a.ptr + 2 * pro->split;
					m2.size = pro->a.size - 2 * pro->split;
				}
				pro->p0 = _toom3_add_positive(m0, m2, ptrBuffer);
				ptrBuffer += pro->p0.size + safety;

				toom3_workspace_t r;
				r.split = 0;
				r.negate_out = false;

				int neg_count = 0;
				// 1. p_1 = p0 - m1
				intx_t m1 = { 0, 0 };
				if (pro->a.size > pro->split) {
					m1.ptr = pro->a.ptr + pro->split;
					m1.size = _min(pro->split, pro->a.size - pro->split);
				}
				pro->p_1 = _toom3_subtract(pro->p0, m1, ptrBuffer, true);
				if (_has_sign_bit(pro->p_1.ptr[pro->p_1.size - 1])) {
					r.a = i8_negate(pro->p_1, pro->p_1.ptr + pro->p_1.size);// negate(pro->p_1, pro->p_1.ptr + pro->p_1.size);
					++neg_count;
				}
				else
					r.a = pro->p_1;
				ptrBuffer = r.a.ptr + r.a.size + safety;

				// 1. calculate q0
				intx_t n0 = { pro->b.ptr, _min(pro->split, pro->b.size) };
				intx_t n2 = { 0, 0 };
				if (pro->b.size > 2 * pro->split) {
					n2.ptr = pro->b.ptr + 2 * pro->split;
					n2.size = pro->b.size - 2 * pro->split;
				}
				pro->q0 = _toom3_add_positive(n0, n2, ptrBuffer);
				ptrBuffer += pro->q0.size + safety;

				// 2. q_1 = q0 - n1
				intx_t n1 = { 0, 0 };
				if (pro->b.size > pro->split) {
					n1.ptr = pro->b.ptr + pro->split;
					n1.size = _min(pro->split, pro->b.size - pro->split);
				}
				pro->q_1 = _toom3_subtract(pro->q0, n1, ptrBuffer, true);
				if (_has_sign_bit(pro->q_1.ptr[pro->q_1.size - 1])) {
					r.b = i8_negate(pro->q_1, pro->q_1.ptr + pro->q_1.size);// negate(pro->q_1, pro->q_1.ptr + pro->q_1.size);
					++neg_count;
				}
				else
					r.b = pro->q_1;
				ptrBuffer = r.b.ptr + r.b.size + safety;

				// 3. 
				r.out.ptr = ptrBuffer;
				memset(ptrBuffer, 0, ((r.a.size + r.b.size) + safety) * sizeof(dig_t));
				ptrBuffer += (r.a.size + r.b.size) + safety;
				r.negate_out = (neg_count == 1);

				// 4. append an item to proVec
				r.state = Toom3Mult_Start;
				proVecx[proVecCounter++] = r;
			}
		}
		else if (pro->state == Toom3Mult_Multiply_P_1_by_Q_1) {	// do multiplication r1 = p1 * q1
			pro->r_1 = ret;
			ptrBuffer = pro->r_1.ptr + pro->r_1.size;
			ret = zero;
			//multiply_r1(pro);
			{
				toom3_workspace_t r;
				r.split = 0;
				r.negate_out = false;
				// 1.	// p1 = p0 + m1
				intx_t m1 = { 0, 0 };
				if (pro->a.size > pro->split) {
					m1.ptr = pro->a.ptr + pro->split;
					m1.size = _min(pro->split, pro->a.size - pro->split);
				}
				pro->p1 = _toom3_add_positive(pro->p0, m1, ptrBuffer);
				r.a = pro->p1;
				ptrBuffer += r.a.size + safety;
				// 2.	// q1 = q0 + n1
				intx_t n1 = { 0, 0 };
				if (pro->b.size > pro->split) {
					n1.ptr = pro->b.ptr + pro->split;
					n1.size = _min(pro->split, pro->b.size - pro->split);
				}
				pro->q1 = _toom3_add_positive(pro->q0, n1, ptrBuffer);
				r.b = pro->q1;
				ptrBuffer += r.b.size + safety;
				// 3.
				r.out.ptr = ptrBuffer;
				memset(ptrBuffer, 0, ((r.a.size + r.b.size) + safety) * sizeof(dig_t));
				ptrBuffer += (r.a.size + r.b.size) + safety;
				// 4.
				r.state = Toom3Mult_Start;
				proVecx[proVecCounter++] = r;
			}
		}
		else if (pro->state == Toom3Mult_Multiply_P_Minus2_by_Q_Minus2) {	// do multiplication r_2 = p_2 * q_2
			pro->r1 = ret;
			ptrBuffer = pro->r1.ptr + pro->r1.size;
			ret = zero;
			//multiply_r_2(pro);
			{	// do multiplication r_2 = p_2 * q_2
				toom3_workspace_t r;
				r.split = 0;
				r.negate_out = false;

				int neg_count = 0;
				// 1. p_2 = (p_1 + m2) * 2 - m0
				intx_t m0 = { pro->a.ptr, _min(pro->split, pro->a.size) };
				intx_t m2 = { 0, 0 };
				if (pro->a.size > 2 * pro->split) {
					m2.ptr = pro->a.ptr + 2 * pro->split;
					m2.size = pro->a.size - 2 * pro->split;
				}
				pro->p_2 = _toom3_subtract_from_twice_sum(pro->p_1, m2, m0, ptrBuffer);
				if (_has_sign_bit(pro->p_2.ptr[pro->p_2.size - 1])) {
					r.a = i8_negate(pro->p_2, pro->p_2.ptr + pro->p_2.size);
					++neg_count;
				}
				else
					r.a = pro->p_2;
				ptrBuffer = r.a.ptr + r.a.size + safety;

				// 2. q_2 = (q_1 + n2) * 2 - n0
				intx_t n0 = { pro->b.ptr, _min(pro->split, pro->b.size) };
				intx_t n2 = { 0, 0 };
				if (pro->b.size > 2 * pro->split) {
					n2.ptr = pro->b.ptr + 2 * pro->split;
					n2.size = pro->b.size - 2 * pro->split;
				}
				pro->q_2 = _toom3_subtract_from_twice_sum(pro->q_1, n2, n0, ptrBuffer);
				if (_has_sign_bit(pro->q_2.ptr[pro->q_2.size - 1])) {
					r.b = i8_negate(pro->q_2, pro->q_2.ptr + pro->q_2.size);
					++neg_count;
				}
				else
					r.b = pro->q_2;
				ptrBuffer = r.b.ptr + r.b.size + safety;

				// 3. 
				r.out.ptr = ptrBuffer;
				memset(ptrBuffer, 0, ((r.a.size + r.b.size) + safety) * sizeof(dig_t));
				ptrBuffer += (r.a.size + r.b.size) + safety;
				r.negate_out = (neg_count == 1);

				// 4. append an item to proVec
				r.state = Toom3Mult_Start;
				proVecx[proVecCounter++] = r;
			}
		}
		else if (pro->state == Toom3Mult_Multiply_P_Inf_by_Q_Inf) {	// do multiplication r4 = m2 * n2
			pro->r_2 = ret;
			ptrBuffer = pro->r_2.ptr + pro->r_2.size;
			ret = zero;
			//multiply_r4(pro);
			{
				if (pro->a.size <= 2 && pro->b.size <= 2) {
					pro->out = _toom3_basic_multiply(pro->a, pro->b, pro->out.ptr);
					pro->state = Toom3Mult_Done;// done();
				}
				else if (pro->a.size > 2 * pro->split && pro->b.size > 2 * pro->split) {
					// append a new item to proVec
					toom3_workspace_t r;
					r.split = 0;
					r.negate_out = false;
					r.a = zero;
					//r.a.ptr = 0;
					//r.a.size = 0;
					if (pro->a.size > 2 * pro->split) {
						r.a.ptr = pro->a.ptr + 2 * pro->split;
						r.a.size = pro->a.size - 2 * pro->split;
					}
					r.b.ptr = 0;
					r.b.size = 0;
					if (pro->b.size > 2 * pro->split) {
						r.b.ptr = pro->b.ptr + 2 * pro->split;
						r.b.size = pro->b.size - 2 * pro->split;
					}
					r.out.ptr = pro->out.ptr + 4 * pro->split;
					r.state = Toom3Mult_Start;
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
				// PLACE _r3 IN SPACE OCCUPIED BY r(-2)
				dig_t* bp = ptrBuffer;
				intx_t _r3_1 = _toom3_adivide_by_3(_toom3_subtract(pro->r_2, pro->r1, ptrBuffer, false));

				//****** _r1 = ( r(1) - r(-1) ) / 2
				// r1 -= r_1
				// r1 /= 2
				// PLACE _r1 IN SPACE OCCUPIED BY r(1)
				intx_t _r1 = _toom3_adivide_by_2(_toom3_subtract(pro->r1, pro->r_1, _r3_1.ptr + 2 * pro->split + 2, false));// _r3_1.to));

				//****** r2 = ( r(-1) - r(0) )
				// r_1 -= _r0
				// PLACE _r2 IN SPACE OCCUPIED BY r_1 [r(-1)]
				intx_t _r2 = _toom3_subtract(pro->r_1, pro->_r0, _r1.ptr + 2 * pro->split + 2, false);// _r1_1.to);

				//****** r3 = (r2 - r3) / 2 + 2 * r(inf)
				// _r3  = (_r2 - _r3) / 2
				// _r3 += 2 * r_inf;
				intx_t _r3_2 = _toom3_add_twice_to(_toom3_adivide_by_2(_toom3_subtract(_r2, _r3_1, _r2.ptr + 2 * pro->split + 2, false)), pro->_r4);

				//****** r2 = r2 + r1 - r4
				// _r2 += _r1 - _r4
				_r2 = _toom3_add_subtraction_to(_r2, _r1, pro->_r4);

				//****** r1 = r1 - r3
				// _r1 -= _r3
				_r1 = _toom3_subtract(_r1, _r3_2, _r1.ptr, false);

				///// Finalize
				_toom3_add_to_result(_r1, pro->out.ptr + 1 * pro->split, pro->a.size + pro->b.size);
				_toom3_add_to_result(_r2, pro->out.ptr + 2 * pro->split, pro->a.size + pro->b.size);
				_toom3_add_to_result(_r3_2, pro->out.ptr + 3 * pro->split, pro->a.size + pro->b.size);

				intx_t out = { pro->out.ptr, pro->a.size + pro->b.size };

				if (_has_sign_bit(out.ptr[out.size - 1]))
					out.ptr[out.size++] = 0;
				while (out.size >= 2 && out.ptr[out.size - 1] == 0 && !_has_sign_bit(out.ptr[out.size - 2]))
					out.ptr[--out.size] = 0;

				ptrBuffer = bp;
				pro->out = (pro->negate_out ? i8_negate_self(out) : out);
			}
			pro->state = Toom3Mult_Done;
		}
		else if(pro->state == Toom3Mult_Finished) {
			ret = pro->out;
			proVecCounter--;
		}
	}
#ifdef INTEX8_TOOM3_DYNAMIC_BUF
	free(toom3_buf);
#endif

	return proVecx[0].out;
}
