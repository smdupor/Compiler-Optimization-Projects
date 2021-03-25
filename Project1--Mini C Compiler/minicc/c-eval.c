#include "c-tree.h"
#include "c-parse.h"

int get_constant_value(c_tree expr, int *res)
{
  if(TREE_CODE(expr)==TREE_EXPR)
    {
      int op0, op1;
      switch(EXPR_TYPE(expr)){
      case ADD_EXPR:
	if(get_constant_value(TREE_EXPR_OPERAND(expr,0),&op0) &&
	   get_constant_value(TREE_EXPR_OPERAND(expr,1),&op1))
	  {
	    *res = op0 + op1;
	    return 1;
	  }
	break;
      case SUBTRACT_EXPR:
	if(get_constant_value(TREE_EXPR_OPERAND(expr,0),&op0) &&
	   get_constant_value(TREE_EXPR_OPERAND(expr,1),&op1))
	  {
	    *res = op0 - op1;
	    return 1;
	  }
	break;
      case MULTIPLY_EXPR:
	if(get_constant_value(TREE_EXPR_OPERAND(expr,0),&op0) &&
	   get_constant_value(TREE_EXPR_OPERAND(expr,1),&op1))
	  {
	    *res = op0 * op1;
	    return 1;
	  }
	break;
      case DIVIDE_EXPR:
	if(get_constant_value(TREE_EXPR_OPERAND(expr,0),&op0) &&
	   get_constant_value(TREE_EXPR_OPERAND(expr,1),&op1))
	  {
	    *res = op0 / op1;
	    return 1;
	  }
	break;
      case MODULO_EXPR:
	if(get_constant_value(TREE_EXPR_OPERAND(expr,0),&op0) &&
	   get_constant_value(TREE_EXPR_OPERAND(expr,1),&op1))
	  {
	    *res = op0 % op1;
	    return 1;
	  }
	break;
      case UMINUS_EXPR:
	if(get_constant_value(TREE_EXPR_OPERAND(expr,0),&op0))	  {
	    *res = -op0;
	    return 1;
	  }
	break;
      case UPLUS_EXPR:
	if(get_constant_value(TREE_EXPR_OPERAND(expr,0),&op0))	  {
	    *res = op0;
	    return 1;
	  }
	break;
      default:
	return 0;
      }
    }
  else if(TREE_CODE(expr)==TREE_INTEGER_CST)
    {
      *res = INTEGER_CST(expr)->val;
      return 1;
    }
  
  return 0;
}
