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
	unsigned char input[38420];
	size_t encoded_bits;
	unsigned char * encoded;
	unsigned char decoded[38420];
	FILE* fp = fopen("cantrbry/sum", "rb");
	fread(input, 1, 38420, fp);
	fclose(fp);

	encoded = arith_encode(input, &encoded_bits, 38420);
	arith_decode(encoded, decoded, 38420);

	ASSERT_EQ(0, memcmp(input, decoded, 38420));
	PASS();
}

TEST arith_encode_decode_kennedy()
{
	unsigned char * input;
	size_t encoded_bits;
	unsigned char * encoded;
	unsigned char * decoded;

	input = (unsigned char *)malloc(1029744);
	decoded = (unsigned char *)malloc(1029744);

	FILE* fp = fopen("cantrbry/kennedy.xls", "rb");
	fread(input, 1, 1029744, fp);
	fclose(fp);

	encoded = arith_encode(input, &encoded_bits, 1029744);
	arith_decode(encoded, decoded, 1029744);

	ASSERT_EQ(0, memcmp(input, decoded, 1029744));
	PASS();
}

SUITE(arith_suite)
{
	/* Compress and decompress */
	RUN_TEST(arith_encode_decode_alice1);
	RUN_TEST(arith_encode_decode_sum);
	RUN_TEST(arith_encode_decode_kennedy);
}
