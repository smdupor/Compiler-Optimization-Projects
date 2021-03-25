#ifndef C_TYPECHECK_H
#define C_TYPECHECK_H

void c_typecheck_init();
int get_uid_for_type(c_tree);
c_tree get_type_for_uid(int);
int register_type(c_tree);
int check_for_typename(char *typename);

char * create_unique_struct_name(enum type_code, const char *);
char * create_pointer_name(c_tree);
char * create_array_name(c_tree, int length);

//void propagate_type(c_tree,c_tree);
c_tree normalize_type(c_tree,c_tree);

c_tree eval_expr_type(c_tree);
int    check_int_expression(c_tree);
c_tree fold_types(c_tree,c_tree);

int check_int(c_tree);

c_tree get_pointed_to_type(c_tree);

#endif
