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

SUITE(lzw_suite)
{
	/* Compress and decompress */
	RUN_TEST(lzw_encode_decode_alice1);
}
