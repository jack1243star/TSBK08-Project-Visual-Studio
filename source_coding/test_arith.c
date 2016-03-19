#include "greatest.h"

#include <string.h>
#include "arith.h"

#include <stdio.h>

TEST arith_encode_decode_alice1()
{
	char * input = "Alice was beginning to get very tired of sitting by her sister\n\
on the bank, and of having nothing to do:  once or twice she had\n\
peeped into the book her sister was reading, but it had no\n\
pictures or conversations in it, `and what is the use of a book, '\n\
thought Alice `without pictures or conversation ? '";
	size_t encoded_bits;
	unsigned char * encoded;
	unsigned char decoded[306];

	encoded = arith_encode(input, &encoded_bits, 305);
	arith_decode(encoded, decoded, 305);
	decoded[305] = '\0';

	ASSERT_STR_EQ(input, decoded);
	PASS();
}

TEST arith_encode_decode_sum()
{
	size_t i;
	unsigned char input[38420];
	size_t encoded_bits;
	unsigned char * encoded;
	unsigned char decoded[38420];
	FILE* fp = fopen("cantrbry/sum", "rb");
	fread(input, 1, 38420, fp);
	fclose(fp);

	encoded = arith_encode(input, &encoded_bits, 38420);
	arith_decode(encoded, decoded, 38420);

	for (i = 0; i < 38420; i++)
	{
		printf("Asserting input[%u] = 0x%x\n", i, input[i]);
		ASSERT_EQ(input[i], decoded[i]);
	}
	PASS();
}

SUITE(arith_suite)
{
	/* Compress and decompress */
	RUN_TEST(arith_encode_decode_alice1);
	RUN_TEST(arith_encode_decode_sum);
}
