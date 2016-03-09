#include "greatest.h"

#include "mtf.h"

TEST mtf_bananaaa()
{
	char * input = "bananaaa";
	unsigned char output[8];
	unsigned char answer[8]
		= { '\x62', '\x62', '\x6E', '\x01', '\x01', '\x01' ,'\0', '\0' };
	mtf((unsigned char *)input, output, 8);

	ASSERT_EQ(0, memcmp(answer, output, 8));
	PASS();
}

TEST mtf_imtf_wikipedia_example2()
{
	char * input = "SIX.MIXED.PIXIES.SIFT.SIXTY.PIXIE.DUST.BOXES";
	unsigned char encoded[45];
	unsigned char decoded[45];
	mtf((unsigned char *)input, encoded, 44);
	imtf(encoded, decoded, 44);
	decoded[44] = '\0';

	ASSERT_STR_EQ(input, (char *)decoded);
	PASS();
}

SUITE(mtf_suite)
{
	/* Encode */
	RUN_TEST(mtf_bananaaa);
	/* Encode and decode */
	RUN_TEST(mtf_imtf_wikipedia_example2);
}
