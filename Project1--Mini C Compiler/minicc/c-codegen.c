#include "cmdline.h"
#include "c-tree.h"
#include "c-parse.h"
#include "c-symbol.h"
#include "c-typecheck.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void c_codegen_entry()
{
  /****************************************************
   * Entry to object file.
   * (1) Generate all code necessary to call Main
   *     See project description.
   ****************************************************/
}

void c_codegen_exit()
{
  /****************************************************
   * Exit of object file.
   * (1) Insert .End, clean up data structures
   ****************************************************/
}

void c_codegen(c_tree func)
{
  /****************************************************
   * Main entry point for assignment.  You can assume
   * that if compilation reaches this point, that it is
   * a well formed MiniC program.
   ****************************************************/
  
  /* Replace this with the correct code */
  printf("Generate code for function: %s\n",DECL_NAME_STRING(func));

}
