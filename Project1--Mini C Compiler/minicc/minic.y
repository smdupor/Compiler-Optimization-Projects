%{
#include <stdio.h>
#include <stdlib.h>
#include "c-tree.h"
#include "c-parse.h"
#include "c-typecheck.h"
#include "c-symbol.h"
#include "c-optimize.h"

int num_errors;
extern c_tree integer_type;

extern c_tree void_type;
extern c_tree error_type;
extern int yylex();   /* lexical analyzer generated from lex.l */

int yyerror();
int parser_error(const char*);
 void minic_abort();
extern void c_tree_print(FILE *fptr, c_tree t, int nesting, const char * p);
 char *get_filename();
 int get_lineno();

extern  c_tree current_fndecl;
 int loops_found=0;
%}

/* Data structure for tree nodes*/

%union {
  int cnt;
  int num;
  char * id;
  c_tree tree;
  enum expr_type exp;
  enum type_code tcode;
}

/* these tokens are simply their corresponding int values, more terminals*/

%token SEMICOLON COMMA COLON
%token LBRACE RBRACE LPAREN RPAREN LBRACKET RBRACKET
%token ASSIGN PLUS MINUS STAR DIV MOD 
%token LT GT LTE GTE EQ NEQ NOT
%token LOGICAL_AND LOGICAL_OR
%token BITWISE_OR BITWISE_XOR LSHIFT RSHIFT BITWISE_INVERT

%token DOT ARROW AMPERSAND QUESTION_MARK

%token FOR WHILE IF ELSE DO STRUCT SIZEOF RETURN 
%token BREAK CONTINUE
%token INT VOID

/* no meaning, just placeholders */
%token STATIC AUTO EXTERN TYPEDEF CONST VOLATILE ENUM UNION REGISTER
/* NUMBER and ID have values associated with them returned from lex*/

%token <num> NUMBER /*data type of NUMBER is num union*/
%token <id>  ID

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

/* values created by parser*/

%type <tree> external_declaration
%type <tree> function_definition
%type <tree> declaration 
%type <tree> type_specifier
%type <tree> struct_declaration
%type <tree> struct_declaration_list
%type <tcode> struct_or_union
%type <tree> struct_or_union_specifier
%type <tree> declarator
%type <tree> direct_decl
%type <cnt> pointer
%type <tree> param_list param_list_opt
%type <tree> param_decl
%type <tree> type_name
%type <tree> statement
%type <tree> iteration_stmt
%type <tree> expr_stmt
%type <tree> compound_stmt
%type <tree> declaration_list
%type <tree> declaration_list_opt
%type <tree> statement_list_opt
%type <tree> statement_list
%type <tree> selection_stmt
%type <tree> expr_opt
%type <tree> jump_stmt
%type <tree> expression
%type <tree> assignment_expression
%type <tree> conditional_expression
%type <tree> constant_expression
%type <tree> logical_OR_expression
%type <tree> logical_AND_expression
%type <tree> inclusive_OR_expression
%type <tree> exclusive_OR_expression
%type <tree> AND_expression
%type <tree> equality_expression
%type <tree> relational_expression
%type <tree> shift_expression
%type <tree> additive_expression
%type <tree> multiplicative_expression
%type <tree> cast_expression
%type <tree> unary_expression
%type <exp> unary_operator
%type <tree> postfix_expression
%type <tree> argument_expr_list_opt
%type <tree> argument_expr_list
%type <tree> primary_expression
%type <tree> constant
%type <exp> assignment_operator
%type <tree> break_stmt continue_stmt
/* 
   The grammar used here is largely borrowed from Kernighan and Ritchie's "The C
   Programming Language," 2nd Edition, Prentice Hall, 1988. 

   But, some modifications have been made specifically for MiniC!
 */

%%

/* 
   Beginning of grammar: Production Rules
   when you want to use the value returned by a token from a lower part of the tree you refer to it as $X, X is the position
   in the expression it is located in ie in this grammar "|type_specifier pointer direct_decl LPAREN param_list_opt RPAREN " 
   if you want to use the value returned by direct_decl you can refer to it as "$3" when you want to return a value you  
   place set $$ = to it. 
*/

translation_unit:	  external_declaration
			| translation_unit external_declaration
;

external_declaration:	  function_definition
{
  /* finish compiling function */
  if(num_errors>100)
    {
      minic_abort();
    }
  else if(num_errors==0)
    {
      c_optimize($$);
    }

    symbol_table_pop(); 
}
                        | declaration 
{ 
  /* nothing to be done here */
}
;

function_definition:	  type_specifier direct_decl LPAREN param_list_opt RPAREN 
{
  /* This is a mid-rule action */
  c_tree id = $2;                                         /*$2 is the value reduced from direct_decl*/

  if( !TREE_CODE_CHECK(id,TREE_IDENTIFIER) )
    {
      parser_error("Illegal function definition in MiniC. ");
      num_errors++;
    }

  if( !check_int($1) )
    {
      parser_error("Return value is not a pointer or integer. ");
      num_errors++;
    }

  $<tree>$ = build_function_decl(id,$1,$4,NULL);
  current_fndecl= $<tree>$;

  if(symbol_table_get_entry(IDENTIFIER($2)->id)==NULL)
    symbol_table_add_entry($<tree>$);

  /* create a new symbol table context for this function */
  symbol_table_push();

  if(DECL_LIST($4)->head)
    {
      /* add arguments to local symbol table */
      c_tree chain = DECL_LIST($4)->head;
      while(chain)
	{
	  if( !check_int( TREE_TYPE(CHAIN_VALUE(chain)) ) )
	    {
	      parser_error("Argument is not an integer or a pointer.");
	      num_errors++;
	    }

	  symbol_table_add_parameter(CHAIN_VALUE(chain));
	  chain = CHAIN_NEXT(chain);
	}
    }

} 
                          compound_stmt 
{ 
  /* this is the rest of the rule's actions */
  $$ = $<tree>6;  /* value returned from this expression*/
  DECL($$)->func.compound_stmt = $7;

  if( DECL($$)->func.ret_cnt == 0 )
    {
      parser_error("Function does not return a value.");
      num_errors++;
    }
}
                        | type_specifier pointer direct_decl LPAREN param_list_opt RPAREN 
{
  /* This is a mid-rule action */
  c_tree id = (c_tree)$3;
  c_tree tmp = build_pointer_type($2,$1);

  if( !TREE_CODE_CHECK(id,TREE_IDENTIFIER) )
    {
      parser_error("Illegal function definition in MiniC. ");
      num_errors++;
    }
  if( !check_int($1) )
    {
      parser_error("Return value is not a pointer or integer. ");
      num_errors++;
    }

  if(symbol_table_get_entry(IDENTIFIER($2)->id)==NULL)
    symbol_table_add_entry($<tree>$);

  $<tree>$ = build_function_decl(id,tmp,$5,NULL);
  current_fndecl= $<tree>$;

  /* create a new symbol table context for this function */
  symbol_table_push();
  if(DECL_LIST($5)->head)
    {
      /* add arguments to local symbol table */
      c_tree chain = DECL_LIST($5)->head;
      while(chain)
	{
	  if( !check_int( TREE_TYPE(CHAIN_VALUE(chain)) ) )
	    {
	      parser_error("Argument is not an integer or a pointer.");
	      num_errors++;
	    }

	  symbol_table_add_parameter(CHAIN_VALUE(chain));
	  chain = CHAIN_NEXT(chain);
	}
    }
} 
                          compound_stmt 
{ 
  /* this is the rest of the action */
  $$ = $<tree>7;
  DECL($$)->func.compound_stmt = $8;
  if( DECL($$)->func.ret_cnt == 0 )
    {
      parser_error("Function does not return a value.");
      num_errors++;
    }
}
;

declaration:    type_specifier pointer declarator SEMICOLON
{
  c_tree type = build_pointer_type($2,$1);
  normalize_type($3,type);
  symbol_table_add_entry($3);
  $$ = $3;
} 
              | type_specifier declarator SEMICOLON
{
  normalize_type($2,$1);
  symbol_table_add_entry($2);
  $$ = $2;
} 
              | type_specifier SEMICOLON
;

declaration_list:	   declaration
{
  c_tree list = build_decl_list();
  DECL_LIST(list)->head = build_chain($1);
  DECL_LIST(list)->tail = DECL_LIST(list)->head;
  $$ = list;
}
                         | declaration_list declaration  
{
  TREE_CHAIN(DECL_LIST($1)->tail) = build_chain($2);
  DECL_LIST($1)->tail = TREE_CHAIN(DECL_LIST($1)->tail);
  $$ = $1;
}
;



type_specifier:		  INT 
{
  $$ = integer_type;
}
			| struct_or_union_specifier 
{
  $$ = $1;
}
;


struct_or_union_specifier: struct_or_union ID LBRACE struct_declaration_list RBRACE
{
  type_info info;
  info.s.fields = $4;
  c_tree t = build_type($1,$2,info);
  $$ = (c_tree) t;
}
                        | struct_or_union LBRACE struct_declaration_list RBRACE
{
  type_info info;
  info.s.fields = $3;
  c_tree t = build_type($1,
			create_unique_struct_name(STRUCT_TYPE,get_filename()),
			info);
  register_type(t);
  $$ = (c_tree) t;
}
                        | struct_or_union ID
{
  type_info info;
  info.s.fields = NULL;
  c_tree t = build_type($1,$2,info);
  register_type(t);
  $$ = (c_tree) t;
}
;

struct_or_union:	  STRUCT    
{ 
  $$ = STRUCT_TYPE; 
}
                        | UNION     
{ 
  $$ = ERROR_TYPE; parser_error("union is not supported in MiniC."); 
}
;

struct_declaration_list:
			  struct_declaration
{
  c_tree list = build_decl_list();
  DECL_LIST(list)->head = build_chain($1);
  DECL_LIST(list)->tail = DECL_LIST(list)->head;
  $$ = list;
}
			| struct_declaration_list struct_declaration
{
  TREE_CHAIN(DECL_LIST($1)->tail) = build_chain($2);
  DECL_LIST($1)->tail = TREE_CHAIN(DECL_LIST($1)->tail);
  $$ = $1;

  //TREE_CHAIN($1->decl_list.tail) = $2->decl_list.head;
  //$1->decl_list.tail = $2->decl_list.tail;
  //$$ = $1;
}
;

struct_declaration:	  type_specifier pointer declarator SEMICOLON
{
  c_tree type = build_pointer_type($2,$1);
  normalize_type($3,type);
  $$ = $3;
}
                      |   type_specifier declarator SEMICOLON
{
  normalize_type($2,$1);
  $$ = $2;
}
;


declarator:		  direct_decl
{
  if( TREE_CODE_CHECK($1,TREE_DECL) )
    $$ = $1;
  else if( TREE_CODE_CHECK($1,TREE_IDENTIFIER) )
    $$ = build_var_decl($1,NULL);
}
;

direct_decl:		  ID                                   
{ 
  $$ = build_identifier(yyval.id);
}
			| direct_decl LBRACKET constant_expression RBRACKET
{
  c_tree id = $1;
  if( !TREE_CODE_CHECK(id,TREE_IDENTIFIER) )
    {
      parser_error("Illegal declaration in MiniC. ");
      num_errors++;
    }
  $$ = build_array_decl(id,NULL,$3);
}
			| direct_decl LBRACKET RBRACKET
{
  c_tree id = $1;
  if( !TREE_CODE_CHECK(id,TREE_IDENTIFIER) )
    {
      parser_error("Illegal declaration in MiniC. ");
      num_errors++;
    }
  $$ = build_array_decl(id,NULL,NULL);
}
                        | direct_decl LPAREN param_list_opt RPAREN
{
  c_tree id = $1;
  if( !TREE_CODE_CHECK(id,TREE_IDENTIFIER) )
    {
      parser_error("Illegal declaration in MiniC. ");
      num_errors++;
    }

  $$ = build_function_decl($1,NULL,$3,NULL);
}
;

pointer:		  STAR 
{
  $$ = 1;
}
			| STAR pointer
{
  $$ = $2 + 1;
}
;

param_list_opt:           
{ $$ = build_decl_list(); }
                        | param_list
{ $$ = $1; }
;

param_list:		  param_decl
{
  c_tree t = build_decl_list();
  t->decl_list.head = build_chain($1);
  t->decl_list.tail = t->decl_list.head;
  $$ = t;
}
			| param_list COMMA param_decl
{
  c_tree tmp = build_chain($3);
  TREE_CHAIN($1->decl_list.tail) = tmp;
  $1->decl_list.tail = tmp;
  $$ = $1;
}
;

param_decl:		  type_specifier pointer declarator
{
  c_tree type = build_pointer_type($2,$1);
  TREE_TYPE($3) = type;
  $$ = $3;
}
                        | type_specifier declarator
{
  TREE_TYPE($2) = $1;
  $$ = $2;
}
			| type_specifier
{
  $$ = build_decl(VAR_DECL);
  TREE_TYPE($$) = $1;
}
;

type_name:		 type_specifier                  { $$ = $1; }
;

statement:		  expr_stmt            
{ $$ = $1; }
			| compound_stmt        
{ $$ = $1; }
			| selection_stmt       
{ $$ = $1; }
			| iteration_stmt       
{ $$ = $1; }
			| jump_stmt            
{ $$ = $1; }
                        | break_stmt
{ $$ = $1; }
                        | continue_stmt
{ $$ = $1; }
;

expr_stmt:	           SEMICOLON            
{ 
  $$ = build_exprstmt(NULL);
}
			|  expression SEMICOLON       
{ 
  $$ = build_exprstmt($1);
}
;

compound_stmt:		  LBRACE declaration_list_opt statement_list_opt RBRACE 
{
  $$ = build_compound_stmt($2, $3);
}
;

declaration_list_opt:	
{
  $$ = build_decl_list();
}
			| declaration_list
{
  $$ = $1;
}
;

statement_list_opt:	
{
  $$ = (c_tree) build_stmt_list();
}
			| statement_list
{
  $$ = $1;
}
;

statement_list:		statement
{
  c_tree tmp = build_stmt_list();
  c_tree chain = build_chain($1);
  tmp->stmt_list.head = chain;
  tmp->stmt_list.tail = chain;
  $$ = (c_tree) tmp;
}
			| statement_list statement
{
  c_tree chain = build_chain($2);
  CHAIN_NEXT($1->stmt_list.tail) = chain;
  $1->stmt_list.tail = chain;
  $$ = $1;
}
;

break_stmt:               BREAK SEMICOLON
{
  if(loops_found==0) {
    num_errors++;
    parser_error("Break statement found outside of loop.");      
  }
  $$ = build_break_stmt();
};

continue_stmt:            CONTINUE SEMICOLON
{
  if(loops_found==0) {
    num_errors++;
    parser_error("Break statement found outside of loop.");      
  }
  $$ = build_continue_stmt();
};

selection_stmt:		  IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE
{ 
  $$ = (c_tree) build_ifelse($3,$5,NULL);
}
			| IF LPAREN expression RPAREN statement ELSE statement 
{ 
  $$ = (c_tree) build_ifelse($3,$5,$7);
}
;

iteration_stmt:		  WHILE {loops_found++;} LPAREN expression RPAREN statement
{
  $$ = (c_tree) build_while($4,$6,0);
  loops_found--;
}
                        | DO {loops_found++;} statement WHILE LPAREN expression RPAREN SEMICOLON
{
  $$ = (c_tree) build_while($3,$6,1);
  loops_found--;
}
                        | FOR {loops_found++;} LPAREN expr_opt SEMICOLON expr_opt SEMICOLON expr_opt RPAREN statement
{
  $$ = (c_tree) build_for($4,$6,$8,$10);
  loops_found--;
}
;

expr_opt:		
{ $$ = (c_tree) NULL; }
			| expression
{ $$ = (c_tree) $1; }
;

jump_stmt:		  RETURN SEMICOLON
{ 
  $$ = (c_tree) build_return_stmt(NULL);
}
			| RETURN expression SEMICOLON
{
  $$ = (c_tree) build_return_stmt($2);
}
;

expression:               assignment_expression
{ 
  $$ = $1; 
}
                        | expression COMMA assignment_expression
{ $$ = build_expr2(COMMA_EXPR, $1, $3); }		
;

assignment_expression:    conditional_expression
{
  $$ = $1;
}
                        | unary_expression assignment_operator assignment_expression
{
  $$ = build_expr2($2,$1,$3);
}
;

assignment_operator:      ASSIGN { $$ = ASSIGN_EXPR; }
;

conditional_expression:   logical_OR_expression
{
  $$ = $1;
}
                        | logical_OR_expression QUESTION_MARK expression COLON conditional_expression
{
  $$ = build_expr3(BAD_EXPR, $1,$3,$5);
  parser_error("Tertiary conditional expression not supported!");
  num_errors++;
}
;

constant_expression:       conditional_expression
{ $$ = $1; }
;

logical_OR_expression:    logical_AND_expression
{
  $$ = $1;
}
                        | logical_OR_expression LOGICAL_OR logical_AND_expression
{
  $$ = build_expr2(LOGICAL_OR_EXPR,$1,$3);
};

logical_AND_expression:   inclusive_OR_expression
{
  $$ = $1;
}
                        | logical_AND_expression LOGICAL_AND inclusive_OR_expression
{
  $$ = build_expr2(LOGICAL_AND_EXPR,$1,$3);
}
;

inclusive_OR_expression:  exclusive_OR_expression
{
  $$ = $1;
}
                        | inclusive_OR_expression BITWISE_OR exclusive_OR_expression
{
  $$ = build_expr2(BAD_EXPR,$1,$3);
}
;

exclusive_OR_expression:  AND_expression
{
  $$ = $1;
}
                        | exclusive_OR_expression BITWISE_XOR AND_expression
{
  $$ = build_expr2(BAD_EXPR,$1,$3);
}
;

AND_expression:           equality_expression
{
  $$ = $1;
}
                        | AND_expression AMPERSAND equality_expression
{
  $$ = build_expr2(BAD_EXPR,$1,$3);
}
;

equality_expression:      relational_expression
{
  $$ = $1;
}
                        | equality_expression EQ relational_expression
{
  $$ = build_expr2(LOGICAL_EQ_EXPR,$1,$3);
}
                        | equality_expression NEQ relational_expression
{
  $$ = build_expr2(LOGICAL_NEQ_EXPR,$1,$3);
}
;

relational_expression:    shift_expression
{
  $$ = $1;
}
                        | relational_expression LT shift_expression
{
  $$ = (c_tree) build_expr2(LOGICAL_LT_EXPR,$1,$3);
}
                        | relational_expression GT shift_expression
{
  $$ = (c_tree) build_expr2(LOGICAL_GT_EXPR,$1,$3);
}
                        | relational_expression LTE shift_expression
{
  $$ = (c_tree) build_expr2(LOGICAL_LTE_EXPR,$1,$3);
}
                        | relational_expression GTE shift_expression
{
  $$ = (c_tree) build_expr2(LOGICAL_GTE_EXPR,$1,$3);
}
;

shift_expression:         additive_expression
{
  $$ = $1;
}
                        | shift_expression LSHIFT additive_expression
{
  $$ = build_expr2(BAD_EXPR,$1,$3);
}
                        | shift_expression RSHIFT additive_expression
{
  $$ = build_expr2(BAD_EXPR,$1,$3);
}
;

additive_expression:      multiplicative_expression
{
  $$ = $1;
}
                        | additive_expression PLUS multiplicative_expression
{
  $$ = (c_tree) build_expr2(ADD_EXPR,$1,$3);
}
                        | additive_expression MINUS multiplicative_expression
{
  $$ = (c_tree) build_expr2(SUBTRACT_EXPR,$1,$3);
}
;

multiplicative_expression:  cast_expression
{
  $$ = $1;
}
                        | multiplicative_expression STAR cast_expression
{
  $$ = (c_tree) build_expr2(MULTIPLY_EXPR,$1,$3);
}
                        | multiplicative_expression DIV cast_expression
{
  $$ = (c_tree) build_expr2(DIVIDE_EXPR,$1,$3);
}
                        | multiplicative_expression MOD cast_expression
{
  $$ = (c_tree) build_expr2(MODULO_EXPR,$1,$3);
}
;

cast_expression:          unary_expression
{ $$ = $1; }
;

unary_expression:         postfix_expression
{
  $$ = $1;
}
                        | unary_operator cast_expression
{
  $$ = build_expr1($1,$2);
}
                        | SIZEOF unary_expression
{
  $$ = build_expr1(SIZEOF_EXPR,$2);
}
                        | SIZEOF LPAREN type_name RPAREN
{
  $$ = build_expr1(SIZEOF_EXPR,$3);
}
;

unary_operator:           AMPERSAND 
{ $$ = ADDR_EXPR; }
                        | STAR      
{ $$ = INDIRECT_REF_EXPR; } 
                        | MINUS
{ $$ = UMINUS_EXPR; }
                        | PLUS
{ $$ = UPLUS_EXPR; }
                        | BITWISE_INVERT
{ $$ = BAD_EXPR; parser_error("'~'-operator not supported in MiniC"); }
                        | NOT
{ $$ = LOGICAL_NOT_EXPR; }
;

postfix_expression:       primary_expression
{
  $$ = $1;
}
                        | postfix_expression LBRACKET expression RBRACKET
{
  $$ = build_expr2(ARRAY_REF_EXPR, $1, $3);
}
                        | postfix_expression LPAREN argument_expr_list_opt RPAREN
{
  $$ = build_expr2(CALL_EXPR, $1, $3);
}
                        | postfix_expression DOT ID
{
  $$ = build_expr2(MEMBER_EXPR, $1, build_identifier($3));
}
                        | postfix_expression ARROW ID
{
  c_tree tmp = build_expr1(INDIRECT_REF_EXPR, $1);
  $$ = build_expr2(MEMBER_EXPR, tmp, build_identifier($3));
}
;

primary_expression:       ID 
{ 
  symbol_table_entry_t tmp = symbol_table_get_entry(yyval.id);
  if(tmp==NULL)
    {
      parser_error("Undeclared identifier.");

      // Error reported... just build something to move on
      $$ = build_var_decl(build_identifier(yyval.id),error_type);
      num_errors++;
    }
  else
    {
      $$ = tmp->decl;
    }
}
                        | constant
{
  $$ = $1;
}
                        | LPAREN expression RPAREN
{
  $$ = $2
}
;

argument_expr_list_opt: { $$ = build_expr_list(); }
                        | argument_expr_list
{
  $$ = $1;
}
;

argument_expr_list:	  assignment_expression       
{ 
  c_tree tmp = build_expr_list();
  c_tree chain = build_chain((c_tree)$1);
  tmp->expr_list.chain = chain;
  tmp->expr_list.tail = chain;
  $$ = tmp;
}
			| argument_expr_list COMMA assignment_expression
{
  c_tree tmp = build_chain((c_tree)$3);
  TREE_CHAIN($1->expr_list.tail) = tmp; /* tail insertion */
  $1->expr_list.tail = tmp;
  $$ = $1;
}
;

constant:	          NUMBER  
{ 
  $$ = build_int_cst(yyval.num); 
} 
;

%%

extern int line_num;
extern char *infile[];
static int   infile_cnt=0;
extern FILE * yyin;

int parser_error(const char *msg)
{
  printf("%s (%d): Error -- %s\n",infile[infile_cnt-1],line_num,msg);
  return 1;
}

int internal_error(const char *msg)
{
  printf("%s (%d): Internal Error -- %s\n",infile[infile_cnt-1],line_num,msg);
  return 1;
}

int yywrap() {
  static FILE * currentFile = NULL;

  if ( (currentFile != 0) ) {
    fclose(yyin);
  }
  
  if(infile[infile_cnt]==NULL)
    return 1;

  currentFile = fopen(infile[infile_cnt],"r");
  if(currentFile!=NULL)
    yyin = currentFile;
  else
    printf("Could not open file: %s",infile[infile_cnt]);

  infile_cnt++;
  
  return (currentFile)?0:1;
}

int yyerror()
{
  parser_error("Un-resolved syntax error.");
  return 1;
}

void set_locus(locus *l)
{
  l->line_no = line_num;
  l->filename = infile[infile_cnt-1];
}

char * get_filename()
{
  return infile[infile_cnt-1];
}

int get_lineno()
{
  return line_num;
}


void minic_abort()
{
  parser_error("Too many errors to continue.");
  exit(1);
}
