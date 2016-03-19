#include <stdio.h>
#include <stdlib.h>

#include "bwt.h"

#define BLOCK_SIZE 4096

/* Testing utilities */
#include "greatest.h"
GREATEST_MAIN_DEFS();
extern SUITE(bwt_suite);
extern SUITE(mtf_suite);
extern SUITE(arith_suite);

int main(int argc, char **argv)
{
	/* Run tests if no input is given */
	if (argc < 2)
	{
		GREATEST_INIT();
		RUN_SUITE(bwt_suite);
		RUN_SUITE(mtf_suite);
		RUN_SUITE(arith_suite);
		GREATEST_PRINT_REPORT();
	}

	return EXIT_SUCCESS;
}
