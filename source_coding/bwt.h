#pragma once

/* Burrows-Wheeler Transform */
void bwt(unsigned char *input, unsigned char *output, size_t *index, size_t length);
/* Inverse Burrows-Wheeler Transform */
void ibwt(unsigned char *input, unsigned char *output, size_t index, size_t length);
