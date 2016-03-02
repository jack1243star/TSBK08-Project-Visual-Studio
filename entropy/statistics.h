#ifndef STATISTICS_H
#define STATISTICS_H

#include <stdio.h>

#include "map.h"

struct Entropy
{
  double memoryless;
  double markov1;
  double markov2;
};

struct Occurrence
{
  /* Occurrence of single symbols (bytes) */
  int single[256];
  /* Occurence of sequences of symbols */
  /* multi[k] contains occurrence of sequences with length k+2 */
  struct Map **multi;
};

struct Statistics
{
  struct Entropy entropy;
  struct Occurrence occurrence;
};

void get_statistics(struct Statistics *stat, FILE* fp);

#endif /* STATISTICS_H */
