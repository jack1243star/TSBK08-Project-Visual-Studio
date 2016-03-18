#pragma once

#include <stddef.h>

/* LZW Compress */
unsigned char * lzw_encode(unsigned char * input, size_t * output_bit_num, size_t length, size_t bits_per_codeword);
/* LZW Decompress */
unsigned char * lzw_decode(unsigned char * input, size_t * output_byte_num, size_t length, size_t bits_per_codeword);
