#ifndef _MTDINSTSPROCESSOR_H
#define _MTDINSTSPROCESSOR_H

#include "InsMemAnalysis.h"
#include "Auxiliary.h"
#include <unordered_set>

//#define TERMINAL_SIDE_OPEN

class SummaryUnit;
class StaticAssertBuilder;
class CallManager;
struct one_side;
class SideNode;
class AllocContextController;
class AllocContext;
class MemNode;





class MtdInstsProcessor {

private:
	static CallManager* callm;

	unsigned int i;

	sail::Function* f;

	string output_folder;

	//vector<sail::Instruction*>* body;

	StaticAssertBuilder* a_builder;

	SummaryUnit* summary_unit;

	AllocContextController* allocc;

	/********
		 *
		 */
	map<MemNode*, sail::Variable*> alloc_mem_pointed_by_var;



	bool has_alloc;

	MemGraph* mem_graph;


	//map<one_side*, set<sail::Instruction*>*> set_including;
	//map<one_side*, set<string>*> track_unique_inst_for_side;

	/********
	 * total Instruction need to process again (because of fix_point)
	 */
	set<sail::Instruction*> set_include;


	//if we want to delete elements from set_include,
	//we need to delete it also from track_unique_inst;
	/*********
	 * used to unify set_include;
	 */
	set<string> track_unique_inst;


	//used to simplify the instruciton in rules;
	//this one isn't needed here!
#ifdef TERMINAL_SIDES_OPEN
	set<SideNode*> terminal_sides;
#endif


	//for the purpose of factoring only in method level;
	map<string, AccessPath*> symbols_used;


	//added for inter-procedural analysis
	map<call_id, vector<sail::FunctionCall*>*> unresolved_fc;


public:
	//used to unique SideNode


	MtdInstsProcessor(const unsigned int& i,
			const vector<sail::Function*>& funs,
			const string& output_folder);

	~MtdInstsProcessor();

	void process_basic_instructions();

	bool is_pointed_by_var(MemNode* deref_alloc_mn);

	void put_to_alloc_pointed_by_var_map(MemNode* deref_alloc_mn,
			sail::Variable* point_to);

	sail::Variable* get_point_to_alloc_var(MemNode* deref_alloc_mn);

	string alloc_mem_pointed_by_var_to_string();

	static void set_call_manager(CallManager* callma);

	static CallManager* get_call_manager() {return callm;}

	unsigned int get_method_idx() const;

	void call_manager_map_init();

	void store_to_symbols(sail::Symbol* s, AccessPath* ap){
		assert(s != NULL);
		assert(ap != NULL);

		if(symbols_used.count(s->to_string()) > 0)
			return;
		symbols_used[s->to_string()] = ap;
	}

	bool find_symbol(sail::Symbol* s){
		assert(s != NULL);
		return (symbols_used.count(s->to_string()) > 0);
	}

	AccessPath* get_ap(sail::Symbol* s){
		assert(s != NULL);
		return symbols_used[s->to_string()];
	}


	map<call_id, vector<sail::FunctionCall*>*>& get_unresolved_fcs(){
		return this->unresolved_fc;
	}

	void add_to_in_rules(sail::Instruction* inst);
	int add_to_set_including(sail::Instruction*);


#ifdef TERMINAL_SIDES_OPEN
	/*
	 * called based on the return value of add_to_set_including();
	 */
	void update_side_nodes(sail::Instruction* ins);

	void get_lhs_one_side(sail::Instruction* inst, one_side*& lhs);

	void get_rhs_one_side(sail::Instruction* inst, set<one_side*>*& one_side);
#endif

	inline MemGraph* get_mem_graph(){

		return mem_graph;
	}

	inline SummaryUnit* get_summary_unit(){
		return summary_unit;
	}


	void store_into_unresolved_fc_map(sail::FunctionCall* fc);

	inline StaticAssertBuilder* get_static_assert_builder(){
		return a_builder;
	}



	inline sail::Function* get_function(){
		return f;
	}

	inline vector<sail::Instruction*>* get_body(){
		return f->get_body();
	}

	const string rule_to_string() const;

	inline vector<sail::FunctionCall*>* get_unresolved_fc_from_callid(
			const call_id& ci){
		if(unresolved_fc.count(ci) == 0)
		{

			cerr << "#####  OMG, callee ci is "
					<< ci.to_string() <<
					"		can't be found in collected fcs ! " << endl;

			cerr <<"	Inside " << f->get_identifier().to_string() << endl;

		}

		assert(unresolved_fc.count(ci) > 0);
		return unresolved_fc[ci];
	}


	void update_unresolved_fc_from_callid(const call_id& ci);

	AllocContextController* get_alloc_ctx_controller(){
		return allocc;
	}

	void fill_alloc_context_controller(AccessPath* lhs_ap,
			AccessPath* alloc_ap,
			sail::FunctionCall* alloc_fc);



	bool has_allocations(){return has_alloc;}

	const void print_alloc_context_insts() const;

	const string unresolved_functioncall_to_string() const;

	const void  unresolved_functioncall_test()const;

	void apply_summary_unit_to_func(sail::FunctionCall* fc,
			SummaryUnit* callee_su, bool& updated);

	/*******
	 * if all rhs can reach to another;
	 */
	void simplify_instructions_in_rule(
			set<sail::Instruction*>& remain_instruction);



	void process_instructions_in_rule();
	void reset_summary_unit();
	//used to simplify instruciton in rules;
	const void print_terminal_sides() const;

private:

	void analysis_ins_without_static_assert(unsigned int i,
			sail::Instruction* ins);

	void analysis_ins(unsigned int i,sail::Instruction* ins);
	void process_first_round_single_instruction(sail::Function* f,
			sail::Instruction* ins,
			unsigned int i,
			const string& output_folder);

	void summary_unit_init();


	void set_up_complete_static_assert_eles();
	void collect_static_assert_boundaries();
	void building_static_assert_eles();



	void add_to_two_sets(sail::Instruction*);



	//	void deleteIns(sail::Instruction* ins, set<sail::Instruction*>* labels);
	//	void deleteLabels(set<sail::Instruction*>* const labels);
};
#endif
