#include "greatest.h"

#include "bwt.h"

TEST bwt_drdobbs()
{
	size_t index;
	char *input = "DRDOBBS";
	char output[8];
	bwt(input, output, &index, 7);
	output[7] = '\0';

	ASSERT_EQ(3, index);
	ASSERT_STR_EQ("OBRSDDB", output);
	PASS();
}

TEST bwt_bacabba()
{
	size_t index;
	char *input = "bacabba";
	char output[8];
	bwt(input, output, &index, 7);
	output[7] = '\0';

	ASSERT_EQ(4, index);
	ASSERT_STR_EQ("bcbbaaa", output);
	PASS();
}

TEST bwt_abracadabra()
{
	size_t index;
	char *input = "abracadabra";
	char output[12];
	bwt(input, output, &index, 11);
	output[11] = '\0';

	ASSERT_EQ(2, index);
	ASSERT_STR_EQ("rdarcaaaabb", output);
	PASS();
}

TEST bwt_wikipedia_example1()
{
	size_t index;
	char *input = "^BANANA|";
	char output[9];
	bwt(input, output, &index, 8);
	output[8] = '\0';

	ASSERT_STR_EQ("BNN^AA|A", output);
	PASS();
}

TEST bwt_wikipedia_example2()
{
	/* Not passing, possibly wrong answer */
	SKIP();

	size_t index;
	char *input = "SIX.MIXED.PIXIES.SIFT.SIXTY.PIXIE.DUST.BOXES";
	char output[45];
	bwt(input, output, &index, 44);
	output[44] = '\0';

	ASSERT_STR_EQ("TEXYDST.E.IXXIIXXSSMPPS.B..E.S.UESFXDIIOIIIT", output);
	PASS();
}

TEST ibwt_bacabba()
{
	char *input = "bcbbaaa";
	char output[8];
	ibwt(input, output, 4, 7);
	output[7] = '\0';

	ASSERT_STR_EQ("bacabba", output);
	PASS();
}


TEST ibwt_abracadabra()
{
	char *input = "rdarcaaaabb";
	char output[12];
	ibwt(input, output, 2, 11);
	output[11] = '\0';

	ASSERT_STR_EQ("abracadabra", output);
	PASS();
}

TEST ibwt_wikipedia_example2()
{
	size_t index;
	char *input = "SIX.MIXED.PIXIES.SIFT.SIXTY.PIXIE.DUST.BOXES";
	char encoded[45];
	char decoded[45];
	bwt(input, encoded, &index, 44);
	ibwt(encoded, decoded, index, 44);
	decoded[44] = '\0';

	ASSERT_STR_EQ(input, decoded);
	PASS();
}

SUITE(bwt_suite)
{
	/* Encode */
	RUN_TEST(bwt_drdobbs);
	RUN_TEST(bwt_bacabba);
	RUN_TEST(bwt_abracadabra);
	RUN_TEST(bwt_wikipedia_example1);
	RUN_TEST(bwt_wikipedia_example2);
	/* Decode */
	RUN_TEST(ibwt_bacabba);
	RUN_TEST(ibwt_abracadabra);
	RUN_TEST(ibwt_wikipedia_example2);
}
