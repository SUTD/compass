/*
 * POE.h
 *
 *  Created on: May 18, 2012
 *      Author: boyang
 */

#ifndef POE_H_
#define POE_H_



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
#include "LoopTree.h"

#include "ProofObligation.h"
#include "POEKey.h"

#include <set>

using namespace std;
using namespace sail;
using namespace il;



class POE {

private:
	vector<Function *> func_vector;
	Function *entry_func;

	map<POEKey *, ProofObligation*> map_poekey_po;


	Constraint precondition;

	/*
	 * eliminate temp variables in the map
	 */
	void eliminate_temp_for_map();
	void eliminate_temp(Constraint & c_before);
	int po_time;
	int unroll_time;

public:

	POE();
	POE(const POE& other);
	POE(Function* f);

	POE(vector<Function *> func_vector, Function *entry_func);

	void setPrecondition(Constraint precondition);

	bool verify();

	void init_env();

	map<Block*, ProofObligation*> get_block_po(BasicBlock * b_entry);

	void print_out_po_info();

	bool strengthen_pre(SuperBlock* b_while, ProofObligation* po_while);

	POE* updatePre(POEKey* w, Constraint added_pre, LoopTree& lt);

	POE* updateInv(POEKey* w, Constraint new_inv);

	void get_previous_sb(SuperBlock* sb_cur, set<SuperBlock*>& previous_blocks);

	void get_map_POEKey_po(map<POEKey*, ProofObligation*> & map_return);

	ProofObligation* find_ProofObligation(POEKey * b_p);

	POE* updateInnerPO(POEKey* pk_w, Constraint r, list<POEKey*>& list_loop_id);

	~POE();

	int get_init_po_time();
	int get_loop_unroll_time();
};



#endif /* POE_H_ */
