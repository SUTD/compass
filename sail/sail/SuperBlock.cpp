/*
 * SuperBlock.cpp
 *
 *  Created on: Jul 11, 2008
 *      Author: isil
 */

#include "SuperBlock.h"
#include "Loop.h"
#include "CfgEdge.h"
#include "BasicBlock.h"
#include "util.h"
#include "type.h"
#include "Variable.h"
#include "Constant.h"
#include "Assignment.h"
#include "Binop.h"
#include "Function.h"
#include <string>
using namespace std;

namespace sail {

SuperBlock::SuperBlock(Loop* l, Function* parent_fn) {
	this->block_id = l->get_header()->get_block_id();
	entry_block = l->get_header();
	exit_block = l->get_exit_block();
	exit_blocks = l->get_exit_blocks();
	this->id = Identifier(parent_fn->get_identifier(),
			entry_block->get_block_id());
	this->parent_fn = parent_fn;
	set<Block*> worklist;
	worklist.insert(entry_block);
	while(worklist.size() > 0)
	{
		Block* cur = *worklist.begin();
		worklist.erase(cur);
		body.insert(cur);
		if(cur == this->get_exit_block())
			continue;
		set<CfgEdge*>::iterator it = cur->get_successors().begin();
		for(;it != cur->get_successors().end(); it++)
		{
			worklist.insert((*it)->get_target());
		}
	}

	set<CfgEdge*>::iterator it = entry_block->get_predecessors().begin();
	for(; it != entry_block->get_predecessors().end(); it++)
	{
		CfgEdge* e = *it;
		e->set_target(this);
		predecessors.insert(e);
	}
	entry_block->get_predecessors().clear();
	assert(exit_block != NULL);
	assert(exit_block->get_successors().size() == 1 ||
			exit_block->get_successors().size() == 0);
	if(exit_block->get_successors().size() == 0)
		return;
	CfgEdge *e = *(exit_block->get_successors().begin());
	e->set_source(this);
	exit_block->get_successors().clear();
	successors.insert(e);


}
string SuperBlock::to_dotty(string prelude, bool pretty_print)
{

	string res = "subgraph cluster" + int_to_string(this->get_block_id()) + "{\n";

	set<Block*>::iterator it = body.begin();
	for(; it!= body.end(); it++)
	{
		res+= (*it)->to_dotty("", pretty_print);
	}


	set<CfgEdge*>& succs = get_successors();


	set<CfgEdge*>::iterator succs_it = succs.begin();
	res += "}\n";
	for(; succs_it!=succs.end(); succs_it++){
		CfgEdge* succ_edge = *succs_it;
		string edge_style = succ_edge->is_backedge() ? "[style=dotted]" : "";
		long succ_id = succ_edge->get_target()->get_block_id();
		if(succ_id == -1) succ_id = (long) succ_edge->get_target();
		res+= "node" + int_to_string(exit_block->get_block_id()) + " -> " +
			"node" + int_to_string(succ_id) + " [label = \"" +
			(succ_edge->get_cond() ?
					escape_string(succ_edge->get_cond()->to_string(true)) : "true") +
			"\"]" + edge_style + "\n";
	}

	return res;
}

BasicBlock* SuperBlock::get_entry_block()
{
	return entry_block;
}
BasicBlock* SuperBlock::get_exit_block()
{
	return exit_block;
}
BasicBlock* SuperBlock::get_exception_block()
{
	return NULL;
}

set<Block*> & SuperBlock::get_body()
{
	return body;
}

bool SuperBlock::is_exit_block(BasicBlock* b)
{
	return exit_blocks.count(b) > 0;
}

sail::Function* SuperBlock::get_parent_function()
{
	return parent_fn;
}

void SuperBlock::get_loop_invocation_blocks(set<Block*>& blocks)
{
	for(auto it = body.begin(); it!= body.end(); it++){
		Block* cur = *it;
		if(cur->is_superblock()) continue;
		BasicBlock* bb = static_cast<BasicBlock*>(cur);
		vector<Instruction*>& stmts = bb->get_statements();
		if(stmts.size() == 0) continue;
		Instruction* last = *stmts.rbegin();
		if(last->get_instruction_id() == LOOP_INVOCATION)
			blocks.insert(cur);
	}
}


SuperBlock::~SuperBlock() {

}

bool SuperBlock::is_superblock()
{
	return true;
}

string SuperBlock::to_string() const
{
	return id.to_string();
}


/*
 * Summary-unit related functions.
 */
bool SuperBlock::is_function()
{
	return false;
}

Identifier SuperBlock::get_identifier()
{
	return this->id;
}




Instruction* SuperBlock::get_first_instruction()
{
	assert(entry_block->get_statements().size() > 0);
	return entry_block->get_statements()[0];
}
Instruction* SuperBlock::get_last_instruction()
{
	Instruction* last = NULL;
	set<Block*>::iterator it = body.begin();
	for(; it!= body.end(); it++) {
		Block* b = *it;
		if(b->is_superblock()) continue;
		BasicBlock* bb = (BasicBlock*)b;
		vector<Instruction*>& stmts = bb->get_statements();
		if(stmts.size() ==0) continue;
		Instruction* cur_last = NULL;
		for(unsigned int i=stmts.size()-1; i>0; i--) {
			Instruction* cur = stmts[i];
			if(cur->line != -1) {
				cur_last = cur;
				break;
			}
		}
		if(cur_last == NULL) continue;

		if(last == NULL || cur_last->line > last->line) {
			last = cur_last;
		}
	}
	//cout << "*******LAST INSTRUCTION: " << last->to_string() << endl;
	return last;
}

}
