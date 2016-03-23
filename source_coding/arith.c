#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "arith.h"

#define N (1<<12)
#define K 2

#define DEBUG_I 13031

/* Write a n-bit codeword to output buffer, sizes are updated */
static unsigned char * pack(
	unsigned char * output,
	size_t * output_mem_size,
	size_t * output_bit_size,
	size_t codeword,
	size_t bits_to_pack)
{
	size_t i;

	/* Check if there is enough space in output buffer */
	if (*output_bit_size + bits_to_pack > *output_mem_size * 8)
	{
		/* Allocate more memory */
		output = (unsigned char *)realloc(output, *output_mem_size == 0 ? 256 : *output_mem_size * 2);
		if (output == NULL)
		{
			printf("Error expanding output buffer in pack()\n");
			exit(EXIT_FAILURE);
		}
		/* Fill with zeros */
		memset(output + *output_mem_size, 0, *output_mem_size == 0 ? 256 : *output_mem_size);
		/* Update memory size */
		*output_mem_size = *output_mem_size == 0 ? 256 : *output_mem_size * 2;
	}
	/* Pack each bit */
	for (i = 0; i < bits_to_pack; i++)
	{
		output[*output_bit_size / 8] |= (0x1 & (codeword >> (bits_to_pack - 1 - i))) << (*output_bit_size % 8);
		(*output_bit_size)++;
	}

	return output;
}

/* Read a n-bit codeword from input buffer */
static size_t unpack(unsigned char * input, size_t start_bit, size_t bits_per_codeword)
{
	size_t i, x;

	x = 0;
	for (i = 0; i < bits_per_codeword; i++)
	{
		x |= (0x1 & (input[(start_bit + i) / 8] >> ((start_bit + i) % 8))) << (bits_per_codeword - 1 - i);
	}

	return x;
}

unsigned char * arith_encode(unsigned char * input, size_t * output_bit_num, size_t length)
{
	/* Loop counters */
	size_t i, j;
	/* Cumulative counters for Markov model, entries stored as `F[previous][current]` */
	uint32_t F[256][257];
	/* Counters used when rescaling */
	uint32_t C[256];
	/* Lower and upper bounds of probabilities*/
	uint32_t u, l, old_l;
	uint64_t u_minus_l;
	/* Numbers of case 3 shifts */
	size_t shifts = 0;
	/* Previous symbol (assume 0 for the first one) */
	unsigned char prev = 0;

	/* Output buffer */
	unsigned char * output = NULL;
	/* Output memory size in bytes */
	size_t output_mem_size = 0;

	*output_bit_num = 0;

	/* Initialize counters */
	for (i = 0; i < 256; i++)
		for (j = 0; j <= 256; j++)
			F[i][j] = j;

	/* Intialize bounds */
	l = 0x00000000;
	u = 0xFFFFFFFF;

	/* Iteratively update bounds */
	for (i = 0; i < length; i++)
	{
		/* Update bounds */
		u_minus_l = u - l;
		old_l = l;
		l = (uint32_t)(old_l + ((u_minus_l + 1)*F[prev][input[i]]) / F[prev][256]);
		u = (uint32_t)(old_l + ((u_minus_l + 1)*F[prev][input[i] + 1]) / F[prev][256] - 1);

		/* Update model */
		for (j = input[i] + 1; j <= 256; j++)
			F[prev][j]++;
		/* Rescale */
		if (F[prev][256] > N)
		{
			for (j = 0; j < 256; j++)
			{
				C[j] = F[prev][j + 1] - F[prev][j];
				C[j] = (C[j] + K - 1) / K;
			}
			F[prev][0] = 0;
			for (j = 1; j <= 256; j++)
			{
				F[prev][j] = F[prev][j - 1] + C[j - 1];
			}
		}

		/* Check for shifts */
	check:
		if (((l >> 31) & 0x1) == 0x1 && ((u >> 31) & 0x1) == 0x1)
		{
			/* Shift 1 into output */
			output = pack(output, &output_mem_size, output_bit_num, 0x1, 1);
			/* Shift 0 into l and 1 into u */
			l = l << 1;
			l &= 0xFFFFFFFE;
			u = u << 1;
			u |= 0x1;

			while (shifts > 0)
			{
				/* Shift all 0 from previous case 3 shifts */
				output = pack(output, &output_mem_size, output_bit_num, 0x0, 1);
				shifts--;
			}

			goto check;
		}
		else if (((l >> 31) & 0x1) == 0x0 && ((u >> 31) & 0x1) == 0x0)
		{
			/* Shift 0 into output */
			output = pack(output, &output_mem_size, output_bit_num, 0x0, 1);
			/* Shift 0 into l and 1 into u */
			l = l << 1;
			l &= 0xFFFFFFFE;
			u = u << 1;
			u |= 0x1;

			while (shifts > 0)
			{
				/* Shift all 1 from previous case 3 shifts */
				output = pack(output, &output_mem_size, output_bit_num, 0x1, 1);
				shifts--;
			}

			goto check;
		}
		else if(((l >> 30) & 0b11) == 0b01 && ((u >> 30) & 0b11) == 0b10)
		{
			while (((l >> 30) & 0b11) == 0b01 && ((u >> 30) & 0b11) == 0b10)
			{
				/* Case 3 shift */
				shifts++;
				l = l << 1;
				l &= 0xFFFFFFFE;
				u = u << 1;
				u |= 0x1;
				/* Invert MSB */
				l = l ^ 0x80000000;
				u = u ^ 0x80000000;
			}
			goto check;
		}

		/* Record previous symbol */
		prev = input[i];
	}
	/* Append l */
	output = pack(output, &output_mem_size, output_bit_num, l>>31, 1);
	if (((l >> 31) & 0b1) == 0b1)
	{
		while (shifts > 0)
		{
			/* Shift all 0 from previous case 3 shifts */
			output = pack(output, &output_mem_size, output_bit_num, 0x0, 1);
			shifts--;
		}
	}
	else if(((l >> 31) & 0b1) == 0b0)
	{
		while (shifts > 0)
		{
			/* Shift all 1 from previous case 3 shifts */
			output = pack(output, &output_mem_size, output_bit_num, 0x1, 1);
			shifts--;
		}
	}
	output = pack(output, &output_mem_size, output_bit_num, l, 31);

	return output;
}

void arith_decode(unsigned char * input, unsigned char * output, size_t length)
{
	/* Loop counters */
	size_t i, j;
	/* Cumulative counters for Markov model, entries stored as `F[previous][current]` */
	uint32_t F[256][257];
	/* Counters used when rescaling */
	uint32_t C[256];
	/* Lower and upper bounds of probabilities*/
	uint32_t u, l, old_l;
	uint64_t u_minus_l;
	/* Current tag */
	uint32_t t;
	uint64_t t_minus_l;
	/* Number calculated from tag */
	uint64_t num;
	/* Previous symbol (assume 0 for the first one) */
	unsigned char prev = 0;

	/* Current bit in input */
	size_t input_bit_pos = 0;

	/* Initialize counters */
	for (i = 0; i < 256; i++)
		for (j = 0; j <= 256; j++)
			F[i][j] = j;

	/* Intialize bounds */
	l = 0x00000000;
	u = 0xFFFFFFFF;
	/* Read initial 32-bit tag */
	t = unpack(input, input_bit_pos, 32);
	input_bit_pos += 32;

	/* Iteratively update bounds */
	for (i = 0; i < length; i++)
	{
		/* Decode symbol */
		u_minus_l = u - l;
		t_minus_l = t - l;
		num = ((t_minus_l + 1)*F[prev][256] - 1) / (u_minus_l + 1);
		for (j = 1; F[prev][j] <= num; j++)
			;
		output[i] = (unsigned char)(0xFF & (j - 1));

		/* Update bounds */
		old_l = l;
		l = (uint32_t)(old_l + ((u_minus_l + 1)*F[prev][output[i]]) / F[prev][256]);
		u = (uint32_t)(old_l + ((u_minus_l + 1)*F[prev][output[i] + 1]) / F[prev][256] - 1);

		/* Update model */
		for (j = output[i] + 1; j <= 256; j++)
			F[prev][j]++;
		/* Rescale */
		if (F[prev][256] > N)
		{
			for (j = 0; j < 256; j++)
			{
				C[j] = F[prev][j + 1] - F[prev][j];
				C[j] = (C[j] + K - 1) / K;
			}
			F[prev][0] = 0;
			for (j = 1; j <= 256; j++)
			{
				F[prev][j] = F[prev][j - 1] + C[j - 1];
			}
		}

		/* Check for shifts */
	check:
		if (((l >> 31) & 0x1) == 0x1 && ((u >> 31) & 0x1) == 0x1)
		{
			/* Shift 0 into l and 1 into u */
			l = l << 1;
			l &= 0xFFFFFFFE;
			u = u << 1;
			u |= 0x1;
			/* Shift a bit into t */
			t = (t << 1) | (0x1 & unpack(input, input_bit_pos, 1));
			input_bit_pos++;

			goto check;
		}
		else if (((l >> 31) & 0x1) == 0x0 && ((u >> 31) & 0x1) == 0x0)
		{
			/* Shift 0 into l and 1 into u */
			l = l << 1;
			l &= 0xFFFFFFFE;
			u = u << 1;
			u |= 0x1;
			/* Shift a bit into t */
			t = (t << 1) | (0x1 & unpack(input, input_bit_pos, 1));
			input_bit_pos++;

			goto check;
		}
		else if (((l >> 30) & 0b11) == 0b01 && ((u >> 30) & 0b11) == 0b10)
		{
			while (((l >> 30) & 0b11) == 0b01 && ((u >> 30) & 0b11) == 0b10)
			{
				/* Case 3 */
				l = l << 1;
				l &= 0xFFFFFFFE;
				u = u << 1;
				u |= 0x1;
				/* Shift a bit into t */
				t = (t << 1) | (0x1 & unpack(input, input_bit_pos, 1));
				input_bit_pos++;
				/* Invert MSB */
				l = l ^ 0x80000000;
				u = u ^ 0x80000000;
				t = t ^ 0x80000000;
			}
			goto check;
		}

		/* Record previous symbol */
		prev = output[i];
	}
}
