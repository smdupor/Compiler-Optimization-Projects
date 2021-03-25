#ifndef C_TREE_H
#define C_TREE_H

/* Here you will find all the declarations for stucts, enumerations used by c_tree*/
typedef struct locus_def {
  int line_no;
  const char * filename;
} locus;

union tree_union;
typedef union tree_union * c_tree;

enum tree_code {

  TREE_INTEGER_CST=0,
  TREE_IDENTIFIER,
  TREE_CHAIN,
  TREE_EXPR,
  TREE_EXPR_LIST,
  TREE_WHILE,
  TREE_FOR,
  TREE_IFELSE,
  TREE_EXPRSTMT,
  TREE_COMPOUND,
  TREE_RETURN,
  TREE_TYPE,
  TREE_DECL,
  TREE_DECL_LIST,
  TREE_STMT_LIST,
  TREE_BREAK,
  TREE_CONTINUE,
  TREE_LAST_CODE
};

struct c_tree_common
{
  enum  tree_code code;
  c_tree  type;
  locus loc;
};

#define TREE_CODE(t) t->common.code
#define TREE_TYPE(t) t->common.type

struct c_tree_empty
{
  struct c_tree_common common;
};

struct c_tree_int_cst
{
  struct c_tree_common common;
  int    val;
};

struct c_tree_identifier
{
  struct c_tree_common common;
  const char*  id;
};

struct c_tree_chain
{
  struct c_tree_common common;
  c_tree chain;
  c_tree value;
};

#define TREE_CHAIN(t) t->chain.chain
#define TREE_CHAIN_VALUE(t) t->chain.value
#define CHAIN_NEXT(t) t->chain.chain
#define CHAIN_VALUE(t) t->chain.value

enum expr_type {
  ASSIGN_EXPR,
  ADD_EXPR,
  SUBTRACT_EXPR,
  MULTIPLY_EXPR,
  DIVIDE_EXPR,
  MODULO_EXPR,
  LOGICAL_AND_EXPR,
  LOGICAL_GT_EXPR,
  LOGICAL_GTE_EXPR,
  LOGICAL_LT_EXPR,
  LOGICAL_LTE_EXPR,
  LOGICAL_OR_EXPR,
  LOGICAL_EQ_EXPR,
  LOGICAL_NEQ_EXPR,
  SIZEOF_EXPR,
  ADDR_EXPR,
  UMINUS_EXPR,
  UPLUS_EXPR,
  LOGICAL_NOT_EXPR,
  ARRAY_REF_EXPR,
  INDIRECT_REF_EXPR,
  MEMBER_EXPR,
  CALL_EXPR,
  BAD_EXPR,
  COMMA_EXPR,
  LAST_EXPR_TYPE
};

#define EXPR_TYPE(t) (t->expr.type)

struct c_tree_expr
{
  struct c_tree_common common;
  enum expr_type type;
  c_tree ops[3];
};

#define TREE_EXPR_OPERAND(t,n) t->expr.ops[n]

struct c_tree_expr_list
{
  struct c_tree_common common;
  c_tree chain;
  c_tree tail;
};
 /* different tree types depending on which expression it maps to*/
struct c_tree_while 
{
  struct c_tree_common common;
  int    dowhile;
  c_tree cond;
  c_tree stmt;
};

struct c_tree_for
{
  struct c_tree_common common;
  c_tree init_expr;
  c_tree cond_expr;
  c_tree reinit_expr;
  c_tree stmt;
};

struct c_tree_ifelse
{
  struct c_tree_common common;
  c_tree cond;
  c_tree ifstmt;
  c_tree elsestmt;
};

struct c_tree_exprstmt
{
  struct c_tree_common common;
  c_tree expr;
};

struct c_tree_compound_stmt
{
  struct c_tree_common common;
  c_tree decllist;
  c_tree stmtlist; /* use tree chain */
};

struct c_tree_return_stmt
{
  struct c_tree_common common;
  c_tree expr;
};

enum type_code {
  BASIC_TYPE,
  ARRAY_TYPE,
  POINTER_TYPE,
  STRUCT_TYPE,
  FUNCTION_TYPE,
  ERROR_TYPE
};

struct array_info {
  int length;
  c_tree type;
};

struct struct_info {
  c_tree fields;
};

struct func_info {
  c_tree ret_type;
  c_tree paramlist;
};

typedef union {
    c_tree reftype;
    struct array_info  a;
    struct struct_info s;
    struct func_info   f;
} type_info;

struct c_tree_type
{
  struct c_tree_common common;
  char   *type_name;
  int     uid;
  enum   type_code code;

  type_info info;
};

enum decl_type
  {
    VAR_DECL,
    FUNCTION_DECL,
    ARRAY_DECL,
  };

struct func_decl {
  c_tree param_decl;
  c_tree compound_stmt;
  int    ret_cnt;
};

struct array_decl {
  c_tree type;
  c_tree length;
};

#define CHECK_TREE_DECL(t) ( TREE_CODE(t) == TREE_DECL )

#define TREE_DECL_TYPE(t) t->decl.type

struct c_tree_decl
{
  struct c_tree_common common;
  enum decl_type type;
  c_tree id;

  union {
    struct func_decl  func;
    struct array_decl array;
  };
};

struct c_tree_list
{
  struct c_tree_common common;
  c_tree head;
  c_tree tail;
};

#ifdef DEBUG
inline tree tree_check_error(tree t, enum tree_code code) 
{
  if( TREE_CODE(t) == code )
    return t;
  else
    {
      fprintf(stderr,"Cast does not match tree code.");
      exit(-1);
    }
  return NULL;
}
#define CHECK(t,code)   (tree_check_error(t,code))
#else
#define CHECK(t,code)   ((int)(t))
#endif

#define TREE_CODE_CHECK(t,code) (TREE_CODE(t)==code)

#define INTEGER_CST(t)  ((struct c_tree_int_cst *) ( CHECK(t,TREE_INTEGER_CST) ))
#define IDENTIFIER(t)   ((struct c_tree_identifier *) ( CHECK(t,TREE_IDENTIFIER) ))
#define CHAIN(t)        ((struct c_tree_chain *)( CHECK(t,TREE_CHAIN) ))
#define EXPR(t)         ((struct c_tree_expr *)( CHECK(t,TREE_EXPR) ))
#define EXPR_LIST(t)    ((struct c_tree_expr_list *)( CHECK(t,TREE_EXPR_LIST) ))
#define WHILE_STMT(t)   ((struct c_tree_while *)( CHECK(t,TREE_WHILE) ))
#define DOWHILE_STMT(t) ((struct c_tree_while *)( CHECK(t,TREE_WHILE) ))
#define FORLOOP_STMT(t) ((struct c_tree_for *)( CHECK(t,TREE_FOR) ))
#define IFELSE_STMT(t)  ((struct c_tree_ifelse *)( CHECK(t,TREE_IFELSE) ))
#define EXPR_STMT(t)    ((struct c_tree_exprstmt *)( CHECK(t,TREE_EXPRSTMT) ))
#define BREAK_STMT(t)    ((struct c_tree_empty *)( CHECK(t,TREE_BREAK) ))
#define CONTINUE_STMT(t)    ((struct c_tree_empty *)( CHECK(t,TREE_CONTINUE) ))
#define COMPOUND_STMT(t)((struct c_tree_compound_stmt *)( CHECK(t,TREE_COMPOUND) ))
#define RETURN_STMT(t)  ((struct c_tree_return_stmt *)( CHECK(t,TREE_RETURN) ))
#define TYPE(t)         ((struct c_tree_type *)( CHECK(t,TREE_TYPE) ))
#define DECL(t)         ((struct c_tree_decl*)( CHECK(t,TREE_DECL) ))
#define DECL_LIST(t)    ((struct c_tree_list *)( CHECK(t,TREE_DECL_LIST) ))
#define STMT_LIST(t)    ((struct c_tree_list *)( CHECK(t,TREE_STMT_LIST) ))

/* these are the all the different types of tree nodes you can have*/

union tree_union {
  struct c_tree_common     common;
  struct c_tree_int_cst    int_cst;
  struct c_tree_identifier id;
  struct c_tree_chain      chain;
  struct c_tree_expr       expr;
  struct c_tree_expr_list  expr_list;
  struct c_tree_while      dowhile;
  struct c_tree_for        forloop;
  struct c_tree_ifelse     ifelse;
  struct c_tree_exprstmt   exprstmt;
  struct c_tree_compound_stmt compound;
  struct c_tree_return_stmt ret;
  struct c_tree_type        type;
  struct c_tree_decl        decl;
  struct c_tree_empty       break_stmt;
  struct c_tree_empty       continue_stmt;
  struct c_tree_list        decl_list;
  struct c_tree_list        stmt_list;
};

extern c_tree global_decl;
extern c_tree current_fdecl;

/* Other Helpful Macros */

#define DECL_NAME_STRING(d) (IDENTIFIER(DECL(d)->id)->id)
#define DECL_NAME_TREE(d) (DECL(d)->id)

#endif
