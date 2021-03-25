#include <sys/types.h>
#include <string.h>
#include "hashmap.h"
#include <stdlib.h>
/* hash_map functionality */

static hashval_t 
hash_map (const void *p)
{
  return htab_hash_pointer ((const void *)((const struct value_map_d *)p)->key);
}

static hashval_t
hash_string (const void *p)
{
  const unsigned char *str = 
    (const unsigned char*)((const struct value_map_d *)p)->key;
  hashval_t r = 0;
  unsigned char c;

  while ((c = *str++) != 0)
    r = r * 67 + c - 113;

  return r;
}

static int       
hash_map_eq (const void *p1, const void *p2)
{
  return ((const struct value_map_d *)p1)->key
	 == ((const struct value_map_d *)p2)->key;
}


static int
string_hmap_eq (const void *s1, const void* s2)
{
  return (strcmp ((char *) ((const struct value_map_d *)s1)->key, 
		  (char *) ((const struct value_map_d *)s2)->key) == 0);
}

static void 
hash_map_del_f (void *p)
{
  struct value_map_d *vm = (struct value_map_d *)p;
  if (vm->data_free_f)
    (*(vm->data_free_f))(vm->data);
  free(vm);  
}

hashmap_type *
hash_map_create(htab_del data_del_f)
{
  hashmap_type *hmap = (hashmap_type *)malloc(sizeof(hashmap_type));
  hmap->data_free_f = data_del_f;
  hmap->hashmap = htab_create (64, hash_map, hash_map_eq, &hash_map_del_f);

  return hmap;
}

int
hash_map_active_size(hashmap_type *hptr)
{
  return htab_elements(hptr->hashmap);
}

hashmap_type *
string_hmap_create(void)
{
  hashmap_type *hmap = (hashmap_type *)malloc(sizeof(hashmap_type));
  hmap->data_free_f = NULL;
  hmap->hashmap = htab_create (64, hash_string, string_hmap_eq, &hash_map_del_f);

  return hmap;
}

void      
hash_map_delete (hashmap_type *hptr) 
{
  htab_delete(hptr->hashmap);
  free(hptr);
}

void      
hash_map_clean (hashmap_type *hptr)
{
  htab_empty(hptr->hashmap);
}

int      
hash_map_exists (hashmap_type *hptr, void *key)

{
  struct value_map_d vm;

  vm.key = key;
  
  return htab_find (hptr->hashmap, (void*) &vm) != NULL;
}

/* insert <key,data> pair into hash map
 * return 1, if it is a new data, otherwise return 0
 */
int      
hash_map_insert (hashmap_type *hptr, void *key, void *data)
{  
  struct value_map_d *vm_p, vm;
  void   **slot;
  int   first = 0;

  vm.key = key;
  slot = htab_find_slot (hptr->hashmap, (void *) &vm, INSERT);

  if (*slot == NULL)
    {
      first = 1;
      vm_p = (struct value_map_d *)malloc (sizeof (*vm_p));
      vm_p->key = key;
      vm_p->data = data;
      vm_p->data_free_f = hptr->data_free_f;
      *slot = (void *) vm_p;
    }
  else
    ((struct value_map_d *)(*slot))->data = data;

  return first;
}

void      
hash_map_remove (hashmap_type *hptr, const void *key) 
{
  struct value_map_d vm;
  
  vm.key = key;
  htab_remove_elt(hptr->hashmap, &vm);
}

void *
hash_map_find (hashmap_type *hptr, void *key)
{
  struct value_map_d vm;
  void   **slot;
  
  vm.key = key;
  slot = htab_find (hptr->hashmap, (void *) &vm);

  if (slot == NULL) /* not found */
    return NULL;

  return ((struct value_map_d *)(slot))->data;
}

void      
hash_map_trav (hashmap_type *hptr, htab_trav ht_func, void *arg)
{
  htab_traverse(hptr->hashmap, ht_func, arg);
}
