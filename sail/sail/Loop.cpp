/*
 * Loop.cpp
 *
 *  Created on: Jul 11, 2008
 *      Author: isil
 */

#include "Loop.h"
#include "BasicBlock.h"
#include "CfgEdge.h"
#include "Cfg.h"
#include "util.h"
#include "assert.h"
#include "type.h"

#include "addressof_expression.h"
#include "array_ref_expression.h"
#include "cast_expression.h"
#include "deref_expression.h"
#include "unop_expression.h"

#include "Variable.h"
#include "Constant.h"
#include "Assignment.h"
#include "Binop.h"
#include "LoopInvocationInstruction.h"
#include "Function.h"
#include "Assume.h"
#include "DropVariable.h"
#include <stack>



namespace sail {

static int loop_counter = 1;

void reset_loop_count()
{
	loop_counter = 1;
}

int get_loop_id()
{
	return loop_counter++;
}


Loop::Loop(CfgEdge* backedge)
{
	this->loop_id = get_loop_id();
	this->exit_block = NULL;
	backedges.insert(backedge);
	this->header = (BasicBlock*)backedge->get_target();
	construct_body(backedge);

}

void Loop::add_backedge(CfgEdge* backedge)
{
	backedges.insert(backedge);
	construct_body(backedge);
}

BasicBlock* Loop::get_header() const
{
	return this->header;
}

BasicBlock* Loop::get_exit_block() const
{
	return this->exit_block;
}


set<BasicBlock*>& Loop::get_body()
{
	return this->body;
}
set<CfgEdge*>& Loop::get_backedges()
{
	return this->backedges;
}

string Loop::to_string() const
{
	string res = "Loop " + int_to_string(loop_id) + "\n";
	res += "\t Header: " + int_to_string(header->get_block_id()) + "\n";
	res += "\t Backedges: " ;
	set<CfgEdge*>::iterator it = backedges.begin();
	for(; it!=backedges.end(); it++){
		BasicBlock* source = (BasicBlock*)(*it)->get_source();
		BasicBlock* target = (BasicBlock*)(*it)->get_target();
		res+= int_to_string(source->get_block_id()) + "->" +
			int_to_string(target->get_block_id()) + " ";
	}
	res += "\n \t Body: ";
	set<BasicBlock*>::iterator bit;
	for(bit=body.begin(); bit!=body.end(); bit++)
	{
		BasicBlock* cur = *bit;
		res += int_to_string(cur->get_block_id()) + " ";
	}
	res += "\n";
	return res;
}

void Loop::construct_body(CfgEdge* backedge)
{
	stack<BasicBlock*> worklist;
	BasicBlock* source = (BasicBlock*)backedge->get_source();
	BasicBlock* target = (BasicBlock*)backedge->get_target();
	body.insert(target);
	body.insert(source);
	/*
	 * Check for degenerate case if loop has only one node.
	 */
	if(source == target)
		return;
	worklist.push((BasicBlock*)backedge->get_source());



	while(!worklist.empty())
	{
		BasicBlock* cur = worklist.top();
		worklist.pop();
		set<CfgEdge*>::iterator pred_it = cur->get_predecessors().begin();
		for(; pred_it != cur->get_predecessors().end(); pred_it++){
			BasicBlock* cur_pred = (BasicBlock*)(*pred_it)->get_source();
			if(body.count(cur_pred)>0)
				continue;
			body.insert(cur_pred);
			worklist.push(cur_pred);
		}
	}
}

void Loop::get_edges(set<CfgEdge*>& edges)
{
	set<BasicBlock*>::iterator it = body.begin();
	for(; it!=body.end(); it++)
	{
		BasicBlock* cur = *it;
		edges.insert(cur->get_successors().begin(), cur->get_successors().end());
		edges.insert(cur->get_predecessors().begin(), cur->get_predecessors().end());
	}

}

void Loop::add_parent_loop(Loop *p)
{
	this->parent_loops.insert(p);
}

void Loop::update_parents(BasicBlock *b, bool add_to_self)
{
	if(add_to_self) body.insert(b);
	set<Loop*>::iterator it = parent_loops.begin();
	for(; it!= parent_loops.end(); it++)
	{
		Loop *p = *it;
		if(add_to_self || p != this)
			p->get_body().insert(b);
	}
}


void Loop::make_unique_exit(set<BasicBlock*> & basic_blocks)
{
	set<CfgEdge*> exit_edges;
	set<CfgEdge*> _loop_edges;
	get_edges(_loop_edges);
	multiset<CfgEdge*, CompareEdge> loop_edges;
	loop_edges.insert(_loop_edges.begin(), _loop_edges.end());
	exit_block = NULL;
	multiset<CfgEdge*, CompareEdge>::iterator it = loop_edges.begin();
	map<BasicBlock*, int> exit_to_id;
	int id = 0;
	bool has_backedge = false;

	map<int, set<il::node*> > exit_conds;


	for(;it!= loop_edges.end(); it++)
	{
		CfgEdge *e = *it;
		if(body.count((BasicBlock*)e->get_target()) != 0)
			continue;
		exit_edges.insert(e);
		if(e->is_backedge())
			has_backedge = true;
		BasicBlock* b = (BasicBlock*)e->get_target();
		if(exit_to_id.count(b) > 0) continue;
		exit_to_id[b] = id++;
	}

	if(exit_to_id.size() == 1 && !has_backedge)
	{
		exit_block = new BasicBlock();
		update_parents(exit_block, true);
		basic_blocks.insert(exit_block);
		it = exit_edges.begin();
		BasicBlock* old_target = (BasicBlock*)(*it)->get_target();
		for(;it != exit_edges.end(); it++)
		{
			CfgEdge *e = *it;
			if(e->get_cond()!=NULL){
				exit_conds[-1].insert(e->get_cond()->get_original_node());
			}
			exit_block->add_predecessor_edge(e);
			old_target->get_predecessors().erase(e);
			e->set_target(exit_block);
		}
		CfgEdge* e2 = new CfgEdge(exit_block, old_target, NULL);
		exit_block->add_successor_edge(e2);
		old_target->add_predecessor_edge(e2);
		exit_blocks.insert(exit_block);
	}
	else if(exit_edges.size() > 1 || has_backedge)
	{
		string name = "exit_" + int_to_string(this->header->get_block_id());
		Variable* cond = new Variable(name, il::get_integer_type(), true);
		exit_block = new BasicBlock();
		exit_blocks.insert(exit_block);
		update_parents(exit_block, true);
		basic_blocks.insert(exit_block);
		BasicBlock *exit_block_outside = new BasicBlock();
		update_parents(exit_block_outside, false);
		basic_blocks.insert(exit_block_outside);

		it = exit_edges.begin();

		map<BasicBlock*, bool> backedge_map;
		for(;it != exit_edges.end(); it++)
		{
			CfgEdge *e = *it;

			BasicBlock* source = (BasicBlock*) e->get_source();
			BasicBlock* target = (BasicBlock*) e->get_target();
			backedge_map[target] = e->is_backedge();
			e->unmark_backedge();
			assert(exit_to_id.count(target) > 0);
			if(exit_edges.size() > 1)
			{
				Constant* c = new Constant(exit_to_id[target], false, 4);
				Instruction* assign = new Assignment(cond, c, NULL, -1);

				if(e->get_cond()!=NULL)
					exit_conds[exit_to_id[target]].
					insert(e->get_cond()->get_original_node());

				// If the source has a single successor, we can add exit
				// condition directly to source
				if(source->get_successors().size() <=1){
					source->get_statements().push_back(assign);
				}
				// If it has multiple successors, we need to
				// generate a new block to insert the appropriate exit code.
				else{
					BasicBlock* exit_code_block = new BasicBlock();
					exit_blocks.insert(exit_code_block);
					update_parents(exit_code_block, true);
					basic_blocks.insert(exit_code_block);
					exit_code_block->add_statement(assign);

					//rewire edges
					CfgEdge* new_edge = new CfgEdge(e->get_source(), exit_code_block,
							e->get_cond());
					e->set_cond(NULL);
					source->get_successors().erase(e);
					source->add_successor_edge(new_edge);
					exit_code_block->add_predecessor_edge(new_edge);
					e->set_source(exit_code_block);
					exit_code_block->add_successor_edge(e);

				}
			}
			BasicBlock* old_target = (BasicBlock*) e->get_target();
			exit_block->add_predecessor_edge(e);
			old_target->get_predecessors().erase(e);

			e->set_target(exit_block);
		}

		CfgEdge* exit_edge = new CfgEdge(exit_block, exit_block_outside, NULL);
		exit_block->add_successor_edge(exit_edge);
		exit_block_outside->add_predecessor_edge(exit_edge);


		map<BasicBlock*, int>::iterator mi = exit_to_id.begin();
		for(; mi!= exit_to_id.end(); mi++)
		{
			BasicBlock* old_target = mi->first;
			assert(exit_to_id.count(old_target) > 0);
			Constant* c = new Constant(exit_to_id[old_target], false, 4);
			Variable *cur_cond = NULL;
			Binop *b = NULL;
			if(exit_to_id.size() != 1)
			{
				if(exit_to_id[old_target] == (int)exit_to_id.size()-1)
				{
					string alias_name = cond->to_string(true) + ">=" +
						c->to_string(true);
					cur_cond = new Variable(alias_name, il::get_integer_type());
					b = new Binop(cur_cond, cond, c, il::_GEQ, NULL, -1);
				}
				else
				{
					string alias_name = cond->to_string(true) + "==" +
										c->to_string(true);
					cur_cond = new Variable(alias_name, il::get_integer_type());
					b = new Binop(cur_cond, cond, c, il::_EQ, NULL, -1);
				}
				((BasicBlock*)exit_block_outside)->get_statements().push_back(b);
			}


			CfgEdge* e2 = new CfgEdge(exit_block_outside, old_target, cur_cond);
			if(backedge_map[old_target] == true)
				e2->mark_backedge();
			exit_block_outside->add_successor_edge(e2);
			old_target->add_predecessor_edge(e2);
		}
	}

	/*
	 * Deal with loops that have no exit point
	 */
	if(exit_block == NULL)
	{
		exit_block = new BasicBlock();
		exit_blocks.insert(exit_block);
		update_parents(exit_block, true);
		basic_blocks.insert(exit_block);
	}
}








set<BasicBlock*>& Loop::get_exit_blocks()
{
	return exit_blocks;
}

void Loop::insert_recursive_calls(set<BasicBlock*> & basic_blocks)
{
	set<CfgEdge*> loop_edges;
	get_edges(loop_edges);
	set<CfgEdge*>::iterator it = loop_edges.begin();
	for(;it!= loop_edges.end(); it++)
	{
		CfgEdge* e = *it;
		if(!e->is_backedge())
			continue;
		BasicBlock* target = (BasicBlock*)e->get_target();
		BasicBlock* source = (BasicBlock*) e->get_source();
		BasicBlock* to_insert_block= source;
		assert(source->get_successors().size()>=1);
		if(source->get_successors().size()>1)
		{
			to_insert_block = new BasicBlock();
			basic_blocks.insert(to_insert_block);
			update_parents(to_insert_block, true);
			BasicBlock* old_target = (BasicBlock*) e->get_target();
			old_target->get_predecessors().erase(e);

			e->set_target(to_insert_block);
			e->unmark_backedge();
			to_insert_block->add_predecessor_edge(e);


		}
		else
		{
			BasicBlock* old_target = (BasicBlock*) e->get_target();
			old_target->get_predecessors().erase(e);
			source->get_successors().erase(e);
			delete e;
		}
		to_insert_block->get_statements().push_back(
				new LoopInvocationInstruction(target));



	}
}





Loop::~Loop()
{

}

bool LoopCompare::operator()(const Loop* b1, const Loop* b2) const
{
	return (b1->get_header()->get_block_id() >
			b2->get_header()->get_block_id());
}

}
