#include "c-tree.h"
#include "c-parse.h"
#include "c-eval.h"
#include "c-typecheck.h"

#include <sys/types.h>
#include "hashtab.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

extern c_tree void_type;
extern c_tree integer_type;
extern c_tree error_type;

static int unique_name_cnt=0;

char * create_unique_struct_name(enum type_code t, const char *fname)
{
  char *name;
  int size;
  int fnamelen = strlen(fname);
  fnamelen = (fnamelen>5)?5:fnamelen;

  size = fnamelen + 2 + (int)log10(unique_name_cnt); // fname + 2 underscores + cnt

  size += 5; // struct
  size += 1; // NULL terminator
  name = (char *)malloc(size);
  sprintf(name,"struct_%5s_%d",fname,unique_name_cnt);

  unique_name_cnt++;
  return name;
}

char * create_pointer_name(c_tree type)
{
  char *name;
  int size;

  if(!TREE_CODE_CHECK(type,TREE_TYPE))
    {
      parser_error("Tree does not specify a type!");
      num_errors++;
      return NULL;
    }

  size = strlen( TYPE(type)->type_name );
  size += 2; // + STAR + NULL terminator
  name = (char *)malloc(size);
  sprintf(name,"%s*",TYPE(type)->type_name);
  return name;
}

char * create_array_name(c_tree type,int length)
{
  char *name;
  int size;

  if(!TREE_CODE_CHECK(type,TREE_TYPE))
    {
      parser_error("Tree does not specify a type!");
      num_errors++;
      return NULL;
    }

  size = strlen( TYPE(type)->type_name );
  size += 3 + log10(length); // + [] + NULL terminator
  name = (char *)malloc(size);
  if(length)
    sprintf(name,"%s[%d]",TYPE(type)->type_name,length);
  else
    sprintf(name,"%s[]",TYPE(type)->type_name);
  return name;
}


static int uid_cnt;
static htab_t typemap;
static htab_t uidmap;
static htab_t namemap;

typedef struct type_slot{
  int uid;
  c_tree type;  
} * type_slot_t;

static hashval_t type_hash(const void *slot)
{
  c_tree t  = ((type_slot_t) slot)->type;
  struct type_slot nslot;
  
  switch(t->type.code) {
  case BASIC_TYPE: 
  case STRUCT_TYPE:
  case FUNCTION_TYPE:
    return htab_hash_string(t->type.type_name);
  case POINTER_TYPE:
    nslot.type = t->type.info.reftype;
    return type_hash(&nslot) * t->type.code;
  case ARRAY_TYPE:
    nslot.type = t->type.info.a.type;
    return type_hash(&nslot) * t->type.code;
  default:
    break;
  }

  return 0;
}

static hashval_t name_hash(const void *slot)
{
  c_tree t  = ((type_slot_t) slot)->type;
  
  switch(t->type.code) {
  case BASIC_TYPE: 
  case STRUCT_TYPE:
  case FUNCTION_TYPE:
    return htab_hash_string(t->type.type_name);
  case POINTER_TYPE:
  case ARRAY_TYPE:
    return 0;
  default:
    break;
  }

  return 0;
}

static int compare_type(c_tree aa, c_tree bb)
{
  if(bb->type.code != aa->type.code)
    return 0;

  switch(aa->type.code) {
  case BASIC_TYPE:
  case STRUCT_TYPE:
  case FUNCTION_TYPE:
    return strcmp(aa->type.type_name,bb->type.type_name)==0;
  case POINTER_TYPE:    
    return compare_type(aa->type.info.reftype,bb->type.info.reftype);
  case ARRAY_TYPE:
    return compare_type(aa->type.info.a.type,bb->type.info.a.type);
  default:
    break;
  }
  return 0;
}

static hashval_t uid_hash(const void *slot)
{
  int val  = ((type_slot_t) slot)->uid;

  return (hashval_t)val;
}


static int type_eq(const void *aslot, const void *bslot)
{
  c_tree aa = ((type_slot_t)aslot)->type;
  c_tree bb = ((type_slot_t)bslot)->type;

  return compare_type(aa,bb);
}

static int uid_eq(const void *aslot, const void *bslot)
{
  return ((type_slot_t)aslot)->uid == ((type_slot_t)bslot)->uid;
}

static int name_eq(const void *aslot, const void *bslot)
{
  c_tree aa = ((type_slot_t)aslot)->type;
  c_tree bb = ((type_slot_t)bslot)->type;

  switch(aa->type.code) {
  case BASIC_TYPE:
  case STRUCT_TYPE:
  case FUNCTION_TYPE:
    return strcmp(aa->type.type_name,bb->type.type_name)==0;
  case POINTER_TYPE:
  case ARRAY_TYPE:
    return 0;
  default:
    break;
  }
  return 0;
}

void c_typecheck_init()
{
  uid_cnt = 1;
  typemap = htab_create_alloc (100, type_hash, type_eq, NULL, calloc, free);  
  uidmap =  htab_create_alloc (100, uid_hash, uid_eq, NULL, calloc, free);  
  namemap =  htab_create_alloc (100, name_hash, name_eq, NULL, calloc, free);  
}

int get_uid_for_type(c_tree type)
{
  struct type_slot slot;
  void *s;
 
  slot.type = type;
  s = htab_find(typemap,&slot);
  
  if(s)
    return ((type_slot_t)s)->uid;

  return 0;
}

c_tree get_type_for_uid(int uid)
{
  void *s; 
  struct type_slot slot;

  slot.uid = uid;
  s = htab_find(uidmap,(void*)&slot);
  
  if(s)
    return ((type_slot_t)s)->type;
      
  return 0;
}

int register_type(c_tree type)
{
  int uid = get_uid_for_type(type);

  if(uid!=0) 
    {
      type->type.uid = uid;
      return uid;
    }

  /* register new subtypes first */
  switch(type->type.code) {
  case BASIC_TYPE: 
  case STRUCT_TYPE:
  case FUNCTION_TYPE:
    break;
  case POINTER_TYPE:
    register_type(type->type.info.reftype);
    break;
  case ARRAY_TYPE:
    register_type(type->type.info.a.type);
    break;
  default:
    break;
  }

  uid = uid_cnt++;
  type->type.uid = uid;
  
  type_slot_t *slot, t;
  t = (type_slot_t) malloc(sizeof(*t));
  t->uid = uid;
  t->type = type;
  
  slot = (type_slot_t*)htab_find_slot(namemap,t,INSERT);
  if(slot)
    *slot = t;

  slot  = (type_slot_t*)htab_find_slot(uidmap,t,INSERT);
  if(slot)
    *slot = t;

  slot = (type_slot_t*)htab_find_slot(typemap,t,INSERT);
  if(slot)
    *slot = t;

  return uid;
}

/* 
 * Only use this function for name equivalent types:
 *     BASIC, STRUCT, FUNCTION
 */
int check_for_typename(char *typename)
{
  void *s; 
  struct type_slot slot;
  union tree_union tmp;

  slot.type = &tmp;
  tmp.type.type_name = typename;
  tmp.type.code = BASIC_TYPE; /* code here doesn't matter */
  
  s = htab_find(namemap,(void*)&slot);
  
  if(s)
    return ((type_slot_t)s)->uid;
      
  return 0;
}

c_tree normalize_type(c_tree decl, c_tree type)
{
  enum type_code typecode;
  type_info info;

  switch( TREE_DECL_TYPE(decl) )
    {
    case VAR_DECL:
      TREE_TYPE(decl) = type;
      return decl;
    case FUNCTION_DECL:
      TREE_TYPE(decl) = type;
      return decl;
    case ARRAY_DECL:
      DECL(decl)->array.type = type;
      typecode = ARRAY_TYPE;
      info.a.type = type;
      info.a.length = 0;
      get_constant_value(DECL(decl)->array.length,&info.a.length);
      TREE_TYPE(decl) =build_type(typecode,create_array_name(type,info.a.length),info);
      break;
    default:
      break;
    }
  
  return decl;
}

#if 0

void propagate_type(c_tree type, c_tree decl_list)
{
  c_tree chain = decl_list->decl_list.head;

  while(chain)
    {
      c_tree decl = TREE_CHAIN_VALUE(chain);
      //TREE_CHECK_DECL(decl);

      // return a VAR_DECL with a registered type
      decl = normalize_type(decl,type);

      TREE_CHAIN_VALUE(chain) = decl;

      chain = TREE_CHAIN(chain);
    }
}
#endif

static int check_types(c_tree type1, c_tree type2)
{
  if(TYPE(type1)->uid == TYPE(type2)->uid)
    return 1;

  return (fold_types(type1,type2)?1:0);
}

int check_int(c_tree type)
{
  return check_types(integer_type,type);
}

c_tree fold_types(c_tree t1, c_tree t2)
{
  if(TYPE(t1)->uid == TYPE(t2)->uid)
    return t1;
  
  /* If one is int, and the other pointer, return pointer */

  if( check_int(t1) )
    {
      if( TYPE(t2)->code == POINTER_TYPE || TYPE(t2)->code == ARRAY_TYPE )
	return t2;
    }
  
  if ( check_int(t2) )
    {
      if( TYPE(t1)->code == POINTER_TYPE || TYPE(t1)->code == ARRAY_TYPE )
	return t1;
    }
  /* if one is pointer and the other array of the same reftype,
   * then return pointer to ref type
   */
  
  if( TYPE(t1)->code == POINTER_TYPE && TYPE(t2)->code == ARRAY_TYPE)
    {
      return fold_types( TYPE(t1)->info.reftype, TYPE(t2)->info.a.type ) ;
    }
  else if( TYPE(t2)->code == POINTER_TYPE && TYPE(t1)->code == ARRAY_TYPE)
    {
      return fold_types( TYPE(t2)->info.reftype, TYPE(t1)->info.a.type ) ;
    }

  return NULL;
}


int check_int_expression(c_tree expr)
{
  c_tree type = eval_expr_type(expr);

  if( TYPE(type)->code == POINTER_TYPE || TYPE(type)->code == ARRAY_TYPE )
    return 1;

  return check_types(integer_type, type);
}

static int check_for_member(c_tree type, c_tree field, c_tree *field_type)
{
  if( TYPE(type)->code != STRUCT_TYPE )
    {
      *field_type = error_type;
      return 0;
    }

  /* need to lookup full definition of type */
  type = get_type_for_uid(TYPE(type)->uid); 

  c_tree decl_list = TYPE(type)->info.s.fields;
  c_tree chain = DECL_LIST(decl_list)? DECL_LIST(decl_list)->head : NULL;
  c_tree fdecl;
  while(chain)
    {
      /* get field in chain */
      fdecl = CHAIN_VALUE(chain);
      if(strcmp( IDENTIFIER(DECL(fdecl)->id)->id, IDENTIFIER(field)->id) == 0)
	{
	  *field_type = TREE_TYPE(fdecl);
	  return 1;
	}
      
      chain = CHAIN_NEXT(chain);
    }

  *field_type = error_type;
  return 0;
}

int check_param_list(c_tree fndecl, c_tree exprlist)
{
  if(TREE_CODE(fndecl) != TREE_DECL && TREE_DECL_TYPE(fndecl) != FUNCTION_DECL)
    {
      return 0;
    }

  c_tree decl_list = DECL(fndecl)->func.param_decl;
  c_tree chain = DECL_LIST(decl_list)->head;
  c_tree echain = EXPR_LIST(exprlist)->chain;

  while(chain && echain)
    {
      c_tree p = CHAIN_VALUE(chain);
      c_tree e = CHAIN_VALUE(echain);

      c_tree op0 = eval_expr_type(e);

      if( !check_types(TREE_TYPE(p),op0) )
	{
	  parser_error("Argument types do not match function declaration.");
	  num_errors++;
	  return 0;
	}

      echain = CHAIN_NEXT(echain);
      chain = CHAIN_NEXT(chain);
    }

  if( chain!=NULL || echain!=NULL ) {
    if( chain!=NULL )
      parser_error("Not enough arguments to function.");
    else
      parser_error("Too many arguments to function.");
      
    num_errors++;
    return 0;
  }

  return 1;
}

int check_location(c_tree t)
{
  c_tree type = TREE_TYPE(t);

  if( TREE_CODE(t) == TREE_DECL )
    {
      if( TYPE(type)->code == ARRAY_TYPE || TYPE(type)->code == FUNCTION_TYPE
	  || TYPE(type)->code == ERROR_TYPE )
	return 0;
      else
	return 1;
    }

  if( TREE_CODE(t) == TREE_EXPR )
    {
      switch(EXPR_TYPE(t)){
      case ARRAY_REF_EXPR:
      case MEMBER_EXPR:
      case INDIRECT_REF_EXPR:
	return 1;
      default:
	return 0;
      }
    }

  return 0;
}

c_tree eval_expr_type(c_tree tree)
{
  c_tree op0,op1,op2;
  
  if(TREE_TYPE(tree)!=NULL)
    {
      /* Constants and variables *must* already have a type set,
       * this will guarantee we handle them properly.
       */
      return TREE_TYPE(tree);
    }

  if(TREE_CODE(tree)!=TREE_EXPR)
    {
      internal_error("input arg. to eval_expr_type is not an expression.");
      num_errors++;
      return error_type;
    }

  switch(EXPR_TYPE(tree))
    {
      /* Handle binary expressions */
    case ASSIGN_EXPR: 
      if(!check_location(TREE_EXPR_OPERAND(tree,0)))
	{
	  parser_error("Not a valid l-value.");
	  num_errors++;
	}
    case ADD_EXPR:
    case SUBTRACT_EXPR:
    case MULTIPLY_EXPR:
    case DIVIDE_EXPR:
    case MODULO_EXPR:
    case LOGICAL_AND_EXPR:
    case LOGICAL_GT_EXPR:
    case LOGICAL_GTE_EXPR:
    case LOGICAL_LT_EXPR:
    case LOGICAL_LTE_EXPR:
    case LOGICAL_OR_EXPR:
    case LOGICAL_EQ_EXPR:
    case LOGICAL_NEQ_EXPR:
      op0 = eval_expr_type(TREE_EXPR_OPERAND(tree,0));
      op1 = eval_expr_type(TREE_EXPR_OPERAND(tree,1));
      if(fold_types(op0,op1) && check_int_expression(TREE_EXPR_OPERAND(tree,0)))
	return fold_types(op0,op1); /* May be a pointer or integer */
      else
	return error_type;

      /* SIZEOF expression is special */
    case SIZEOF_EXPR:
      return integer_type;

    case ADDR_EXPR:
      if(check_location(TREE_EXPR_OPERAND(tree,0)))
	return build_pointer_type(1,eval_expr_type(TREE_EXPR_OPERAND(tree,0)));
      else
	return error_type;

    case UMINUS_EXPR:
    case UPLUS_EXPR:
    case LOGICAL_NOT_EXPR:
      op0 = eval_expr_type(TREE_EXPR_OPERAND(tree,0));
      if(check_int(op0))
	return op0;
      else
	return error_type;

    case ARRAY_REF_EXPR:
      op0 = eval_expr_type(TREE_EXPR_OPERAND(tree,0));
      if(TYPE(op0)->code == ARRAY_TYPE)
	return TYPE(op0)->info.a.type;
      else
	return error_type;

    case INDIRECT_REF_EXPR:
      op0 = eval_expr_type(TREE_EXPR_OPERAND(tree,0));
      if(TYPE(op0)->code == POINTER_TYPE)
	return TYPE(op0)->info.reftype;
      else
	return error_type;

    case MEMBER_EXPR:
      op0 = eval_expr_type(TREE_EXPR_OPERAND(tree,0));
      if(check_for_member(op0,TREE_EXPR_OPERAND(tree,1),&op2))
	return op2;
      else
	return error_type;

    case CALL_EXPR:
      op0 = eval_expr_type(TREE_EXPR_OPERAND(tree,0));
      if( check_param_list(TREE_EXPR_OPERAND(tree,0),TREE_EXPR_OPERAND(tree,1)) )
	return op0;
      else
	return error_type;

    case BAD_EXPR:
      return error_type;

    case COMMA_EXPR:
      {
	c_tree chain = TREE_EXPR_OPERAND(tree,0);
	c_tree prev = chain;
	while(chain) {
	  prev = chain;
	  chain = CHAIN_NEXT(chain);
	}	
	op0 = eval_expr_type(CHAIN_VALUE(prev));
	return op0;
      }      
    default:
      return error_type;
    }

  return error_type;
}

c_tree get_pointed_to_type(c_tree t)
{
  switch( TYPE(t)->code )
    {
    case POINTER_TYPE:
      return TYPE(t)->info.reftype;
    case ARRAY_TYPE:
      return TYPE(t)->info.a.type;
    default:
      return NULL;
    }
}
