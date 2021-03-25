#include "c-tree.h"
#include "c-parse.h"
#include "c-typecheck.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char * tree_code_name[] = 
  {
    "TREE_INTEGER_CST",
    "TREE_IDENTIFIER",
    "TREE_CHAIN",
    "TREE_EXPR",
    "TREE_EXPR_LIST",
    "TREE_WHILE",
    "TREE_FOR",
    "TREE_IFELSE",
    "TREE_EXPRSTMT",
    "TREE_COMPOUND",
    "TREE_RETURN",
    "TREE_TYPE",
    "TREE_DECL",
    "TREE_DECL_LIST",
    "TREE_STMT_LIST",
    "TREE_LAST_CODE",
  };
#define TREE_CODE_NAME(code) tree_code_name[(int)code]

const char * expr_type_name[] = 
  {
  "ASSIGN_EXPR",
  "ADD_EXPR",
  "SUBTRACT_EXPR",
  "MULTIPLY_EXPR",
  "DIVIDE_EXPR",
  "MODULO_EXPR",
  "LOGICAL_AND_EXPR",
  "LOGICAL_GT_EXPR",
  "LOGICAL_GTE_EXPR",
  "LOGICAL_LT_EXPR",
  "LOGICAL_LTE_EXPR",
  "LOGICAL_OR_EXPR",
  "LOGICAL_EQ_EXPR",
  "LOGICAL_NEQ_EXPR",
  "SIZEOF_EXPR",
  "ADDR_EXPR",
  "UMINUS_EXPR",
  "UPLUS_EXPR",
  "LOGICAL_NOT_EXPR",
  "ARRAY_REF_EXPR",
  "INDIRECT_REF_EXPR",
  "MEMBER_EXPR",
  "CALL_EXPR",
  "BAD_EXPR",
  "COMMA_EXPR",
  "LAST_EXPR_TYPE",
  };
#define EXPR_TYPE_NAME(type) expr_type_name[(int)type]

const char * type_code_name[] = 
  {
    "BASIC_TYPE",
    "ARRAY_TYPE",
    "POINTER_TYPE",
    "STRUCT_TYPE",
    "FUNCTION_TYPE",
    "ERROR_TYPE",
  };
#define TYPE_CODE_NAME(code) type_code_name[(int)code]

const char * decl_type_name[] = 
  {
    "VAR_DECL",
    "FUNCTION_DECL",
    "ARRAY_DECL",
  };
#define DECL_TYPE_NAME(t) decl_type_name[(int)t]

void c_tree_print(FILE *fptr, c_tree t, int nesting, const char * p)
{
  char * prefix = (char *) malloc( nesting + strlen(p) + 1 );
  int i=0;

  fprintf(fptr,"\n");

  for(i=0; i<nesting; i++) 
    {
      prefix[i] = '\t';
    }
  //strncpy(prefix+i,p,strlen(p));
  prefix[nesting] = '\0';
  
  switch(TREE_CODE(t))
    {
    case TREE_INTEGER_CST:
      fprintf(fptr,"%s<%s val=%d> ",prefix,TREE_CODE_NAME(TREE_CODE(t)), INTEGER_CST(t)->val);
      break;
    case TREE_IDENTIFIER:
      fprintf(fptr,"%s<%s id=%s> ",prefix,TREE_CODE_NAME(TREE_CODE(t)), IDENTIFIER(t)->id);
      break;
    case TREE_CHAIN:
      fprintf(fptr,"%s<%s> ",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      while(t)
	{
	    c_tree_print(fptr,CHAIN_VALUE(t),nesting+1,"");
	    t = CHAIN_NEXT(t);
	}
      fprintf(fptr,"\n%s</%s> ",prefix,TREE_CODE_NAME(TREE_CHAIN));
      break;
    case TREE_EXPR:
      fprintf(fptr,"%s<%s:%s> ",prefix,TREE_CODE_NAME(TREE_CODE(t)), EXPR_TYPE_NAME(EXPR_TYPE(t)));
      switch(EXPR_TYPE(t))
	{
	case ASSIGN_EXPR:
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
	case COMMA_EXPR:
	  c_tree_print(fptr,TREE_EXPR_OPERAND(t,0),nesting+1,"Op 0:");
	  c_tree_print(fptr,TREE_EXPR_OPERAND(t,1),nesting+1,"Op 1:");
	  break;
	case CALL_EXPR:
	  {
	    c_tree fndecl = TREE_EXPR_OPERAND(t,0);	    
	    c_tree_print(fptr,DECL(fndecl)->id,nesting+1,"Op 0:");
	    c_tree_print(fptr,TREE_EXPR_OPERAND(t,1),nesting+1,"Op 1:");	  
	  }
	  break;
	case SIZEOF_EXPR:
	case ADDR_EXPR:
	case UMINUS_EXPR:
	case UPLUS_EXPR:
	case LOGICAL_NOT_EXPR:
	case ARRAY_REF_EXPR:
	case INDIRECT_REF_EXPR:
	case MEMBER_EXPR:
	  c_tree_print(fptr,TREE_EXPR_OPERAND(t,0),nesting+1,"Op 0:");
	  break;	  
	case BAD_EXPR:
	default:
	  fprintf(fptr,"\n");
	  break;
	}
      fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      break;
    case TREE_EXPR_LIST:
      fprintf(fptr,"%s<%s> ",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      if(EXPR_LIST(t)->chain)
	c_tree_print(fptr,EXPR_LIST(t)->chain,nesting,"");
      fprintf(fptr,"\n%s</%s> ",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      break;
    case TREE_WHILE:
      fprintf(fptr,"%s<%s do=%d> ",prefix,TREE_CODE_NAME(TREE_CODE(t)), WHILE_STMT(t)->dowhile);
      c_tree_print(fptr,WHILE_STMT(t)->cond,nesting+1,"Cond: ");
      c_tree_print(fptr,WHILE_STMT(t)->stmt,nesting+1,"Stmt: ");
      fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      break;
    case TREE_FOR:
      fprintf(fptr,"%s<%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      c_tree_print(fptr,FORLOOP_STMT(t)->init_expr,nesting+1,"Init: ");
      c_tree_print(fptr,FORLOOP_STMT(t)->cond_expr,nesting+1,"Cond: ");
      c_tree_print(fptr,FORLOOP_STMT(t)->reinit_expr,nesting+1,"Re-Init: ");
      c_tree_print(fptr,FORLOOP_STMT(t)->stmt,nesting+1,"Stmt: ");
      fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      break;    
    case TREE_IFELSE:
      fprintf(fptr,"%s<%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      c_tree_print(fptr,IFELSE_STMT(t)->cond,nesting+1,"Cond: ");
      c_tree_print(fptr,IFELSE_STMT(t)->ifstmt,nesting+1,"If: ");
      if(IFELSE_STMT(t)->elsestmt)
	c_tree_print(fptr,IFELSE_STMT(t)->elsestmt,nesting+1,"Else: ");
      fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      break;
    case TREE_EXPRSTMT:
      fprintf(fptr,"%s<%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      c_tree_print(fptr,EXPR_STMT(t)->expr,nesting+1,"");
      fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      break;
    case TREE_COMPOUND:
      fprintf(fptr,"%s<%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      if(COMPOUND_STMT(t)->decllist)
	c_tree_print(fptr,COMPOUND_STMT(t)->decllist,nesting+1,"Decl:");
      if(COMPOUND_STMT(t)->stmtlist)
	c_tree_print(fptr,COMPOUND_STMT(t)->stmtlist,nesting+1,"Stmt:");
      fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      break;
    case TREE_RETURN:
      fprintf(fptr,"%s<%s ",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      if(RETURN_STMT(t)->expr)
	{
	  c_tree_print(fptr,RETURN_STMT(t)->expr,nesting+1,"");
	  fprintf(fptr,"%s>",prefix);
	}
      else
	{
	  fprintf(fptr,"> ");
	}
      break;
    case TREE_TYPE:
      fprintf(fptr,"%s<%s %s ",prefix,TREE_CODE_NAME(TREE_CODE(t)), TYPE_CODE_NAME(TYPE(t)->code));
      switch( TYPE(t)->code )
	{
	case BASIC_TYPE:
	  fprintf(fptr,"typename=%s>",TYPE(t)->type_name);
	  break;
	case ARRAY_TYPE:
	  fprintf(fptr,"typename=%s",TYPE(t)->type_name);	  
	  if(TYPE(t)->info.a.type)
	    c_tree_print(fptr,TYPE(t)->info.a.type,nesting+1,"type:");
	  fprintf(fptr,"\n%s\tlength=%d\n",prefix,TYPE(t)->info.a.length);
	  fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
	  break;
	case POINTER_TYPE:
	  fprintf(fptr,"typename=%s",TYPE(t)->type_name);	  
	  if(TYPE(t)->info.reftype)
	    c_tree_print(fptr,TYPE(t)->info.reftype,nesting+1,"type:");
	  fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));      
	  break;
	case STRUCT_TYPE:
	  fprintf(fptr,"typename=%s",TYPE(t)->type_name);	  
	  if(TYPE(t)->info.s.fields)
	    c_tree_print(fptr,TYPE(t)->info.s.fields,nesting+1,"fields:");
	  fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));      
	  break;
	case FUNCTION_TYPE:
	  fprintf(fptr,"typename=%s",TYPE(t)->type_name);	  
	  c_tree_print(fptr,TYPE(t)->info.f.ret_type,nesting+1,"ret:");
	  c_tree_print(fptr,TYPE(t)->info.f.paramlist,nesting+1,"params:");
	  fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
	  break;
	default:
	  break;
	}
      break;
    case TREE_DECL:
      fprintf(fptr,"%s<%s:%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)),DECL_TYPE_NAME(DECL(t)->type));
      c_tree_print(fptr,DECL(t)->id,nesting+1,"name: ");
      c_tree_print(fptr,TREE_TYPE(t),nesting+1,"type: ");
      switch( DECL(t)->type )
	{
	case VAR_DECL:
	  fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
	  break;
	case FUNCTION_DECL:
	  c_tree_print(fptr,DECL(t)->func.param_decl,nesting+1,"params: ");
	  if(DECL(t)->func.compound_stmt)
	    c_tree_print(fptr,DECL(t)->func.compound_stmt,nesting+1,"params: ");
	  fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
	  break;
	case ARRAY_DECL:
	  fprintf(fptr,"\n%s</%s>",prefix,TREE_CODE_NAME(TREE_CODE(t)));
	  if(DECL(t)->array.length)
	    c_tree_print(fptr,DECL(t)->array.length,nesting+1,"length: ");
	  break;	  
	default:
	  break;
	}
      break;
    case TREE_DECL_LIST:
      if(DECL_LIST(t)->head) 
	{
	  fprintf(fptr,"%s<%s>  ",prefix,TREE_CODE_NAME(TREE_CODE(t)));
	  c_tree_print(fptr,DECL_LIST(t)->head,nesting+1,"");
	  fprintf(fptr,"\n%s</%s>  ",prefix,TREE_CODE_NAME(TREE_CODE(t)));
	} 
      else
	fprintf(fptr,"%s<%s (NULL)>  ",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      break;
    case TREE_STMT_LIST:
      if(STMT_LIST(t)->head) 
	{
	  fprintf(fptr,"%s<%s>  ",prefix,TREE_CODE_NAME(TREE_CODE(t)));
	  c_tree_print(fptr,STMT_LIST(t)->head,nesting+1,"");
	  fprintf(fptr,"\n%s</%s>  ",prefix,TREE_CODE_NAME(TREE_CODE(t)));
	} 
      else
	fprintf(fptr,"%s<%s (NULL)>  ",prefix,TREE_CODE_NAME(TREE_CODE(t)));
      break;
    default:
      break;
    }

  //  fprintf(fptr,"\n");
  free(prefix);
}
