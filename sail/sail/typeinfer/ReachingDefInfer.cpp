/*
 * ReachingDefInfer.cpp
 *
 *  Created on: Sep 20, 2012
 *      Author: boyang
 */

#include "ReachingDefInfer.h"
#include "ReachingDefVarStat.h"
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
#define OPTIMIZE_CODE true
#define OPTIMIZE_MINI_REACHING false

#define RENAMING_PHASE_TESTING false


namespace sail {



ReachingDefInfer::ReachingDefInfer(BasicBlock* bb_entry, BasicBlock* bb_exit,
		BasicBlock* exception_b, Function* f,
		LivenessInfer* lni){

	//haiyan added the order_insts vector to
	//control the order to print the instruction;
	//cout << "&&&&&&&&&&&&& ReachingDefInfer start " << endl;
	ordered_insts = f->get_body();
	this->lni = lni;
	this->f = f;
	//haiyan ended;


	this->entry_block = bb_entry;
	this->exit_block = bb_exit;
	this->exception_block = exception_b;


	set<Block*, sail::CompareBlock> worklist;
	if(DEBUG)
	{
		cout << "ReachingDefInfer start!!! " << endl;
		cout << "entry_block  id : " << this->entry_block->get_block_id() << endl;
		cout << "exit_block  id : " << this->exit_block->get_block_id() << endl;
	}
	//initialize the map

	//added by haiyan
	set<Block*> to_unify;
	//ended by haiyan

	worklist.insert(bb_entry);

	//added by haiyan
	to_unify.insert(bb_entry);
	//ended by haiyan;
	//cout << "right before loop 2nd  phase! " << endl;
	while(worklist.size() > 0)
	{
		Block* b_cur = *(worklist.begin());
		worklist.erase(b_cur);
		if(DEBUG){
		//if(1){
			cout << "\n\ncur block id : " << b_cur->get_block_id() << endl;
		}
		if(b_cur->is_basicblock())
		{
			BasicBlock* bb_cur = static_cast<BasicBlock*>(b_cur);
			//possible is entry and exit block;
			if (0) {
				cout << "bb_cur > " << bb_cur->get_block_id() << endl;
				cout << " is entry ? " << (bb_cur == this->entry_block) << endl;
				cout << " is exit ? " << (bb_cur == this->exit_block) << endl;
				cout << " is exception ? " << (bb_cur == this->exception_block)
						<< endl;
			}



			vector<Instruction*> vector_inst =  bb_cur->get_statements();
			vector<Instruction*>::iterator it = vector_inst.begin();
			//cout << "SSSSSSSize is " << vector_inst.size() << endl;
			for (; it != vector_inst.end(); ++it) {
				// ================ debug printout ==============
				if(DEBUG)
				{
					cout << "\ninstruction  : " << (*it)->to_string();
				}
				if(DEBUG)
				{
					vector<Symbol*> vect_syms;
					get_symbols_inst((*it), vect_syms);
					vector<Symbol*>::iterator iter = vect_syms.begin();
					for(;iter != vect_syms.end();iter++)
					{
						cout << (*iter)->to_string()<<":" <<
								(*iter)->get_type()->to_string() << " , " ;
					}
					cout <<"\t inst id : " <<
							(*it)->get_instruction_id() << endl;
				}
				// ================ end  ==============
				Instruction* inst = *it;
				//cout << "	occupy for " << inst->to_string() << endl;
				map <string, ReachingDefVarStat*> newMap;
				//the default is empty set;
				map_reaching.insert(pair<Instruction*,
						map <string, ReachingDefVarStat*> >(*it, newMap));
				set<Instruction*> set_insts;
				map_mini_reaching.insert(make_pair(*it, set_insts));
				set<Instruction*> set_insts2;
				map_rev_reaching.insert(make_pair(*it, set_insts2));

				if(inst->get_instruction_id() == LOOP_INVOCATION){


					LoopInvocationInstruction* li = (LoopInvocationInstruction*)inst;

					Block* target = li->get_header();

					this->backedge_target_source[target] = bb_cur;
				}
			}
		}
		set<CfgEdge*>& set_Edges = b_cur->get_successors();

		//cout << "  &&& size of set_edges is " << set_Edges.size() << endl;
		set<CfgEdge*>::iterator iter_Edges = set_Edges.begin();

		int i = 0;
		for(; iter_Edges != set_Edges.end(); iter_Edges++)
		{
			//haiyan added
			if(to_unify.count((*iter_Edges)->get_target()) > 0){
				continue;
			}
			//ended by haiyan;



			worklist.insert((*iter_Edges)->get_target());

			//added by haiyan
			to_unify.insert((*iter_Edges)->get_target());
			//haiyan ended
			if(DEBUG){
			//if(1){
				cout << "     ==> inserting target block : " << i++ <<
						(*iter_Edges)->get_target()->get_block_id() << endl;
			}
		}
		if(DEBUG)
			cout << "        - - - --  -end of adding targets ! " << endl;
	}

//	//added by haiyan
//	to_unify.clear();
//	//haiyan ended

	if(DEBUG)
	{
		cout << "ReachingDefInfer end!!! " << endl;
	}
}

ReachingDefInfer::ReachingDefInfer() {

}



ReachingDefInfer::~ReachingDefInfer() {

}

const string ReachingDefInfer::rename_variable(
		Variable* orig_var) {

	string orig_name = orig_var->get_var_name();
	string new_name = orig_name;
	int index = 0;
	if (rename_most_recently_index.count(orig_name) == 0) { //first time;

		if (orig_var->is_argument()) { //should start from index 1;
			++ index;
			new_name = orig_name + "_" + int_to_string(index);
		}

		rename_most_recently_index[orig_name] = index;

		//cout << "1) new_name is " << new_name << endl;
		return new_name;
	}


	//otherwise found use, then generate a new one;
	index = rename_most_recently_index[orig_name];

	++index;

	new_name = orig_name + "_" + int_to_string(index);

	rename_most_recently_index[orig_name] = index;
	//cout << "2) new_name is " << new_name << endl;
	return new_name;

}





void ReachingDefInfer::reachingDefAnalysis() {
	set<Block*, sail::CompareBlock> worklist;
	bool has_changing = true;
	int i = 0;


	bool has_loop = false;
	if(CONSIDER_LOOP){
		if(this->backedge_target_source.size() != 0)
			has_loop = true;
	}

	while(has_changing)
	{
//		i++;
//		cout << "ReachingDefAnalysis Round " << i << endl;
		has_changing = false;

		worklist.insert(entry_block);
		while(worklist.size() > 0)
		{
			Block* b_cur = *(worklist.begin());
			//cout << "      HHHHH current block is " << b_cur->get_block_id() << endl;
			worklist.erase(b_cur);
			if(b_cur->is_basicblock())
			{
				map <string, ReachingDefVarStat*> preInstsOut;
				//step 1. compute all variables defined points (joint set) at the begin of each block
				compute_block_preoutput(b_cur, preInstsOut);



				//step 2. process each instruction for current block
				bool has_changing_cur = block_reachingDef_analysis(b_cur, preInstsOut);


				if(has_changing == false && has_changing_cur == true)
				{
					has_changing = true;
				}
			}
			else{
				assert(false);
			}




			set<CfgEdge*>& set_Edges = b_cur->get_successors();
			set<CfgEdge*>::iterator iter_Edges = set_Edges.begin();
			for(; iter_Edges != set_Edges.end(); iter_Edges++)
			{
				Block* succ = (*iter_Edges)->get_target();
				//cout << "	==> succ is " << succ->get_block_id() << endl;
				worklist.insert(succ);
			}
			//cout <<"-----------------------" << endl;

		}

		//if it is loop, need to do another around
		if(CONSIDER_LOOP){
			if((i == 1)&& has_loop){
				has_changing = true;
			}
		}
	}

	//cout << "EEEEEEnd of ReachingDefAnalysis !!! " << endl;
	//no long use it again;
	this->backedge_target_source.clear();

	compute_reverse_reaching();
	//cout << "EEEEEnd of compute_reverse_reaching !!!" << endl;

	//no long need reaching_mapping anymore;
	map_reaching.clear();
}

set<Reaching*>& ReachingDefInfer::get_reaching_points(Def* def){

	assert(def_use_chain.count(def) > 0);

	set<Reaching*>& reachings = def_use_chain[def];

	return reachings;
}



void ReachingDefInfer::renaming_phase(){

	//no matter about the order of blocks; just need to traverse each instruction;

	set<Block*> worklist;
	set<Block*> visited;
	worklist.insert(this->entry_block);

	while(!worklist.empty()){
		Block* cur_block = *(worklist.begin());
		worklist.erase(cur_block);
		visited.insert(cur_block);

		//certain block is both entry_block and exit_block;
		if(cur_block != this->entry_block){

			if(cur_block == this->exit_block)
				continue;

			if(cur_block == this->exception_block)
				continue;

		}

		if(cur_block->is_basicblock()){

			vector<Instruction*>& insts = ((BasicBlock*) cur_block)->get_statements();

			for(int i = 0; i < (int) insts.size(); i ++){

				Instruction* cur_inst = insts.at(i);

				if (cur_inst->is_save_instruction()) { //only save_instruction has definition;

					Variable* lhs = cur_inst->get_lhs();

					if (lhs == NULL)
						continue;

					//but sometimes argument register
					//will be used as local register, how to resolve that case ?
					//PAY ATTENTION;

					if(RENAMING_PHASE_TESTING){
						cout << "@@PProcessing instruction  "
								<< cur_inst->to_string() << endl;
						cout << "LHS is " << lhs->to_string() << endl;
					}

					string orig_name = lhs->get_var_name();
					Def* def = this->make_def(cur_inst);

					//must be unresolved at this point;
					assert(this->unresolved_def.count(def) > 0);

					set<Reaching*>& reachings = get_reaching_points(def);
					string resolved_name = "";

					for (auto it = reachings.begin(); it != reachings.end();
							it++) {

						if (resolved_reaching.count(*it) > 0) { //have already been resolved
							resolved_name = resolved_reaching[*it];
							break;
						}

					}

					//now need to get the name that current lhs use;
					string use_new_name;

					if (resolved_name != "") { //have found been resolve about the variable;

						if(RENAMING_PHASE_TESTING)
							cout << "	== >find its reaching has been resolved ! "
								<< resolved_name << endl;


						Variable* new_lhs = lhs->clone();

//						if (lhs->is_argument()) {
//							new_lhs->set_as_local();
//						}

						new_lhs->set_name(resolved_name);
						cur_inst->set_lhs(new_lhs);

						use_new_name = resolved_name;

					} else { //need to rename;

						if(RENAMING_PHASE_TESTING)
							cout << "lhs " << lhs->to_string()
								<< "needs to rename ! " << endl;
						string new_name;
						if (!lhs->is_argument()) {

							new_name = rename_variable(lhs);

							if (RENAMING_PHASE_TESTING)
								cout << "	== >generate new_name is " << new_name
										<< endl;

							if (orig_name != new_name) { //generate a new name;
								Variable* new_lhs = lhs->clone();
								//							if (lhs->is_argument()) {
								//								new_lhs->set_as_local();
								//							}

								new_lhs->set_name(new_name);
								cur_inst->set_lhs(new_lhs);
							}
						}else{ //if it is argument, no need to rename!
							new_name = lhs->get_var_name();
						}

						use_new_name = new_name;

					}

					//propagate it to all its reaching points
					//no matter it is a new name or not,propagate it
					//into its reachingpoints
					//meaning reaching points have been touched;

					for (auto it = reachings.begin(); it != reachings.end();
							it++) {

						//no long needs to be changed anymore!
						if (resolved_reaching.count(*it) > 0)
							continue;

						//must have been
						sail::Instruction* re_inst = (*it)->get_reaching_inst();

						if(RENAMING_PHASE_TESTING)
							cout << "		^^^^^^^ updating reaching points  "
								<< re_inst->to_string() << endl;

						//make sure that the reaching point ,
						//will reset the field of argument or not, but won't change the store address;
						if(RENAMING_PHASE_TESTING){
						//if(1){
							cout << "			|| origi_name is "
									<< orig_name << endl;

							cout << "			|| new name is "
									<< use_new_name << endl;
						}

						//based on the original name to replace it with use_new_name;
						if (orig_name != use_new_name) {
							bool successful =
									re_inst->replace_in_use_symbol_name(
											orig_name, use_new_name);
							assert(successful == true);
						}

						resolved_reaching[*it] = use_new_name;
						this->unresolved_reaching.erase(*it);

						if (RENAMING_PHASE_TESTING) {

							cout << "			LLet  " << (*it)->to_string()
									<< "	to be resolved ! " << endl
									<< "		!!!!!!!!!!! " << endl;
						}
						//rename its corresponding variable's name, and put it to resolved map;

					}

					//remove from unresolved_def;
					this->unresolved_def.erase(def);

					if(RENAMING_PHASE_TESTING)
						cout << endl << endl;

				}

			}

		}else{
			cout << "cur_block " << cur_block->to_string() << "	is not a basicblock! " << endl;
			assert(false);
		}


		set<CfgEdge*>& succs = cur_block->get_successors();

		for(auto iter = succs.begin(); iter != succs.end(); iter ++){

			Block* succ = (*iter)->get_target();

			if(visited.count(succ) > 0)
				continue;

			worklist.insert(succ);
		}
	}

	visited.clear();


	//no unresolved Defs and Reachings;
	if (this->unresolved_def.size() != 0) {

		for(auto it = this->unresolved_def.begin();
				it != this->unresolved_def.end(); it++){
			cout << "Unresolved def " << (*it)->to_string() << endl;
		}

		assert(false);
	}

	if (this->unresolved_reaching.size() != 0) {

		for(auto it = this->unresolved_reaching.begin();
				it != this->unresolved_reaching.end(); it++){
			cout << "Unresolved reaching! " << (*it)->to_string() << endl;

		}
		assert(false);
	}

	//we need to delete all Defs and Reachings (unwine memory)
	//then we clear auxiliary containers;
	this->resolved_reaching.clear();
	this->def_use_chain.clear();
	this->use_def_chain.clear();
	this->delete_defs();
	this->delete_reachings();
}






void ReachingDefInfer::find_succesor_first_inst(Block* block, set<Instruction*>& insts){
	cout << "current block is " << block->get_block_id() << endl;

	if((block != this->exit_block)
			&&(block != this->exception_block)){

		set<CfgEdge*>& succes = block->get_successors();

		for(auto it = succes.begin(); it != succes.end(); it++){
			Block* succ = (*it)->get_target();
			cout << "succ is " << succ->get_block_id() << endl;
			if((block == this->exit_block) ||(block == this->exception_block))
				continue;

			if(succ->is_basicblock()){
				vector<Instruction*> inst = ((BasicBlock*)succ)->get_statements();
				if(inst.size() != 0){
					insts.insert(inst.at(0));
					continue;
				}else{
					find_succesor_first_inst(succ, insts);
				}
			}
		}
	}

}







void ReachingDefInfer::compute_reverse_reaching(){
	map<Instruction*, set<Instruction*> >::iterator itrev;
	map<Instruction*,map <string , ReachingDefVarStat*>  >::iterator  map_iter;
	map_iter = map_reaching.begin();
	for(; map_iter != map_reaching.end(); map_iter++)
	{
		map <string , ReachingDefVarStat*>::iterator sec_iter = map_iter->second.begin();
		for(; sec_iter != map_iter->second.end(); sec_iter++)
		{
			set<Instruction*> set1;
			sec_iter->second->get_set_reaching(set1);
			set<Instruction*>::iterator it_set1 = set1.begin();
			for(;it_set1 != set1.end(); it_set1++)
			{
				itrev = map_rev_reaching.find(*it_set1);
				itrev->second.insert(map_iter->first);
			}
		}
	}
}





void ReachingDefInfer::compute_block_preoutput(Block* b_cur,
		map <string, ReachingDefVarStat*>& preInstsOut)
{


	set <map <string, ReachingDefVarStat*> > set_preInstsOut;
	/*
	 * if b_cur is not entry_block, we collect all pre blocks
	 */
	if(b_cur != entry_block)
	{
		//get liveness of its first instruction, used to filter renaming phase;


		//set <CfgEdge*> set_Edges = b_cur->get_predecessors();
		//hzhu: avoid set copying
		const set<CfgEdge*>& set_Edges = b_cur->get_predecessors();


		set<CfgEdge*>::iterator iter_Edges = set_Edges.begin();

		set<Block*> preds;

		for (; iter_Edges != set_Edges.end(); iter_Edges++){
			Block * b_pre = (*iter_Edges)->get_source();
			preds.insert(b_pre);
		}

		if (CONSIDER_LOOP) {
			if (this->backedge_target_source.count(b_cur)) {
				Block* source = this->backedge_target_source[b_cur];
				assert(source->is_basicblock());
				preds.insert(source);
				cout << "find source is " << source->to_string() << endl;
			}
		}


		//for (; iter_Edges != set_Edges.end(); iter_Edges++) {
		for(auto it = preds.begin(); it != preds.end(); it ++){
			//Block * b_pre = (*iter_Edges)->get_source();
			Block* b_pre = *it;
			if (b_pre->is_basicblock()) {
				BasicBlock* bb_pre = static_cast<BasicBlock*>(b_pre);
				vector<Instruction*> pre_statements = bb_pre->get_statements();
				if (pre_statements.size() == 0) {
					map<string, ReachingDefVarStat*> cur_pre_VarR;
					compute_block_preoutput(bb_pre, cur_pre_VarR);
					set_preInstsOut.insert(cur_pre_VarR);
					continue;
				}

				// get the last instruction of one predecessor, and process it
				vector<Instruction*>::reverse_iterator riter;
				riter = pre_statements.rbegin();
				map<string, ReachingDefVarStat*> cur_pre_VarR;
				get_instOUT((*riter), cur_pre_VarR);
				set_preInstsOut.insert(cur_pre_VarR);
			}
		//}
		}


	}
	// merge set_preInstsOut all together and get preInstsOut for the current block
	//fed the second parameter;
	merge_reaching_info(set_preInstsOut, preInstsOut);
}



bool ReachingDefInfer::block_reachingDef_analysis(Block* b_cur,
		map <string, ReachingDefVarStat*>& preInstsOut){
	bool is_first_inst = true;
	bool has_changing = false;

	BasicBlock* bb_cur = static_cast<BasicBlock*>(b_cur);
	vector<Instruction*> vector_inst =  bb_cur->get_statements();
	vector<Instruction*>::iterator iter_vector_inst;


	iter_vector_inst = vector_inst.begin();
	for (; iter_vector_inst != vector_inst.end(); ++iter_vector_inst) {
		//not the fist instruction inside the block;
		if(is_first_inst == false){
			preInstsOut.clear();
			get_instOUT((*(iter_vector_inst-1)), preInstsOut);
		}

		//first time, comes here
		is_first_inst = false;

		//for test purpose;
		if (DEBUG) {
			cout << endl << endl << "################################# "
					<< endl;
			cout << "Processing instruction "
					<< (*iter_vector_inst)->to_string() << endl;
			cout << endl << "Before updating  ================ " << endl;
			print_current_map_reaching(*iter_vector_inst);
		}
		if(update_mapping(*iter_vector_inst, preInstsOut, false))
		{
			//for test purpose
			if(DEBUG)
				cout << endl << "After updating ==================== " << endl;

			//print_current_map_reaching(*iter_vector_inst);



			has_changing = true;
		}
	}
	return has_changing;
}




bool ReachingDefInfer::update_mapping(Instruction* inst,
		map <string, ReachingDefVarStat*>& new_stat, bool isException){
	map<Instruction*,map <string, ReachingDefVarStat*> >::iterator  map_iter;
	map_iter = map_reaching.find(inst);
	//should find an instruction in the map, because in the initialization function
	// all map_reaching elements are initialized.
	assert(map_iter != map_reaching.end());

	map <string, ReachingDefVarStat*> old_stat =  map_iter->second;
	//update
	map_iter->second = new_stat;


	map <string, ReachingDefVarStat*>::const_iterator itold;
	map <string, ReachingDefVarStat*>::const_iterator itnew;

	/*
	 * check the map has been update or not.
	 */
	for(itold = old_stat.begin(); itold != old_stat.end();itold++)
	{
		itnew = new_stat.find(itold->first);
		if(itnew != new_stat.end() && !(*(itnew->second) == *(itold->second)))
		{
			//cout << "has change !! ReachingDefInfer" << endl;
			return true;
		}
	}

	for(itnew = new_stat.begin(); itnew != new_stat.end(); itnew++)
	{
		itold = old_stat.find(itnew->first);
		if(itold != old_stat.end() && !(*(itnew->second) == *(itold->second)))
		{
			return true;
		}
	}

	return false;
}



void ReachingDefInfer::merge_reaching_info(
		set <map <string, ReachingDefVarStat*> >& input_set,
		map <string, ReachingDefVarStat*>& output_set){
	set <map <string, ReachingDefVarStat*> >::iterator it = input_set.begin();
	for(; it != input_set.end(); it++)
	{
		map <string, ReachingDefVarStat*> cur_set = (*it);
		merge_set_VarReachingStat(cur_set, output_set);
	}
}




void ReachingDefInfer::merge_set_VarReachingStat(
		map <string, ReachingDefVarStat*> & c_map,
		map <string, ReachingDefVarStat*> & o_map){
	map <string, ReachingDefVarStat*>::iterator c_it = c_map.begin();
	for(; c_it != c_map.end(); c_it++)
	{
		//Variable* cur_set_iter_var = (*it)->get_var();
		string name = c_it->first;
		map <string, ReachingDefVarStat*>::iterator o_it = o_map.find(name);
		if(o_it != o_map.end())
		{
			o_it->second->merge(c_it->second);
		}else
		{
			ReachingDefVarStat* newst =  new ReachingDefVarStat(*(c_it->second));
			o_map.insert(pair<string, ReachingDefVarStat* >(
					name, newst));
		}
	}
}
}




void ReachingDefInfer::get_instOUT(Instruction* inst,
		map<string, ReachingDefVarStat*>& set_inst_out)
{

	map<Instruction*, map<string, ReachingDefVarStat*> >::iterator map_iter;
	map_iter = map_reaching.find(inst);
	//should find an instruction in the map
	assert(map_iter != map_reaching.end());

	Variable* defined_var = sail::get_defined_variable(inst);
//	Variable* defined_var = inst->get_lhs();
	string define_name = "";
	string alias_name;
	bool find_defined_var = false;
	if (defined_var != NULL) {
		find_defined_var = true;
		define_name = defined_var->get_var_name();
		alias_name = defined_var->to_string(true);
	}

	if (!OPTIMIZE_CODE) { //this branch is not executed;
		map<string, ReachingDefVarStat*>::iterator it =
				map_iter->second.begin();
		for (; it != map_iter->second.end(); it++) {
			string cur_name = it->first;
			if (defined_var != NULL && cur_name == define_name) {
				ReachingDefVarStat* newstat = new ReachingDefVarStat(
						defined_var, inst);
				find_defined_var = false;
				// hzhu:
				// the following statement is a very questionable statement.
				// "Because element keys in a map are unique, the insertion
				// operation checks whether each inserted element has a key equivalent
				// to the one of an element already in the container, and if so,
				// the element is not inserted, returning an iterator to this existing
				// element (if the function returns a value)."

				set_inst_out.insert(
						pair<string, ReachingDefVarStat*>(cur_name, newstat));

//			set_inst_out.erase(cur_name);
//			set_inst_out.insert(pair<string, ReachingDefVarStat* >(
//					cur_name, newstat));

//			set_inst_out[cur_name] = newstat;
			} else {
				set_inst_out.insert(
						pair<string, ReachingDefVarStat*>(cur_name,
								it->second));
			}
		}

		if (find_defined_var == true) {
			if (DEBUG) {
				cout << endl << "Find " << define_name << "       inside  "
						<< inst->to_string() << endl;
				cout << "Alias name is " << alias_name << endl;
			}
			ReachingDefVarStat* newstat = new ReachingDefVarStat(defined_var,
					inst);
			set_inst_out.insert(
					pair<string, ReachingDefVarStat*>(define_name, newstat));
		}


	} else {
		/*if the current instruction define a new variable. Just replace/insert it into set_inst_out*/
		//set_inst_out = *map_iter;
		assert(map_reaching.count(inst) > 0);
		map<string, ReachingDefVarStat*> map_reaching_content = map_reaching[inst];
		set_inst_out.insert(map_reaching_content.begin(),
				map_reaching_content.end());

		if (find_defined_var) {

			if (DEBUG) {
				cout << endl << "Find " << define_name << "       inside  "
						<< inst->to_string() << endl;
				cout << "Alias name is ::  " << alias_name << endl;
			}

			ReachingDefVarStat* newstat = new ReachingDefVarStat(defined_var,
					inst);
			set_inst_out.erase(define_name);
			set_inst_out[define_name] = newstat;
		}

	}
}



void ReachingDefInfer::compute_miniReaching(){



	if (OPTIMIZE_MINI_REACHING) {

		for (int u = 0; u < (int) this->ordered_insts->size(); u++) {
			Instruction* inst_key = this->ordered_insts->at(u);
			assert(inst_key != NULL);

//			cout << endl << " ^^^^^^^^^^^^^^ {{" << u << "}}   "
//								<< inst_key->to_string() << " ^^^^^^^^^^^^^^^^^ " << "  "
//								<< inst_key << endl;

			if (inst_key->is_save_instruction()) {
				get_reachingset(inst_key, map_mini_reaching[inst_key]);
//				Variable* var = sail::get_defined_variable(inst_key);
				Variable* var = inst_key->get_lhs();
				if (var == NULL)
					continue;

				/*set<Instruction*> set_reaching;
				 get_reachingset(inst_key, set_reaching);

				 lni->eliminate_reaching(var->get_var_name(), set_reaching);

				 map_mini_reaching[inst_key] = set_reaching;*/

				lni->eliminate_reaching(var->get_var_name(),
						map_mini_reaching[inst_key]);
			}
		}

	} else {

		map<Instruction*, set<Instruction*> >::iterator it1 =
				map_mini_reaching.begin();
		//int i = 0;
		for (; it1 != map_mini_reaching.end(); it1++) {
			Variable* var = sail::get_defined_variable(it1->first);
			if (var == NULL) {
				continue;
			}
			set<Instruction*> set_reaching;
			get_reachingset(it1->first, set_reaching);
			//cout << "reaching set  size " << set_reaching.size() << endl;
			lni->eliminate_reaching(var->get_var_name(), set_reaching);
			it1->second = set_reaching;
		}
	}
}





void ReachingDefInfer::get_reachingset(Instruction* inst,
		set<Instruction*>& set_reaching)
{
	if(map_rev_reaching.count(inst) > 0){
		set_reaching = map_rev_reaching[inst];
		return;
	}

	cout << "need to find inst " << inst->to_string() <<
			"	in map_rev_reaching! " << endl;
	assert(false);
}






void ReachingDefInfer::get_reachingmini(Instruction* inst,
		set<Instruction*>& set_reaching)
{
	if(map_mini_reaching.count(inst) > 0){
		set_reaching = map_mini_reaching[inst];
		return;
	}

	cout << "need to find inst " << inst->to_string()
			<< " in map_mini_reaching!! " << endl;

	assert(false);

}


void ReachingDefInfer::print_current_map_reaching(Instruction* inst){
	cout << endl << "********** cur_point ******   "
			<< inst->to_string() << endl;
	//skip those branch, goto instruction;
	if (inst->is_save_instruction()) {

		if (!map_reaching.count(inst)) {
			cout << "can't find inst whose id is "
					<< inst->get_instruction_id() << endl;
		}

		assert(map_reaching.count(inst) > 0);

		map<string, ReachingDefVarStat*>* start_points =
				&(map_reaching[inst]);

		map<string, ReachingDefVarStat*>::iterator it =
				start_points->begin();
		int i = 0;
		for (; it != start_points->end(); it++) {

			cout << "       ********** [" << i++ << "] starting from " <<
					it->second->to_string() << endl;

		}

	}

}


void ReachingDefInfer::print_out_mapping() {
	cout << "=======  ReachingDefInfer::print_out_mapping  start ========"
			<< endl;

	if (1) {
		for (int i = 0; i < (int) this->ordered_insts->size(); i++) {
			Instruction* key_inst = ordered_insts->at(i);

			cout << endl << "********** cur_point ******   "
					<< key_inst->to_string() << endl;
			//skip those branch, goto instruction;
			if (key_inst->is_save_instruction()) {

				if (!map_reaching.count(key_inst)) {
					cout << "can't find inst whose id is "
							<< key_inst->get_instruction_id() << endl;
				}

				//assert(map_reaching.count(key_inst) > 0);

				if(map_reaching.count(key_inst) > 0){

					map<string, ReachingDefVarStat*>* start_points =
							&(map_reaching[key_inst]);

					map<string, ReachingDefVarStat*>::iterator it =
							start_points->begin();
					int i = 0;
					for (; it != start_points->end(); it++) {

						cout << "       ********** [" << i++ << "] starting from " <<
								"	String is " << it->first << endl <<	"	Variable is "
								<< it->second->to_string() << endl;

					}
				}
			}

		}

	} else {

		map<Instruction*, map<string, ReachingDefVarStat*> >::iterator map_iter;
		for (map_iter = map_reaching.begin(); map_iter != map_reaching.end();
				map_iter++) {
			Instruction* inst = map_iter->first;
			cout << "first element: " << inst->to_string()
					<< "; get_instruction_id : "
					<< map_iter->first->get_instruction_id();
			cout << "second element: " << endl;
			map<string, ReachingDefVarStat*>::iterator sec_iter;
			sec_iter = map_iter->second.begin();
			for (; sec_iter != map_iter->second.end(); sec_iter++) {
				cout << "var  : " << sec_iter->first << endl;
				cout << sec_iter->second->to_string() << endl;
			}
		}

	}

	cout << "=======  ReachingDefInfer::print_out_mapping  end ========"
			<< endl;
}


void ReachingDefInfer::print_out_rev_map(){
	cout << "=======  ReachingDefInfer::print_out_rev_map  start ========" << endl;
	map<Instruction*, set<Instruction*> >::iterator  map_iter;
	map_iter = map_rev_reaching.begin();
	for(; map_iter != map_rev_reaching.end(); map_iter++)
	{
		Instruction* inst = map_iter->first;
		cout << "first element: " << inst->to_string() << "; get_instruction_id : "
				<< map_iter->first->get_instruction_id();
		cout << "second element: " << endl;
		set<Instruction*>::iterator sec_iter;
		sec_iter = map_iter->second.begin();
		for(; sec_iter != map_iter->second.end(); sec_iter++)
		{
			cout << (*sec_iter)->to_string() << endl;
		}
	}
	cout << "=======  ReachingDefInfer::print_out_rev_map  end ========" << endl;
}


Def* ReachingDefInfer::make_def(Instruction* inst) {

	if (def_factory_map.count(inst) > 0)
		return def_factory_map[inst];


	Def* def = new Def(inst, inst->get_lhs());
	def_factory_map[inst] = def;
	unresolved_def.insert(def);
	return def;

}


Reaching* ReachingDefInfer::make_reaching(Instruction* inst,
		string& in_use)
{
	pair<Instruction*,string> p(inst, in_use);

	if(reaching_factory_map.count(p) > 0)
		return reaching_factory_map[p];

	Reaching* reaching = new Reaching(inst, in_use);
	reaching_factory_map[p] = reaching;
	unresolved_reaching.insert(reaching);

	return reaching;

}


void ReachingDefInfer::delete_defs()
{

	for(auto it = def_factory_map.begin();
			it != def_factory_map.end(); it++){
		Def* def = it->second;

		delete def;
	}
	def_factory_map.clear();
}


void ReachingDefInfer::delete_reachings()
{
	for(auto it = reaching_factory_map.begin();
			it != reaching_factory_map.end(); it++){

		Reaching* reach = it->second;
		delete reach;
	}

	reaching_factory_map.clear();
}

void ReachingDefInfer::build_def_use_chain(){

	for(auto it = map_mini_reaching.begin();
			it != map_mini_reaching.end(); it++){

		Instruction* inst = it->first;



		if(inst->is_save_instruction()){ //which must have the defined variable;

			if(DEBUG)
				cout << "****** build_def_use_chain for instruction "
					<< inst->to_string() << endl;



			Variable* lhs = inst->get_lhs(); //in the funcitioncall inst;
			if(lhs == NULL)
				continue;

			Def* define = NULL;
			if(lhs != NULL){
				//cout << "lhs is " << lhs->to_string() << endl;
				define = make_def(inst);
				assert(define != NULL);
			}




			set<Instruction*>& reaching_points = it->second;

			set<Reaching*> in_uses;

			for(auto iter = reaching_points.begin();
					iter != reaching_points.end(); iter++){

				Instruction* reaching_inst = *iter;

				vector<Symbol*> syms;
				reaching_inst->get_in_use_symbols(syms);

				set<string> syms_str;
				for(auto it = syms.begin(); it != syms.end(); it++){
					Variable* var = (Variable*)(*it);
					syms_str.insert(var->get_var_name());
				}

				//it is not guarantee that they share the same address (e.g, defintion has type,
				//but reaching point variable is invalid type)
				string define_var_name = lhs->get_var_name();

				if(syms_str.count(define_var_name) > 0){

					Reaching* rea = make_reaching(reaching_inst,
							define_var_name);

					in_uses.insert(rea);

					if(use_def_chain.count(rea) > 0){ //already build, then directly insert
						use_def_chain[rea].insert(define);
					}else{
						set<Def*> defines;
						defines.insert(define);
						use_def_chain[rea] = defines;
					}
				}
			}

			this->def_use_chain[define] = in_uses;



			if (DEBUG) {

				print_def_use_chain_for_instruction(define);

				for (auto it = in_uses.begin(); it != in_uses.end(); it++) {
					Reaching* reach = *it;
					print_use_def_chain_for_instruction(reach);
				}
				cout << "------------------------ "<< endl << endl << endl;
			}




		}

		//check the result after processing build_use_define_chain


	}

}


void ReachingDefInfer::print_def_use_chain_for_instruction(Def* def)
{
	cout <<"...." <<def->to_string() << endl;
	cout << "	ADDR [" << def << "]" << endl;
	assert(this->def_use_chain.count(def) > 0);

	set<Reaching*>& reachings = def_use_chain[def];

	for (auto it = reachings.begin(); it != reachings.end(); it++) {
		cout << "	==> " << (*it)->to_string() << endl;
		cout << "	Addr [" << *it << "]" << endl;
	}
}


void ReachingDefInfer::print_use_def_chain_for_instruction(
		Reaching* reach)
{
	cout << "........... " << reach->to_string() << endl;
	cout << "	ADDR [" << reach << "]" << endl;

	assert(this->use_def_chain.count(reach) > 0);
	set<Def*>& defs = this->use_def_chain[reach];

	for (auto it = defs.begin(); it != defs.end(); it++) {
		cout << "	Birth point is "
				<< (*it)->to_string() << endl;
		cout << "	Addr [" << (*it) << "]" << endl;
	}
}

void ReachingDefInfer::print_def_use_chain()
{

	for(int i = 0; i < (int) ordered_insts->size(); i++){
		Instruction* cur_inst = ordered_insts->at(i);

		if (cur_inst->is_reachable()) {
			if (cur_inst->is_save_instruction()) { //make sure it has been defined;

				if (cur_inst->get_lhs() != NULL) {

					Def* def = this->make_def(cur_inst);
					cout << "@@@@@@@@ " << def->to_string() << endl;

					assert(this->def_use_chain.count(def) > 0);

					set<Reaching*>& reachings = def_use_chain[def];

					for (auto it = reachings.begin(); it != reachings.end();
							it++) {
						cout << "	==> " << (*it)->to_string() << endl;
					}

					cout << endl;
				}

			}
	}
	}
}

void ReachingDefInfer::print_use_def_chain(){

	for (int i = 0; i < (int) ordered_insts->size(); i++) {

		Instruction* inst = ordered_insts->at(i);

		if (inst->is_reachable()) {

			vector<Symbol*> syms;
			inst->get_in_use_symbols(syms);

			for (int i = 0; i < (int) syms.size(); i++) {
				Symbol* sym = syms.at(i);

				assert(sym->is_variable());
				Variable* in_use_var = (Variable*) sym;

				if (in_use_var->is_argument())
					continue;

				string in_use_str = in_use_var->get_var_name();

				Reaching* re = make_reaching(inst, in_use_str);

				cout << "Reaching point is :: " << re->to_string() << endl;

				//must be defined;
				assert(this->use_def_chain.count(re) > 0);

				set<Def*>& defs = this->use_def_chain[re];

				for (auto it = defs.begin(); it != defs.end(); it++) {
					cout << "	Birth point is " << (*it)->to_string() << endl;
				}

				cout << endl;

			}

		}

	}

}


void ReachingDefInfer::print_out_mini_map() {
	cout << "=======  ReachingDefInfer::print_out_mini_map  start ========"
			<< endl;

	if (1) {
		for (int i = 0; i < (int) ordered_insts->size(); i++) {

			Instruction* key_inst = ordered_insts->at(i);
			cout << endl << "********** Birth Point  " << key_inst->to_string() << endl;


			//skip those branch, goto instruction; only for save_instruction has birth point;
			if (key_inst->is_save_instruction()) {

				if (!map_mini_reaching.count(key_inst)) {
					cout << "can't find inst whose id is "
							<< key_inst->get_instruction_id() << endl;
				}

				//assert(map_mini_reaching.count(key_inst) > 0);

				//and probably not reachable(for those exception blocks, which is not occupied);
				if(map_mini_reaching.count(key_inst)){

					set<Instruction*>* reaching_points =
							&(map_mini_reaching[key_inst]);

					set<Instruction*>::iterator it = reaching_points->begin();

					for (; it != reaching_points->end(); it++) {
						cout << "	 	reaching ==> " << (*it)->to_string() << endl;
					}
				}
			} else {

				cout << "	isn't save instruction!! " << endl;
			}

		}
	} else {

		map<Instruction*, set<Instruction*> >::iterator map_iter;
		map_iter = map_mini_reaching.begin();
		for (; map_iter != map_mini_reaching.end(); map_iter++) {
			Instruction* inst = map_iter->first;
			cout << "first element: " << inst->to_string()
					<< "; get_instruction_id : "
					<< map_iter->first->get_instruction_id();
			cout << "second element: " << endl;
			set<Instruction*>::iterator sec_iter;
			sec_iter = map_iter->second.begin();
			for (; sec_iter != map_iter->second.end(); sec_iter++) {
				cout << (*sec_iter)->to_string() << endl;
			}
		}
	}
	cout << "=======  ReachingDefInfer::print_out_mini_map  end ========"
			<< endl;
}




