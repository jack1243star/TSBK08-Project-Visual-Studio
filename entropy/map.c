#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "map.h"

/* Create a structure that maps byte sequence to integers */
struct Map *new_map(size_t keylen, int init_size)
{
  struct Map *map;

  map = (struct Map *)malloc(sizeof(struct Map));
  if(map == NULL)
  {
    printf("Failed to create map\n");
    exit(EXIT_FAILURE);
  }

  if(keylen < 4)
  {
    /* Direct addressing */
    map->mem = calloc((size_t)pow(256, keylen), sizeof(size_t));
    if(map->mem == NULL)
    {
      printf("Failed to allocate memory for map\n");
      exit(EXIT_FAILURE);
    }    
  }
  else
  {
    map->keylen = keylen;
    map->size = 0;
    map->memsize = init_size;
    map->keymem = malloc(keylen * init_size * sizeof(char));
    map->mem = calloc(init_size, sizeof(size_t));
    if(map->mem == NULL || map->keymem == NULL)
    {
      printf("Failed to create map\n");
      exit(EXIT_FAILURE);
    }
  }

  return map;
}

/* Free the map and its memory */
void free_map(struct Map *map)
{
  free(map->mem);
  free(map);
  return;
}

/* Create new entry in the map and return its index */
int new_key(size_t keylen, unsigned char *key, struct Map *map)
{
  unsigned char *newkeymem;
  size_t *newmem;
  /* Check if there is enough memory */
  if(map->memsize < map->size)
  {
    /* Still free, insert the entry */
    memcpy(map->keymem + map->size, key, keylen);
    *(map->mem + map->size) = 0;
    map->size++;
  }
  else
  {
    /* Need more memory */
    newkeymem = (unsigned char *)realloc(map->keymem,
					 2 * map->memsize * keylen * sizeof(unsigned char));
    newmem = (size_t *)realloc(map->mem, 2 * map->memsize * sizeof(size_t));
    if(newkeymem == NULL || newmem == NULL)
    {
      printf("Failed to allocate more memory for map\n");
      exit(EXIT_FAILURE);
    }
    map->keymem = newkeymem;
    map->mem = newmem;
    /* Insert the entry */
    memcpy(map->keymem + map->size, key, keylen);
    *(map->mem + map->size) = 0;
    map->size++;
  }
  return map->size-1;
}

/* Do a linear search on the map to get the index of the entry */
int search_key(size_t keylen, unsigned char *key, struct Map *map)
{
  size_t i;

  /* Fails for an empty map */
  if(map->size == 0)
    return -1;

  for(i=0; i<map->size; i++)
  {
    if(memcmp(key, (map->mem) + i*keylen, keylen)==0)
      return i;
  }

  return -1;
}

void increment_count(size_t seqlen, unsigned char *seq, struct Map *map)
{
  int i;

  if(seqlen < 4)
  {
    /* Direct addressing */
    switch(seqlen)
    {
    case 2:
      i = seq[0] * 256 + seq[1];
      break;
    case 3:
      i = seq[0] * 256 * 256 + seq[1] * 256 + seq[2];
      break;
    default:
      printf("Error: increment_count called with seqlen %d\n", seqlen);
      exit(EXIT_FAILURE);
      break;
    }
    /* Increment the entry */
    (*(map->mem + i))++;
  }
  else
  {
    /* Find the bucket */
    i = search_key(seqlen, seq, map);
    if(i == -1)
    {
      /* Create new entry */
      i = new_key(seqlen, seq, map);
      *(map->mem + i) = 1;
    }
    else
    {
      /* Intrement the entry */
      (*(map->mem + i))++;
    }
  }
    
}
