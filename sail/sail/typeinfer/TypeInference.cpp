/*
 * TypeInference.cpp
 *
 *  Created on: Oct 3, 2012
 *      Author: boyang
 */

#include "TypeInference.h"

#include "sail.h"
#include "il.h"

#define DEBUG false

#define USING_DECLARED_TYPE_NARROW_DOWN_TYPE true
#define JAVA_LANG_EXCEPTION "Ljava/lang/Exception;"
#define JAVA_LANG_INTEGER "Ljava/lang/Integer;"
#define JAVA_OBJECT_POINTER "Ljava/lang/Object;*"
#define JAVA_LANG_CHAR "Ljava/lang/Character;"

#define PHASE_CHECK true
#define CATCH_METHOD false
#define TYPE_WARNING false

//#define RENAMING_OPTIMIZE false

using namespace std;
using namespace sail;
using namespace il;

namespace sail {

TypeInference::TypeInference(Function* f, BasicBlock* entry_b,
		BasicBlock* exit_b, BasicBlock* exception_b) {
	this->f = f;
	this->entry_block = entry_b;
	this->exit_block = exit_b;
	this->exception_block = exception_b;
	this->is_changing = true;
}



TypeInference::~TypeInference() {
	// TODO Auto-generated de-construct stub
}

void TypeInference::assign_new_type() {

	basic_flow_analysis();
	//return;

	bool isFirst = true;
	int u = 0;
	while (is_changing) {
		is_changing = false;

		if(u == 10){
			assert(false);
		}

		if(PHASE_CHECK)
			cout << endl<<"########### The main fix point round ################# "
				<< u << endl;

		precise_type_analysis(isFirst);

		type_infer_analysis(isFirst, u);


		isFirst = false;
		u ++;
	}

	delete this->lni;
	delete this->rdi;

}


void TypeInference::basic_flow_analysis(){
	Liveness_reachingdef_analysis();

	compute_miniReaching();

	compute_renaming();

	//because of the renaming phase has changed some of the strings in map;
	update_livessness();
}

void TypeInference::Liveness_reachingdef_analysis() {

	if (PHASE_CHECK)
		cout << "	###--------------------- " << endl << "1st phase begin    @@ "
				<< get_time() << endl << "--------------------" << endl;

	lni = new LivenessInfer(entry_block, exit_block, exception_block, f);
	lni->LivenessInferAnalysis();


	if (PHASE_CHECK)
		cout << "--------------------- " << endl
				<< "1st phase (LivenessInferAnalysis) done ! @@ " << get_time()
				<< endl << "############################### " << endl;




	if (PHASE_CHECK)
			cout << "	###--------------------- " << endl << "2nd phase "
					"(ReachingDefAnalysis) begin    @@ "
					<< get_time() << endl << "--------------------" << endl;

	rdi = new ReachingDefInfer(entry_block, exit_block, exception_block,
			this->f, lni);
	rdi->reachingDefAnalysis();


	if (PHASE_CHECK)
		cout << "--------------------- " << endl
				<< " 2nd phase (ReachingDefAnalysis) done    @@ " << get_time()
				<< endl << "############################### " << endl;
}

void TypeInference::compute_miniReaching(){
	if(PHASE_CHECK)
		cout << "	###--------------------- " << endl
		<< "3nd phase (compute_miniReaching) begin ! @@ " << get_time()
		<< endl ;

	rdi->compute_miniReaching();

	if(PHASE_CHECK)
		cout << " 3nd phase (compute_miniReaching) done @@ " << get_time()
		<< endl << "-----------------------######" << endl;
}


void TypeInference::compute_renaming(){
	if(PHASE_CHECK)
		cout << "	###--------------------- " << endl
			<< " def_use_chain() built begin !!! @@ " << get_time()
			<< endl;


	rdi->build_def_use_chain();

	if(PHASE_CHECK)
		cout<< " def_use_chain() built done !!! @@ " << get_time()
		<< endl << "---------------------#######" << endl;



	if (0) {
		cout << "PPPPPPPrint def_use_chain() " << endl;
		rdi->print_def_use_chain();
		cout << "EEEEEEEEEnd of def_use_chain() " << endl;
		rdi->print_use_def_chain();
	}



	if(PHASE_CHECK)
			cout << "	###------------------------ " << endl
			<< " Renaming phase begin ! @@@@@@@@@ " << get_time()
			<< endl ;

	rdi->renaming_phase();

	if(PHASE_CHECK)
		cout<< " Renaming phase done ! @@@@@@@@@ " << get_time()
		<< endl << "------------------------#######" << endl;

}



void TypeInference::update_livessness() {
	//do the liveness again, because renaming has changed the string;
	delete lni;
	lni = new LivenessInfer(entry_block, exit_block, exception_block, f);
	lni->LivenessInferAnalysis();

	if (PHASE_CHECK)
		cout << " LivenessInfer after renaming done ! @@@@@@@@@ "
				<< get_time() << endl
				<< "------------------------####### " << endl;

}


void TypeInference::precise_type_analysis(const bool& isFirst){

	if(PHASE_CHECK)
		cout << "	###------------------------ " << endl
		<< "4th phase (PreciseTypeInfer) begin !" << " isFirst?	 "
		<< isFirst << endl;

	pti = new PreciseTypeInfer(entry_block, exit_block, exception_block,
			lni, f);

	pti->precise_type_analysis(isFirst);

	if(PHASE_CHECK)
		cout << "4th phase (precise_type_analysis) done!!!! @@ " << get_time()
		<< endl << "---------------------########## " << endl;

	if(DEBUG)
		pti->print_out_precise_typeinfer_mapping();
}


bool TypeInference::type_infer_analysis(const bool& isFirst, const int& round){

	set<Block*, sail::CompareBlock> worklist;

	worklist.insert(entry_block);

	int i = 0;

	while (worklist.size() > 0) {

		if(i == 100){
			assert(false);
		}

		Block* b_cur = *(worklist.begin());
		worklist.erase(b_cur);

		if (DEBUG)
			cout << endl << endl << "RRRound (type_infer_analysis()) " << i++
			<< "  &&&&&&&&&& for block id is " << b_cur->get_block_id() << endl;


		if (b_cur->is_basicblock()) {

			BasicBlock* bb_cur = static_cast<BasicBlock*>(b_cur);
			vector<Instruction*> vector_inst = bb_cur->get_statements();

			if(1){
			//if (DEBUG) {
				cout << endl << endl << "||| ============================= "
						<< endl << "Block id is " << b_cur->get_block_id()
						<< endl;
				cout << "instructions size is ? " << vector_inst.size()
						<< endl << "================================= |||"
						<< endl;
			}

			int j = 0;

			vector<Instruction*>::iterator it;

			for (it = vector_inst.begin(); it != vector_inst.end(); ++it) {
				//if (DEBUG) {
				if(1){
					cout << endl;
					cout << "\t" << " ==> index " << j++ << endl;
					cout << "\t" << " 	updating instruction ::"
							<< (*it)->to_string() << endl;
					cout << "\t" << " 	instruction pp :: "
							<< (*it)->to_string(true) << endl;
				}

				bool has_changing_cur =
						assign_new_type_inst(*it, isFirst, round);

				/******
				 * has_changing  == fix point
				 */
				if (has_changing_cur == true) {
					//if (DEBUG) {
					if(1){
						cout << "%%%%++%%%% IInstruction " << (*it)->to_string() << endl;
						cout << "%%%%++%%%% TType has change!!  " << endl;
					}
					this->is_changing = true;
				}
			}

		}

		//processing its successors;
		set<CfgEdge*>& set_Edges = b_cur->get_successors();
		set<CfgEdge*>::iterator iter_Edges = set_Edges.begin();
		for (; iter_Edges != set_Edges.end(); iter_Edges++) {
			Block* succ = (*iter_Edges)->get_target();
			if (DEBUG) {
				cout << "	success is " << succ->get_block_id();
				if (succ->is_basicblock()) {
					cout << "	Size of instruction inside it is "
							<< ((BasicBlock*) succ)->get_statements().size()
							<< endl;
				}
			}
			worklist.insert((*iter_Edges)->get_target());
		}

		if(DEBUG)
			cout << "	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~  "
				<< endl;
	}

	//delete lni;
	//delete rdi;
	delete pti;
}
/**********
 * fixed mean if it is type from the original;
 *
 * isFirst means first iteration
 *
 * return bool is any changed changed(for fix point)
 */

bool TypeInference::assign_new_type_inst(Instruction* inst,
		bool isFirst, const int& round)
{

	//get lhs variable;
	Variable* var = sail::get_defined_variable(inst);

	if (var == NULL) {
		return false;
	}



	if (isFirst) {
		if ((!var->get_type()->is_invalid_type())
				&& (var->get_type()
					!= type::get_type_from_name(JAVA_LANG_EXCEPTION))) {
			var->fixed = true;
		} else //didn't found then need to infer
		{
			var->fixed = false;
		}

	} else {

		if (var->fixed == true)  //if already found
			return false;

	}

	//lhs variable type(only change defined type), which including computer LUB.
	type* new_type = var->get_type();
	//if not declared, need to infer based on all its reaching points' type!
	if (!var->fixed) {
		cout << "isFirst ? " << isFirst << endl;
		cout << "var is not fixed! " << var->to_string() << endl;
		new_type = get_new_def_type_inst(inst, isFirst);
	}

	assert(new_type != NULL);
	bool has_change = false;

	Variable* rename_var = var->clone();
	if(new_type != var->get_type()){
		rename_var->set_type(new_type);
		cout << "@@ @@@@@@@@ def variable original type is 	" <<
				var->get_type()->to_string() << endl;
		cout << "@@ @@@@@@@@@ def variable new type is 	" <<
				new_type->to_string() << endl;
		has_change = true;
	}

	//this one will skip to process, this will produce problem;
	//(the first time not change, but will change the second type!)
	if(!isFirst){
		if(!has_change){
			if(round > 2)
				return false;
		}
	}


#if 0
		if (mapvp.find(Key(var->get_var_name(), var->get_type())) == mapvp.end()
				&& (!var->get_type()->is_invalid_type())) {

			this->mapvp.insert(
					pair<Key, Variable*>(
							Key(var->get_var_name(), var->get_type()), var));
			used_var_names.insert(var->get_var_name());

			if (0) {
				cout << "	= >inserting (1st) " << var->get_var_name() << " as "
						<< var->get_type()->to_string() << endl
						<< "	--------------------------------" << endl;
			}
		}

		//variable_name + type is unique, so even if same variable name is used,
		//if not found (name + type), store it to map, otherwise do not change the map;
		if (mapvp.find(Key(var->get_var_name(), new_type)) == mapvp.end()) {
			assert(new_type != NULL);
			Variable* new_var = var->clone();
			//changed by haiyan; if 1 changed to 0, then back to boyang's code!
			if (1) {
				if (used_var_names.count(var->get_var_name()) > 0) //added by haiyan;
					new_var->set_var_name_as_temp_name();
				else {

					used_var_names.insert(var->get_var_name());
				}

				new_var->set_type(new_type);
				this->mapvp.insert(
						pair<Key, Variable*>(Key(var->get_var_name(), new_type),
								new_var));

				if (0)
					cout << "	=>inserting (2nd) " << var->get_var_name()
							<< "	as type " << new_type->to_string() << endl
							<< "	----------------------------------------- "
							<< endl;
			} else {
				//get new temp name;
				Variable* new_var_naming = new Variable(var->get_var_name(),
						new_type);
				//new_var->set_name(var->get_var_name());
				new_var->set_name(new_var_naming->get_var_name());
				new_var->set_type(new_type);
				this->mapvp.insert(
						pair<Key, Variable*>(Key(var->get_var_name(), new_type),
								new_var));
			}
		}

#endif
	//for the purpose of SSA form,
	//we need each usage of variable refers to unique definition;


	//**** generate map on-the-fly end ****

	/*******
	 * using the mapvp,  write variable information to itself and reaching points;
	 * 从当点出发的type 信息写到它所有的reaching 点
	 */
#if 0
	{
		map<Key, Variable*>::iterator iter = mapvp.find(
				Key(var->get_var_name(), new_type));
	}
#endif

	//Variable* rename_var =
	//Variable * rename_var = iter->second;



	//renaming phase
	vector<Symbol*> vect_syms;
	//get all symbols(lhs + rhs)
	sail::get_symbols_inst(inst, vect_syms);

	if (0) { //used for assertion only
		if (vect_syms.at(0)->is_variable()) {
			Variable* var0 = static_cast<Variable*>(vect_syms.at(0));
			assert(var0->get_var_name() == var->get_var_name());
			assert(var0->get_type() == var->get_type());
		} else {
			assert(false);
		}
	}

	if (0) {
		//need to know why they have different variable name;
		if ((var->get_var_name() != rename_var->get_var_name())
				|| (var->get_type() != rename_var->get_type())) {
			has_change = true;
		}
	}
	//change the lhs of current instruction;
	vect_syms.at(0) = rename_var;
	set_symbols_inst(inst, vect_syms);

	set<Instruction*> set_reaching;
	//rdi->get_reachingset(inst, set_reaching);
	rdi->get_reachingmini(inst, set_reaching);
	//	//only changed the lhs variable
	//	set_symbols_inst(inst, vect_syms);

	set<Instruction*>::iterator iter_reaching = set_reaching.begin();
	for (; iter_reaching != set_reaching.end(); iter_reaching++) {
		bool cur_has_change = false;
		//if it's not exit block and not exception
		if ((*iter_reaching) != NULL && (*iter_reaching)->line != -99) {
			//processing lhs of reached points
			Variable* v_cur = sail::get_defined_variable(*iter_reaching);

			if (v_cur != NULL) {
				//if(v_cur->get_var_name() != var->get_var_name())
				vect_syms.clear();
				sail::get_symbols_inst(*iter_reaching, vect_syms);
				//used it to update the the reaching points;
				//find the rhs that matching current instruction's lhs and update
				cur_has_change = vect_syms_replace(vect_syms, var, rename_var,
						true);
				//use the vect_syms to update (*iter_reaching);
				set_symbols_inst(*iter_reaching, vect_syms);
			} else {
				vect_syms.clear();
				sail::get_symbols_inst(*iter_reaching, vect_syms);
				cur_has_change = vect_syms_replace(vect_syms, var, rename_var,
						false);
				set_symbols_inst(*iter_reaching, vect_syms);
			}
		}
		if (cur_has_change) {
			//if (DEBUG) {
			if(1){
				cout << "	== $$ assign_new_type_inst has change : "
						<< (*iter_reaching)->to_string() << endl;
			}
			has_change = true;
		}
	}
	return has_change;
}



bool TypeInference::vect_syms_replace( vector<Symbol*>& vect_syms,
		Variable* var_ori,
		Variable* rename_var,
		bool ignore_first)
{
	bool has_change = false;
	vector<Symbol*>::iterator iter;
	int counter = 0;
	while (counter < (int) vect_syms.size()) {
		if (ignore_first && counter == 0) {
			counter++;
			continue;
		}
		if (vect_syms.at(counter)->is_variable()) {
			Variable* var = static_cast<Variable*>(vect_syms.at(counter));
			if (var->get_var_name() == var_ori->get_var_name()) {
				if ((var->get_var_name() != rename_var->get_var_name())
						|| (var->get_type() != rename_var->get_type())) {
					//if (DEBUG) {
					if(1){
						cout << "	[[ has change !!! " << endl;
						cout << "	[[ var : " << var->get_var_name() << ";"
								<< var->get_type()->to_string() << endl;
						cout << "	[[var_ori : " << var_ori->get_var_name() << ";"
								<< var_ori->get_type()->to_string() << endl;
						cout << "	[[rename_var : " << rename_var->get_var_name()
								<< ";" << rename_var->get_type()->to_string()
								<< endl;
					}
					has_change = true;
				}
				vect_syms.at(counter) = rename_var;
			}
		}
		counter++;
	}
	return has_change;
}



type* TypeInference::get_new_def_type_inst(Instruction* inst, const bool& isFirst) {

	//if (DEBUG)
		cout << "============== get_new_def_type_inst start for inst ==========="
				<< endl << inst->to_string() << endl;

	type* new_type = NULL;


	Variable* def = sail::get_defined_variable(inst);
	if (def == NULL) {
		return new_type;
	}


	/******
	 * get a set of instruction that current inst can reach
	 */
	set<Instruction*> set_reaching;
	rdi->get_reachingmini(inst, set_reaching);





	//get all possible types (from instructions can be reached), could be used to computer LUB
	//added by haiyan

	if (USING_DECLARED_TYPE_NARROW_DOWN_TYPE) {

		//get from the precisetype map;
		type* start_type = NULL;
		if(isFirst){
			start_type = pti->get_cur_type(inst);
		}
		else{
			start_type = def->get_type();
			if (inst->get_instruction_id() == sail::ASSIGNMENT) {
				if (start_type == il::get_integer_type()) { //rhs may propagate to lhs;
					start_type = pti->get_cur_type(inst);
				}
			}
		}

		//type* start_type = pti->get_cur_type(inst); //this one added by haiyan


		if(start_type->is_invalid_type()){
			cerr << "instruction is " << inst->to_string() << endl;
			cerr << "get type from rhs of this instruction, shouldn't be invalid !" << endl;
			assert(false);
		}




		if (1)
			cout << "current instruction is " << inst->to_string() << "[[ "
					<< "Start Type :: " << start_type->to_string() << " ]]" << endl;

		//if interger and its rhs is 0, probably it is NULL pointer rather than int;
		bool need_to_infer = false;

		if(inst->get_instruction_id() == sail::ASSIGNMENT){
			if(start_type == il::get_integer_type()){

				Symbol* rhs = ((Assignment*)inst)->get_rhs();

				if(rhs->is_constant()){
					Constant* const_value = static_cast<Constant*>(rhs);
					if(const_value->get_integer_value() == 0){
						need_to_infer = true;
					}
				}
			}
		}
		cout << "need to infer ?? " << need_to_infer << endl;

		map<Instruction*, type*> reaching_points_info;
		pti->get_reaching_points_inf(def, set_reaching, reaching_points_info);

		if (1) {
			for (auto it = reaching_points_info.begin();
					it != reaching_points_info.end(); it++) {
				cout << "	((Reaching insts " << it->first->to_string() << endl;
				cout << "	((Reaching type is " << it->second->to_string() << endl;
			}
		}

		//traverse the map;
		map<Instruction*, type*>::iterator reaching_iter =
				reaching_points_info.begin();

		int reching_size = reaching_points_info.size();

		if(reching_size == 0)
			return start_type;

		if ((isFirst) && (reching_size == 1)) {
			//check if narrow down if work;
			cout << " Case 11 " << endl;
			Instruction* reaching_inst = reaching_iter->first;

			type* pick_type = reaching_iter->second;

			if (need_to_infer) {
				cout << "pick_type is " << pick_type->to_string() << endl;
				return pti->get_better_type(def, pick_type, reaching_inst, f,
						need_to_infer);
			}

			if (reaching_inst->is_return_inst()
					|| (reaching_inst->get_instruction_id() == FUNCTION_CALL)) {
				type* better_t = pti->get_better_type(def, pick_type,
						reaching_inst, f, need_to_infer);
				return better_t;
			}
			int indicate = 0;
			il::type* ret_type = sail::lub_compute(start_type, pick_type, indicate);
			assert(ret_type != NULL);
			return ret_type;
		}



		type* type_in_return_inst = NULL;
		set<type*> better_types;
		int i = 0;
		for (; reaching_iter != reaching_points_info.end();
				reaching_iter++) {

			type* pick_type = reaching_iter->second;

			//add narrow down here
			Instruction* reaching_point_inst = reaching_iter->first;



			if (pick_type->is_invalid_type())
				continue;




			type* better_t = pti->get_better_type(def, pick_type,
					reaching_point_inst, f, need_to_infer);

			if (1) {
				cout << endl << "      ||  ==> picked_inst is "
						<< reaching_point_inst->to_string() << endl;
				cout << "      || ==> picked precise type is  "
						<< pick_type->to_string() << endl;
				cout <<"		|| ==> better type is "
						<< better_t->to_string() << endl << endl;
			}

			//store this one, of lub doesn't make sense;
			if(reaching_point_inst->is_return_inst()){
				type_in_return_inst = better_t;
			}

			if(better_t != pick_type){
				better_types.insert(better_t);
			}
			//if any of its reaching points is object type, then it must have its LUB is object type!
			if(better_t == type::get_type_from_name(JAVA_OBJECT_POINTER) ){
				return better_t;
			}

			assert(!better_t->is_invalid_type());


			int indicate = 1;
			if(1){
				cout << "	!!!!!!!!! " <<i << "	times !! " << endl;
				cout << "	~~~~~~~~$$$$$$$$$$$ start type is "
						<< start_type->to_string() << endl;
				cout << "	~~~~~~~~$$$$$$$$$$$ better_t is "
						<< better_t->to_string() << endl;
			}
			type* lub_type = sail::lub_compute(start_type, better_t, indicate);

			if(lub_type == NULL){
				if(need_to_infer){ //because it may be null pointer


					lub_type = type_in_return_inst;
					if(lub_type != NULL){

						if(1){
							cerr <<endl<<"######" <<this->f->get_signature()->to_string() << endl;
							cerr <<endl<< "^^^^^^Define point " << inst->to_string() << endl;
							for(auto it = reaching_points_info.begin();
									it != reaching_points_info.end(); it++)
							{
								cerr << " 	== >> Reaching points " << it->first->to_string() << endl;
							}
							if(CATCH_METHOD)
								assert(false);
						}

						start_type = lub_type;
						break;
					}
				}
			}

			if(lub_type == NULL){
				if(need_to_infer){
					assert(better_types.size() > 1);
					if(1){
						cerr <<endl <<"#######" <<  this->f->get_signature()->to_string() << endl;
						cerr <<endl << "^^^^^^Define point " << inst->to_string() << endl;
						for(auto it = reaching_points_info.begin();
								it != reaching_points_info.end(); it++)
						{
							cerr << " 	== >> Reaching points " << it->first->to_string() << endl;
						}
						if(CATCH_METHOD)
							assert(false);
					}
					lub_type = type::get_type_from_name(JAVA_OBJECT_POINTER);
					start_type = lub_type;
					break;
				}
			}

			assert(lub_type != NULL);



#if 0
			if (0) {
				cout << endl << "== >pick type is " << pick_type->to_string()
						<< endl;
				cout << "better_t " << better_t->to_string() << endl;
				cout << "start type " << start_type->to_string() << endl;
			}



			if (pick_type == better_t) {
				if ((!reaching_point_inst->is_return_inst())
						&& (reaching_point_inst->get_instruction_id()
								!= FUNCTION_CALL)){ //normal case, compute lub;
					//lub_type = sail::compute_lub(start_type, pick_type);
					lub_type = sail::lub_compute(start_type, pick_type);

					if(lub_type->is_invalid_type()){
						cerr << "Fix me !, LUB type shouldn't invalid type here!" << endl;
						cerr << "check code in compute_lub !! " << endl;
						assert(false);
					}
				}else
					lub_type = better_t;
			}
			else
			{ //which indicate it is the return/function_call instruction;
				lub_type = better_t;
			}
#endif
			if (1)
				cout << "  @@@ computed lub type is " << lub_type->to_string()
						<< endl;

			start_type = lub_type;

			i++;


		}

		new_type = start_type;
		if (0)
			cout << "new type is " << new_type->to_string() << endl << endl;

	}


	assert(!new_type->is_invalid_type());
	return new_type;
}

void TypeInference::set_symbols_inst(Instruction* inst,
		vector<Symbol*>& vect_syms) {

	switch (inst->get_instruction_id()) {
	case ASSIGNMENT: {

//		if(inst->is_return_inst())
//			cout << " !!!  !!! !!!  set up the rhs type of return instruction " << endl;

		int size = vect_syms.size();
		assert(size == 2);
		Assignment* inst_assignment = static_cast<sail::Assignment*>(inst);
		//======= lhs ========
		Symbol* s0 = vect_syms.at(0);
		if (s0->is_variable()) {
			Variable* v0 = static_cast<Variable*>(s0);
			inst_assignment->set_lhs(v0);
		} else {
			assert(false);
		}
		//======= rhs ========
		Symbol* s1 = vect_syms.at(1);
		if (s1->is_variable()) {
			Variable* v1 = static_cast<Variable*>(s1);
//			if(inst->is_return_inst())
//				cout << " <MMM  == > set rhs variable is " << v1->to_string() << endl;
			inst_assignment->set_rhs(v1);
		}
		break;
	}
	case FUNCTION_CALL: {
		FunctionCall* functionCall = static_cast<FunctionCall*>(inst);
		//======= lhs ========
		if (functionCall->get_lhs() != 0) {
			Symbol* s0 = vect_syms.at(0);
			if (s0->is_variable()) {
				Variable* v0 = static_cast<Variable*>(s0);
				functionCall->set_lhs(v0);
			} else {
				assert(false);
			}
		}

		//===== args ======
		vector<Symbol*>* arguments = functionCall->get_arguments();
		int size_vec = vect_syms.size();
		int size_arg = arguments->size();

		//		cout << "inst : " << functionCall->to_string() << endl;
		//		cout << "size_vec " << size_vec << endl;
		//		cout << "size " << size_arg << endl;
		assert(size_vec == size_arg || size_vec == size_arg + 1);
		int pos = 0;
		int start = size_vec - size_arg;
		vector<Symbol*>::iterator iter;
		for (iter = vect_syms.begin() + start; iter != vect_syms.end();
				iter++) {
			functionCall->set_rhs(*iter, pos);
			pos++;
		}
		break;
	}
	case ARRAY_REF_READ: {
		//no such test cases
		assert(false);
		break;
	}
	case ARRAY_REF_WRITE: {
		assert(false);
		break;
	}
	case BINOP: {
		int size = vect_syms.size();
		assert(size == 3);
		Binop* binop = static_cast<Binop*>(inst);
		//======= lhs ========
		Symbol* s0 = vect_syms.at(0);
		if (s0->is_variable()) {
			Variable* v0 = static_cast<Variable*>(s0);
			binop->set_lhs(v0);
		} else {
			assert(false);
		}
		//======= rhs ========
		Symbol* s1 = vect_syms.at(1);
		if (s1->is_variable()) {
			Variable* v1 = static_cast<Variable*>(s1);
			binop->set_rhs1(v1);
		}
		Symbol* s2 = vect_syms.at(2);
		if (s2->is_variable()) {
			Variable* v2 = static_cast<Variable*>(s2);
			binop->set_rhs2(v2);
		}
		break;
	}
	case CAST: {
		int size = vect_syms.size();
		assert(size == 2);
		Cast* cast = static_cast<Cast*>(inst);
		//======= lhs ========
		Symbol* s0 = vect_syms.at(0);
		if (s0->is_variable()) {
			Variable* v0 = static_cast<Variable*>(s0);
			cast->set_lhs(v0);
		} else {
			assert(false);
		}
		//======= rhs ========
		Symbol* s1 = vect_syms.at(1);
		if (s1->is_variable()) {
			Variable* v1 = static_cast<Variable*>(s1);
			cast->set_rhs(v1);
		}
		break;
	}
	case FIELD_REF_READ: {
		assert(false);
		break;
	}
	case LOAD: {
		int size = vect_syms.size();
		assert(size == 2);
		Load* load = static_cast<Load*>(inst);
		//======= lhs ========
		Symbol* s0 = vect_syms.at(0);
		if (s0->is_variable()) {
			Variable* v0 = static_cast<Variable*>(s0);
			load->set_lhs(v0);
		} else {
			assert(false);
		}
		//======= rhs ========
		Symbol* s1 = vect_syms.at(1);
		if (s1->is_variable()) {
			Variable* v1 = static_cast<Variable*>(s1);
			load->set_rhs(v1);
		}
		break;
	}
	case FIELD_REF_WRITE: {
		assert(false);
		break;
	}
	case STORE: {
		int size = vect_syms.size();
		assert(size == 2);
		Store* store = static_cast<Store*>(inst);
		//======= lhs ========
		Symbol* s0 = vect_syms.at(0);
		if (s0->is_variable()) {
			Variable* v0 = static_cast<Variable*>(s0);
			store->set_lhs(v0);
		}
		//======= rhs ========
		Symbol* s1 = vect_syms.at(1);
		if (s1->is_variable()) {
			Variable* v1 = static_cast<Variable*>(s1);
			store->set_rhs(v1);
		}
		break;
	}
	case JUMP: {
		assert(false);
		break;
	}
	case SAIL_LABEL: {
		assert(false);
		break;
	}
	case FUNCTION_POINTER_CALL: {
		assert(false);
		break;
	}
	case UNOP: {
		int size = vect_syms.size();
		assert(size == 2);
		Unop* unop = static_cast<Unop*>(inst);
		//======= lhs ========
		Symbol* s0 = vect_syms.at(0);
		if (s0->is_variable()) {
			Variable* v0 = static_cast<Variable*>(s0);
			unop->set_lhs(v0);
		} else {
			assert(false);
		}
		//======= rhs ========
		Symbol* s1 = vect_syms.at(1);
		if (s1->is_variable()) {
			Variable* v1 = static_cast<Variable*>(s1);
			unop->set_rhs(v1);
		}
		break;
	}
	case LOOP_INVOCATION: {
		//assert(false);
		break;
	}
	case DROP_TEMPORARY: {
		assert(false);
		break;
	}
	case STATIC_ASSERT: {
		int size = vect_syms.size();
		assert(size == 1);
		StaticAssert* st = static_cast<StaticAssert*>(inst);

		Symbol* s0 = vect_syms.at(0);
		if (s0->is_variable()) {
			Variable* v0 = static_cast<Variable*>(s0);
			st->set_predicate(v0);
		}
		break;
	}
	case ASSUME: {
		assert(false);
		break;
	}
	case ASSUME_SIZE: {
		assert(false);
		break;
	}
	case INSTANCEOF: {
		int size = vect_syms.size();
		assert(size == 2);
		InstanceOf* instanceOf = static_cast<InstanceOf*>(inst);

		//======= lhs ========
		Symbol* s0 = vect_syms.at(0);
		if (s0->is_variable()) {
			Variable* v0 = static_cast<Variable*>(s0);
			instanceOf->set_lhs(v0);
		} else {
			assert(false);
		}
		//======= rhs ========
		Symbol* s1 = vect_syms.at(1);
		if (s1->is_variable()) {
			Variable* v1 = static_cast<Variable*>(s1);
			instanceOf->set_rhs(v1);
		}
		break;
	}
	case EXCEPTION_RETURN: {
		assert(false);
		break;
	}
	case ADDRESS_LABEL: {
		assert(false);
		break;
	}
	case ADDRESS_VAR: {
		assert(false);
		break;
	}
	case ADDRESS_STRING: {
		int size = vect_syms.size();
		assert(size == 1);
		AddressString* addressstring = static_cast<AddressString*>(inst);
		Symbol* s0 = vect_syms.at(0);
		if (s0->is_variable()) {
			Variable* v0 = static_cast<Variable*>(s0);
			addressstring->set_lhs(v0);
		} else {
			assert(false);
		}
		break;
	}
	case SAIL_ASSEMBLY: {
		assert(false);
		break;
	}
	case BRANCH: {
		assert(false);
		break;
	}
	default: {
		assert(false);
		break;
	}
	}
}



record_type* base_to_record(base_type* b){
	if(b == il::get_char_type())
		return (record_type*)(type::get_type_from_name(JAVA_LANG_CHAR));

	if(b == il::get_integer_type()){
		return (record_type*)(type::get_type_from_name(JAVA_LANG_INTEGER));
	}
}


type* lub_base_base(base_type* b1, base_type* b2){
	assert(b1->is_base_type());
	assert(b2->is_base_type());
	if(b1->is_char_type())
		return b1;

	if(b2->is_char_type())
		return b2;

	assert(b1 == b2);
	return b1;
}

/*********
 *
 *  find in com.fun apps [ org.mozilla.javascript.Kit]
 *
 *  public static Object  [More ...] removeListener(Object bag, Object listener){
 *
 *	if(bag == listener){
 *		bag = null;
 *	}else if(bag.instanceof Object[]){
 *
 *		Object[] array = (Object[])bag;
 *		int L = array.length;
 *
 *		if(L = = 2){
 *			if(array[1] = listener){
 *			bag = array[0];
 *			}else if(array[0] == listener){
 *			bag = array[1];
 *			}
 *		}
 *	}
 *
 *	return bag;
 *
 * }
 */

type* lub_compute(type* t1, type* t2 , int& indicator){
	if(1){
			cout << "%%%%%%%%% compute_lub  " << "type1 :" << t1->to_string()
					<< "   ;type2 :" << t2->to_string() << endl;
		}
	if(t1 == t2)
		return t1;


	type* object_pt = type::get_type_from_name(JAVA_OBJECT_POINTER);
	if(t1 == object_pt)
		return t1;
	if(t2 == object_pt)
		return t2;



	if(t1->is_invalid_type())
		return t2;

	if(t2->is_invalid_type())
		return t1;

	if((t1->is_base_type())&&(t2->is_base_type())){
		return lub_base_base((base_type*)t1, (base_type*)t2);
	}

	//for the purpose of NULL (0) which actually is a pointer type;
	if(t1->is_pointer_type() && t2 == il::get_integer_type()){
		if(TYPE_WARNING)
			cerr << "PPProbably doesn't make sense, "
				" please check if the instruction rhs is zero! " << endl;
		return t1;
	}

	if(t1 == il::get_integer_type() && t2->is_pointer_type()){
		if(TYPE_WARNING)
			cerr << "PPProbably doesn't make sense, "
				" please check if the instruction rhs is zero! " << endl;
		return t2;
	}


	if((t1->is_base_type()) && (t2->is_pointer_type())){
		cerr << " can't compute LUB for  " << t1->to_string()
				<< " & " << t2->to_string() << endl;
		indicator = -1;
		return NULL;
		//assert(false);
	}

	if((t1->is_pointer_type()) && (t2->is_base_type())){
		cerr << " can't compute LUB for  " << t1->to_string()
						<< " & " << t2->to_string() << endl;
		indicator = -2;
		return NULL;
		//assert(false);
	}


	assert(t1->is_pointer_type() && t2->is_pointer_type());

	type* inner_t1 = t1->get_inner_type();
	type* inner_t2 = t2->get_inner_type();



	if(inner_t1->is_pointer_type() && inner_t2->is_pointer_type()){
		int indic = 0;
		type* inner_lub = lub_compute(inner_t1, inner_t2, indic);
		return il::pointer_type::make(inner_lub, inner_lub->to_string());
	} else {
		//inner type lub compute;
		record_type* inner_lub = NULL;
		if (inner_t1->is_record_type() && inner_t2->is_record_type()) {
			inner_lub = lub_record_record((record_type*) inner_t1,
					(record_type*) inner_t2);

		} else if (inner_t1->is_record_type() && inner_t2->is_base_type()) {

			inner_t2 = base_to_record((base_type*) inner_t2);

			inner_lub = lub_record_record((record_type*) inner_t1,
					(record_type*) inner_t2);

		} else if (inner_t1->is_base_type() && inner_t2->is_record_type()) {

			inner_t1 = base_to_record((base_type*) inner_t1);

			inner_lub = lub_record_record((record_type*) inner_t1,
					(record_type*) inner_t2);

		}else{
			cerr << "%%%%%%%%% compute_lub  " << "type1 :" << t1->to_string()
								<< "   ;type2 :" << t2->to_string() << endl;
			cerr << "shouldn't be here ! " << endl;
			cerr << "inner_t1 " << inner_t1->to_string() << endl;
			cerr << "inner t2 " << inner_t2->to_string() << endl;
			if(indicator == 1)
				cerr << "Catch here ! " << endl;
			indicator = -3;
			return NULL;
		}

		return il::pointer_type::make(inner_lub, inner_lub->name);

	}






}

void build_type_hierarchy_for_lib_record_type(record_type* rt){

	record_type* base = get_base(rt);

	//lib record type that don't have type hierarchy ( point to object type);
	if (base->name != JAVA_LANG_OBJECT
			&& base->name == rt->name) {
		cerr << "SHOULD BUILT LIB'S RECORD TYPE'S HIERARCHY IN TYPEMAPPING ON FRONT END!! "<< endl;
		assert(false);

		type* obj_rt = type::get_type_from_name(JAVA_LANG_OBJECT);

		if(!obj_rt->is_record_type()){
			cout << obj_rt->to_string() ;
			obj_rt->print_kind();
			assert(false);
		}

		rt->add_base((record_type*)obj_rt, 0);
		if (!alreadyInDerivatives((record_type*)obj_rt, rt))
			((record_type*)obj_rt)->add_derivative(rt);
	}

}

record_type* lub_record_record(record_type* rt1, record_type* rt2){

	if(rt1 == rt2)
		return rt1;

	if(rt1->get_typedef_name() == JAVA_LANG_OBJECT)
		return rt1;

	if(rt2->get_typedef_name() == JAVA_LANG_OBJECT)
		return rt2;


	//otherwise, find their lub;
	record_type* rt1_temp = rt1;

	record_type* rt2_temp = rt2;

	while (true) {

		//return condition;
		if(rt1_temp == rt2_temp)
			return rt1_temp;

		//rt1_temp fixed;
		//traverse rt2_temp's type hierarchy(from bottom to up)
		while (rt2_temp->name != JAVA_LANG_OBJECT) {

			if (DEBUG) {
				cerr << "rt1_temp->name  " << rt1_temp->name << endl;
				cerr << "rt1_temp get base name " << get_base(rt1_temp)->name
						<< endl << endl;
				cerr << "rt2_temp->name  " << rt2_temp->name << endl;
				cerr << "rt2_temp get base name " << get_base(rt2_temp)->name
						<< endl << endl;
			}

			build_type_hierarchy_for_lib_record_type(rt2_temp);
			//one level up
			rt2_temp = get_base(rt2_temp);

			//return condition;
			if (rt1_temp == rt2_temp) {
				return rt1_temp;
			}
		}


		//rt1_temp one level up, and set rt2_temp to the start type again;
		rt1_temp = get_base(rt1_temp);
		rt2_temp = rt2;
	}

}

#if 0
/***
 * if int* or string* is one of the type, just return
 */
type* compute_lub(type* type1, type* type2) {
	//if (DEBUG) {
	if(1){
		cout << "compute_lub  " << "type1 :" << type1->to_string()
				<< "   ;type2 :" << type2->to_string() << endl;
	}

	//type* invalidType = il::invalid_type::make();
	//assert(type1 == invalidType && type2 == invalidType);

	if(type1 == type::get_type_from_name(JAVA_OBJECT_POINTER))
		return type1;

	if(type2 == type::get_type_from_name(JAVA_OBJECT_POINTER))
		return type2;


	if (type1 == type2) {
		return type1;
	}

	if (type1->is_invalid_type()) {
		return type2;
	}
	if (type2->is_invalid_type()) {
		return type1;
	}



	/***************************************************************************
	 * when char and int has same variable, should refer to char type,
	 * when pointer and int has same variable, should refer to pointer type
	 **************************************************************************/
	if (type1 == il::get_integer_type()) {
		return type2;
	}
	if (type2 == il::get_integer_type()) {
		return type1;
	}
	if (type1 == il::get_char_type()) {
		return type2;
	}
	if (type2 == il::get_char_type()) {
		return type1;
	}

	//** pointer

	if((type1->is_pointer_type())&&(type2->is_pointer_type())){

		pointer_type* pt1 = static_cast<pointer_type*>(type1);
		pointer_type* pt2 = static_cast<pointer_type*>(type2);

		if((pt1->get_inner_type()->is_pointer_type())
				&&(pt2->get_inner_type()->is_pointer_type())){
			type* inner1_pt = pt1->get_inner_type();
			type* inner2_pt = pt2->get_inner_type();

			type* t = compute_lub(inner1_pt, inner2_pt);

			assert(t->is_pointer_type());

			return il::pointer_type::make(t,t->to_string());
		}
	}




	record_type* rt1 = NULL;
	record_type* rt2 = NULL;

	//get record_type of type1.
	if (type1->is_pointer_type()) {
		pointer_type* pt1 = static_cast<pointer_type*>(type1);
		type* innert1 = pt1->get_inner_type();
		if (DEBUG) {
			//cout << "innert1  : " << innert1->to_string() << endl;
		}
		if (innert1->is_record_type()) {
			//cout << "inner 1 is record type " << endl;
			rt1 = static_cast<record_type*>(innert1);
			//cout << "rt1 == NULL ?" << (rt1 ==NULL) << endl;
		} else {

			//cout << "inner 1 is not record type " << endl;

			if (innert1 == il::get_integer_type()) {
				innert1 = type::get_type_from_name(JAVA_LANG_INTEGER);

				if (!innert1->is_record_type()) {
					innert1->print_kind();
					assert(false);
				}

				rt1 = static_cast<record_type*>(innert1);
			}
		}
	}

	//get record_type of type2.
	if (type2->is_pointer_type()) {
		pointer_type* pt2 = static_cast<pointer_type*>(type2);
		type* innert2 = pt2->get_inner_type();
		if (DEBUG) {
			//cout << "innert2  : " << innert2->to_string() << endl;
		}
		if (innert2->is_record_type()) {
			//cout << "inner 2 is record type " << endl;
			rt2 = static_cast<record_type*>(innert2);
		} else {
			//cout << "inner 2 is not record type " << endl;
			if (innert2 == il::get_integer_type()) {
				innert2 = type::get_type_from_name(JAVA_LANG_INTEGER);


			assert(innert2->is_record_type());
			rt2 = static_cast<record_type*>(innert2);
			}
		}
	}

	//find LUB of rt1 and rt2.
	if (rt1 != NULL && rt2 != NULL) {
		//cout << "find LUB rt1 and rt2 . " << endl;
		record_type* rt1_temp = rt1;
		record_type* rt2_temp = rt2;
//		cerr << "rt1_temp is " << rt1_temp->to_string() << endl;
//		cerr << "rt2_temp is " << rt1_temp->to_string() << endl;
		while (true) {
			if (rt1_temp == rt2_temp) {
				return il::pointer_type::make(rt1_temp, rt1_temp->name);
				//return rt1_temp;
			}
			//while(rt2_temp->name != "Ljava/lang/Object;")
			while (rt2_temp->name != JAVA_LANG_OBJECT) {

//				cerr << "rt1_temp->name  " << rt1_temp->name << endl;
//				cerr << "rt1_temp get base name " << get_base(rt1_temp)->name <<
//						endl << endl;
//				cerr << "rt2_temp->name  " << rt2_temp->name << endl;
//				cerr << "rt2_temp get base name " << get_base(rt2_temp)->name
//						<< endl << endl;
				record_type* rt2_temp_base = get_base(rt2_temp);
//				if(rt2_temp_base->name != "Ljava/lang/Object;" &&
//						rt2_temp_base->name == rt2_temp->name)
				if (rt2_temp_base->name != JAVA_LANG_OBJECT
						&& rt2_temp_base->name == rt2_temp->name) {
					//type* obj_type = type::get_type_from_name("Ljava/lang/Object;");
					type* obj_type = type::get_type_from_name(JAVA_LANG_OBJECT);
					record_type* obj_recordtype = NULL;
					if (obj_type->is_record_type()) {
						obj_recordtype = static_cast<record_type*>(obj_type);
					} else {
						assert(false);
					}
					rt2_temp->add_base(obj_recordtype, 0);
					if (!alreadyInDerivatives(obj_recordtype, rt2_temp))
						obj_recordtype->add_derivative(rt2_temp);
				}

				rt2_temp = get_base(rt2_temp);
				if (rt1_temp == rt2_temp) {
					return il::pointer_type::make(rt1_temp, rt1_temp->name);
					//return rt1_temp;
				}
			}
			rt1_temp = get_base(rt1_temp);
			rt2_temp = rt2;
		}
	}
	//assert(false); //no object and int compute LUB
	return il::invalid_type::make();
	//assert(false);
	//return NULL;
}
#endif

bool alreadyInDerivatives(il::record_type* base, il::record_type* deriv) {
	vector<il::record_type*> deriv_types = base->get_derivatives();
	for (unsigned int i = 0; i < deriv_types.size(); i++) {
		if (deriv_types.at(i) == deriv)
			return true;
	}
	return false;
}

record_type* get_base(record_type* rt_cur) {
	map<int, record_type*> map_rt_cur_bases = rt_cur->get_bases();
	return (map_rt_cur_bases.begin())->second;
}

Variable* get_defined_variable(Instruction* inst) {
	Variable* var = NULL;
	switch (inst->get_instruction_id()) {
	case ASSIGNMENT: {
		Assignment* inst_assignment = static_cast<sail::Assignment*>(inst);
		var = inst_assignment->get_lhs();
		break;
	}
	case INSTANCEOF: { //added by haiyan;
		InstanceOf* io = static_cast<InstanceOf*>(inst);
		var = io->get_lhs();
		break;
	}

	case FUNCTION_CALL: {
		FunctionCall* functionCall = static_cast<FunctionCall*>(inst);
		//		if(functionCall->is_allocator()){
		//			var = functionCall->get_lhs();
		//		}
		var = functionCall->get_lhs();
		break;
	}
	case ARRAY_REF_READ: {
		//no such test cases
		assert(false);
		break;
	}
	case ARRAY_REF_WRITE: {
		assert(false);
		break;
	}
	case BINOP: {
		Binop* binop = static_cast<Binop*>(inst);
		var = binop->get_lhs();
		break;
	}
	case CAST: {
		Cast* cast = static_cast<Cast*>(inst);
		var = cast->get_lhs();
		break;
	}
	case FIELD_REF_READ: {

		assert(false);
		break;
	}
	case LOAD: {
		Load* load = static_cast<Load*>(inst);
		var = load->get_lhs();
		break;
	}
	case FIELD_REF_WRITE: {
		assert(false);
		break;
	}
	case STORE: {
		break;
	}
	case JUMP: {
		assert(false);
		break;
	}
	case SAIL_LABEL: {
		break;
	}
	case FUNCTION_POINTER_CALL: {
		assert(false);
		break;
	}
	case UNOP: {
		Unop* unop = static_cast<Unop*>(inst);
		var = unop->get_lhs();
		break;
	}
	case LOOP_INVOCATION: {
	//	assert(false);
		break;
	}
	case DROP_TEMPORARY: {
		assert(false);
		break;
	}
	case STATIC_ASSERT: {
		//assert(false);
		break;
	}
	case ASSUME: {
		assert(false);
		break;
	}
	case ASSUME_SIZE: {
		assert(false);
		break;
	}

	case EXCEPTION_RETURN: {
//		assert(false);
		break;
	}
	case ADDRESS_LABEL: {
		assert(false);
		break;
	}
	case ADDRESS_VAR: {
		assert(false);
		break;
	}
	case ADDRESS_STRING: {
		AddressString* addressstring = static_cast<AddressString*>(inst);
		var = addressstring->get_lhs();
		break;
	}
	case SAIL_ASSEMBLY: {
		assert(false);
		break;
	}
	case BRANCH: {
		assert(false);
		break;
	}
	default: {
		assert(false);
		break;
	}
	}
	return var;
}

void get_symbols_inst(Instruction* inst, vector<Symbol*>& vect_syms) {
	switch (inst->get_instruction_id()) {
	case ASSIGNMENT: {
		Assignment* inst_assignment = static_cast<sail::Assignment*>(inst);
		vect_syms.push_back(inst_assignment->get_lhs());
		vect_syms.push_back(inst_assignment->get_rhs());
		break;
	}
	case FUNCTION_CALL: {
		FunctionCall* functionCall = static_cast<FunctionCall*>(inst);
		if (functionCall->get_lhs() != NULL)
			vect_syms.push_back(functionCall->get_lhs());
		vector<Symbol*>* arguments = functionCall->get_arguments();
		vector<Symbol*>::iterator iter;
		for (iter = arguments->begin(); iter != arguments->end(); iter++) {
			vect_syms.push_back(*iter);
		}
		break;
	}
	case ARRAY_REF_READ: {
		//no such test cases
		assert(false);
		break;
	}
	case ARRAY_REF_WRITE: {
		assert(false);
		break;
	}
	case BINOP: {
		Binop* binop = static_cast<Binop*>(inst);
		vect_syms.push_back(binop->get_lhs());
		vect_syms.push_back(binop->get_first_operand());
		vect_syms.push_back(binop->get_second_operand());
		break;
	}
	case CAST: {
		Cast* cast = static_cast<Cast*>(inst);
		vect_syms.push_back(cast->get_lhs());
		vect_syms.push_back(cast->get_rhs());
		break;
	}
	case FIELD_REF_READ: {
		assert(false);
		break;
	}
	case LOAD: {
		Load* load = static_cast<Load*>(inst);
		vect_syms.push_back(load->get_lhs());
		vect_syms.push_back(load->get_rhs());
		break;
	}
	case FIELD_REF_WRITE: {
		assert(false);
		break;
	}
	case STORE: {
		Store* store = static_cast<Store*>(inst);
		vect_syms.push_back(store->get_lhs());
		vect_syms.push_back(store->get_rhs());
		break;
	}
	case JUMP: {
		assert(false);
		break;
	}
	case SAIL_LABEL: {
		assert(false);
		break;
	}
	case FUNCTION_POINTER_CALL: {
		assert(false);
		break;
	}
	case UNOP: {
		Unop* unop = static_cast<Unop*>(inst);
		vect_syms.push_back(unop->get_lhs());
		vect_syms.push_back(unop->get_operand());
		break;
	}
	case LOOP_INVOCATION: {
		//<LoopInvocation>5 just skip
		//assert(false);
		break;
	}
	case DROP_TEMPORARY: {
		assert(false);
		break;
	}
	case STATIC_ASSERT: {
		StaticAssert* st = static_cast<StaticAssert*>(inst);
		vect_syms.push_back(st->get_predicate());
		break;
	}
	case ASSUME: {
		assert(false);
		break;
	}
	case ASSUME_SIZE: {
		assert(false);
		break;
	}
	case INSTANCEOF: {
		InstanceOf* instanceOf = static_cast<InstanceOf*>(inst);
		vect_syms.push_back(instanceOf->get_lhs());
		vect_syms.push_back(instanceOf->get_rhs());
		break;
	}
	case EXCEPTION_RETURN: {
		assert(false);
		break;
	}
	case ADDRESS_LABEL: {
		assert(false);
		break;
	}
	case ADDRESS_VAR: {
		assert(false);
		break;
	}
	case ADDRESS_STRING: {
		AddressString* addressstring = static_cast<AddressString*>(inst);
		vect_syms.push_back(addressstring->get_lhs());
		break;
	}
	case SAIL_ASSEMBLY: {
		assert(false);
		break;
	}
	case BRANCH: {
		assert(false);
		break;
	}
	default: {
		assert(false);
		break;
	}
	}
}

double to_time(int ticks) {
	return ((double) (ticks)) / ((double) CLOCKS_PER_SEC);
}

}
