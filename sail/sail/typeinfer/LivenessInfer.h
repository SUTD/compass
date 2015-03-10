/*
 * LivenessInfer.h
 *
 *  Created on: Mar 15, 2013
 *      Author: boyang
 */

#ifndef LivenessInfer_H_
#define LivenessInfer_H_

#include <set>
#include "BasicBlock.h"
#include "ReachingDefVarStat.h"
#include "Function.h"
#include "Loop.h"

#define CONSIDER_LOOP true
namespace sail {



class LivenessInfer{

private:

	//The entry block of Cfg
	BasicBlock* entry_block;

	BasicBlock* exit_block;

	BasicBlock* exception_block;

	//a set of blocks without following block
	set<Block*> set_ends;


	/*
	 * map of instruction and name of live variables
	 */
	/*****
	 *	to each point(instruction level), set<string> a set of variable name that it's still alive
	 */
	map<Instruction*,set<string> >  map_liveness;

	Function* f;

	map<Block*, Block*> backedge_source_target;



public:

	LivenessInfer();


	LivenessInfer(BasicBlock* bb_entry, BasicBlock* bb_exit,
			BasicBlock* exception_b, Function* f);


	~LivenessInfer();


	void LivenessInferAnalysis();



	bool block_liveness_analysis(BasicBlock* bb_cur, set<string> set_in);


	bool update_mapping(Instruction* inst, const set<string>& new_stat);


	/*
	 * compute liveness right after the current block. We merge all
	 * information from following blocks for the current block.
	 */
	void compute_block_afteroutput(Block* b_cur,
			set<string>& afterInstsOut);


	/*
	 * get liveness right before the instruction.
	 * Find the information from storage map.
	 */
	void getLivenessByInst(Instruction* inst, set<string>& set_liveness);



	void eliminate_reaching(string v_name, set<Instruction*>& set_inst);



	/*
	 * compute current instruction's liveness (right before)
	 * The set_out is computed based on set_in.
	 */
	void get_instOUT(Instruction* inst, set<string>& set_in,
			set<string>& set_out);


	void print_out_mapping();

};

}

#endif /* LivenessInfer_H_ */
