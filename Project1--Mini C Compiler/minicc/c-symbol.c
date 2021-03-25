#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "c-symbol.h"
#include "c-parse.h"


static symbol_table_t symbols = NULL;

symbol_table_t symbol_table_push(void)
{
  symbol_table_t s = malloc(sizeof(*s));
  
  s->local_offset = 0;
  s->id_map = string_hmap_create();
  s->head = NULL;
  s->tail = NULL;
  s->parent = symbols;
  s->size = 0;
  symbols = s;
  return s;
}

void symbol_table_pop(void)
{
  if(symbols==NULL)
    return;

  symbol_table_t del = symbols;
  symbols = symbols->parent;

  symbol_table_entry_t tmp,head = del->head;
  while(head)
    {
      tmp = head;
      head = head->next;
      free(tmp);
    }
  hash_map_delete(del->id_map);

  free(del);
}

symbol_table_t symbol_table_top()
{
  return symbols;
}

symbol_table_entry_t symbol_table_add_entry(c_tree decl)
{
  if( IDENTIFIER(DECL(decl)->id)->id == NULL )
    {
      parser_error("Internal error in symbol_table_add_entry.");
      num_errors++;
      return NULL;
    }

  const char * id = IDENTIFIER(DECL(decl)->id)->id;
  symbol_table_entry_t entry = symbol_table_get_entry_in_scope(id);
  if( entry != NULL )
    {
      parser_error("Identifier already in symbol table!");
      num_errors++;
      return entry;
    }

  entry = (symbol_table_entry_t) malloc(sizeof(*entry));
  entry->decl = decl;

  if( TREE_DECL_TYPE(decl) != FUNCTION_DECL )
    {
      entry->offset = symbols->local_offset;
      entry->size = get_decl_size(decl);
      symbols->local_offset += entry->size;
      symbols->size += entry->size;
    }
  else
    symbols->func_count++;

  entry->local = (symbols->parent)?1:0;
  entry->next = NULL;

  if(symbols->head==NULL)
    {
      symbols->head = entry;
      symbols->tail = entry;
    }
  else
    {
      symbols->tail->next = entry;
      symbols->tail = entry;
    }

  hash_map_insert(symbols->id_map,(void*)id,(void*)entry);
  return entry;
}

symbol_table_entry_t symbol_table_add_parameter(c_tree decl)
{
  if( IDENTIFIER(DECL(decl)->id)->id == NULL )
    {
      parser_error("Internal error in symbol_table_add_entry.");
      num_errors++;
      return NULL;
    }

  const char * id = IDENTIFIER(DECL(decl)->id)->id;
  symbol_table_entry_t entry = symbol_table_get_entry_in_scope(id);
  if( entry != NULL )
    {
      parser_error("Identifier already in symbol table!");
      num_errors++;
      return entry;
    }

  if(symbols->parent==NULL)
    {
      parser_error("Internal error: adding parameter to global scope.");
      num_errors++;
    }

  entry = (symbol_table_entry_t) malloc(sizeof(*entry));
  entry->decl = decl;

  if( TREE_DECL_TYPE(decl) == FUNCTION_DECL )
    {
      parser_error("Internal error. FUNCTION_DECL passed as parameter.");
      num_errors++;
      return NULL;
    }

  entry->offset = symbols->param_offset;
  entry->size = get_decl_size(TREE_TYPE(decl));
  symbols->param_offset -= entry->size;
  symbols->size += entry->size;

  entry->local = (symbols->parent)?1:0;
  entry->next = NULL;

  if(symbols->head==NULL)
    {
      symbols->head = entry;
      symbols->tail = entry;
    }
  else
    {
      symbols->tail->next = entry;
      symbols->tail = entry;
    }

  hash_map_insert(symbols->id_map,(void*)id,(void*)entry);
  return entry;
}


symbol_table_entry_t symbol_table_get_entry(const char * id)
{
  symbol_table_t t = symbols;

  while(t)
    {
      if(hash_map_exists(t->id_map,(void*)id))
	return (symbol_table_entry_t)hash_map_find(t->id_map,(void*)id);
      t = t->parent;
    }
  return NULL;
}

symbol_table_entry_t symbol_table_get_entry_in_scope(const char * id)
{
  symbol_table_t t = symbols;

  if(hash_map_exists(t->id_map,(void*)id))
    return (symbol_table_entry_t)hash_map_find(t->id_map,(void*)id);

  return NULL;
}

int get_decl_size(c_tree decl)
{
  if( TREE_CODE(decl) != TREE_DECL )
    return -1;

  if( TREE_TYPE(decl) == NULL )
    return -1;

  return get_type_size( TREE_TYPE(decl) );
}

int get_type_size(c_tree type)
{
  if( TREE_CODE(type) != TREE_TYPE )
    return -1;

  int size=-1;

  switch( TYPE(type)->code )
    {
    case BASIC_TYPE: /* must be an int */
      size = 1;
      break;
    case ARRAY_TYPE: /* could be an int, pointer or aggregate */
      size = get_type_size( TYPE(type)->info.a.type);
      size *= TYPE(type)->info.a.length;
      break;
    case POINTER_TYPE:
      size = 1;
      break;
    case STRUCT_TYPE:
      {
	c_tree chain = TYPE(type)->info.s.fields;
	while(chain)
	  {
	    size += get_type_size( CHAIN_VALUE(chain) );
	    chain = CHAIN_NEXT(chain);
	  }
      }
      break;
    case FUNCTION_TYPE:
      internal_error("Should never ask for a function's size.");
      size = 1;
      break;
    case ERROR_TYPE:
      internal_error("Should never ask for an error_type's size.");
      size = 1;
      break;
    }

  return size;
}

int get_symbol_table_size()
{
  return symbols->size;
}

int get_field_offset(c_tree type, c_tree id)
{
  int offset=0;
  c_tree chain = TYPE(type)->info.s.fields;
  while(chain)
    {
      if( strcmp( IDENTIFIER(id)->id, DECL_NAME_STRING(CHAIN_VALUE(chain))) == 0 )
	{
	  break;
	}

      offset += get_type_size( CHAIN_VALUE(chain) );
      chain = CHAIN_NEXT(chain);
    }

  return offset;
}
