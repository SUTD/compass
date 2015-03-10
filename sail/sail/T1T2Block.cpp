/*
 * T1T2Block.cpp
 *
 *  Created on: Jul 8, 2008
 *      Author: isil
 */


#include "T1T2Block.h"
#include "BasicBlock.h"
#include "CfgEdge.h"
#include <map>
#include <set>
#include <assert.h>
#include "util.h"
#include <iostream>
using namespace std;

#define DEBUG false

namespace sail {

static map<BasicBlock*, T1T2Block*> existing_blocks;
static set<T1T2Block*> to_delete;

T1T2Block* make_T1T2Block(BasicBlock* b)
{
	if(existing_blocks[b]!=NULL)
		return existing_blocks[b];
	T1T2Block* t = new T1T2Block(b);
	to_delete.insert(t);
	existing_blocks[b] = t;
	set<CfgEdge*>::iterator it = b->get_successors().begin();
	for(;it!= b->get_successors().end(); it++)
	{
		BasicBlock *cur = (BasicBlock*)(*it)->get_target();
		if(cur == b) continue;
		T1T2Block *cur_t = make_T1T2Block(cur);
		t->succs.insert(cur_t);
	}
	it = b->get_predecessors().begin();
	for(;it!= b->get_predecessors().end(); it++)
	{
		BasicBlock *cur = (BasicBlock*)(*it)->get_source();
		if(cur == b) continue;
		T1T2Block *cur_t = make_T1T2Block(cur);
		t->preds.insert(cur_t);
	}
	t->pred_count = t->preds.size();

	return t;

}

/**
 * This constructor is only to be used for node splitting!!
 * This constructor needs the set of blocks in the CFG because
 * it needs to add the newly splitted basic block to the
 * set of nodes in the CFG.
 */
T1T2Block::T1T2Block(const T1T2Block& other, T1T2Block*
		pred, BasicBlock* rep, set<BasicBlock*>& representees)
{
	// The reason we split is to make pred count 1.
	this->pred_count = 1;
	this->preds.insert(pred);

	this->succs = other.succs;
	this->representative_block = rep;
	this->representees = representees;



	// After node splitting, the representative of the T1T2 block
	// becomes invalid
	this->representative_block = NULL;
}

void clear_map_t1t2()
{
	set<T1T2Block*>::iterator it =
		to_delete.begin();
	for(;it!= to_delete.end(); it++)
	{
		delete *it;
	}
	existing_blocks.clear();
	to_delete.clear();

}

T1T2Block::T1T2Block(BasicBlock *b) {

	this->representative_block = b;
	this->representees.insert(b);

}

/*
 * Splits a node that is the source of irreducibility
 */
void T1T2Block::split_node(set<T1T2Block*, Compare>& heap,
			set<BasicBlock*>& blocks, BasicBlock* & old_exit,
			BasicBlock* & old_exception)
{


	heap.erase(this);
	set<BasicBlock*> exit_blocks;
	set<BasicBlock*> exception_blocks;

	BasicBlock* old_rep = this->representative_block;
	set<CfgEdge*>::iterator it = old_rep->get_predecessors().begin();
	for(; it!= old_rep->get_predecessors().end(); it++)
	{
		BasicBlock *pred = (BasicBlock*)(*it)->get_source();
		if(this->representees.count(pred) > 0)
			continue;
		BasicBlock *new_rep = copy_equivalence_class(old_rep, this->representees,
				blocks, exit_blocks, exception_blocks, old_exit, old_exception);

		set<CfgEdge*>::iterator pred_it = new_rep->get_predecessors().begin();
		CfgEdge* actual_pred = NULL;
		for(;pred_it!=new_rep->get_predecessors().end(); pred_it++)
		{
			if((*pred_it)->get_source() == pred)
				actual_pred = *pred_it;
			else{
				BasicBlock* source  =(BasicBlock*)(*pred_it)->get_source();
				source->get_successors().erase(*pred_it);
				delete *pred_it;
			}
		}
		assert(actual_pred != NULL);
		new_rep->get_predecessors().clear();
		new_rep->get_predecessors().insert(actual_pred);
	}
	clear_original_class(blocks);

	set<T1T2Block*>::iterator pred_it = this->preds.begin();
	for(; pred_it!=preds.end(); pred_it++)
	{
		T1T2Block* pred = *pred_it;
		set<BasicBlock*> empty_set;
		T1T2Block* copy = new T1T2Block(*this, pred,
				NULL, empty_set);
				//pred_to_representative[pred_rep], pred_to_class[pred_rep]);
		to_delete.insert(copy);
		pred->succs.erase(this);
		pred->succs.insert(copy);
		heap.insert(copy);
	}

	if(exit_blocks.size()==1)
		old_exit = *exit_blocks.begin();
	if(exception_blocks.size()==1)
		old_exception = *exception_blocks.begin();
	if(exit_blocks.size()>1)
	{
		BasicBlock * new_exit = new BasicBlock();
		blocks.insert(new_exit);
		old_exit = new_exit;
		//do not make a new T1T2 block
		set<BasicBlock*>::iterator itt = exit_blocks.begin();
		for(;itt != exit_blocks.end(); itt++)
		{
			connect_blocks((*itt), new_exit, NULL);
		}

	}


	if(exception_blocks.size()>1)
	{
		BasicBlock * new_exception = new BasicBlock();
		blocks.insert(new_exception);
		old_exception = new_exception;
		//do not make a new T1T2 block
		set<BasicBlock*>::iterator itt = exception_blocks.begin();
		for(;itt != exception_blocks.end(); itt++)
		{
			connect_blocks((*itt), new_exception, NULL);
		}

	}







}

/**
 * Conects b1 to b2 via cond edge
 */
void T1T2Block::connect_blocks(BasicBlock* b1, BasicBlock*b2, Symbol* cond)
{
	CfgEdge* e = new CfgEdge(b1, b2, cond);
	b1->get_successors().insert(e);
	b2->get_predecessors().insert(e);
}

void T1T2Block::clear_original_class(set<BasicBlock*>& blocks)
{
	set<CfgEdge*>* edges =  get_edges(this->representees);
	set<CfgEdge*>::iterator ei = edges->begin();
	for(; ei != edges->end(); ei++)
	{
		BasicBlock* source = (BasicBlock*)(*ei)->get_source();
		BasicBlock* target = (BasicBlock*)(*ei)->get_target();
		delete *ei;
		if(representees.count(source) > 0)
		{
			if(blocks.count(source) > 0)
			{
				blocks.erase(source);
				delete source;
			}
		}
		else assert(blocks.count(source) > 0);

		if(representees.count(target) > 0)
		{
			if(blocks.count(target) > 0)
			{
				blocks.erase(target);
				delete target;
			}
		}
		else assert(blocks.count(target) > 0);
	}
	delete edges;
}


set<CfgEdge*>* T1T2Block::get_edges(set<BasicBlock*> & rep_class)
{

	set<CfgEdge*>* res = new set<CfgEdge*>();
	set<BasicBlock*>::iterator it = rep_class.begin();
	for(;it!=rep_class.end(); it++)
	{
		BasicBlock *cur = *it;
		res->insert(cur->get_successors().begin(), cur->get_successors().end());
		res->insert(cur->get_predecessors().begin(), cur->get_predecessors().end());
	}
	return res;

}


BasicBlock* T1T2Block::copy_equivalence_class(BasicBlock* rep,
			set<BasicBlock*> & rep_class, set<BasicBlock*>& blocks,
			set<BasicBlock*> & exit_blocks, set<BasicBlock*> & exception_blocks,
			BasicBlock* & old_exit, BasicBlock* & old_exception)
{
	map<BasicBlock*, BasicBlock*> old_to_new;
	set<CfgEdge*>* edges =  get_edges(rep_class);
	set<CfgEdge*>::iterator ei = edges->begin();
	for(; ei != edges->end(); ei++)
	{
		CfgEdge* new_edge = new CfgEdge(**ei);
		BasicBlock *new_source = get_block((BasicBlock*)new_edge->get_source(), rep_class,
				blocks, old_to_new, exit_blocks, exception_blocks, old_exit,
				old_exception);
		BasicBlock *new_target = get_block((BasicBlock*)new_edge->get_target(), rep_class,
					blocks, old_to_new, exit_blocks, exception_blocks, old_exit,
					old_exception);
		new_edge->set_source(new_source);
		new_edge->set_target(new_target);



		new_source->get_successors().erase(*ei);
		new_source->get_successors().insert(new_edge);
		new_target->get_predecessors().erase(*ei);
		new_target->get_predecessors().insert(new_edge);

	}

	assert(old_to_new.count(rep)>0);
	delete edges;
	return old_to_new[rep];
}


BasicBlock* T1T2Block::get_block(BasicBlock*old, set<BasicBlock*> & rep_class,
			set<BasicBlock*>& blocks, map<BasicBlock*, BasicBlock*> &old_to_new,
			set<BasicBlock*> & exit_blocks, set<BasicBlock*> & exception_blocks,
			BasicBlock* & old_exit, BasicBlock* & old_exception)
{
	if(rep_class.count(old) == 0)
		return old;
	if(old_to_new.count(old)>0)
		return old_to_new[old];


	BasicBlock* new_block = new BasicBlock(*old);
	blocks.insert(new_block);
	old_to_new[old] = new_block;
	if(old_exit == old)
		exit_blocks.insert(new_block);
	if(old_exception == old)
		exception_blocks.insert(new_block);
	return new_block;
}

/**
 * Performs T1-T2 transformation
 */
void T1T2Block::merge(T1T2Block *other,
		set<T1T2Block*, Compare>  & heap)
{
	assert(other->pred_count == 1);
	heap.erase(this);
	//T2
	// Join equivalence classes; break edges.
	this->representees.insert(other->representees.begin(),
				other->representees.end());
	//invariant check: all pred of other must be in representees
	if(DEBUG)
	{
		set<BasicBlock*>::iterator other_reps = other->representees.begin();
		for(; other_reps!= other->representees.end();other_reps++)
		{
			set<CfgEdge*>::iterator check_it = (*other_reps)->get_predecessors().begin();
			for(; check_it != (*other_reps)->get_predecessors().end(); check_it++)
			{

					assert(representees.count((BasicBlock*)(*check_it)->get_source()) > 0);
			}
		}
	}



	this->succs.erase(other);
	other->preds.erase(this);
	other->pred_count--;
	// Redirect edges of other
	set<T1T2Block*> to_remove;
	set<T1T2Block*>::iterator it = other->succs.begin();
	for(; it!= other->succs.end(); it++){
		T1T2Block* to_update = *it;
		// unshared successor
		if(this->succs.count(to_update) == 0){
			to_update->preds.erase(other);
			to_remove.insert(to_update);
			to_update->preds.insert(this);
			this->succs.insert(to_update);
			continue;
		}

		// shared successor
		heap.erase(to_update);
		to_update->preds.erase(other);
		to_remove.insert(to_update);
		(to_update)->pred_count--;
		heap.insert(to_update);

	}
	set<T1T2Block*>::iterator _it = to_remove.begin();
	for(;_it!=to_remove.end(); _it++){
		other->succs.erase(*_it);
	}
	heap.insert(this);
	// If there is a self loop, delete self loop.
	if(this->preds.count(this) != 0){
		heap.erase(this);
		this->pred_count--;
		this->preds.erase(this);
		assert(this->succs.count(this) !=0);
		this->succs.erase(this);
		heap.insert(this);
	}
}


T1T2Block::~T1T2Block() {
	// TODO Auto-generated destructor stub
}

bool Compare::operator()(const T1T2Block* b1, const T1T2Block* b2) const
{
	if(b1->pred_count < b2->pred_count) return true;
	if(b1->pred_count > b2->pred_count) return false;

	if(b1->representative_block == NULL && b2->representative_block != NULL)
	{
		return true;
	}
	if(b1->representative_block != NULL && b2->representative_block == NULL)
	{
		return false;
	}

	if(b1->representative_block == NULL && b2->representative_block == NULL)
	{
		if(b1<b2) return true;
			return false;
	}
	if(b1->representees.size() < b2->representees.size())
		return true;

	if(b1->representees.size() > b2->representees.size())
		return false;

	if(b1->representative_block->get_statements().size() <
			b2->representative_block->get_statements().size())
		return true;
	if(b1->representative_block->get_statements().size() >
			b2->representative_block->get_statements().size())
		return false;

	if(b1<b2) return true;
	return false;

}

}
