/*
 * LivenessInfer.cpp
 *
 *  Created on: Mar 15, 2013
 *      Author: boyang
 */

#include "LivenessInfer.h"
#include "Variable.h"
#include "Cfg.h"
#include "util.h"
#include "Instruction.h"
#include <map>
#include <set>
#include <assert.h>
#include <iostream>
#include "TypeInference.h"


#include "sail.h"
#include "il.h"

using namespace std;
using namespace sail;
using namespace il;


#define DEBUG false

#define OPTIMIZE_ELIMINATE_REACHING true

#define LIVENESS_TEST false

namespace sail {

LivenessInfer::LivenessInfer()
{

}


LivenessInfer::~LivenessInfer()
{

}



LivenessInfer::LivenessInfer(BasicBlock* bb_entry, BasicBlock* bb_exit,
		BasicBlock* exception_b,
		Function* f){
	this->entry_block = bb_entry;
	this->exit_block = bb_exit;
	this->exception_block = exception_b;
	this->f = f;





	set<Block*, sail::CompareBlock> worklist;
	if(DEBUG)
	//if (1)
	{
		cout << "liveness infer  start!!! " << endl;
	}

	//find blocks with out following block first
	//added by haiyan
	set<Block*> to_unify;

	worklist.insert(entry_block);

	//added by haiyan
	to_unify.insert(entry_block);

	while(worklist.size() > 0)
	{
		Block* b_cur = *(worklist.begin());
		worklist.erase(b_cur);
		set<CfgEdge*>& set_Edges = b_cur->get_successors();
		if(set_Edges.size() == 0)
		{
			set_ends.insert(b_cur);
		}
		set<CfgEdge*>::iterator iter_Edges = set_Edges.begin();
		for(; iter_Edges != set_Edges.end(); iter_Edges++)
		{
			//haiyan added to unify;
			if(to_unify.count((*iter_Edges)->get_target()))
				continue;

			worklist.insert((*iter_Edges)->get_target());
		}
	}
	to_unify.clear();
	//initialize the map


	set<Block*> to_unify_set;
	set<Block*>::iterator biter = set_ends.begin();
	for(;biter != set_ends.end(); biter++)
	{
		worklist.insert(*biter);
		//added by haiyan
		to_unify_set.insert(*biter);
	}



	worklist.insert(exit_block);
	worklist.insert(exception_block);

	//added by haiyan
	to_unify_set.insert(exit_block);
	to_unify_set.insert(exception_block);

	//cout << "just before collecting insts! " << endl;
	while(worklist.size() > 0)
	{
		Block* b_cur = *(worklist.begin());
		//cout << "Checking block is " << b_cur->get_block_id() << endl;
		worklist.erase(b_cur);
		if(b_cur->is_basicblock())
		{

			BasicBlock* bb_cur = static_cast<BasicBlock*>(b_cur);
			vector<Instruction*> vector_inst =  bb_cur->get_statements();
			vector<Instruction*>::reverse_iterator riter = vector_inst.rbegin();
			for (; riter != vector_inst.rend(); ++riter) {
				// ================ debug printout ==============
				if(DEBUG)
				{
					cout << "\ninstruction  : " << (*riter)->to_string();
				}
				if(DEBUG)
				{
					vector<Symbol*> vect_syms;
					get_symbols_inst((*riter), vect_syms);
					vector<Symbol*>::iterator iter = vect_syms.begin();
					for(;iter != vect_syms.end();iter++)
					{
						cout << (*iter)->get_type()->to_string() << " , " ;
					}
					cout <<"\t inst id : " <<
							(*riter)->get_instruction_id() << endl;
				}
				// ================ end  ==============

				set<string> newSet;
				//the default is empty set;
				map_liveness.insert(pair<Instruction*,
						set<string> >(*riter, newSet));

				if((*riter)->get_instruction_id() == LOOP_INVOCATION){

					LoopInvocationInstruction* li = (LoopInvocationInstruction*)(*riter);

					Block* target = li->get_header();

					backedge_source_target[bb_cur] = target;
				}

			}



		}
		set<CfgEdge*>& set_Edges = b_cur->get_predecessors();
		set<CfgEdge*>::iterator iter_Edges = set_Edges.begin();
		for(; iter_Edges != set_Edges.end(); iter_Edges++)
		{
			//cout << "      === > insert block is " << (*iter_Edges)->get_source()->get_block_id() << endl;
			//added by haiyan
			if(to_unify_set.count((*iter_Edges)->get_source()) > 0)
				continue;

			worklist.insert((*iter_Edges)->get_source());
			//added by haiyan
			to_unify_set.insert((*iter_Edges)->get_source());

		}
	}

	to_unify_set.clear();



	if(DEBUG)
	//if(1)
	{
		cout << "Liveness infer end!!! " << endl;
	}
}



void LivenessInfer::LivenessInferAnalysis() {
	set<Block*, sail::CompareBlock> worklist;
	bool has_changing = true;
	int round = 0;
	while(has_changing)
	{
		has_changing = false;
		set<Block*>::iterator biter = set_ends.begin();
		for(;biter != set_ends.end(); biter++)
		{
			worklist.insert(*biter);
		}
		worklist.insert(exit_block);
		worklist.insert(exception_block);

		if(LIVENESS_TEST)
			cout << "Round "  << round ++<< " processing worklist of liveness ! " << endl;
		while(worklist.size() > 0)
		{
			Block* b_cur = *(worklist.rbegin());
			worklist.erase(b_cur);

			if(b_cur->is_basicblock())
			{
				if(LIVENESS_TEST)
					cout << endl << "PROCESSING b_cur id : " << b_cur->get_block_id()
						<< "	--	"<< endl;

				BasicBlock* bb_cur = static_cast<BasicBlock*>(b_cur);
				set<string> block_in;
				// step 1. merge all the "liveness" set of successor blocks (could be multiple) to block_in
				compute_block_afteroutput(b_cur, block_in);
				// step 2. add current block "liveness" to block_in and finally update this->map_liveness.
				bool has_changing_cur = block_liveness_analysis(bb_cur, block_in);

				if(LIVENESS_TEST)
					cout << endl << endl;

				if(has_changing == false && has_changing_cur == true)
				{
					has_changing = true;
				}
			}else{
				assert(false);
			}
			//====  for the case that exit block == entry block

			//=== end ===

			set<CfgEdge*>& set_Edges = b_cur->get_predecessors();
			set<CfgEdge*>::iterator iter_Edges = set_Edges.begin();
			for(; iter_Edges != set_Edges.end(); iter_Edges++)
			{
				worklist.insert((*iter_Edges)->get_source());
			}
		}
	}
}

bool LivenessInfer:: block_liveness_analysis(BasicBlock* bb_cur, set<string> set_in)
{
	bool is_first_inst = true;
	bool has_changing = false;

	vector<Instruction*> vector_inst =  bb_cur->get_statements();


	//-------------
	set<string> set_out;
	vector<Instruction*>::reverse_iterator iter_inst = vector_inst.rbegin();
	int i = 0;
	for (; iter_inst != vector_inst.rend(); ++iter_inst) {

		Instruction* inst = *iter_inst;
		if(LIVENESS_TEST)
			cout << "	processing liveness of inst "
				<< inst->to_string() << endl;

		set_out.clear();
		//cout << "\ninstruction : " << (*iter_inst)->to_string() << endl;
		if(is_first_inst == false){
			set_in.clear();

			getLivenessByInst(*(iter_inst-1), set_in);
			get_instOUT(*iter_inst, set_in, set_out);
		}
		else{
			is_first_inst = false;

			get_instOUT(*iter_inst, set_in, set_out);
		}

		if(update_mapping(*iter_inst, set_out))
		{
			//cout << "has change. Inst : " << (*iter_inst)->to_string() << endl;
			has_changing = true;
		}
	}
	return has_changing;
}


/***********************************************************************
 * Input: a instruction pointer, a set of variables that is "live" of this inst.
 * Output: update this->map_liveness and return true (when it is changed) or false (otherwise)
 * *********************************************************************/

bool LivenessInfer::update_mapping(Instruction* inst, const set<string>& new_stat){

	map<Instruction*,set<string> >::iterator  map_iter = this->map_liveness.find(inst);
	//If the instruction is found.
	bool is_changed = false;
	if(map_iter != this->map_liveness.end())
	{
		if(map_iter->second == new_stat)
		{
			is_changed = false;
		}else
		{
			map_iter->second = new_stat;
			is_changed = true;
		}
	}else //Otherwise, error. Should find an instruction in the map
		assert(false);

	return is_changed;
}



/****
 * fill the content of afterInstOut
 */
void LivenessInfer::compute_block_afteroutput(Block* b_cur,
		set<string>& afterInstsOut){

//	cout << " == = = Processing liveness for "
//			 << 	b_cur->get_block_id() << endl;

	set <set<string> > set_afterInstsOut;
	/*
	 * if b_cur is not entry_block, we collect all following blocks
	 */
	if(b_cur != this->exception_block && b_cur != this->exit_block)
	{
		if(CONSIDER_LOOP){
			if(this->backedge_source_target.count(b_cur) > 0){

				Block* tar = this->backedge_source_target[b_cur];

				//cout << b_cur->get_block_id() << "	 is an exit of super block!  " << endl;
				assert(tar->is_basicblock());
				if(tar->is_basicblock()){
					BasicBlock* bb_after = static_cast<BasicBlock*>(tar);
					vector<Instruction*>& statements =
							bb_after->get_statements();

					assert(bb_after != exception_block);
					assert(bb_after != exit_block);

					if(statements.size() == 0){
						set<string> cur_after_setLive;
						compute_block_afteroutput(bb_after, cur_after_setLive);
						set_afterInstsOut.insert(cur_after_setLive);

					}else{

						Instruction* first_inst = statements.at(0);
						if(LIVENESS_TEST)
							cout << "loop entry first instruction " << first_inst->to_string() << endl;
						set<string> cur_after_setLive;
						getLivenessByInst(first_inst, cur_after_setLive);

						if (LIVENESS_TEST) {
							for (auto it = cur_after_setLive.begin();
									it != cur_after_setLive.end(); it++) {
								cout << "	var  " << *it << "	is alive " << endl;
							}
						}
						//insert to the set_afterInstOut;
						set_afterInstsOut.insert(cur_after_setLive);
					}
				}
			}
		}



		set <CfgEdge*> set_Edges = b_cur->get_successors();
		set <CfgEdge*>::iterator iter_Edges = set_Edges.begin();
		for (; iter_Edges!=set_Edges.end(); iter_Edges++)
		{
			Symbol * s = (*iter_Edges)->get_cond();
			string str_con = ""; //consider the variable that used by edge(should alive)
			if(s != NULL){
				//cout << "s is " << s->to_string() << endl;
				if (s->is_variable()) {
					Variable* v = static_cast<Variable *> (s);
					str_con = v->get_var_name();
				}
			}

			Block * b_after = (*iter_Edges)->get_target();
			//cout << "		== successor is " << b_after->get_block_id() << endl;
			if(b_after->is_basicblock())
			{
				BasicBlock* bb_after = static_cast<BasicBlock*>(b_after);
				vector<Instruction*> statements = bb_after->get_statements();

				if(statements.size() == 0
						&& bb_after != exception_block && bb_after != exit_block){
					set<string> cur_after_setLive;
					compute_block_afteroutput(bb_after, cur_after_setLive);
					if(str_con != "")
						cur_after_setLive.insert(str_con);
					set_afterInstsOut.insert(cur_after_setLive);
					continue;
				}

				// bb_after_statements.size() == 0
				if(bb_after == exception_block)
				{
					set<string> cur_after_setLive;
					if(str_con != "")
						cur_after_setLive.insert(str_con);
					set_afterInstsOut.insert(cur_after_setLive);
					continue;
				}


				if(bb_after == exit_block)
				{
					//if bb_after != entry block then it should be empty block;
					assert(bb_after->get_statements().size() == 0);
					set<string> cur_after_setLive;
					if(str_con != "")
						cur_after_setLive.insert(str_con);
					set_afterInstsOut.insert(cur_after_setLive);
					continue;
				}

				vector<Instruction*>::iterator statement_iter;
				statement_iter = statements.begin();
				set<string> cur_after_setLive;
				getLivenessByInst((*statement_iter), cur_after_setLive);
				if(str_con != "")
					cur_after_setLive.insert(str_con);
				set_afterInstsOut.insert(cur_after_setLive);
			}
		}
	}

	// merge set_afterInstsOut all together
	set <set<string> >::iterator iter = set_afterInstsOut.begin();
	for(;iter != set_afterInstsOut.end(); iter++)
	{
		afterInstsOut.insert((*iter).begin(), (*iter).end());
	}

	if (LIVENESS_TEST) {
		cout << "	Result in bottom facts is ==============" << endl;
		for (auto it = afterInstsOut.begin();
				it != afterInstsOut.end(); it++)
		{
			cout << "		^^ == " << *it << endl;
		}
		cout << "	end of bottom facts ================================ "
				<< endl<<endl;
	}
}


void LivenessInfer::getLivenessByInst(Instruction* inst,
		set<string>& set_liveness)
{
	if(LIVENESS_TEST)
		cout << "Instruction is " << inst->to_string() << endl;
	//cout << "map_liveness size " << this->map_liveness.size() << endl;
	if (this->map_liveness.count(inst) == 0){
		cerr << "Can not find instruction: " << inst->to_string() << " in map_liveness" << endl;
		assert(false);
	}

	set_liveness = map_liveness[inst];

	if (LIVENESS_TEST) {
		cout << "		GGGGGGGet liveness of instruction "
				<< inst->to_string() << endl;
		for (auto it = set_liveness.begin(); it != set_liveness.end(); it++) {
			cout << " 		==> get living variable is " << *it << endl;
		}

		if(set_liveness.size() == 0)
			cout << "		==> IS EMPTY !" << endl;
		cout << endl;
	}
}




void LivenessInfer::get_instOUT(Instruction* inst, set<string>& set_in,
		set<string>& set_out){

	if(DEBUG)
	{
		cout << "get_instOUT inst : " << inst->to_string() << endl;
	}
	set_out = set_in;

	if(DEBUG)
	{
		set<string>::iterator it = set_in.begin();
		for(;it != set_in.end(); it++)
		{
			cout << "set_in " <<  (*it) << endl;
		}
	}

	// get_lhs();
	Variable* defined_var = sail::get_defined_variable(inst);

	bool find_defined_var = false;
	if(defined_var != NULL)
	{
		find_defined_var = true;
	}


	//kill the defined variable
	if(find_defined_var)
	{
		//cout << "var name : " << defined_var->get_var_name() << endl;
		set<string>::iterator iter = set_out.find(defined_var->get_var_name());
		if(iter != set_out.end())
		{
			set_out.erase(iter);
		}
	}
	vector<Symbol*> vect_syms;
	sail::get_symbols_inst(inst, vect_syms);
	int counter = 0;
	while(counter < (int)vect_syms.size())
	{
		// if the instruction is a definition. Skip the defined variable (first one)
		if(find_defined_var && counter==0)
		{
			counter++;
			continue;
		}

		if(vect_syms.at(counter)->is_variable())
		{
			Variable* var = static_cast<Variable*>(vect_syms.at(counter));
			set_out.insert(var->get_var_name());
		}
		counter++;
	}

	if (LIVENESS_TEST) {
		for (auto it = set_out.begin(); it != set_out.end(); it++) {
			cout << "		liveness of current instruction is  " << *it << endl;
		}
		cout << "			~~~ ---------------" << endl << endl;
	}

}

/*******
 * in terms of liveness information;
 */
void LivenessInfer::eliminate_reaching(string v_name,
		set<Instruction*>& set_inst) {
	set<Instruction*>::iterator it = set_inst.begin();
//	cout << "Eliminate_reaching " << "  Size of set_inst " << set_inst.size()
//			<< endl;
	//int i = 0;
	if (!OPTIMIZE_ELIMINATE_REACHING) {
		//cout << "using boyang's version! " << endl;
		for (; it != set_inst.end();) {
			//cout << "i is " << i++ << endl;/*
			set<string> set_liveness;
			getLivenessByInst(*it, set_liveness);

			set<string>::iterator it2 = set_liveness.find(v_name);
			if (it2 == set_liveness.end()) {
				// if not find (not alive), then delete the instruction in the set.
				it = set_inst.erase(it);
			} else {
				it++;
			}
		}
	} else {
		//cout << "using haiyan' version !" << endl;
		for (; it != set_inst.end();) {

			assert(this->map_liveness.count(*it) > 0);
			set<string>* set_liveness = &(this->map_liveness[*it]);
			if (set_liveness->count(v_name) == 0) { // not find
				set_inst.erase(it++);
			} else {
				++it;
			}
		}
	}
}




void LivenessInfer::print_out_mapping(){
	cout << "=======  LivenessInfer::print_out_mapping  start ========" << endl;
	vector<Instruction*>* body = this->f->get_body();

	for(int i = 0; i < (int)body->size(); i ++){
		Instruction* cur_inst = body->at(i);
		cout << "instruction is " << cur_inst->to_string() << endl;

		if(map_liveness.count(cur_inst) > 0){

			set<string> liveness = map_liveness[cur_inst];
			cout << "	== Instruction point is " << cur_inst->to_string()
					<< "{ " << cur_inst  <<" }"<< endl;
			for(auto it = liveness.begin(); it != liveness.end(); it++){
				cout << "		>> alive variable string is " << *it << endl;
			}
			cout << endl;
		}

	}

	cout << "=======  LivenessInfer::print_out_mapping  end ========" << endl;
}


}
