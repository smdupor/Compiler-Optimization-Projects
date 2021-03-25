#ifndef C_PARSE_H
#define C_PARSE_H

#include "c-tree.h"

extern int num_errors;

int parser_error(const char *msg);
int internal_error(const char *msg);

void c_parse();
void c_init();

c_tree build_tree();
c_tree build_int_cst(int val);
c_tree build_identifier(const char *);
c_tree build_chain(c_tree);
c_tree build_expr1(enum expr_type, c_tree);
c_tree build_expr2(enum expr_type, c_tree, c_tree);
c_tree build_expr3(enum expr_type, c_tree, c_tree, c_tree);
c_tree build_expr_list();
c_tree build_while(c_tree, c_tree, int);
c_tree build_for(c_tree, c_tree, c_tree, c_tree);
c_tree build_ifelse(c_tree, c_tree, c_tree);
c_tree build_exprstmt(c_tree);
c_tree build_compound_stmt(c_tree, c_tree);
c_tree build_return_stmt(c_tree);

c_tree build_break_stmt();
c_tree build_continue_stmt();

c_tree build_type(enum type_code, char *, type_info);
c_tree build_pointer_type(int, c_tree);

c_tree build_decl(enum decl_type);
c_tree build_pointer_decl(c_tree);
c_tree build_array_decl(c_tree, c_tree, c_tree);
c_tree build_var_decl(c_tree, c_tree);
c_tree build_function_decl(c_tree,c_tree,c_tree,c_tree);

c_tree build_decl_list();
c_tree build_stmt_list();

#endif
