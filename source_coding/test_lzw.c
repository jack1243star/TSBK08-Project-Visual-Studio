#include "greatest.h"

#include <string.h>
#include "lzw.h"

TEST lzw_encode_decode_alice1()
{
	char * input = "Alice was beginning to get very tired of sitting by her sister\n\
on the bank, and of having nothing to do:  once or twice she had\n\
peeped into the book her sister was reading, but it had no\n\
pictures or conversations in it, `and what is the use of a book, '\n\
thought Alice `without pictures or conversation ? '";
	size_t encoded_bits;
	unsigned char * encoded;
	size_t decoded_bytes;
	unsigned char * decoded;
	size_t length = strlen(input);
	encoded = lzw_encode((unsigned char *)input, &encoded_bits, length, 16);
	decoded = lzw_decode(encoded, &decoded_bytes, length, 16);

	ASSERT_EQ(length, decoded_bytes);
	ASSERT_EQ(0, memcmp(input, decoded, decoded_bytes));
	PASS();
}

TEST lzw_encode_decode_sum()
{
	size_t i;
	unsigned char input[38420];
	size_t encoded_bits;
	unsigned char * encoded;
	unsigned char * decoded;
	size_t decoded_bytes;
	FILE* fp = fopen("cantrbry/sum", "rb");
	fread(input, 1, 38420, fp);
	fclose(fp);

	encoded = lzw_encode(input, &encoded_bits, 38420, 16);
	decoded = lzw_decode(encoded, &decoded_bytes, 38420, 16);

	ASSERT_EQ(38420, decoded_bytes);
	ASSERT_EQ(0, memcmp(input,  decoded, 38420));
	PASS();
}

TEST lzw_encode_decode_kennedy()
{
	unsigned char * input;
	size_t encoded_bits;
	unsigned char * encoded;
	unsigned char * decoded;
	size_t decoded_bytes;

	input = (unsigned char *)malloc(1029744);

	FILE* fp = fopen("cantrbry/kennedy.xls", "rb");
	fread(input, 1, 1029744, fp);
	fclose(fp);

	encoded = lzw_encode(input, &encoded_bits, 1029744, 16);
	decoded = lzw_decode(encoded, &decoded_bytes, 1029744, 16);

	ASSERT_EQ(1029744, decoded_bytes);
	ASSERT_EQ(0, memcmp(input, decoded, 1029744));
	PASS();
}

SUITE(lzw_suite)
{
	/* Compress and decompress */
	RUN_TEST(lzw_encode_decode_alice1);
	RUN_TEST(lzw_encode_decode_sum);
	RUN_TEST(lzw_encode_decode_kennedy);
}
