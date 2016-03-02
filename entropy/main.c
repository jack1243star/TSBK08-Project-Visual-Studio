#include <stdio.h>
#include <stdlib.h>

#include "statistics.h"

int main(int argc, char *argv[])
{
  FILE *fp = NULL;
  struct Statistics stats;

  if(argc != 2)
  {
    printf("Usage: %s <filename>\n", argv[0]);
    return EXIT_SUCCESS;
  }

  /* Open file to read */
  fp = fopen(argv[1], "rb");
  if(fp == NULL)
  {
    printf("Cannot open file %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  /* Get statistics of file */
  get_statistics(&stats, fp);

  /* Close file */
  fclose(fp);

  return EXIT_SUCCESS;
}
