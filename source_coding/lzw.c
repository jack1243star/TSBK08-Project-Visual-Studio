#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lzw.h"

#define DICT_MEM_INIT_SIZE 512

/* A codeword in the dictionary */
struct DictWord
{
	unsigned char * mem;
	size_t len;
};

/* Match a codeword to input, returns codeword length if matched or zero if not match */
static size_t match(
	struct DictWord * entry,
	unsigned char * input,
	size_t start, size_t max)
{
	size_t i;

	for (i = 0; i < max - start; i++)
	{
		/* Abort if mismatch */
		if (entry->mem[i] != input[start + i])
			return 0;

		/* Check if reaching end of dictionary word */
		if (i == entry->len - 1)
			return entry->len;
	}

	/* Matched all the way to end of input. Is this possible? */
	return 0;
}

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
		output[*output_bit_size / 8] |= (0x1 & (codeword >> i)) << (*output_bit_size % 8);
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
		x |= (0x1 & (input[(start_bit + i) / 8] >> ((start_bit + i) % 8))) << i;
	}

	return x;
}

unsigned char * lzw_encode(unsigned char * input, size_t * output_bit_num, size_t length, size_t max_bits_per_codeword)
{
	/* Loop counter */
	size_t i, j;

	/* Dictionary */
	struct DictWord * dict = NULL;
	/* Dictionary size */
	size_t dict_size;
	/* Number of dicitonary entries */
	size_t dict_num, dict_num_next_pow_of_2, dict_num_next_exp_of_2;

	/* Dictionary memory */
	unsigned char * dict_mem = NULL;
	/* Byte size of total dictionary memory */
	size_t dict_mem_size;
	/* Bytes used in dictionary memory */
	size_t dict_mem_used;

	/* Match length */
	size_t match_len, current_match_len;
	/* Matched dict entry */
	size_t matched_entry;
	/* Last matched dict entry */
	size_t last_matched_entry;

	/* Output buffer */
	unsigned char * output = NULL;
	/* Output memory size in bytes */
	size_t output_mem_size = 0;

	if (max_bits_per_codeword < 9)
	{
		printf("Error invoking LZW compression: bits per codeword must be at least 9\n");
		exit(EXIT_FAILURE);
	}

	/* Calculate dictionary size */
	dict_size = (size_t)1 << max_bits_per_codeword;
	/* Initiailze dictionary */
	dict = (struct DictWord *)malloc(dict_size * sizeof(struct DictWord));
	/* Initialize dictionary memory */
	dict_mem = (unsigned char *)malloc(DICT_MEM_INIT_SIZE * sizeof(unsigned char));
	dict_mem_size = DICT_MEM_INIT_SIZE;
	/* Ensure malloc success */
	if (dict == NULL || dict_mem == NULL)
	{
		printf("Error creating dictionary for LZW compression\n");
		exit(EXIT_FAILURE);
	}
	/* Create default entries in dictionary */
	for (i = 0; i < 256; i++)
	{
		/* Point default entries onto memory */
		dict[i].mem = dict_mem + i;
		dict[i].len = 1;
		/* Fill default entries into memory */
		dict_mem[i] = i & 0xFF;
	}
	dict_num = 256;
	dict_mem_used = 256;

	/* Encode the first symbol */
	last_matched_entry = input[0];
	*output_bit_num = 0;
	output = pack(output, &output_mem_size, output_bit_num, last_matched_entry, 8);
	dict_num_next_exp_of_2 = 8;
	dict_num_next_pow_of_2 = 256;
	/* Read each input */
	i = 1;
	while (i < length)
	{
		/* Default to encoding only one symbol */
		matched_entry = input[i];
		match_len = 1;
		/* Find longest match in dictionary */
		for (j = 256; j < dict_num; j++)
		{
			current_match_len = match(dict+j, input, i, length);
			if (current_match_len > match_len)
			{
				match_len = current_match_len;
				matched_entry = j;
			}
		}
		/* Encode the message with log2(dict_num) bits */
		output = pack(output, &output_mem_size, output_bit_num, matched_entry, dict_num_next_exp_of_2);
		/* Move to next input */
		i += match_len;

		/* Ensure dictionary still has space */
		if (dict_num == dict_size)
		{
			/* Dictionary is full, clear it */
			dict_num = 256;
			dict_mem_used = 256;
			dict_num_next_exp_of_2 = 8;
			dict_num_next_pow_of_2 = 256;
		}
		/* Add the last match + current match to the dictionary */
		if (dict_mem_used + dict[last_matched_entry].len + dict[matched_entry].len > dict_mem_size)
		{
			/* Need more memory for dictionary words */
			dict_mem = (unsigned char *)realloc(dict_mem, 2 * dict_mem_size);
			if (dict_mem == NULL)
			{
				printf("Error expanding dict_mem\n");
				exit(EXIT_FAILURE);
			}
			dict_mem_size *= 2;
		}
		/* Add entry */
		dict[dict_num].len = dict[last_matched_entry].len + dict[matched_entry].len;
		dict[dict_num].mem = dict_mem + dict_mem_used;
		if (dict_num == dict_num_next_pow_of_2)
		{
			dict_num_next_exp_of_2++;
			dict_num_next_pow_of_2 *= 2;
		}
		dict_num++;
		/* Copy contents */
		memcpy(dict_mem + dict_mem_used, dict[last_matched_entry].mem, dict[last_matched_entry].len);
		dict_mem_used += dict[last_matched_entry].len;
		memcpy(dict_mem + dict_mem_used, dict[matched_entry].mem, dict[matched_entry].len);
		dict_mem_used += dict[matched_entry].len;

		/* Record this match */
		last_matched_entry = matched_entry;
	}

	return output;
}

unsigned char * lzw_decode(unsigned char * input, size_t * output_byte_num, size_t length, size_t max_bits_per_codeword)
{
	/* Loop counter */
	size_t i;

	/* Dictionary */
	struct DictWord * dict = NULL;
	/* Dictionary size */
	size_t dict_size;
	/* Number of dicitonary entries */
	size_t dict_num, dict_num_next_pow_of_2, dict_num_next_exp_of_2;

	/* Dictionary memory */
	unsigned char * dict_mem = NULL;
	/* Byte size of total dictionary memory */
	size_t dict_mem_size;
	/* Bytes used in dictionary memory */
	size_t dict_mem_used;

	/* Matched dict entry */
	size_t matched_entry;
	/* Last matched dict entry */
	size_t last_matched_entry;

	/* Current bit in input */
	size_t input_bit_pos = 0;
	/* Output buffer */
	unsigned char * output = NULL;
	/* Output memory size in bytes */
	size_t output_mem_size = 0;

	/* Calculate dictionary size */
	dict_size = (size_t)1 << max_bits_per_codeword;
	/* Initiailze dictionary */
	dict = (struct DictWord *)malloc(dict_size * sizeof(struct DictWord));
	/* Initialize dictionary memory */
	dict_mem = (unsigned char *)malloc(DICT_MEM_INIT_SIZE * sizeof(unsigned char));
	dict_mem_size = DICT_MEM_INIT_SIZE;
	/* Ensure malloc success */
	if (dict == NULL || dict_mem == NULL)
	{
		printf("Error creating dictionary for LZW decompression\n");
		exit(EXIT_FAILURE);
	}
	/* Create default entries in dictionary */
	for (i = 0; i < 256; i++)
	{
		/* Point default entries onto memory */
		dict[i].mem = dict_mem + i;
		dict[i].len = 1;
		/* Fill default entries into memory */
		dict_mem[i] = i & 0xFF;
	}
	dict_num = 256;
	dict_mem_used = 256;

	/* Allocate memory for output */
	output = (unsigned char *)malloc(256 * sizeof(unsigned char *));
	if (output == NULL)
	{
		printf("Error allocating memory for output in lzw_decode\n");
		exit(EXIT_FAILURE);
	}
	output_mem_size = 256;

	/* Decode the first symbol (always a byte) */
	last_matched_entry = input[0];
	output[0] = 0xFF & unpack(input, input_bit_pos, 8);
	input_bit_pos += 8;
	*output_byte_num = 1;
	dict_num_next_exp_of_2 = 8;
	dict_num_next_pow_of_2 = 256;
	/* Read each input */
	i = 1;
	while (i < length)
	{
		/* Read log2(dict_num) bits and decode the message */
		matched_entry = unpack(input, input_bit_pos, dict_num_next_exp_of_2);
		/* Ensure big enough output buffer */
		if (dict[matched_entry].len + *output_byte_num > output_mem_size)
		{
			output = (unsigned char *)realloc(output, 2 * output_mem_size * sizeof(unsigned char));
			if (output == NULL)
			{
				printf("Error expanding memory for output in lzw_decode\n");
				exit(EXIT_FAILURE);
			}
			output_mem_size *= 2;
		}
		memcpy(output + *output_byte_num, dict[matched_entry].mem, dict[matched_entry].len);
		input_bit_pos += dict_num_next_exp_of_2;
		*output_byte_num += dict[matched_entry].len;
		/* Move to next input */
		i += dict[matched_entry].len;

		/* Ensure dictionary still has space */
		if (dict_num == dict_size)
		{
			/* Dictionary is full, clear it */
			dict_num = 256;
			dict_mem_used = 256;
			dict_num_next_exp_of_2 = 8;
			dict_num_next_pow_of_2 = 256;
		}
		/* Add the last match + current match to the dictionary */
		if (dict_mem_used + dict[last_matched_entry].len + dict[matched_entry].len > dict_mem_size)
		{
			/* Need more memory for dictionary words */
			dict_mem = (unsigned char *)realloc(dict_mem, 2 * dict_mem_size);
			if (dict_mem == NULL)
			{
				printf("Error expanding dict_mem\n");
				exit(EXIT_FAILURE);
			}
			dict_mem_size *= 2;
		}
		/* Add entry */
		dict[dict_num].len = dict[last_matched_entry].len + dict[matched_entry].len;
		dict[dict_num].mem = dict_mem + dict_mem_used;
		if (dict_num == dict_num_next_pow_of_2)
		{
			dict_num_next_exp_of_2++;
			dict_num_next_pow_of_2 *= 2;
		}
		dict_num++;
		/* Copy contents */
		memcpy(dict_mem + dict_mem_used, dict[last_matched_entry].mem, dict[last_matched_entry].len);
		dict_mem_used += dict[last_matched_entry].len;
		memcpy(dict_mem + dict_mem_used, dict[matched_entry].mem, dict[matched_entry].len);
		dict_mem_used += dict[matched_entry].len;

		/* Record this match */
		last_matched_entry = matched_entry;
	}

	return output;
}
