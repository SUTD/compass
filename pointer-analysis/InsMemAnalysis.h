#ifndef _INS_MEM_ANALYSIS_H
#define _INS_MEM_ANALYSIS_H

#include "MemGraph.h"
#include "BasicBlock.h"
#include "Constraints.h"
#include "TwoBranchConstraints.h"
#include "Auxiliary.h"
#include "Alloc.h"



struct symbol_ap_mem {
	sail::Symbol* s;
	AccessPath* p;
};


enum analysis_type{

	BASIC_FIRST_ROUND,

	BASIC,

	CHECK_BEFORE_BBS,

	FIND_FIX_POINT,

};



class CallManager;

class GlobalAssert;
class MemGraph;
class MtdInstsProcessor;

class Alloc;
class InsMemAnalysis {

private:

	sail::Instruction* ins;

	int instruction_number;

	MtdInstsProcessor* mp;

	//static CallManager* cm;



public:

	InsMemAnalysis(){}

	InsMemAnalysis(sail::Instruction* ins, MtdInstsProcessor* mp, int i = -1);

	~InsMemAnalysis();

	void inprecise_ins_analysis(const analysis_type& at);



//	static void set_call_manager(CallManager* callm){
//		cm = callm;
//	}

private:

	bool need_process() const;


	void set_nodes_for_symbols(AccessPath* lhs_ap,
			AccessPath* rhs_ap,
			sail::Symbol* lhs,
			sail::Symbol* rhs);

	void set_nodes_for_three_symbols(AccessPath* lhs_ap,
			AccessPath* rhs_ap_op1,
			AccessPath* rhs_ap_op2,
			sail::Symbol* lhs_symbol,
			sail::Symbol* rhs_op1_symbol,
			sail::Symbol* rhs_op2_symbol);

	void set_nodes_for_address_string(AccessPath* lhs_ap,
			AccessPath* rhs_ap,
			sail::Symbol* lhs_s);

	void set_nodes_for_alloc_three_symbols(AccessPath* lhs_ap,
			AccessPath* allo_ap,
			AccessPath* size_ap,
			sail::Symbol* lhs_s,
			sail::Symbol* size_s);

	void set_node_for_symbol(AccessPath* ap,
			sail::Symbol* s,
			bool rhs_flag);

	symbol_ap_mem* build_symbol_ap_mem(sail::Symbol* symb,
			AccessPath* ap);




	/*******************************************
	 * get access_path for each instruction-type
	 * that is currently processing;
	 ************************************************/
	vector<symbol_ap_mem*>* get_access_paths();

	/*
	 * the following supports for get_access_paths();
	 */
	void get_assignment_aps(vector<symbol_ap_mem*>& symbol_ap);
	void get_unop_aps(vector<symbol_ap_mem*>& symbol_ap);
	void get_binop_aps(vector<symbol_ap_mem*>& symbol_ap);
	void get_load_aps(vector<symbol_ap_mem*>& symbol_ap);
	void get_store_aps(vector<symbol_ap_mem*>& symbol_ap);
	void get_address_string_aps(vector<symbol_ap_mem*>& symbol_ap);
	void get_cast_aps(vector<symbol_ap_mem*>& symbol_ap);
	void get_memory_alloc_aps(vector<symbol_ap_mem*>& symbol_ap);


//	void get_iterator(
//			vector<sail::Instruction*>::iterator it);
};

#endif
