#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lzw.h"

#define DICT_MEM_INIT_SIZE 512

/* A codeword in the dictionary */
struct DictWord
{
	struct DictWord * prev;
	unsigned char byte;
};

/* Initialize and return a dictionary */
static struct DictWord * init_dict(size_t * dict_num, size_t max_bits_per_codeword)
{
	struct DictWord * dict;
	size_t dict_size = (size_t)1 << max_bits_per_codeword;
	size_t i;

	/* Initiailze dictionary */
	dict = (struct DictWord *)calloc(dict_size, sizeof(struct DictWord));
	/* Ensure malloc success */
	if (dict == NULL)
	{
		printf("Error creating dictionary for LZW\n");
		exit(EXIT_FAILURE);
	}
	/* Create default entries in dictionary */
	for (i = 0; i < 256; i++)
	{
		dict[i].prev = NULL;
		dict[i].byte = 0xFF & i;
	}

	*dict_num = 256;
	return dict;
}

/* Update dictionary, clear if dictionary full and add entry */
static void update_dict(
	struct DictWord * dict,
	size_t * dict_num,
	size_t max_bits_per_codeword,
	struct DictWord * prev,
	unsigned char byte)
{
	size_t dict_size = (size_t)1 << max_bits_per_codeword;

	/* Ensure dictionary still has space */
	if (*dict_num >= dict_size)
	{
		/* Dictionary is full, clear it */
		*dict_num = 256;
	}

	/* Add new entry */
	dict[*dict_num].prev = prev;
	dict[*dict_num].byte = byte;
	(*dict_num)++;

	return;
}

/* Match a codeword to input, returns first matched codeword */
static struct DictWord * match(
	struct DictWord * dict,
	size_t dict_num,
	unsigned char * input,
	size_t start, size_t input_length,
	size_t * match_length)
{
	size_t i, j;
	struct DictWord * match;

	/* First symbol */
	match = dict + input[start];
	*match_length = 1;
	/* Match subsequent symbols */
	i = start + 1; 
	for (j = 256; j < dict_num && i < input_length; j++)
	{
		if (dict[j].prev == match && dict[j].byte == input[i])
		{
			/* Match found */
			match = &dict[j];
			*match_length += 1;
			/* Match next input */
			i++;
		}
	}

	return match;
}

/* Return the first symbol of a dictionary entry */
static unsigned char head(struct DictWord * w)
{
	while (w->prev != NULL)
		w = w->prev;
	return w->byte;
}

/* Return the length of a dictionary entry */
static size_t codelen(struct DictWord * w)
{
	size_t len = 1;
	while(w->prev != NULL)
	{
		w = w->prev;
		len++;
	}
	return len;
}

/* Write dictionary entry to memory */
static void write_code(unsigned char * dst, struct DictWord * w, size_t code_length)
{
	size_t i = 1;
	do {
		dst[code_length - i] = w->byte;
		i++;
		w = w->prev;
	} while (w != NULL);
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

unsigned char * lzw_encode(unsigned char * input, size_t * output_bit_num, size_t length, size_t max_bits_per_codeword)
{
	/* Loop counter */
	size_t i;

	/* Dictionary */
	struct DictWord * dict = NULL;
	/* Current number of entries in dictionary */
	size_t dict_num;

	/* Current match */
	struct DictWord * matched_entry;
	/* Match length */
	size_t match_length;
	/* Last match */
	struct DictWord * last_matched_entry;

	/* Output buffer */
	unsigned char * output = NULL;
	/* Output memory size in bytes */
	size_t output_mem_size = 0;

	size_t dict_num_next_exp_of_2 = 9;
	size_t dict_num_next_pow_of_2 = 512;

	if (max_bits_per_codeword < 9)
	{
		printf("Error invoking LZW compression: bits per codeword must be at least 9\n");
		exit(EXIT_FAILURE);
	}

	/* Initiailze dictionary */
	dict = init_dict(&dict_num, max_bits_per_codeword);

	/* First symbol */
	*output_bit_num = 0;
	output = pack(output, &output_mem_size, output_bit_num, input[0], 8);
	last_matched_entry = dict + input[0];

	/* Read each input */
	i = 1;
	while (i < length)
	{
		/* Find entry in dictionary */
		matched_entry = match(dict, dict_num, input, i, length, &match_length);
		/* Encode the message with log2(dict_num) bits */
		output = pack(output, &output_mem_size, output_bit_num, matched_entry - dict, dict_num_next_exp_of_2);
		/* Add entry to dictionary */
		update_dict(dict, &dict_num, max_bits_per_codeword, last_matched_entry, head(matched_entry));

		/* Record this match */
		last_matched_entry = matched_entry;
		/* Move to next input */
		i += match_length;

		/* Calculate log2 */
		if (dict_num == 256)
		{
			dict_num_next_exp_of_2 = 8;
			dict_num_next_pow_of_2 = 256;
		}
		else if (dict_num > dict_num_next_pow_of_2)
		{
			dict_num_next_exp_of_2++;
			dict_num_next_pow_of_2 *= 2;
		}
	}

	/* Free dictionary */
	free(dict);

	return output;
}

unsigned char * lzw_decode(unsigned char * input, size_t * output_byte_num, size_t length, size_t max_bits_per_codeword)
{
	/* Loop counter */
	size_t i;
	/* Flag for special case */
	int special_case = 0;

	/* Dictionary */
	struct DictWord * dict = NULL;
	/* Number of dicitonary entries */
	size_t dict_num;

	/* Matched dict entry */
	size_t matched_entry;
	/* Match length */
	size_t match_length;
	/* Last matched dict entry */
	size_t last_matched_entry;

	/* Current bit in input */
	size_t input_bit_pos = 0;
	/* Output buffer */
	unsigned char * output = NULL;
	/* Output memory size in bytes */
	size_t output_mem_size = 0;

	size_t dict_num_next_exp_of_2 = 9;
	size_t dict_num_next_pow_of_2 = 512;

	/* Initiailze dictionary */
	dict = init_dict(&dict_num, max_bits_per_codeword);

	/* Allocate memory for output */
	output = (unsigned char *)malloc(256 * sizeof(unsigned char *));
	if (output == NULL)
	{
		printf("Error allocating memory for output in lzw_decode\n");
		exit(EXIT_FAILURE);
	}
	output_mem_size = 256;

	/* First symbol */
	matched_entry = unpack(input, input_bit_pos, 8);
	last_matched_entry = matched_entry;
	/* Write to output buffer */
	write_code(output, dict + matched_entry, 1);
	*output_byte_num = 1;
	/* Move to next input */
	input_bit_pos += 8;

	/* Read each input */
	i = 1;
	while (i < length)
	{
		/* Read log2(dict_num) bits and decode the message */
		matched_entry = unpack(input, input_bit_pos, dict_num_next_exp_of_2);

		/* Fix special case from the previous run */
		if (special_case)
		{
			dict[last_matched_entry].byte = head(dict + matched_entry);
			output[*output_byte_num - 1] = head(dict + matched_entry);
		}

		/* Check if we have a special case */
		if (matched_entry >= dict_num)
		{
			printf("Special case\n");
			special_case = 1;
			match_length = codelen(dict + last_matched_entry) + 1;
		}
		else
		{
			special_case = 0;
			match_length = codelen(dict + matched_entry);
		}

		/* Ensure big enough output buffer */
		if (*output_byte_num + match_length > output_mem_size)
		{
			output = (unsigned char *)realloc(output, 2 * output_mem_size * sizeof(unsigned char));
			if (output == NULL)
			{
				printf("Error expanding memory for output in lzw_decode\n");
				exit(EXIT_FAILURE);
			}
			output_mem_size *= 2;
		}

		/* Write to output buffer */
		write_code(output + *output_byte_num, dict + matched_entry, match_length);
		*output_byte_num += match_length;
		/* Move to next input */
		input_bit_pos += dict_num_next_exp_of_2;
		i += codelen(dict + matched_entry);

		/* Add entry to dictionary */
		update_dict(dict, &dict_num, max_bits_per_codeword, dict+last_matched_entry, head(dict + matched_entry));

		/* Record this match */
		last_matched_entry = matched_entry;

		/* Calculate log2 */
		if (dict_num == 256)
		{
			dict_num_next_exp_of_2 = 8;
			dict_num_next_pow_of_2 = 256;
		}
		else if (dict_num > dict_num_next_pow_of_2)
		{
			dict_num_next_exp_of_2++;
			dict_num_next_pow_of_2 *= 2;
		}
	}

	/* Free dictionary */
	free(dict);

	return output;
}
