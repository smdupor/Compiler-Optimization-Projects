struct cfg_inst_list{
	instL *inst;
	struct cfg_inst_list *next;
};
typedef struct cfg_inst_list CIL;

struct cfg_arrow_list{
	struct cfg_block *block;
	struct cfg_arrow_list *next;
};
typedef struct cfg_arrow_list CAL;

struct cfg_block{
	CIL *instructions;
	CAL *flow_arrow;	
};
typedef struct cfg_block Block;
