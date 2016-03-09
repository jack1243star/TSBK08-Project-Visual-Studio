#include <stdlib.h>

#include "mtf.h"

void mtf(unsigned char * input, unsigned char * output, size_t length)
{
	/* Loop counter */
	size_t i, j;
	/* Position of byte in list */
	unsigned char position[256];
	/* Current input byte */
	unsigned char b;

	/* Initialize list */
	for (i = 0; i < 256; i++)
	{
		position[i] = i & 0xFF;
	}
	/* Go through input */
	for (i = 0; i < length; i++)
	{
		/* Read one byte */
		b = input[i];
		/* Output the byte's position */
		output[i] = position[b];
		/* Move the byte to the front */
		for (j = 0; j < 256; j++)
		{
			if (position[j] < position[b])
				position[j]++;
		}
		position[b] = 0;
	}

	return;
}

void imtf(unsigned char * input, unsigned char * output, size_t length)
{
	/* Loop counter */
	size_t i, j;
	/* Position of byte in list */
	unsigned char position[256];
	/* Current input position */
	unsigned char p;

	/* Initialize list */
	for (i = 0; i < 256; i++)
	{
		position[i] = i & 0xFF;
	}
	/* Go through input */
	for (i = 0; i < length; i++)
	{
		/* Read one position */
		p = input[i];
		/* Output the byte at that position */
		for (j = 0; j < 256; j++)
		{
			if (position[j] == p)
				break;
		}
		output[i] = j & 0xFF;
		/* Move the byte to the front */
		for (j = 0; j < 256; j++)
		{
			if (position[j] < position[output[i]])
				position[j]++;
		}
		position[output[i]] = 0;
	}

	return;
}
