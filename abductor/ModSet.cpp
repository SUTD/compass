/*
 * ModSet.cpp
 *
 *  Created on: Nov 26, 2012
 *      Author: tdillig
 */

#include "ModSet.h"
#include "BasicBlock.h"
#include "SuperBlock.h"
#include "Assignment.h"
#include "CfgEdge.h"
#include "sail/Variable.h"
#include "term.h"
#include <queue>

using namespace sail;

ModSet::ModSet() {

}

const set<VariableTerm*>& ModSet::get_modset(sail::Block* b)
{
	if(mod_sets.count(b) > 0) {
		return mod_sets[b];
	}

	compute_modset(b, mod_sets[b]);
	return mod_sets[b];

}



void ModSet::compute_modset(sail::Block* b, set<VariableTerm*>& mod_set)
{
	if(b->is_basicblock()){
		BasicBlock* bb = static_cast<BasicBlock*>(b);
		compute_modset_basic_block(bb, mod_set);
		return;
	}

	SuperBlock* sb = static_cast<SuperBlock*>(b);
	BasicBlock* entry = sb->get_entry_block();
	queue<Block*> worklist;
	worklist.push(entry);
	while(!worklist.empty())
	{
		Block* cur = worklist.front();
		worklist.pop();
		const set<VariableTerm*>& cur_mod_set = get_modset(cur);
		mod_set.insert(cur_mod_set.begin(), cur_mod_set.end());
		set<CfgEdge*>& succs = cur->get_successors();
		for(auto it = succs.begin(); it!= succs.end(); it++){
			CfgEdge* e = *it;
			Block* succ = e->get_target();
			worklist.push(succ);
		}
	}




}

void ModSet::compute_modset_basic_block(sail::BasicBlock* b,
		set<VariableTerm*>& mod_set)
{
	vector<Instruction*>& inst = b->get_statements();
	for(auto it =inst.begin(); it!= inst.end(); it++)
	{
		Instruction* cur = *it;
		if(!cur->is_save_instruction()) continue;
		SaveInstruction* si = static_cast<SaveInstruction*>(cur);
		Variable* lhs = si->get_lhs();
		Term* lhs_term = VariableTerm::make(lhs->get_var_name());
		assert(lhs_term->get_term_type() == VARIABLE_TERM);
		VariableTerm* var = static_cast<VariableTerm*>(lhs_term);
		mod_set.insert(var);

	}
}

void ModSet::print_modset(sail::Block* b)
{
	const set<VariableTerm*> modset = get_modset(b);
	cout << "====> MODSET FOR BLOCK " << b->get_block_id() << endl;
	for(auto it = modset.begin(); it!= modset.end(); it++){
		VariableTerm* v = *it;
		cout << "\t" << v->to_string() << endl;
	}
}

ModSet::~ModSet() {

}
