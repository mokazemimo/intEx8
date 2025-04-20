#include <stdio.h>
#include <vector>
#include "intEx8.h"

std::vector< std::vector<dig_t> > read_file(const std::string &file_name)
{
    std::vector< std::vector<dig_t> > ret;

    std::fstream fin(fname);
    char str[25000];
    while (fin.getline(str, sizeof(str))) {

        if (*str == '#')
            continue;

        std::vector<dig_t> x;

        dig_t num = 0;
        for (const char *ptr = str; *ptr; ) {
            if (*ptr == ',' || *ptr == ']') {
                x.push_back(num);
                num = 0;
                ++ptr;
            }
            else if ('.' == *ptr)
                ptr += 2;
            else if ('0' <= *ptr && *ptr <= '9') {
                num = 10 * num + (*ptr - '0');
                ++ptr;
            }
            else
                ++ptr;
        }
        ret.emplace_back(x);
    }
    fin.close();

    return ret;
}

void test_mul_div_mod()
{
	std::vector<std::string> files = {
		"3-3_1000",
		"4-4_sp_1000",
		"5-5_10000",
		"10-10_10000"
	};
	printf("test multiplication/division/mod; started\n");
	for(const auto &f : files) {
		auto vec = read_file(f);
		for(int i = 0; i + 2 < vec.size(); i += 3) {
		}
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
	}
	printf("test addition/subtraction; finished\n");
}

int main()
{
	test_add_sub();
	test_mul_div_mod();

	return 0;
}


#define IX8_BINARY_OPERATION(x, y, opr)		(x ##opr y)

