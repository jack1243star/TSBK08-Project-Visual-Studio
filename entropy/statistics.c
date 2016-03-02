#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "statistics.h"
#include "map.h"

#define BUFFER_SIZE 4096

void init_statistics(struct Statistics *stats);
void free_statistics(struct Statistics *stats);

void init_statistics(struct Statistics *stats)
{
  int i;
  /* Initialize occurrence counts */
  for(i=0; i<256; i++)
    stats->occurrence.single[i] = 0;
  stats->occurrence.multi = NULL;
  stats->occurrence.multi = (struct Map **)malloc(2 * sizeof(struct Map *));
  if(stats->occurrence.multi == NULL)
  {
    printf("Failed to initialize occurrence counts\n");
    exit(EXIT_FAILURE);
  }
  stats->occurrence.multi[0] = new_map(2, 256*256);
  stats->occurrence.multi[1] = new_map(3, 256*256*256);
}


/* Estimate entropy and Markov models */
void get_statistics(struct Statistics *stats, FILE* fp)
{
  unsigned char *buffer;
  unsigned char old_buffer[BUFFER_SIZE+2];
  int total = 0;
  double sum = 0.0;
  double probability;
  size_t i, bytes_read;

  init_statistics(stats);
  rewind(fp);
  buffer = old_buffer+2;

  /* Read the first two characters */
  old_buffer[0] = (unsigned char)fgetc(fp);
  stats->occurrence.single[old_buffer[0]]++;
  old_buffer[1] = (unsigned char)fgetc(fp);
  stats->occurrence.single[old_buffer[1]]++;
  increment_count(2, old_buffer, stats->occurrence.multi[0]);
  total += 2;
  while(!feof(fp))
  {
    /* Fill buffer */
    bytes_read = fread(buffer, 1, BUFFER_SIZE, fp);

    for(i=0; i<bytes_read; i++)
    {
      /* Count occurrence of the byte */
      stats->occurrence.single[buffer[i]]++;
      /* Count occurrence of the latest two bytes */
      increment_count(2, buffer+i-1, stats->occurrence.multi[0]);
      /* Count occurrence of the latest three bytes */
      increment_count(3, buffer+i-2, stats->occurrence.multi[1]);
      /* Count total number of bytes */
      total++;
    }
  }

  /* Report occurrences */
  for(i=0; i<256; i++)
    printf("occurrence[%3d] = %d\n", i, stats->occurrence.single[i]);
  /* Report file size */
  printf("total = %d\n", total);

  /* Estimate memoryless entropy */
  for(i=0; i<256; i++)
  {
    if(stats->occurrence.single[i] > 0)
    {
      /* Calculate probability of the byte */
      probability = stats->occurrence.single[i] / (double)total;
      /* Add to sum */
      sum = sum - (probability * log2(probability));
    }
  }
  stats->entropy.memoryless = sum;

  /* Calculate entropy rate of first-order Markov model */
  sum = 0;
  for(i=0; i<256*256; i++)
  {
    if(stats->occurrence.multi[0]->mem[i] > 0)
    {
      /* Calculate probability of the sequence */
      probability = stats->occurrence.multi[0]->mem[i] / (double)(total-1);
      /* Add to sum */
      sum = sum - (probability * log2(probability));
    }
  }
  stats->entropy.markov1 = sum - stats->entropy.memoryless;

  /* Calculate entropy rate of second-order Markov model */
  sum = 0;
  for(i=0; i<256*256*256; i++)
  {
    if(stats->occurrence.multi[1]->mem[i] > 0)
    {
      /* Calculate probability of the sequence */
      probability = stats->occurrence.multi[1]->mem[i] / (double)(total-2);
      /* Add to sum */
      sum = sum - (probability * log2(probability));
    }
  }
  stats->entropy.markov2 = sum
    - stats->entropy.markov1
    - stats->entropy.memoryless;

  /* Report memoryless entropy */
  printf("Memoryless entropy: %f bit(s)\n", stats->entropy.memoryless);
  /* Report entropy rate of first-order Markov model */
  printf("Markov model (Order 1) entropy rate: %f bit(s)\n", stats->entropy.markov1);
  /* Report entropy rate of second-order Markov model */
  printf("Markov model (Order 2) entropy rate: %f bit(s)\n", stats->entropy.markov2);

  free_statistics(stats);
  return;
}

void free_statistics(struct Statistics *stats)
{
  free_map(stats->occurrence.multi[0]);
  free_map(stats->occurrence.multi[1]);
  free(stats->occurrence.multi);
}
