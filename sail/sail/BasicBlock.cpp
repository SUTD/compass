/*
 * BasicBlock.cpp
 *
 *  Created on: Jul 6, 2008
 *      Author: isil
 */

#include "BasicBlock.h"
#include "Instruction.h"
#include "CfgEdge.h"
#include "Symbol.h"
#include "util.h"
#include "stdlib.h"
#include "SuperBlock.h"
#include "node.h"
using namespace std;

#define SET_REACHABLE_FIELD true

namespace sail {

BasicBlock::BasicBlock()
{
	this->block_id = INVALID_BLOCK_ID;
	this->dominators.insert(this);
	this->post_dominators.insert(this);

}

BasicBlock::BasicBlock(const BasicBlock& other)
{
	this->block_id = other.block_id;
	this->statements = other.statements;
	this->dominators.insert(this);
	this->post_dominators.insert(this);
}


void BasicBlock::add_statement(Instruction* inst)
{
	statements.push_back(inst);
	if(SET_INS_TO_BLOCK)
		inst->set_inside_basic_block(this);

}


void BasicBlock::add_statement_before(Instruction* inst, Instruction* inst_pos)
{
	vector<Instruction*>::iterator it = statements.begin();
	for (; it < statements.end(); it++){
		if((*it) == inst_pos)
			break;
	}
	it = statements.insert(it , inst);
}


void BasicBlock::add_statements(vector<Instruction*>& stmts)
{
	for(int i=0; i<(int)stmts.size(); i++)
	{
		statements.push_back(stmts[i]);
		//added by haiyan 4.12.2013
		if(SET_INS_TO_BLOCK)
			stmts[i]->set_inside_basic_block(this);
		//ended by haiyan 4.12.2013
	}
}

void BasicBlock::set_post_dominators(set<BasicBlock*>& pdoms)
{
	this->post_dominators.clear();
	set<BasicBlock*>::iterator it = pdoms.begin();
	for(; it != pdoms.end(); it++){
		this->post_dominators.insert(*it);
	}

}


void BasicBlock::set_dominators(set<BasicBlock*>& doms)
{
	this->dominators.clear();
	set<BasicBlock*>::iterator dom_it = doms.begin();
	for(; dom_it != doms.end(); dom_it++){
		this->dominators.insert(*dom_it);
	}
	//this->dominators.insert(this);

	/*this->dominators.insert(doms.begin(), doms.end());*/
	//this->dominators = doms;
}

set<BasicBlock*>& BasicBlock::get_dominators()
{
	return this->dominators;
}

set<BasicBlock*>& BasicBlock::get_post_dominators()
{
	return this->post_dominators;
}


bool BasicBlock::is_empty_block()
{
	return (statements.size()==0);
}

vector<Instruction*>& BasicBlock::get_statements()
{
	return statements;
}


string BasicBlock::to_string() const
{
	string res = "Block " + int_to_string(block_id) +": \n";

	for(int i=0; i<(int)statements.size(); i++)
	{
		//cout << "ADDR :: " << statements[i] << endl;
		res += "\t" + statements[i]->to_string() + "\n";
	}

	res += "Successors: ";
	set<CfgEdge*>::iterator it = this->successors.begin();
	for(;it!=this->successors.end(); it++)
	{
		res += int_to_string((*it)->get_target()->get_block_id()) + " ";
	}
	res += "\n";
	res += "Predecessors: ";
	it = this->predecessors.begin();
	for(;it!=this->predecessors.end(); it++)
	{
		res += int_to_string((*it)->get_source()->get_block_id()) + " ";
	}
	res += "\n";
	return res;
}

string BasicBlock::to_string(bool pretty_print, bool print_block_id) const
{
	string res;
	if(print_block_id)
		res += "Block " + int_to_string(block_id) +": \n";

	for(int i=0; i<(int)statements.size(); i++)
	{
		int line = statements[i]->line;
		if(pretty_print && statements[i]->is_removable()) continue;
		if(pretty_print && statements[i]->get_original_node() != NULL)
			res +=   statements[i]->get_original_node()->to_string() + "\n";
		else res +=   statements[i]->to_string(pretty_print) +
				" (" + int_to_string(line) + ")\n";
	}
	/*res += "Successors: ";
		set<CfgEdge*>::iterator it = this->successors.begin();
		for(;it!=this->successors.end(); it++)
		{
			res += int_to_string((*it)->get_target()->get_block_id()) + " ";
		}
		res += "\n";
		res += "Predecessors: ";
		it = this->predecessors.begin();
		for(;it!=this->predecessors.end(); it++)
		{
			res += int_to_string((*it)->get_target()->get_block_id()) + " ";
		}
		res += "\n";
		return res;*/
	return res;
}

bool BasicBlock::is_basicblock()
{
	return true;
}

string BasicBlock::to_dotty(string prelude, bool pretty_print)
{
	BasicBlock* b = this;
	long block_id = b->get_block_id();
	if(b->get_block_id() == -1){
		block_id =  (long)b;
		int a = block_id;
		block_id = a<0? -a:a;

	}

	string block_string = prelude;

	block_string += escape_string(line_wrap(b->to_string(pretty_print), 50));


	set<CfgEdge*>& succs = b->get_successors();
	string res = "node" + int_to_string(block_id) + " [shape = box][label= \"" +
			block_string + "\"] \n";

	set<CfgEdge*>::iterator succs_it = succs.begin();

	for(; succs_it!=succs.end(); succs_it++){
		CfgEdge* succ_edge = *succs_it;
		string edge_style = succ_edge->is_backedge() ? "[style=dotted]" : "";
		long succ_id;
		if(succ_edge->get_target()->is_basicblock())
			succ_id = succ_edge->get_target()->get_block_id();
		else
		{
			SuperBlock* sb =(SuperBlock*) succ_edge->get_target();
			succ_id = sb->get_entry_block()->get_block_id();
		}

		if(succ_id == -1) succ_id = (long) succ_edge->get_target();
		res+= "node" + int_to_string(block_id) + " -> " +
				"node" + int_to_string(succ_id) + " [label = \"" +
				(succ_edge->get_cond() ?
						escape_string(succ_edge->get_cond()->to_string(pretty_print)) : "true") +
						"\"]" + edge_style + "\n";
	}

	return res;
}

void BasicBlock::set_statements_unreachable(){

	for(int i = 0; i < (int)statements.size(); i ++){
		Instruction* inst = statements.at(i);
//		cout << " set inst " << inst->to_string() <<
//				"  as unreachable ! " <<endl;

		inst->set_as_unreachable();
	}
}


BasicBlock::~BasicBlock() {

}

}
