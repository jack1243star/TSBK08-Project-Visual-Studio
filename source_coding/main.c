#include <stdio.h>
#include <stdlib.h>

#include "bwt.h"
#include "mtf.h"
#include "arith.h"
#include "lzw.h"

#define BLOCK_SIZE 32768

/* Testing utilities */
#include "greatest.h"
GREATEST_MAIN_DEFS();
extern SUITE(bwt_suite);
extern SUITE(mtf_suite);
extern SUITE(lzw_suite);
extern SUITE(arith_suite);

int main(int argc, char **argv)
{
	unsigned char * input;
	size_t encoded_bits;
	long input_length;
	unsigned char * encoded;
	unsigned char * decoded;
	unsigned char * output;
	size_t decoded_bytes;
	FILE* fp;
	int dictsize;

	/* Run tests if no input is given */
	if (argc < 2)
	{
		GREATEST_INIT();
		RUN_SUITE(bwt_suite);
		RUN_SUITE(mtf_suite);
		RUN_SUITE(lzw_suite);
		RUN_SUITE(arith_suite);
		GREATEST_PRINT_REPORT();

		return EXIT_SUCCESS;
	}

	/* Read input */
	fp = fopen(argv[1], "rb");
	/* Get file size */
	fseek(fp, 0L, SEEK_END);
	input_length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	printf("Input size: %d bytes\n", input_length);
	/* Allocate memory */
	input = (unsigned char *)malloc(input_length);
	decoded = (unsigned char *)malloc(input_length);
	output = (unsigned char *)malloc(input_length);
	/* Read data */
	fread(input, 1, input_length, fp);
	fclose(fp);

	/* Arithmetic encoding */
	encoded = arith_encode(input, &encoded_bits, input_length);
	printf("Arith: %u bits (%f bits/symbol)...", encoded_bits, (float)encoded_bits / input_length);
	arith_decode(encoded, decoded, input_length);
	if (0 != memcmp(input, decoded, input_length))
		printf("decode error\n");
	else
		printf("decode success\n");
	free(encoded);
	free(decoded);

	/* LZW encoding (12, 16, 20-bit dict) */
	for (dictsize = 12; dictsize <= 20; dictsize += 4)
	{
		encoded = lzw_encode(input, &encoded_bits, input_length, dictsize);
		printf("LZW(%d): %u bits (%f bits/symbol)...", dictsize, encoded_bits, (float)encoded_bits / input_length);
		decoded = lzw_decode(encoded, &decoded_bytes, input_length, dictsize);
		if (0 != memcmp(input, decoded, input_length))
			printf("decode error\n");
		else
			printf("decode success\n");

		free(encoded);
		free(decoded);
	}

	return EXIT_SUCCESS;
}
