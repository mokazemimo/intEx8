/*
 *  File: util.c
 *  Description:
 *      Provides utility functions required by intEx8 library.
 */

#include "util.h"

/*
 * Computes the number of `dig_t` digits required to store sum of two big integers.
 */
cntx_t _required_digits_for_sum(intx_t x, intx_t y)
{
	cntx_t x_size = _abs(x.size), y_size = _abs(y.size);
	if (x_size == 0 || y_size == 0) {
		return _max(x_size, y_size);
	}
	else if (_sgn(x.size) == _sgn(y.size)) {
#ifdef INTEX8_FAVOR_SPEED
		{
			if (x_size > y_size) {
				return x_size + ((x.ptr[x_size - 1] == INTEX8_DIGIT_MAX_VALUE) ? 1 : 0);
			}
			else if (y_size > x_size) {
				return y_size + ((y.ptr[y_size - 1] == INTEX8_DIGIT_MAX_VALUE) ? 1 : 0);
			}
			else {
				return x_size + (((uint64_t)x.ptr[x_size - 1] + y.ptr[y_size - 1] >= INTEX8_DIGIT_MAX_VALUE) ? 1 : 0);
			}
		}
#else // FAVOR SPACE
		{
			if (x_size >= y_size) {
				cntx_t i = x_size;
				for (; i >= y_size && x.ptr[i] == INTEX8_DIGIT_MAX_VALUE; --i)
					;
				for (; i >= 0 && x.ptr[i] + y.ptr[i] == INTEX8_DIGIT_MAX_VALUE; --i)
					;
				return x_size + (i < 0 || x.ptr[i] + y.ptr[i] < INTEX8_DIGIT_MAX_VALUE) ? 0 : 1;
			}
			else {
				cntx_t i = y_size;
				for (; i >= x_size && y.ptr[i] == INTEX8_DIGIT_MAX_VALUE; --i)
					;
				for (; i >= 0 && x.ptr[i] + y.ptr[i] == INTEX8_DIGIT_MAX_VALUE; --i)
					;
				return y_size + (i < 0 || x.ptr[i] + y.ptr[i] < INTEX8_DIGIT_MAX_VALUE) ? 0 : 1;
			}
		}
#endif
	}
	else {
		return _max(x_size, y_size);
	}
}

