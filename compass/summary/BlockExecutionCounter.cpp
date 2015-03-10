/*
 * BlockExecutionCounter.cpp
 *
 *  Created on: Sep 26, 2009
 *      Author: tdillig
 */

#include "BlockExecutionCounter.h"
#include <vector>
#include "sail/SuperBlock.h"
#include "sail/BasicBlock.h"
#include "sail/Block.h"
#include "sail/Cfg.h"
#include "sail/CfgEdge.h"
#include "ConstraintGenerator.h"
#include "Variable.h"
#include "ConstantValue.h"
#include "IterationCounter.h"
#include <set>
#include "compass_assert.h"

#define DEBUG false

BlockExecutionCounter::BlockExecutionCounter(sail::SuperBlock* su)
{
	counter = 0;
	this->su = su;
	sail::Block* entry = su->get_entry_block();
	set<sail::Block*, sail::CompareBlock> worklist;
	worklist.insert(entry);

	set<int> invocation_blocks;

	while(worklist.size() > 0)
	{
		sail::Block* b = *worklist.begin();
		worklist.erase(b);
		if(is_exit_block(b)) continue;
		set<sail::CfgEdge*>& preds = b->get_predecessors();
		set<sail::CfgEdge*>::iterator it = preds.begin();
		int cur_id = counter;

		if(b->is_basicblock())
		{
			sail::BasicBlock* bb= (sail::BasicBlock*) b;
			vector<sail::Instruction*>& stmts = bb->get_statements();
			if(stmts.size() > 0) {
				sail::Instruction* last = stmts[stmts.size() - 1];
				if(last->get_instruction_id() == sail::LOOP_INVOCATION ) {
					invocation_blocks.insert(bb->get_block_id());
				}
			}
		}

		for(; it!= preds.end(); it++)
		{
			sail::Block* pred = (*it)->get_source();
			int pred_id = block_to_counter[pred->get_block_id()];
			bool has_exit = has_exit_children(pred);
			if( pred_id != cur_id || has_exit) {
				cur_id = counter+1;
				IterationCounter* cur_v = IterationCounter::make_last(cur_id);
				IterationCounter* pred_v =  IterationCounter::make_last(pred_id);
				Constraint c = ConstraintGenerator::get_leq_constraint(cur_v, pred_v);
				counter_relations &=c;

			}

		}



		if(cur_id != counter) {
			counter++;
		}
		block_to_counter[b->get_block_id()] = cur_id;

		// enque successors
		set<sail::CfgEdge*>& succs = b->get_successors();
		for(it=succs.begin(); it!=succs.end(); it++){
			worklist.insert((*it)->get_target());
		}

	}

	/*
	 * Special blocks indicating which exit point was taken should have the
	 * same counter as the parent block.
	 */
	sail::BasicBlock* exit = su->get_exit_block();
	set<sail::CfgEdge*>& preds = exit->get_predecessors();
	set<sail::CfgEdge*>::iterator it = preds.begin();
	for(; it!= preds.end(); it++) {
		sail::CfgEdge* e = *it;
		sail::Block* s = e->get_source();
		if(block_to_counter.count(s->get_block_id()) > 0) continue;
		c_assert(s->get_predecessors().size() == 1);
		sail::Block* pred = (*s->get_predecessors().begin())->get_source();
		c_assert(block_to_counter.count(pred->get_block_id()) > 0);
		block_to_counter[s->get_block_id()] =
				block_to_counter[pred->get_block_id()];
	}


	if(counter!=0) {
		IterationCounter* entry_v = IterationCounter::make_last(0);
		ConstantValue* cv = ConstantValue::make(1);
		set<int>::iterator it = invocation_blocks.begin();
		for(; it!=invocation_blocks.end(); it++)
		{
			int id = *it;
			int counter = block_to_counter[id];
			IterationCounter* cur_v = IterationCounter::make_last(counter);
			if(cur_v == entry_v) continue;
			Constraint c = ConstraintGenerator::get_sum_constraint(entry_v,
					cur_v, cv);
			counter_relations &= c;

		}

	}
	counter_relations &=
			ConstraintGenerator::get_gtz_constraint(
					IterationCounter::make_last(0));
	c_assert(counter_relations.sat());

	if(DEBUG)
	{
		cout << "===========BLOCK TO COUNTERS=========" << endl;
		map<int, int>::iterator it = block_to_counter.begin();
		for(; it!= block_to_counter.end(); it++)
		{
			int block_id = it->first;
			int var = it->second;

			cout << " \t Block " <<  block_id << ": N" << var << endl;
		}
		cout << "===================================" << endl;
		cout << "Counter relations: " << counter_relations << endl;
	}

}

Constraint BlockExecutionCounter::get_counter_relations()
{
	return counter_relations;
}


int BlockExecutionCounter::get_max_counter()
{
	return counter;
}

BlockExecutionCounter::~BlockExecutionCounter()
{

}

int BlockExecutionCounter::get_iteration_counter(int block_id)
{
	if(block_to_counter.count(block_id) == 0) return -1;
	return block_to_counter[block_id];
}

bool BlockExecutionCounter::has_exit_children(sail::Block* b)
{
	set<sail::CfgEdge*> & succs = b->get_successors();
	set<sail::CfgEdge*>::iterator it = succs.begin();
	for(; it!=succs.end(); it++) {
		sail::CfgEdge* c = *it;
		sail::Block* t = c->get_target();
		if(is_exit_block(t)) return true;
	}
	return false;
}

bool BlockExecutionCounter::is_exit_block(sail::Block* b)
{
	if(b == su->get_exit_block()) return true;
	if(b->get_successors().size() != 1) return false;
	sail::Block* succ = (*(b->get_successors().begin()))->get_target();
	return succ == su->get_exit_block();
}
