#ifndef MAP_H
#define MAP_H

struct Map
{
  /* Length of keys */
  size_t keylen;
  /* Memory for keys */
  unsigned char *keymem;
  /* Current number of entries in the Map */
  size_t size;
  /* Size of memory now */
  size_t memsize;
  /* Memory for entries */
  size_t *mem;
};

struct Map *new_map(size_t keylen, int init_size);
void free_map(struct Map *map);
int new_key(size_t keylen, unsigned char *key, struct Map *map);
int search_key(size_t keylen, unsigned char *key, struct Map *map);
void increment_count(size_t seqlen, unsigned char *seq, struct Map *map);

#endif /* MAP_H */
