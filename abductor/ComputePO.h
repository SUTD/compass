/*
 * ComputePO.h
 *
 *  Created on: May 22, 2012
 *      Author: boyang
 */

#ifndef COMPUTEPO_H_
#define COMPUTEPO_H_




#include <iostream>
#include <string.h>


#include "Serializer.h"
#include "Function.h"
#include "Instruction.h"
#include "Cfg.h"
#include "Constraint.h"
#include "term.h"
#include "cnode.h"
#include "sail.h"
#include "il.h"
#include "SuperBlock.h"

#include "ProofObligation.h"
#include "POE.h"
#include "CallingContext.h"
#include "ModSet.h"

#include <set>

using namespace std;
using namespace sail;
using namespace il;



class ComputePO {

private:

	/*
	 * the work list
	 */
	set<POEKey*, ComparePOEKey> worklist;

	/*
	 * P Q info of whole function or Superblock
	 */
	Constraint extra_above_info;
	Constraint extra_after_info;
	ModSet ms;

	int time;
	int unroll_time;


	/*
	 * return ProofObligation based on the block
	 */
	ProofObligation * find_ProofObligation(POEKey * b_p);

	void poe_map_adder(Function * func, CallingContext * calling_seq);
	Function * function_finder(string name);// , vector<Term*> args);
	void get_fuc_po(BasicBlock * b_entry,  map<Block*, ProofObligation*>& cur_map);


	/*
	 * eliminate variables start with __temp
	 */
	static void eliminate_temp(Constraint & cons);



	/*
	 * eliminate variables start with __temp other than Term t
	 */
	static void eliminate_temp(Constraint & cons, Term* t);


	/*
	 * for nested loop wp initialization
	 */
	void superBlock_wp_adder(SuperBlock* sb, Constraint cons,
				CallingContext* cur_callingcontext);


	Constraint wp(vector<Instruction*>&, Constraint);
	Constraint sp(vector<Instruction*>& s, Constraint phi);


	/*
	 * help compute_all_loop_condition() to compute all loop conditions
	 */
	Constraint compute_loop_condition(POEKey* pk_sb);
	Constraint compute_wp_between_blocks(POEKey* pk_entry,
			POEKey* pk_bottom, POEKey* pk_top, Constraint c_start);
	Constraint compute_wp_between_blocks_helper(POEKey* b_bottom, POEKey* b_top,
				Constraint c_start);


	void get_follow_instructions(BasicBlock* entry, vector<Instruction*> & vect_inst);

	SuperBlock* find_superblock(int block_id , BasicBlock* func_entry);


	//--------------------------------  wp part----------------------------

	Constraint wphelper(Instruction* inst, Constraint phi);

	Constraint wp_assignment(Assignment* , Constraint);

	Constraint wp_unop(Unop* assign, Constraint phi);

	Constraint wp_binop(Binop* , Constraint);

	Constraint wp_assume(Assume* inst_assume, Constraint phi);

	Constraint wp_assert(StaticAssert* , Constraint);

	Constraint wp_functionCall(FunctionCall* functionCall, Constraint phi);

	Constraint wp_mod_c(Term* vt, Term* t1, int cons, Constraint phi);

	Constraint wp_cast(Cast* cast, Constraint phi);

	Constraint wp_sub_function(POEKey* poekey_func_entry, POEKey* poekey_func_exit,
				Constraint begin,Function* callingfunc);

	//----------------------------------sp part -------------------------

	/*
	 * phi,  \existv' (phi[v'/vt]&vt=term_E[v'/vt])
	 */
	Constraint sp_assign_helper(Constraint phi, Term * vt, Term * term_E);

	Constraint sp_assignment(Assignment* assignment, Constraint phi);

	Constraint sp_unop(Unop* inst_unop, Constraint phi);

	Constraint sp_functionCall(FunctionCall* functionCall, Constraint phi);

	Constraint sp_binop(Binop* inst_binop, Constraint phi);

	Constraint sp_assume(Assume* inst_assume, Constraint phi);

	Constraint sp_assert(StaticAssert* assertion, Constraint phi);

	Constraint sphelper(Instruction* inst, Constraint phi);

	Constraint sp_mod_c(Term* vt, Term* t1, int cons, Constraint phi);

	Constraint sp_cast(Cast* cast, Constraint phi);

	Constraint sp_sub_function(POEKey* poekey_func_entry, POEKey* poekey_func_exit,
				Constraint begin,Function* callingfunc);



	void compute_all_observed_and_I();

	void compute_observedInfo(POEKey* pk_body_entry, set<Constraint>& observedInfo,
				Constraint& ivariantInfo);

	void compute_observedInfo_helper(POEKey* pk_body_entry,
				set<Constraint>& observedInfo, Constraint& ivariantInfo);

	void adjust_I_based_on_P();


public:

	//ComputePO(POEKey* b_entry,Function* func_entry, vector<Function *>& func_vector, bool is_body);

	ComputePO(Function * f,  vector<Function *>& func_vector);

	//ComputePO(BasicBlock * b_entry);

	~ComputePO();

	ComputePO(POEKey *poekey_entry, Function *func, vector<Function *>& func_vector);

	ComputePO(Function* callingfunc, map<POEKey*, ProofObligation*>& map_poekey_po,
				vector<Function *>& func_vector);

	//map<Block*, ProofObligation*> mapBlock;
	map<POEKey*, ProofObligation*> map_poekey_po;
	vector<Function *> func_vector;
	POEKey* cur_eval_key;
	Function* entry_func;
	string var_prefix;



	void print_out_po_info();


	void get_init_evironment(POEKey *poekey_entry, Constraint begin,
			POEKey *poekey_exit, Constraint end, map<POEKey*, ProofObligation*>& map_return);

	Constraint get_block_spInfo(POEKey * b);
	Constraint get_block_wpInfo(POEKey * b);

	/*
	 * getter and setter
	 */
	void get_map_poekey_po(map<POEKey*, ProofObligation*>  & map);
	Constraint get_extra_after_info();

	Constraint get_extra_above_info();

	void set_entry_func(Function * entry_f);


	//void compute_superBlock_R();

	void compute_func_sp(POEKey *poekey_entry, POEKey *poekey_exit, Constraint phi);

	void compute_func_wp(POEKey *poekey_entry, POEKey *poekey_exit, Constraint phi);

	Term* symbol_to_term(Symbol* sym);

	void print_out_block(Block * b);

	void update_phi(Block * b_p, Constraint c_pq, bool is_forward);


	/*
	 * if is_forward is true, then the function computes sp. Otherwise, computes wp.
	 * all results store in mapBlock.
	 */
	void compute_block_swp(bool is_forward, POEKey* key_end_block, bool nested_analysis);


	Constraint compute_superBlock_wp_helper(SuperBlock* sb,
				CallingContext *calling_seq,  Constraint cons, bool is_body);


	void recompute_block_wp(Constraint added_phi, bool is_first_while,
				int line_num, LoopTree& lt);



	void updatePre(POEKey* w, Constraint phi, map<POEKey*,
			ProofObligation*>& mappoekey, LoopTree& lt);

	/*
	 * compute wp(body, c)
	 */
	static Constraint compute_superBlock_wp(POEKey* sb_poekey, Constraint c,
			vector<Function *>& func_vector, Function* entry_func, bool is_body);

	/*
	 * compute all loop conditions
	 */
	void compute_all_loop_condition();
	void compute_all_superBlock_R();



	void sandwitch_loop_sp(POEKey* pk_sb, Constraint phi);
	void sandwitch_loop_wp(POEKey* pk_sb, Constraint cons);
	void updateInnerPO(map<POEKey*, ProofObligation*>& mapBlock,
			list<POEKey*> list_loop_id);

	/*
	 * Isil's additions
	 */
	void compute_modsets();
	void compute_unrolled_sp();
	/*
	 * Adds equalities i=i0 & j=j0 etc. to variables modified in loop.
	 */
	Constraint initialize_precond(Constraint precond, POEKey* key);

	/*
	 * Renames i to ik
	 */
	Constraint add_version_number(Constraint c, POEKey* key,
			ProofObligation* po, int k);

	int get_time();
	int get_unroll_time();



};

#endif /* COMPUTEPO_H_ */
