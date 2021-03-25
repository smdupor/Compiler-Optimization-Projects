#ifndef C_SYMBOL_H
#define C_SYMBOL_H

#include "c-tree.h"
#include "hashmap.h"

struct symbol_table_entry
{
  c_tree decl;
  int size;
  int offset;
  int local;
  struct symbol_table_entry *next;
};

typedef struct symbol_table_entry * symbol_table_entry_t;

struct symbol_table 
{
  int       local_offset;
  int       param_offset;
  int       size;
  int       func_count;
  hashmap_type *id_map;
  symbol_table_entry_t head;
  symbol_table_entry_t tail;
  struct symbol_table *parent;
};

typedef struct symbol_table * symbol_table_t;

symbol_table_t symbol_table_push(void);
void symbol_table_pop(void);
symbol_table_t symbol_table_top(void);

symbol_table_entry_t symbol_table_add_entry(c_tree);
symbol_table_entry_t symbol_table_add_parameter(c_tree);
symbol_table_entry_t symbol_table_get_entry(const char *);
symbol_table_entry_t symbol_table_get_entry_in_scope(const char *);

int get_decl_size(c_tree);
int get_type_size(c_tree);
int get_symbol_table_size();
int get_field_offset(c_tree,c_tree);


#endif
