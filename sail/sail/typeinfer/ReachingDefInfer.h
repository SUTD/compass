/*
 * ReachingDefInfer.h
 *
 *  Created on: Sep 20, 2012
 *      Author: boyang
 */

#ifndef ReachingDefInfer_H_
#define ReachingDefInfer_H_

#include <set>
#include "BasicBlock.h"
#include "ReachingDefVarStat.h"
#include "LivenessInfer.h"
#include "Function.h"
#define TESTING_RENAME true

#define CONSIDER_LOOP true

namespace sail {


class Def{
public:
	sail::Instruction* _inst;
	sail::Symbol* _def;


	Def(sail::Instruction* inst,
			sail::Symbol* var){
		_inst = inst;
		_def = var;
	}

	const string to_string() const{
		string str = "Defintion : ";
		str += _inst->to_string();
		str += "	DEF:: ";
		str += _def->to_string();
		return str;
	}

	~Def(){;}

};

class Reaching{
public:
	sail::Instruction* _inst;
	string _in_use;

	Reaching(sail::Instruction* inst,
			string& s){
		_inst = inst;
		_in_use = s;
	}

	const string to_string() const{
		string str = "Reaching :: ";

		str += _inst->to_string();
		str += "		|| in_use = ";
		str += _in_use;
		return str;
	}

	string& get_in_use(){
		return this->_in_use;
	}

	sail::Instruction* get_reaching_inst(){
		return this->_inst;
	}

	~Reaching(){;}
};



class ReachingDefInfer{

private:

	//The entry block of Cfg
	BasicBlock* entry_block;

	//The exit block of Cfg
	BasicBlock* exit_block;

	//The exception block of Cfg
	BasicBlock* exception_block;




	/************
	 *
	 */
	vector<Instruction*>* ordered_insts;

	/*
	 * map of reaching definition.
	 * Definitions from the second element can reach to the first element.
	 */
	/**********
	 * at some point, string(variable name) comes from a set of instruction that can reach here;
	 * 从当点往上看
	 */
	map<Instruction*,map <string , ReachingDefVarStat*> >  map_reaching;


//	/************************************************************
//	 *** the two map change at each operation(instruction)
//	 *** (the purpose is to keep information at exit point of each block)
//	 *** may be changed when tracking back
//	*******************************************************************/
//	map<Block*, map<string, int>*> most_recently_defined_index;
//
//
//	map<Block*, map<string, Variable*>*> most_recently_defined_var;



	/*********************************************************************
	 * used to track the idx of each variable name;(used in renaming phase)
	 ******************************************************************/
	map<string, int> rename_most_recently_index;

	/********
	 * up-to-bottom, 从当点往下看
	 */
	/*
	 * the reverse reaching map
	 */

	map<Instruction*, set<Instruction*> > map_rev_reaching;


	/*

	 *
	 * mini version of up-to-bottom;

	 * an optimal reaching map. It is generated after liveness analysis.

	 */
	map<Instruction*, set<Instruction*> > map_mini_reaching;



	//added by haiyan;
	LivenessInfer* lni;

	Function* f;


	map<Block*, Block*> backedge_target_source;




	map<Def*, set<Reaching*>> def_use_chain;
	map<Reaching*, set<Def*>> use_def_chain;


	//combined together for the renaming_phase;
	set<Def*> unresolved_def;
	set<Reaching*> unresolved_reaching;
	map<Reaching*, string> resolved_reaching;


	////////////////////////function member/////////////////////
	//used to rename; return bool mean is it renamed or not
//	Variable* rename_variable(Block* cur_block,
//				Variable* original_var);

	//can't not return local variable's reference, it will be
	const string rename_variable(Variable* orig_var);

//	Variable* find_most_recently_var(Block* cur_block, Variable* var);



	/*
	 * transfer function: instIn --> instOUT.
	 * Information of instIn stores in map_reaching.
	 */
	void get_instOUT(Instruction* inst,
			map <string, ReachingDefVarStat*>& set_inst_out);


	/*
	 * updates the map and returns true if data has been changed.
	 */
	bool update_mapping(Instruction* inst,
			map <string, ReachingDefVarStat*>& new_stat, bool isException);


	/*
	 * merge ReachingDefVarStat information from predecessor. (Meet operation)
	 */
	void merge_reaching_info(
			set <map <string, ReachingDefVarStat*> >& input_set,
			map <string, ReachingDefVarStat*>& output_set);


	/*
	 * Compute the block b_cur's start environment by using its predecessors.
	 */
	void compute_block_preoutput(Block* b_cur,
			map <string, ReachingDefVarStat*>& preInstsOut);


	/*
	 * Do reaching definition analysis for a specific block.
	 * The start environment is preInstsOut.
	 * Return true if map_reaching is changed.
	 */
	bool block_reachingDef_analysis(Block* b_cur,
			map <string, ReachingDefVarStat*>& preInstsOut);


	/*
	 * Merge two ReachingDefVarStat set.
	 */
	void merge_set_VarReachingStat(
			map <string, ReachingDefVarStat*> & c_map,
			map <string, ReachingDefVarStat*> & o_map);



//	bool block_renaming(Block* cur_block);

	//compute the content of "most_recently_defined_index" and "most_recently_defined_var"
	//at the start of each block;
//	/void compute_block_input_facts(Block* cur_block);
	void find_succesor_first_inst(Block* block, set<Instruction*>& inst);

//	void compute_entry_block_input_facts(Block* cur_block);

//	bool track_back_defined_vars(Block* cur_block, set<string>& update_vars,
//			set<Instruction*>& reaching_points);

//	void print_out_most_recently_var_index(Block* b_cur);
//
//	void print_out_most_recently_vars(Block* b_cur);

	//keep it and let it used by precise-type-inference!
	map<Instruction*, Def*> def_factory_map;
	map<pair<Instruction*, string>, Reaching*> reaching_factory_map;

	Def* make_def(Instruction* inst);
	Reaching* make_reaching(Instruction* inst, string& in_use);

	void delete_defs();
	void delete_reachings();

public:

	ReachingDefInfer();

	ReachingDefInfer(BasicBlock* bb_entry, BasicBlock* bb_exit,
			BasicBlock* exception_b, Function* f,
			LivenessInfer* lni);

	~ReachingDefInfer();

	/*
	 * invoke reaching definition analysis for current Cfg.
	 */
	void reachingDefAnalysis();



	void compute_miniReaching();

	//for the purpose of renaming;
	void build_def_use_chain();

	set<Reaching*>& get_reaching_points(Def*);


	//only renaming local variables,
	//do not rename argument and return variable;
	void renaming_phase();
	//doing the renaming;
	//void renaming();

	/******
	 * print out current instruction's definition points;
	 */
	void print_current_map_reaching(Instruction* inst);
	/*
	 * print out map_reaching.
	 */
	void print_out_mapping();

	void print_out_rev_map();




	void print_out_mini_map();


	/*****
	 * for the purpose of print the use-def chain or def-use chain;
	 */
	void print_def_use_chain_for_instruction(Def* def);
	void print_use_def_chain_for_instruction(Reaching* reach);
	void print_def_use_chain();
	void print_use_def_chain();


	void get_reachingset(Instruction* inst, set<Instruction*>& set_reaching);


	void get_reachingmini(Instruction* inst, set<Instruction*>& set_reaching);


	void compute_reverse_reaching();




};


}

#endif /* ReachingDefInfer_H_ */
