#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bwt.h"

#define MAX_BLOCK_SIZE 4096

/* Buffer containing the block to be transformed*/
static unsigned char *buffer;
/* Size of the block */
static size_t block_size;

/* Wrapped memcmp for use in BWT */
static int wrap_cmp(const void *p1, const void *p2)
{
	size_t i;
	size_t i1 = *(size_t *)p1;
	size_t i2 = *(size_t *)p2;

	/* Compare */
	for (i = 0; i < block_size; i++)
	{
		if (buffer[(i1 + i) % block_size] > buffer[(i2 + i) % block_size])
			return 1;
		else if (buffer[(i1 + i) % block_size] < buffer[(i2 + i) % block_size])
			return -1;
	}
	return i2 - i1;
}

/* Burrows-Wheeler Transform */
void bwt(unsigned char *input, unsigned char *output, size_t *index, size_t length)
{
	size_t i;
	size_t indices[MAX_BLOCK_SIZE];

	/* Set input buffer */
	buffer = input;
	/* Set block size */
	block_size = length;

	/* Fill the array of indices*/
	for (i = 0; i < length; i++)
	{
		indices[i] = i;
	}
	/* Sort the shifted strings */
	qsort(indices, length, sizeof(size_t), wrap_cmp);
	/* Output the transformed string and the index of the original string */
	for (i = 0; i < length; i++)
	{
		output[i] = buffer[(indices[i]+length-1) % block_size];
		if (indices[i] == 0)
			*index = i;
	}

	return;
}

/* Inverse Burrows-Wheeler Transform */
void ibwt(unsigned char *input, unsigned char *output, size_t index, size_t length)
{
	size_t i, j, k;
	unsigned char c;
	size_t occurrence[0xFF] = { 0 };
	size_t T[MAX_BLOCK_SIZE];
	unsigned char *L = input;

	/* Create T by first counting occurrence and inserting positions */
	/* Count occurrences */
	for (i = 0; i < length; i++)
	{
		occurrence[L[i]]++;
	}
	/* Insert position of each alphabet */
	j = 0;
	for (c = 0; c < 0xFF; c++)
	{
		for (i = 0; occurrence[c] > 0; occurrence[c]--)
		{
			/* Find position of c in L*/
			while (L[i] != c)
			{
				i++;
				if (i >= length)
				{
					printf("\nBug in iBWT, index out of bound! c = %u, i = %u, j = %u\n", c, i, j);
					exit(EXIT_FAILURE);
				}
			}
			/* Insert this position into T */
			T[j] = i;
			j++;
			/* Skip this occurrence */
			i++;
		}
	}

	/* Decode using the method shown on the slides */
	k = T[index];
	output[0] = L[k];
	for (j = 1; j < length; j++)
	{
		k = T[k];
		output[j] = L[k];
	}
}
