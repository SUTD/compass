/*
 * PreciseTypeInfer.h
 *
 *  Created on: Sep 27, 2012
 *      Author: boyang
 */

#ifndef PRECISETYPEINFER_H_
#define PRECISETYPEINFER_H_


#include <set>
#include "BasicBlock.h"
#include "Binop.h"
#include "Unop.h"
#include "FunctionCall.h"
#include "PreciseTypeVarStat.h"
#include "Function.h"
#include "LivenessInfer.h"

namespace sail {

class PreciseTypeInfer {

private:

	BasicBlock* entry_block;
	BasicBlock* exit_block;
	BasicBlock* exception_block;

	/*
	 * if it's not first, we use the type of defined variable directly
	 */
	bool isFirst;

	/*
	 * map of precise_type
	 * set<PreciseTypeVarStat*> records type of variables at a certain instruction.
	 * including all variables; e.g. (v1 = v2 + v3)
	 * all of the variables have its type;
	 */
	map<Instruction*, map<string, PreciseTypeVarStat*> >  map_precise_type;

	LivenessInfer* lni;


	vector<Instruction*>* order_insts;



	/*
	 * Merge a set of PreciseTypeVarStat set to a set.
	 */
	void merge_type_info(
			set <map <string ,PreciseTypeVarStat*> >& input_set,
			map <string ,PreciseTypeVarStat*>& output_set);


	/*
	 * Merge two PreciseTypeVarStat set.
	 * Add information to output_set.
	 */
	void merge_set_TypeVarStat(
			map <string ,PreciseTypeVarStat*>& cur_set,
			map <string ,PreciseTypeVarStat*>& output_set);


	/*
	 * Do precise type analysis for a specific block.
	 * The start environment is preInstsOut.
	 * Return true if map_precise_type is changed.
	 */
	bool block_precise_type_analysis(Block* b_cur,
			map<string, PreciseTypeVarStat*>& preInstsOut);


	/*
	 * updates map and returns true if data has been changed.
	 */
	bool update_mapping(Instruction* inst,
			map <string ,PreciseTypeVarStat*>& new_stat, bool isException);


	void filter_pre_vars_with_liveness(map<string, PreciseTypeVarStat*>& prevarStatOut,
			const set<string>& liveness);


	/*
	 * Compute the block b_cur's start environment.
	 */
	void compute_block_preoutput(Block* b_cur,
			map<string, PreciseTypeVarStat*>& preInstsOut);


	/*
	 * transfer function: instIn --> instOUT.
	 * Information of instIn stores in map_precise_type.
	 * only two calls made, decide which one
	 */
	void get_instOUT(Instruction* inst1,
			map <string, PreciseTypeVarStat*>& set_inst_out, bool entry_point);


	type* find_var_type_in_Inst(Variable* var, Instruction* inst);


	//void find_inst_stats(Instruction* inst_find,
	//set<PreciseTypeVarStat*>& set_return);

	/*
	 * get the type of Binop instruction return
	 */
	type* get_binop_type(Binop* binop);


	type* get_unop_type(Unop* unop);


	type* get_unop_type_helper(Unop* unop);


	type* get_func_type(FunctionCall* functionCall);

	type* find_type_by_func_info(vector<string>& vec_str);

	type* get_return_or_function_type_info (
			Variable* define_var, type* orig_t,
			Instruction* reaching_point,
			Function* f);
public:


	PreciseTypeInfer(BasicBlock* bb_entry, BasicBlock* bb_exit,
			BasicBlock* exception_b,
			LivenessInfer* lni,
			Function* f);


	/*
	 * invoke reaching definition analysis for current Cfg.
	 */
	void precise_type_analysis(const bool& isFirst);

	~PreciseTypeInfer();

	/*
	 * print out map_reaching.
	 */
	void print_out_mapping();

	void print_out_precise_typeinfer_mapping();


	void get_typeset(Instruction* inst, set<Instruction*>& set_reaching,
			set<type*>& set_type);

	/************
	 * added by haiyan to replace the get_typeset();
	 */

	void get_reaching_points_inf(Variable* lhs, set<Instruction*>& set_reaching,
			map<Instruction*, type*>& reaching_points_info);

	/*************
	 * added by haiyan to support get better type,
	 * if "precise_type" is worse than declared type in "inst"!
	 * "lhs " is used to find declared
	 */
	type* get_better_type(Variable* lhs, type* precise_type,
			Instruction* inst, Function* f,
			bool& need_to_infer_null_pointer);

	/*
		 * get the type has been defined in inst_cur.
		 */
	type* get_cur_type(Instruction* inst_cur);



};

}

#endif /* PRECISETYPEINFER_H_ */
