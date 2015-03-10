#ifndef _AP_TREE_H_
#define _AP_TREE_H_
#include "sail.h"
#include "il.h"
#include "Auxiliary.h"
#include <stdlib.h>

#define IMPOSSIBLE_OFF -1000

struct ap_node_info {
	int idx;
	int level;
	Term* term;
	long int op_code;

	ap_node_info(Term* t, long int opcode, int l, int index);
	string to_string();
};

struct ap_node {
	//bool resolved;
	ap_node_info* node_info;
	ap_node* pred;
	vector<ap_node*>*succes;
	vector<long int> unresolved_succes_idx;

	ap_node(ap_node_info* node_info, ap_node* pred);
	void add_to_succes(ap_node* apn);
	//bool has_unresolved_succs();
	int get_idx();
	//void set_as_resolved();
	string to_string();
	long int get_op_code();
};

class SummaryUnit;
class ApTree {
private:
	SummaryUnit* su;
	bool store_ins;

	sail::Variable* sail_var;
	//this one could be Variable or FieldSelection Type
	AccessPath* arg_ap;

	//the expression that need to be processed(point to target)
	AccessPath* target_ap;
	set<unsigned int> unresolved_ap_idx;
	int ap_node_counter;
	map<unsigned int, ap_node*> ap_nodes;
	map<unsigned int, sail::Symbol*> idx_sail_symbols;




public:
	ApTree(){}


	ApTree(SummaryUnit* sum, sail::Variable* svar,
			AccessPath* argument_ap,
			AccessPath* tar_ap,
			bool store = false)
	{
		store_ins = store;
		su = sum;
		sail_var = svar;
		arg_ap = argument_ap;
		target_ap = tar_ap;
		ap_node_counter = 0;
	}

	//static vector<sail::Instruction*>& get_return_var_arithmetic_insts();
//	static vector<sail::Instruction*>& get_arithmetic_insts();
//	static map<AccessPath*, sail::Symbol*>& get_solved_ap_symbols();

//	static void test_arithmetic_instruction_for_argument();


	bool operator < (const ApTree& other) const ;


//	static string solved_ap_symbols_to_string();

	ap_node* get_ap_node(Term* t,
			long int opcode,
			int l,
			int index,
			ap_node* pred);

	inline void add_to_ap_graph(ap_node* apn) {

		unsigned int idx = apn->get_idx();
		this->ap_nodes[idx] = apn;
		this->unresolved_ap_idx.insert(idx);
	}

	inline ap_node* get_ap_node_from_idx(int idx) {
		assert(ap_nodes.count(idx) >0);
		return ap_nodes[idx];
	}
	sail::Symbol* get_var(Term* t);



	void add_to_predecessor(ap_node* ap_node, int pred_index) ;
	ap_node* build_arithvalue_ap_expression_tree(Term* t,
			int level,
			long int op_code,
			ap_node* pred);

	bool has_unresolved_ap_node(ap_node* ap_node);

	sail::Variable* already_built_in_solved_ap_symbols(sail::Symbol* rhs_1,
			sail::Symbol* rhs_2,
			il::binop_type bt);


	void produce_arith_fun_insts(AccessPath* cur_ap);

	void produce_store_inst(sail::Variable*& lhs,
			sail::Symbol* rhs_1,
			sail::Symbol* rhs_2,
			il::binop_type bt,
			bool target_of_argument);

	void produce_arith_inst(sail::Variable*& lhs,
			sail::Symbol* rhs_1,
			sail::Symbol* rhs_2,
			bool binop_f,
			long int op_code,
			bool target_of_argument);

	void traverse_ap_tree() ;

	string to_string();
};


#endif
