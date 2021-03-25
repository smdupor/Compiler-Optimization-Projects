#include "c-tree.h"
#include "c-optimize.h"
#include "cmdline.h"
#include "c-codegen.h"
#include <stdlib.h>
#include <stdio.h>

extern void c_tree_print(FILE *fptr, c_tree t, int nesting, const char * p);

void c_optimize(c_tree func)
{
  if(verbose) {
    FILE *fptr = fopen("minic.verbose.txt","a");
    
    fprintf(fptr,"\n\nFunction: %s ---- \n",IDENTIFIER(DECL(func)->id)->id);

    c_tree_print(fptr,func,0,"");
    fclose(fptr);
  }

  /* Code gen pass: convert syntax tree directly to IR and write to output */  
  c_codegen(func);
}
