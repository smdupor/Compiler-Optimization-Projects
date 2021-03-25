#include <stdio.h>
#include <stdlib.h>
#include "s3inst.h"

extern int verbose;
extern int num_errors;
char *infile;
extern int yyerror();
extern int yywrap();
extern int yyparse();
extern void print_inst(inst_t);
extern void print_list(inst_t);

inst_t instList; /* list of instructions found by parser */

int main(int argc, char **argv)
{
  if(argc < 2)
    {
      printf("Usage: ./opt [-v] file.asm\n");
      return -1;
    }
  
  int arg=1;
  while( *(argv[arg]) == '-' )
    {
      if( *(argv[arg]+1) == 'v' )
	{
	  verbose = 1;
	}
      arg++;
    }

  if(arg>=argc)
    {
      printf("Usage: ./opt [-v] file.asm\n");
      return -1;
    }

  infile = *(argv+arg);

  yywrap();
  yyparse();

  if(num_errors>0)
    return 0;

  if(verbose)
    print_list(instList);

  /* 1. build liveness information*/

  /* 2. perform DCE */

  /* 3. use liveness information to perform register allocation  */

  return 0;
}

/* append to list if not found in list*/
instList_t instList_append(instList_t head, inst_t inst, int *found)
{
  instList_t tail = head;
  instList_t new;

  *found = 0;

  new = (instList_t) malloc(sizeof(*head));
  new->inst = inst;
  new->next = NULL;

  if(head == NULL)
    return new;

  if( tail->inst == inst )
    {
      *found = 1;  
      return head;
    }

  while(tail && tail->next) {
    /* so long as I never copy an instruction found during parsing, I can test equality
       by comparing pointers */
    /* we've already inserted this once */
    if(tail->inst == inst) 
      {
	*found = 1;  
	return head;
      }
    tail = tail->next;
  }

  tail->next = new;
  return head;
}
