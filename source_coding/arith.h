#pragma once

/* Adaptive Arithmetic Encoding */
unsigned char * arith_encode(unsigned char * input, size_t * output_bit_num, size_t length);
/* Adaptive Arithmetic Decoding */
void arith_decode(unsigned char * input, unsigned char * output, size_t length);