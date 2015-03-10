#ifndef _SUMMARY_UNIT_H_
#define _SUMMARY_UNIT_H_
#include "sail.h"
#include "il.h"
#include "MtdInstsProcessor.h"
#include "Auxiliary.h"
#include <stdlib.h>
#include "ApTree.h"
#include <unordered_set>
#include "SideNode.h"

//#define INVALID_OPCODE -3
//#define IMPOSSIBLE_OFFSET  -200


enum type_of_ins {
	ASSIGNMENT, LOAD, STORE, LOAD_AND_STORE
};

class MemGraph;
class MemNode;
class ApTree;

class CallManager;

class MtdInstsProcessor;
class AllocContextController;
struct side_ele;
struct node_set;
struct variable_info;
struct av_target;
struct ins_two_side;
struct ap_with_operator;
struct instruction_side;
class SummaryUnit {

	friend class ApTree;

	/**
	 * one application should have non-conflict temp name
	 * temp_num use to generate temp variable
	 */
	static int temp_num;

	MtdInstsProcessor* mtdp;

	//side_ele make sure key is unique for each instruction

	bool has_return_var;

	set<side_ele*> track_lhs_sides;

	map<string, side_ele*> side_ele_map;

	map<one_side*, int> lhs_time;

	map<string, one_side*> lhs_sides;

	/*
	 * the equivalent instruction set
	 */
	list<sail::Instruction*> eq_ins;


	/*
	 * return var instructions
	 */
	vector<sail::Instruction*> return_ins;

	set<string> unique_insts_string;

	bool has_alloc;
	/*******
	 * AllocContextController*
	 */
	AllocContextController* allocc;


	set<sail::Variable*> argument_and_return_var;

	/*
	 * argument& ret have a set of reachable MemNode;
	 */
	vector<node_set> arg_return_nodes_set;

//	struct node_set return_node;
	node_set* return_node;

	vector<node_set*> alloc_nodes_set;

	/*******
	 * keep track the processing of shared node,
	 * generate instruction only once
	 *******/
	set<MemNode*> processed_node;

	set<MemNode*> processed_return_node;

	set<AccessPath*> return_targets_in_memory_graph;


	/***
	 * argument and its representative aps
	 * which are used to generate instructions
	 *******/
	map<AccessPath*, av_target*> arg_arith_target;

	/***
	 * set<AccessPath*> elements are Variable,
	 * Constant or ArithmeticValue or FunctionValue ap type
	 ****/
	map<AccessPath*, set<AccessPath*>> argument_ap_targets_in_memory_graph;

	/*********
	 * for generate arithmetic instruction
	 *********/

	vector<sail::Instruction*> arithmetic_ins;

	map<AccessPath*, sail::Symbol*> solved_ap_symbols;

public:

	SummaryUnit(MtdInstsProcessor* mdp);

	~SummaryUnit();

	void set_return_var_flag(bool return_flag);

	void build_equivalent_insts(MemGraph mg);

	inline list<sail::Instruction*>& get_equivalent_insts() {
		return eq_ins;
	}

//	void set_alloc_context_controller(AllocContextController*);

	const string to_string() const;

	MtdInstsProcessor* get_mtd_insts_processor();

private:

	/******
	 * used to generate temp variables of instructions in summary-unit
	 */
	static sail::Variable* get_summary_unit_var();



	/*********************************
	 * collect all reachable nodes from alloc, arg, return aps;
	 ********************************/
	void collect_reachable_nodes(MemGraph& mg);
	void collect_alloc_related_nodes(MemGraph& mg,
			AccessPath* lhs_ap,
			MemNode* node);

	void collect_return_related_nodes(MemGraph& mg,
			MemNode*node);

	void collect_args_related_nodes(MemGraph& mg,
			MemNode*node);

	void simplify_nodes(MemGraph& mg,
			set<MemNode*>* res);

	void arg_node_sets_test();
	void return_node_set_test();
	void alloc_node_set_test();

	/************
	 * process the collect nodes; (based on the collect_reachable_nodes for 3 types)
	 ************/
	void process_node_sets();
	void process_alloc_nodes();
	void process_arguments_nodes();
	void process_return_nodes();


	AccessPath* collect_alloc_insts_from_shared_nodes(AccessPath* lhs_ap);


	void add_to_eq_insts(sail::Instruction* );
	void add_to_return_insts(sail::Instruction* );

	/******************************************
	 * used to generate size = int value(this one)
	 * for "vi = alloc_array_I(size)";
	 *******************************************/
	sail::Assignment* size_of_alloc_inst(
				sail::FunctionCall* allo_fc,
				MemNode* deref_of_alloc);

	/**************************************************
	 * alloc=>ret; alloc=>arg && arg=>alloc flow;
	 *************************************************/
	void process_different_node_set_for_alloc(set<MemNode*>* alloc_relateds,
			set<MemNode*>* others,
			AccessPath* alloc_ap,
			AccessPath* ap);

	/**********************************
	 * return => arg flow
	 **********************************/
	void process_different_node_set_for_return(set<MemNode*>* ns1,
			set<MemNode*>* ns2);
	/***************************************
	 * arg  => arg flow;
	 **********************************/
	void process_different_node_set_for_args(set<MemNode*>* ns1,
			set<MemNode*>* ns2);


	/*******
	 * return -1, no default edge
	 * return 0, default edge belongs to alloc
	 * return 1, default edge belongs to argument(return is not possible)
	 */
	int collect_relation_pairs_for_alloc(set<MemNode*>& shared_nodes,
			AccessPath* alloc_ap,
			AccessPath* alloc_deref,
			AccessPath* other_ap,
			set<pair<AccessPath*, AccessPath*>>& relation_pairs);





	void process_arithmetic_fun_deref_node(MemNode* arith_deref);

	/***********************************************************
	 * *  generate the type of instruction
	 * * * based on the node information;
	 **********************************************/
	void collect_assignment_insts(MemNode* node);
	void collect_assignment_for_return_var(MemNode* node);

	void build_alloc_var_side(AccessPath* s_ap,
			variable_info& alloc_side);

	void build_ret_var_side(AccessPath* s_ap,
			variable_info& ret_side,
			bool& ret_flag);

	void build_ins_assi_side(AccessPath* s_ap,
			variable_info& side,
			bool& arg_flag);

	void produce_assignment_ins(variable_info& lhs,
			int lhs_offset,
			variable_info& rhs,
			int rhs_offsets,
			bool return_var_f = false);

	///////////////////////////////////
	void update_arg_arith_target_for_arg(AccessPath* arg_ap,
			set<AccessPath*>& target_aps);

	void process_argument_node_with_arith_value_target(MemNode* arg_node,
			AccessPath* ap,
			int offset,
			unsigned int ap_size);

	void process_arithvalue_source_node(MemNode* source,
			int ap_size,
			set<AccessPath*>& target_aps,
			int offset);

	void update_arg_max_arithvalue_target(AccessPath* arg,
			AccessPath* target,
			int size,
			int offset);

	void collect_arithvalue_insts();

	bool inside_return_var_target_in_memory_graph(AccessPath* tar);

	bool inside_argument_target_in_memory_graph(AccessPath* tar,
			AccessPath* arg);

	inline void store_into_argument_and_target_map(AccessPath* arg,
			set<AccessPath*>& targets) {

		argument_ap_targets_in_memory_graph[arg] = targets;

	}



	inline void add_to_arithmetic_inst(sail::Instruction* ins) {

		arithmetic_ins.push_back(ins);
	}

	inline map<AccessPath*, sail::Symbol*>& get_solved_ap_symbols() {
		return solved_ap_symbols;
	}


	void testing_argument_ap_targets_in_memory_graph();

	string sub_ap_with_operators_to_string(vector<ap_with_operator>& sub_terms);

	void test_arithmetic_instruction_for_argument();

	string solved_ap_symbols_to_string();

	/*************************************************
	 * flag means if only collect terminal terms
	 *****************************************************/
	void collect_sub_terms(Term* t, vector<ap_with_operator>& sub_terms,
			bool collect_only_terminals = false);


	bool is_sub_ap(AccessPath* cur_ap,
			vector<ap_with_operator>& refer_ap_terms);

	bool is_sub_ap_with_operator(ap_with_operator& cur_ap,
			vector<ap_with_operator>& refer_sub_terms);

	bool belong_to_parts_of_terms(AccessPath* ap,
			vector<ap_with_operator>& biggest_ap_sub_terms);

	bool ap_strip_term_deletable(AccessPath*& ap, Term* strip_t,
			vector<ap_with_operator>& refer_ap_terms);



	//somehow some generated instruction are redundant, do a remove step
	//some prepare struct and functioncalls
	/*****
	 * begin prepare of deleting, if times > 1, we need to check if it needs deleting
	 */
	void update_lhs_times_map(sail::Instruction* ins);

	ins_two_side* make_complete_two_sides(sail::Instruction* ins,
			bool lhs);

	one_side* get_lhs_side(sail::Instruction* ins);

	side_ele* make_side_ele(instruction_side* instruction_side,
			bool is_return,
			sail::Instruction* ins);

	void collect_same_lhs_side_eles_for_diff_rhs(set<side_ele*>&,
			one_side*);

	void check_rhs_and_delete_instructions(one_side* one_side);

	void update_track_include(sail::Instruction* ins,
			bool return_flag);

	string track_lhs_sides_to_string();

	//void remove_redundant_instructions();

	void delete_ins_from_eq_ins(sail::Instruction* ins);

//	void delete_ins_from_return_ins(sail::Instruction* ins);

	/********
	 * end of prepare of deleting
	 */

	/*******
	 * deleting them
	 */
	void remove_redundant_instruction();

	//void collect_argument_and_return_var();

	void order_insts();

};
#endif
