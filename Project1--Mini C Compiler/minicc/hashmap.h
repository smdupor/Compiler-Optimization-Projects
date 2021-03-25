#ifndef HASHMAP_H
#define HASHMAP_H

#include "hashtab.h"

/* Hash map : behaves the way you would like a hash map to behave */
struct value_map_d
{
  const void  *key;
  void        *data;
  htab_del    data_free_f;  /* not nice, but no need to change hash table interface */
};

typedef struct hashmap_def
{
  htab_del data_free_f;
  htab_t   hashmap;
} hashmap_type;

/* Types of hash map */
hashmap_type *hash_map_create (htab_del);  
hashmap_type *string_hmap_create (void);

#define HASH_MAP_DATA(slot)  ( (struct value_map_d*) *slot )->data
#define HASH_MAP_KEY(slot)  ( (struct value_map_d*) *slot )->key

void      hash_map_delete  (hashmap_type *);
void      hash_map_clean   (hashmap_type *);
int       hash_map_exists  (hashmap_type *, void *);
void      hash_map_remove  (hashmap_type *, const void *);
int       hash_map_insert     (hashmap_type *, void *, void *);
void     *hash_map_find    (hashmap_type *, void *);
void      hash_map_trav    (hashmap_type *, htab_trav, void *);
int       hash_map_active_size  (hashmap_type *);

#endif
