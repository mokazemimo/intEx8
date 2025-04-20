#include <stdio.h>
#include "intEx8.h"

intx_t *read_file(const char *file_name, int *count)
{
	FILE *fp = fopen(file_name);
	if(fp == NULL) {
		printf("Couldn't open the file: %s", file_name);
		return NULL;
	}
	*count = 0;
	fclose(fp);

	return NULL;
}

void test_mul_div_mod()
{
	const char *files[] = {
		"3-3_1000",
		"4-4_sp_1000",
		"5-5_10000",
		"10-10_10000",
		""
	};
	printf("test multiplication/division/mod; started\n");
	for(char *f = files; *f != '\0'; ++f) {
		int n;
		intx_t *arr = read_file(f, &n);
		for(int i = 0; i + 2 < n; i += 3) {
			intx_t x0 = arr[i];
			intx_t x1 = arr[i + 1];
			intx_t x2 = arr[i + 2];
			
			// x0 * x1 == x2
			intx_t m = ix8_x_mul_x(x0, x1);
			assert(ix8_is_equal(m, x2));
			ix8_free(m);
			
			// x1 * x0 == x2
			m = ix8_x_mul_x(x1, x0);
			assert(ix8_is_equal(m, x2));
			ix8_free(m);

			// x2 / x0 == x1
			m = ix8_x_div_x(x2, x0);
			if(ix8_is_zero(x0))
				assert(intEx8_errno == INTEX8_ERR_DIVISION_BY_ZERO);
			else if(intEx8_errno == INTEX8_OK)
				assert(ix8_is_equal(m, x1));
			ix8_free(m);

			// x2 / x1 == x0
			m = ix8_x_div_x(x2, x1);
			if(ix8_is_zero(x1))
				assert(intEx8_errno == INTEX8_ERR_DIVISION_BY_ZERO);
			else if(intEx8_errno == INTEX8_OK)
				assert(ix8_is_equal(m, x0));
			ix8_free(m);

			// x2 % x0 == 0
			m = ix8_x_mod_x(x2, x0);
			if(ix8_is_zero(x0))
				assert(intEx8_errno == INTEX8_ERR_DIVISION_BY_ZERO);
			else if(intEx8_errno == INTEX8_OK)
				assert(ix8_is_zero(m));
			ix8_free(m);

			// x2 % (-x0) == 0
			intx_t nx = ix8_negate(x0);
			m = ix8_x_mod_x(x2, nx);
			if(ix8_is_zero(nx))
				assert(intEx8_errno == INTEX8_ERR_DIVISION_BY_ZERO);
			else if(intEx8_errno == INTEX8_OK)
				assert(ix8_is_zero(m));
			ix8_free(m);
			ix8_free(nx);

			// x2 % x1 == 0
			m = ix8_x_mod_x(x2, x1);
			if(ix8_is_zero(x1))
				assert(intEx8_errno == INTEX8_ERR_DIVISION_BY_ZERO);
			else if(intEx8_errno == INTEX8_OK)
				assert(ix8_is_zero(m));
			ix8_free(m);

			ix8_free(x0);
			ix8_free(x1);
			ix8_free(x2);
		}
		free(arr);
	}
	printf("test multiplication/division/mod; finished\n");
}

void test_add_sub()
{
	const char *files[] = {
		"20-20_10000",
		"100-100_cnt_10000",
		""
	};
	printf("test addition/subtraction; started\n");
	for(char *f = files; *f != '\0'; ++f) {
		int n;
		intx_t *arr = read_file(f, &n);
		for(int i = 0; i + 2 < n; ++i) {
			intx_t x0 = arr[i];
			intx_t x1 = arr[i + 1];
			intx_t x2 = arr[i + 2];
			
			// x0 + x1 == x2
			intx_t y = ix8_x_add_x(x0, x1);
			assert(ix8_is_equal(y, x2));
			ix8_free(y);

			// x1 + x0 == x2
			y = ix8_x_add_x(x1, x0);
			assert(ix8_is_equal(y, x2));
			ix8_free(y);

			// x2 - x0 == x1
			intx_t y = ix8_x_sub_x(x2, x0);
			assert(ix8_is_equal(y, x1));
			ix8_free(y);

			// x2 - x1 == x0
			intx_t y = ix8_x_sub_x(x2, x1);
			assert(ix8_is_equal(y, x0));
			ix8_free(y);
		}
	}
	printf("test addition/subtraction; finished\n");
}

int main()
{
	test_add_sub();
	test_mul_div_mod();

	return 0;
}