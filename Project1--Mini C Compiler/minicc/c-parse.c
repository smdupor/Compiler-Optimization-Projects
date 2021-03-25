#include "c-tree.h"
#include "cmdline.h"
#include "stdlib.h"
#include "c-parse.h"
#include "c-typecheck.h"
#include "string.h"
#include "c-symbol.h"
#include "c-eval.h"
#include "c-codegen.h"

extern void yyparse();
extern int yywrap();
extern void set_locus(locus *);

symbol_table_t global_symbols;
c_tree current_fndecl;

c_tree integer_type;
c_tree void_type;
c_tree error_type;

void c_init()
{
  c_typecheck_init();  
  global_symbols = symbol_table_push();

  /*
    Add basic types
   */
  type_info info;
  integer_type = build_type(BASIC_TYPE,"int",info);
  void_type = build_type(BASIC_TYPE,"void",info);
  error_type = build_type(ERROR_TYPE,"error",info);

  /* 
     Add some global symbols for use by the programs.
   */
  
  symbol_table_add_entry(build_var_decl(build_identifier("cin"),integer_type));
  symbol_table_add_entry(build_var_decl(build_identifier("cout"),integer_type));


  register_type(integer_type);
}

void c_parse()
{
  c_init();
  c_codegen_entry();

  yywrap();
  yyparse();

  c_codegen_exit();
}


c_tree build_tree()
{
  c_tree t = (c_tree) malloc( sizeof(*t) );
  memset(t,0,sizeof(*t));
  set_locus(&t->common.loc);
  return t;  
}
/* used to build the c_tree for an int_cst called from minic.y*/
c_tree build_int_cst(int val)
{
  c_tree t = build_tree();
  t->common.code = TREE_INTEGER_CST;
  t->common.type = integer_type;
  t->int_cst.val = val;
  return t;
}

/* used to build the c_tree for an identifier called from minic.y*/
c_tree build_identifier(const char *id)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_IDENTIFIER;
  TREE_TYPE(t) = NULL;
  t->id.id = strdup(id);
  return t;
}

/* used to build the c_tree for an int_cst  called from minic.y*/
c_tree build_chain(c_tree value)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_CHAIN;
  TREE_TYPE(t) = NULL;
  t->chain.value = value;
  t->chain.chain = NULL;
  return t;
}

/* used to build the c_tree for an expr1  called from minic.y*/
c_tree build_expr1(enum expr_type type, c_tree op1)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_EXPR;
  t->expr.type = type;
  TREE_EXPR_OPERAND(t,0) = op1;

  TREE_TYPE(t) = eval_expr_type(t);

  return t; 
}

/* used to build the c_tree for an expr2  called from minic.y*/
c_tree build_expr2(enum expr_type type, c_tree op1, c_tree op2)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_EXPR;
  t->expr.type = type;
  TREE_EXPR_OPERAND(t,0) = op1;
  TREE_EXPR_OPERAND(t,1) = op2;
  TREE_TYPE(t) = eval_expr_type(t);
  return t; 
}

/* used to build the c_tree for an expr3 called from minic.y*/
c_tree build_expr3(enum expr_type type, c_tree op1, c_tree op2, c_tree op3)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_EXPR;
  t->expr.type = type;
  TREE_EXPR_OPERAND(t,0) = op1;
  TREE_EXPR_OPERAND(t,1) = op2;
  TREE_EXPR_OPERAND(t,2) = op3;

  TREE_TYPE(t) = eval_expr_type(t);
  return t; 
}

/* used to build the c_tree for an expr_list called from minic.y*/
c_tree build_expr_list()
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_EXPR_LIST;
  TREE_TYPE(t) = void_type;
  t->expr_list.chain = NULL;
  t->expr_list.tail = NULL;
  return t; 
}

/* used to build the c_tree for a while statement called from minic.y*/
c_tree build_while(c_tree cond, c_tree stmt, int dowhile)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_WHILE;
  TREE_TYPE(t) = void_type;

  t->dowhile.dowhile = dowhile; /* 0 while loop, 1 do-while loop */
  t->dowhile.cond = cond;
  t->dowhile.stmt = stmt;
  if(!check_int_expression(cond))
    {
      parser_error("Conditional expression in while is not an integer.");
      num_errors++;
    }
  return t; 
}

/* used to build the c_tree for a for statement called from minic.y*/
c_tree build_for(c_tree icond, c_tree cond, c_tree rcond, c_tree stmt)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_FOR;

  t->forloop.init_expr = icond;
  t->forloop.cond_expr = cond;
  t->forloop.reinit_expr = rcond;
  t->forloop.stmt = stmt;

  if(!check_int_expression(cond))
    {
      parser_error("Conditional expression in for-loop is not an integer.");
      num_errors++;
    }

  TREE_TYPE(t) = void_type;

  return t; 
}

/* used to build the c_tree for an if/else statement called from minic.y*/
c_tree build_ifelse(c_tree cond, c_tree ifstmt, c_tree elsestmt)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_IFELSE;

  t->ifelse.cond = cond;
  t->ifelse.ifstmt = ifstmt;
  t->ifelse.elsestmt = elsestmt;

  if(!check_int_expression(cond))
    {
      parser_error("Conditional expression in if-stmt is not an integer.");
      num_errors++;
    }

  TREE_TYPE(t) = void_type;

  return t; 
}

/* used to build the c_tree for an exprstmt called from minic.y*/
c_tree build_exprstmt(c_tree expr)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_EXPRSTMT;
  TREE_TYPE(t) = void_type;

  t->exprstmt.expr = expr;

  return t; 
}


c_tree build_break_stmt()
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_BREAK;
  TREE_TYPE(t) = void_type;
  return t; 
}

c_tree build_continue_stmt()
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_CONTINUE;
  TREE_TYPE(t) = void_type;
  return t; 
}

/* used to build the c_tree for a compound_stmt called from minic.y*/

c_tree build_compound_stmt(c_tree decllist, c_tree stmtlist)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_COMPOUND;
  TREE_TYPE(t) = void_type;

  t->compound.decllist = decllist;
  t->compound.stmtlist = stmtlist;

  return t; 
}

/* used to build the c_tree for a return_stmt called from minic.y*/
c_tree build_return_stmt(c_tree expr)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_RETURN;
  TREE_TYPE(t) = NULL;

  t->ret.expr = expr;

  c_tree type = fold_types( TREE_TYPE(current_fndecl), eval_expr_type(expr));
  if(type)
    TREE_TYPE(t) = type;
  else
    {
      parser_error("Return statement has the wrong type.");
      num_errors++;
      TREE_TYPE(t) = error_type;
    }

  DECL(current_fndecl)->func.ret_cnt++;

  return t; 
}

/* used to build the c_tree for a type of union called from minic.y*/
c_tree build_type(enum type_code code, char *typename, type_info info)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_TYPE;
  TREE_TYPE(t) = NULL;

  t->type.code = code;
  if(typename)
    t->type.type_name = strdup(typename);
  else
    t->type.type_name = NULL;

  t->type.info = info;

  if( code == ARRAY_TYPE && info.a.length<0 )
    {
      parser_error("Array length cannot be negative!");
    }

  t->type.uid  = register_type(t);

  return t;
}

/* used to build the c_tree for a pointer types called from minic.y*/
c_tree build_pointer_type(int cnt, c_tree type)
{
  type_info info;
  c_tree t;

  t = type;

  while(cnt--) 
    {
      info.reftype = t;  
      t = build_type(POINTER_TYPE,create_pointer_name(t),info);
    }

  return t;
}

/* used to build the c_tree for decl called from minic.y*/
c_tree build_decl(enum decl_type decl)
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_DECL;
  TREE_TYPE(t) = NULL;
  DECL(t)->type = decl;
  return t; 
}

/* used to build the c_tree for a function declaration called from minic.y*/
c_tree build_function_decl(c_tree id, c_tree type, c_tree params, c_tree compound)
{
  c_tree f = build_decl(FUNCTION_DECL);
  TREE_TYPE(f) = type;
  DECL(f)->id = id;
  DECL(f)->func.param_decl = params;
  DECL(f)->func.compound_stmt = compound;
  return f;
}

/* used to build the c_tree for a variable declaration called from minic.y*/
c_tree build_var_decl(c_tree id, c_tree type)
{
  c_tree t = build_decl(VAR_DECL);
  TREE_TYPE(t) = type;

  DECL(t)->id = id;
  return t; 
}

/* used to build the c_tree for an array declaration called from minic.y*/
c_tree build_array_decl(c_tree id, c_tree type, c_tree length)
{
  c_tree t = build_decl(ARRAY_DECL);

  type_info info;
  info.a.type = type;
  info.a.length=0;
  get_constant_value(length,&info.a.length);
  if(type)
    TREE_TYPE(t) = build_type(ARRAY_TYPE,create_array_name(type,info.a.length),info);
  else
    TREE_TYPE(t) = NULL;

  if(info.a.length==0)
    {
      parser_error("Invalid constant in array declaration.");
      num_errors++;
    }

  DECL(t)->id = id;
  DECL(t)->type = ARRAY_DECL;  
  DECL(t)->array.type = type;
  DECL(t)->array.length = length;
  return t; 
}

/* used to build the c_tree for a declaration list called from minic.y*/
c_tree build_decl_list()
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_DECL_LIST;
  TREE_TYPE(t) = NULL;

  t->decl_list.head = NULL;
  t->decl_list.tail = NULL;

  return t; 
}

/* used to build the c_tree for a statement list called from minic.y*/
c_tree build_stmt_list()
{
  c_tree t = build_tree();
  TREE_CODE(t) = TREE_STMT_LIST;
  TREE_TYPE(t) = void_type;

  t->stmt_list.head = NULL;
  t->stmt_list.tail = NULL;

  return t; 
}
