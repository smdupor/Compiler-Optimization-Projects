
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "cmdline.h"
#include "c-parse.h"

extern int cmdlex();

int main(int argc, char **argv)
{
  arglim = argv + argc;
  targv = argv+1;

  cmdlex();

  if(outfile==NULL)
    outfile = strdup("out.asm");

  if(infile[0]) { /* at least one input file */    
    c_parse();
  }

  return 0;
}
