#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "s3inst.h"

#define TRUE 1
#define FALSE 0

#define vbmode if(verbose)
#define LIST_ADD(a,b) ((a)->next = (b)); ((b) = (a));

/* given globals */
extern int verbose;
extern int num_errors;
char *infile;
extern int yyerror();
extern int yywrap();
extern int yyparse();
extern void print_inst(inst_t);
extern void print_list(inst_t);

inst_t instList; /* list of instructions found by parser */

/* my globals */

/* structs and the like */
struct reg_set{
    int reg;        /* the number of the register */
    struct reg_set *next;  /* the next register in the list */
};
typedef struct reg_set regset;

struct instruction {
	int    num;  /* unique id         */
    regset *src;  /* registers read    */
    regset *dest;  /* registers written */
	/* register usage sets */
	int *def;
	int *use;
	int *livein;
	int *liveout;
    int deleted; /* a boolean if the instruction shoudl be deleted */
	struct successor *succ;  /* a list of the instruction's successors */
    inst_t instance;  /* a link to the instruction as scanned in */
    struct instruction *next; /* the next instruction */
    struct instruction *prev; /* the previous instruction */
};
typedef struct instruction instL;

struct successor{
	struct instruction *inst;
	struct successor *next;
};

/* Global Variables */
instL *masterList;
instL *masterListTail;
int MAX_REGS;
int NUM_INSTR;

/* My Function Declarations */
void setup_arrays();
void add_instruction(inst_t instruction, int number);
void build_regsets(instL *inst);
void create_regsets(instL *entry, int numops, int src_first);
void check_op_usage(struct operand op);
void flip_master_list();
void print_masterList();

void build_CFG();
instL *find_label(char *find);
instL *find_next(instL *find);
void DCE();
void calculate_defset();
void calculate_useset();
void calculate_liveness();
void print_regset(int set);
void pirnt_final_list();

/* begin code body below */

/*  pirnt_final_list()
 *  This function prints the final instruction list.
 */
void pirnt_final_list(){
    instL *curr = masterList;
    
    while(curr){
        if(curr->next && curr->instance->label && curr->next->instance->label){
            printf("%s:\t",curr->instance->label);
            print_inst(curr->instance);
        }
        else{
            if(curr->instance->label){
                printf("%s:",curr->instance->label);
            }
            if(!curr->deleted){
                printf("\t");
                print_inst(curr->instance);
            }        
        }
        curr = curr->next;
    }
}


/*  build_CFG()
 *  This function builds the control flow graph.
 */
void build_CFG(){
	instL *curr_inst = masterList;
	struct successor *temp;

	while(curr_inst){
		if((curr_inst->instance->op == OP_BRA) || 
		  ((curr_inst->instance->op == OP_BR) && (curr_inst->instance->ccode == 3584))){
			if((curr_inst->succ = (struct successor *)malloc(sizeof(struct successor))) == NULL){
				vbmode{ printf("Could not allocate memory!\n"); }
			}
			curr_inst->succ->next = NULL;
			if(curr_inst->instance->op == OP_BRA){
				curr_inst->succ->inst =	find_label(curr_inst->instance->ops[0].label);
			}
			else{
				curr_inst->succ->inst =	find_label(curr_inst->instance->ops[1].label);
			}
		}
		else if(curr_inst->instance->op == OP_BR){
			if((curr_inst->succ = (struct successor *)malloc(sizeof(struct successor))) == NULL){
				vbmode{ printf("Could not allocate memory!\n"); }
			}
			curr_inst->succ->next = NULL;
			curr_inst->succ->inst = find_label(curr_inst->instance->ops[1].label);

			temp = curr_inst->succ;

			if((curr_inst->succ = (struct successor *)malloc(sizeof(struct successor))) == NULL){
				vbmode{ printf("Could not allocate memory!\n"); }
			}
			curr_inst->succ->next = temp;
			curr_inst->succ->inst = find_next(curr_inst->next);
		}
		else{
			if((curr_inst->succ = (struct successor *)malloc(sizeof(struct successor))) == NULL){
				vbmode{ printf("Could not allocate memory!\n"); }
			}
			curr_inst->succ->next = NULL;
			curr_inst->succ->inst = find_next(curr_inst->next);
		}
		curr_inst = curr_inst->next;
	}

}

/*  find_label(char *find)
 *  Args:
 *      find:  The label to find
 *  This function finds the next valid instruction at a given label and returns it.
 */
instL *find_label(char *find){
	instL *curr = masterList;

	while(curr){
		if(curr->instance->label && !strcmp(find,curr->instance->label)){
			break;
		}
		curr = curr->next;
	}

    return find_next(curr);

}

/*  find_next(instL *find)
 *  Args:
 *      find: the instruction from where to start looking.
 *  This function finds the next valid instruction relative to a given instruction.
 */
instL *find_next(instL *find){
    instL *curr = find;
    while(curr){
        if(!curr->deleted) return curr;
        else curr = curr->next;
    }
	return curr;

}

/*  DCE()
 *  This function proformes dead code elimination.
 */
void DCE(){
    int previter,num_lines;
    instL *curr;

	vbmode{ printf("Proforming Dead Code Elimination:\n"); }

	vbmode{ printf("  Calculating def[]..."); }
	calculate_defset();
	vbmode{ printf("done.\n"); 
			print_regset(0);
	}

	vbmode{ printf("  Calculating use[]..."); }
	calculate_useset();
	vbmode{ printf("done.\n"); 
			print_regset(1);
            printf("  Eliminating dead code...");
	}

    num_lines = NUM_INSTR;
    previter = -1;
   
    while(num_lines != previter){
        previter = num_lines;
        build_CFG();
        calculate_liveness();
        curr = masterList;
/*
        vbmode{ printf("\n  CFG:\n"); 
                print_regset(4); 
                printf("  livein[]:\n");
                print_regset(2);
                printf("  liveout[]:\n");
                print_regset(3);
        }
*/ 
       while(curr){
            if(curr->deleted == FALSE && curr->dest && curr->liveout[curr->dest->reg] == FALSE){
                curr->deleted = TRUE;
                num_lines--;
            }
            curr = curr->next;
        }
    }
    
    vbmode{
        printf(  "done.\n  Final CFG:\n");
        print_regset(4);
        printf("  livein[]:\n");
        print_regset(2);
        printf("  liveout[]:\n");
        print_regset(3);
        printf("\nFinal List to be sent to Register Mapping:\n");
        pirnt_final_list();
    }
}

/*  print_regset(int set)
 *  Args:
 *		set:  A number to print out a particular set
 *			case 0: def
 *			case 1: use
 *			case 2: livein
 *			case 3: liveout
 *          case 4: CFG
 *  This function prints out one of the sets for the entire read in program.
 */
void print_regset(int set){
	instL *curr = masterList;
    struct successor *temp;
	int *printset, i, firstprinted;

    if(set == 4){
		while(curr){
            if(!curr->deleted){
			    printf("    %3d:  ",curr->num);
			    temp = curr->succ;
			    firstprinted = FALSE;
			    while(temp){
				    if(firstprinted) printf(", ");
				    if(temp->inst) printf("%d",temp->inst->num);
				    firstprinted = TRUE;
				    temp = temp->next;
			    }
                printf("\n");
            }
			curr = curr->next;
		}
    }
    else{
	    while(curr){
		    switch(set){
			    case 0: printset = curr->def; break;
			    case 1: printset = curr->use; break;
			    case 2: printset = curr->livein; break;
			    case 3: printset = curr->liveout; break;
			    default: printf("  Invalid argument to print_regset()\n"); return;
		    }
            if(set > 1 && !curr->deleted){
		        printf("    %3d:  ",curr->num);
		        firstprinted = FALSE;
		        for(i = 0; i<MAX_REGS; i++){
			        if(printset[i]){
				        if(firstprinted){
					        printf(", ");
				        }
				        printf("R%d",i);
				        firstprinted = TRUE;
			        }
		        }
		        printf("\n");
            }
		    curr = curr->next;
	    }
    }
}

/*  calculate_defset()
 *  This function calculates the defset for each function.
 */ 
void calculate_defset(){
	instL *curr = masterList;

	while(curr){
		if(curr->dest && curr->dest->reg){
			curr->def[curr->dest->reg] = TRUE;
		}	
		curr = curr->next;		
	}
}

/*  calculate_useset()
 *  This function calculates the defset for each function.
 */ 
void calculate_useset(){
	instL *curr = masterList;
	regset *curreg;

	while(curr){
		curreg = curr->src;
		while(curreg){
			if(curreg->reg){
				curr->use[curreg->reg] = TRUE;
			}
			curreg = curreg->next;
		}	
		curr = curr->next;		
	}
}

/*  calculate_liveness()
 *  This function calculates the liveness (livein[],liveout[] for each function.
 */ 
void calculate_liveness(){
    instL *curr;
    struct successor *succ;
    int changed=TRUE, i;
    int tempout[MAX_REGS],tempin[MAX_REGS];

    curr = masterList;
    while(curr){
        for(i=0; i<MAX_REGS; i++){
            curr->liveout[i] = FALSE;
            curr->livein[i] = FALSE;
        }
        curr = curr->next;
    }

    while(changed){
        changed = FALSE;
        curr = masterListTail;
        while(curr){
            if(curr == masterListTail){
                for(i=0; i<MAX_REGS; i++){
                    curr->livein[i] = curr->use[i];
                } 
            }
            else{
                for(i=0; i<MAX_REGS; i++){
                    tempout[i] = FALSE;
                    tempin[i] = FALSE;
                }
                succ = curr->succ;
                while(succ){
                    for(i=0; i<MAX_REGS; i++){
                        tempout[i] |= succ->inst->livein[i];
                    }                
                    succ=succ->next;
                }
                for(i=0; i<MAX_REGS; i++){
                    tempin[i] = curr->use[i] | (tempout[i] &(!(curr->def[i])));
                    
                    if(tempout[i] != curr->liveout[i] || tempin[i] != curr->livein[i]){
                        changed = TRUE;
                    }

                    curr->livein[i] = tempin[i];
                    curr->liveout[i] = tempout[i];
                }
            }
            curr = curr->prev;
        }
    }
}

/*  void setup_arrays()
 *  This function creates the master list, and allocates all the needed memory for it.
 */
void setup_arrays(){
	instL *curr, *last;
	inst_t current;
	int number, i;

	vbmode{ printf("Setting up arrays:\n"); }

	masterList = NULL;
	NUM_INSTR = 0;
	MAX_REGS = 0;

	vbmode{ printf("  Building master array..."); }
	current = instList;
	number = 0;
	while(current){
		add_instruction(current,number);
		current=current->next;
		number++;
		NUM_INSTR++;
	}

    /* flip the list so it is in order */
    flip_master_list();

	vbmode{ printf("done.\n");
			printf("  Found %d instructions using largest register: R%d.\n",NUM_INSTR,MAX_REGS-1);
			print_masterList();
			printf("  Allocating register sets..."); 
	}

	curr = masterList;
    last = NULL;
	while(curr){
		if((curr->def = (int *)malloc(MAX_REGS*sizeof(int))) == NULL){
		    vbmode{ printf("Could not allocate memory!\n"); }
		}
		if((curr->use = (int *)malloc(MAX_REGS*sizeof(int))) == NULL){
		    vbmode{ printf("Could not allocate memory!\n"); }
		}
		if((curr->livein = (int *)malloc(MAX_REGS*sizeof(int))) == NULL){
		    vbmode{ printf("Could not allocate memory!\n"); }
		}
		if((curr->liveout = (int *)malloc(MAX_REGS*sizeof(int))) == NULL){
		    vbmode{ printf("Could not allocate memory!\n"); }
		}
        curr->deleted = FALSE;        
        curr->prev = last;
        last = curr;
		curr = curr->next;
        
	}

    masterListTail = last;

	vbmode{ printf("done.\n"); 
			printf("  Initializing register sets..."); 
	}

	curr = masterList;
	while(curr){
		for(i=0; i<MAX_REGS; i++){
				curr->def[i] = FALSE;
				curr->use[i] = FALSE;
				curr->livein[i] = FALSE;
				curr->liveout[i] = FALSE;			
		}
        if(curr->instance->op == OP_NOP){
            curr->deleted = TRUE;
        }
		curr = curr->next;
	}
	vbmode{ printf("done.\n"); }
}

/* void print_masterList()
 * This function prints the ddg instruction list
 */
void print_masterList(){
    instL *curr = masterList;
    regset *currreg;
    
    while(curr){
        printf("    %d.\tDest: ",curr->num);
        currreg = curr->dest;
        if(currreg == NULL){
            printf("NA ");
        }
        else{
            while(currreg){
                printf("R%d ",currreg->reg);
                currreg = currreg->next;
                }
            }
        printf("\tSrc: ");
        currreg = curr->src;
        if(currreg == NULL){
            printf("NA\t");
        }
        else{
            while(currreg){
                printf("R%d ",currreg->reg);
                currreg = currreg->next;
                }
            }
        printf("\n");
        curr=curr->next;
    }

}

/* void flip_instr_list()
 * This function flips the ddg instruction list so it is in order 
 */
void flip_master_list(){
    instL *new, *curr, *temp;

    new = NULL;
    curr = masterList;
    while (curr != NULL) {
        temp = curr->next;
        LIST_ADD(curr, new);
        curr = temp;
    }
    masterList = new;
}

/*  void add_instruction(inst_t instruction, int number)
 *  Args:
 *		instruction: the instruction to add
 *		number:  the instruction number
 *  This function adds instructions to the master list of instructions 
 */
void add_instruction(inst_t instruction, int number){
	instL *new;

    if((new = (instL *)malloc(sizeof(instL))) == NULL){
        vbmode{ printf("Could not allocate memory!\n"); }
    }

	new->instance = instruction;
	new->num = number;

	build_regsets(new);

	LIST_ADD(new,masterList);
}

void build_regsets(instL *inst){    
	int src_first = FALSE;    

    switch(inst->instance->op) {
        /* 3 operands */
        case OP_STR:
        case OP_ADD:
        case OP_AND:
        case OP_ANDL:
        case OP_DIV:
        case OP_LDR:
        case OP_MUL:
        case OP_OR:
        case OP_ORL:
        case OP_SUB:
            check_op_usage(inst->instance->ops[0]);
            check_op_usage(inst->instance->ops[1]);
            check_op_usage(inst->instance->ops[2]);
            create_regsets(inst, 2, src_first);
            break;
        /* 2 operands */
        case OP_BR:
        case OP_ST:
        case OP_STI:
            src_first = TRUE;
        case OP_SET:
        case OP_LD:
        case OP_LDI:
        case OP_LEA:
        case OP_NOT:
        case OP_NOTL:
            check_op_usage(inst->instance->ops[0]);
            check_op_usage(inst->instance->ops[1]); 
            create_regsets(inst, 1, src_first);
            break;
        /* one operand */
        case OP_JSRR:
        case OP_BRA:
        case OP_JMP:
        case OP_JSR:
            src_first = TRUE;
            check_op_usage(inst->instance->ops[0]); 
            create_regsets(inst, 0, src_first);
			break;
		/* no ops */		
		case OP_RET:
            if((inst->src = (regset *)malloc(sizeof(regset))) == NULL){
                vbmode{ printf("Could not allocate memory!\n"); }
            }
			inst->src->next = NULL;
			inst->src->reg = 7;
			inst->dest = NULL;
			break;			
        default:
            break;
    }

}

/* void create_regsets(inst_t entry, int numops, int inst_num)
 * Args:
 *      entry: a link to the origional instruction
 *      numops: the number of operands to consider
 *      src_first: boolean to calculate the src reg fist
 * This function adds items to the ddg instruction list
 */
void create_regsets(instL *entry, int numops, int src_first){
    int i;
    regset *temp;
    enum op_type t; 

    entry->src = NULL;
    entry->dest = NULL;

    for(i=numops; i>=0; i--){
        t = entry->instance->ops[i].t;
        if(t == op_reg){
            if(entry->instance->op == OP_STR){
                temp = entry->src;
                if((entry->src = (regset *)malloc(sizeof(regset))) == NULL){
                    vbmode{ printf("Could not allocate memory!\n"); }
                }
                entry->src->reg = entry->instance->ops[i].reg;
                entry->src->next = temp;
            }
            else if(src_first){
                if(i == 0){
                    temp = entry->src;
                    if((entry->src = (regset *)malloc(sizeof(regset))) == NULL){
                        vbmode{ printf("Could not allocate memory!\n"); }
                    }
                    entry->src->reg = entry->instance->ops[i].reg;
                    entry->src->next = temp;
                }
                else{
                    temp = entry->dest;
                    if((entry->dest = (regset *)malloc(sizeof(regset))) == NULL){
                        vbmode{ printf("Could not allocate memory!\n"); }
                    }
                    entry->dest->reg = entry->instance->ops[i].reg;
                    entry->dest->next = temp;
                }
            }
            else{
                if(i == 0){
                    temp = entry->dest;
                    if((entry->dest = (regset *)malloc(sizeof(regset))) == NULL){
                        vbmode{ printf("Could not allocate memory!\n"); }
                    }
                    entry->dest->reg = entry->instance->ops[i].reg;
                    entry->dest->next = temp;
                }
                else{
                    temp = entry->src;
                    if((entry->src = (regset *)malloc(sizeof(regset))) == NULL){
                        vbmode{ printf("Could not allocate memory!\n"); }
                    }
                    entry->src->reg = entry->instance->ops[i].reg;
                    entry->src->next = temp;
                }
            }
        }
    }

}

/* void check_op_usage(struct operand op)
 * Args:
 *      op: the operand for the instruction
 * This function checks if the operand is a register and then updates
 *      the maximum register used.
 */
void check_op_usage(struct operand op){
    enum op_type t =  op.t;
    switch(t) {
        case op_reg:
            if( op.reg >= MAX_REGS ){
                MAX_REGS = op.reg + 1;
            }
            break;
        default:
            break;
    }
}

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

  vbmode{ printf("Instructions read in:\n"); print_list(instList); }

  /* 1. build liveness information*/

  /* 2. perform DCE */

  /* 3. use liveness information to perform register allocation  */

  setup_arrays();
  DCE();


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
