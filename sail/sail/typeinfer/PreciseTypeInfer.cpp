/*
 * PreciseTypeInfer.cpp
 *
 *  Created on: Sep 27, 2012
 *      Author: boyang
 */

#include "PreciseTypeInfer.h"
#include "TypeInference.h"


#include "sail.h"
#include "il.h"

using namespace std;
using namespace sail;
using namespace il;


#define DEBUG false


namespace sail {

PreciseTypeInfer::PreciseTypeInfer(BasicBlock* bb_entry,
		BasicBlock* bb_exit,
		BasicBlock* exception_b,
		LivenessInfer* lni,
		Function* f){
	this->entry_block = bb_entry;
	this->exit_block = bb_exit;
	this->exception_block = exception_b;
	this->lni = lni;

	order_insts = f->get_body();
	assert(order_insts != NULL);

	set<Block*, sail::CompareBlock> worklist;

	//initialize the map
	worklist.insert(bb_entry);
	while(worklist.size() > 0)
	{
		Block* b_cur = *(worklist.begin());
		worklist.erase(b_cur);
		if(b_cur->is_basicblock())
		{
			BasicBlock* bb_cur = static_cast<BasicBlock*>(b_cur);
			vector<Instruction*> vector_inst =  bb_cur->get_statements();
			vector<Instruction*>::iterator it  = vector_inst.begin();
			for (; it != vector_inst.end(); ++it) {
				map<string , PreciseTypeVarStat*>  newSet;
				//the default is empty set;
				this->map_precise_type.insert(pair<Instruction*,
						map<string , PreciseTypeVarStat*>  >(*it, newSet));
			}

		}
		set<CfgEdge*>& set_Edges = b_cur->get_successors();
		set<CfgEdge*>::iterator iter_Edges = set_Edges.begin();
		for(; iter_Edges != set_Edges.end(); iter_Edges++)
		{
			if(!(*iter_Edges)->is_backedge()) //added by haiyan;
				worklist.insert((*iter_Edges)->get_target());
		}
	}
	order_insts = NULL;
}




PreciseTypeInfer::~PreciseTypeInfer() {

}


void PreciseTypeInfer::precise_type_analysis(const bool& isFirst){

	set<Block*, sail::CompareBlock> worklist;
	bool has_changing = true;
	this->isFirst = isFirst;

	while(has_changing)
	{

		has_changing = false;
		worklist.insert(entry_block);
		while(worklist.size() > 0)
		{
			Block* b_cur = *(worklist.begin());
			worklist.erase(b_cur);
			if(b_cur->is_basicblock())
			{

				map<string, PreciseTypeVarStat*> preInstsOut;
				compute_block_preoutput(b_cur, preInstsOut);

				bool has_changing_cur =
						block_precise_type_analysis(b_cur, preInstsOut);

				if(has_changing == false && has_changing_cur == true)
				{
					has_changing = true;
				}
			}

			else
			{
				cout << "b_cur " << b_cur->to_string() << endl;
				cout << "encounter non basic-block " << endl;
				assert(false);
			}

			set<CfgEdge*>& set_Edges = b_cur->get_successors();
			set<CfgEdge*>::iterator iter_Edges = set_Edges.begin();
			for(; iter_Edges != set_Edges.end(); iter_Edges++)
			{
				worklist.insert((*iter_Edges)->get_target());
			}
		}
	}
}

/****************************************
 * preInstOut has been computed!
 ************************************/


bool PreciseTypeInfer::block_precise_type_analysis(Block* b_cur,
		map<string, PreciseTypeVarStat*>& preInstsOut){
	bool is_first_inst = true;
	bool has_changing = false;
	BasicBlock* bb_cur = static_cast<BasicBlock*>(b_cur);

	vector<Instruction*> vector_inst =  bb_cur->get_statements();
	vector<Instruction*>::iterator iter_vector_inst;
	int i = 0;
	iter_vector_inst = vector_inst.begin();
	for (; iter_vector_inst != vector_inst.end(); iter_vector_inst++) {

		if(is_first_inst == false){
			Instruction* cur_inst = *iter_vector_inst;
			Instruction* last_inst = *(iter_vector_inst-1);
			//use it to compute the last instruction's output!
			preInstsOut.clear();
			if(0){
				cout <<endl<< "	|| Block id " << b_cur->get_block_id()
				 << " [ " << ++i  << "]"<< endl;
				cout << "	|| Cur INST " << cur_inst->to_string() << endl;
				cout << "	|| Last INS == " << last_inst->to_string() << endl;
			}

			get_instOUT(last_inst, preInstsOut, true);
		}


		is_first_inst = false;

		//if it has been updated?
		if(update_mapping(*iter_vector_inst, preInstsOut, false))
		{
			has_changing = true;
		}

	}
	return has_changing;
}


bool PreciseTypeInfer::update_mapping(Instruction* inst,
		map<string, PreciseTypeVarStat*>& new_stat, bool isException)
{
	map<Instruction*,map<string ,PreciseTypeVarStat*> >::iterator  map_iter;
	map_iter = map_precise_type.find(inst);
	assert(map_iter != map_precise_type.end());


	map<string, PreciseTypeVarStat*> old_stat =  map_iter->second;

	//update
	map_iter->second = new_stat;




	map<string, PreciseTypeVarStat*>::const_iterator itold;
	map<string, PreciseTypeVarStat*>::const_iterator itnew;
	for(itold = old_stat.begin(); itold != old_stat.end();itold++)
	{
		//		for(itnew = new_stat.begin(); itnew != new_stat.end(); itnew++)
		//		{
		//			if((*itnew)->get_var()->get_var_name() ==
		//					(*itold)->get_var()->get_var_name())
		//			{
		itnew = new_stat.find(itold->first);
		if(itnew != new_stat.end() && !((*(itnew->second)) == (*(itold->second))))
		{
			return true;
		}
	}
	for(itnew = new_stat.begin(); itnew != new_stat.end(); itnew++)
	{
		//		for(itold = old_stat.begin(); itold != old_stat.end();itold++)
		//		{
		//			//if((*iter_new_stat)->get_var() == (*iter_old_stat)->get_var())
		//			if((*itnew)->get_var()->get_var_name() ==
		//					(*itold)->get_var()->get_var_name())
		//			{
		itold = old_stat.find(itnew->first);
		if(itold != old_stat.end() &&!((*(itnew->second)) == (*(itold->second))))
		{
			return true;
		}
	}
	return false;
}


void PreciseTypeInfer::filter_pre_vars_with_liveness(
		map<string, PreciseTypeVarStat*>& prevarStatOut,
		const set<string>& liveness){
	map<string, PreciseTypeVarStat*>::iterator it = prevarStatOut.begin();
	//set<string>::iterator it = liveness.begin();

	for(; it != prevarStatOut.end(); ){
		if(liveness.count(it->first) == 0 )
			prevarStatOut.erase(it++);
		else
			it++;
	}

}

/********
 * need to fill preInstOut;
 */
void PreciseTypeInfer::compute_block_preoutput(Block* b_cur,
		map<string, PreciseTypeVarStat*>& preInstsOut)
{
	//cout << "Compute block preoutput for block  " << b_cur->to_string() << endl;
	set <map<string, PreciseTypeVarStat*> > set_preInstsOut;
	/*
	 * if b_cur is not entry_block, we collect all pre blocks
	 */
	if((b_cur != this->entry_block)&&(b_cur != this->exception_block) && (b_cur != this->exit_block))
	//if((b_cur != this->entry_block))
	{


		//first get the liveness for current instruction;
		//check the first inst for b_cur;
		//added by haiyan
		set<string> liveness;
		if(b_cur->is_basicblock()){

			if (((BasicBlock*)b_cur)->get_statements().size() != 0){
				Instruction* first_inst = ((BasicBlock*)b_cur)->get_statements().at(0);
				assert(first_inst != NULL);
				//cout << "instruction is " << first_inst->to_string() << endl;
				lni->getLivenessByInst(first_inst, liveness);
			}else{
				lni->compute_block_afteroutput(b_cur, liveness);
			}
		}
		//ended by haiyan

		set <CfgEdge*> set_Edges = b_cur->get_predecessors();

		set <CfgEdge*>::iterator iter_Edges = set_Edges.begin();
		for (; iter_Edges!=set_Edges.end(); iter_Edges++)
		{
			Block * b_pre = (*iter_Edges)->get_source();
			if(b_pre->is_basicblock())
			{
				BasicBlock* bb_pre = static_cast<BasicBlock*>(b_pre);
//				cout << "<<< pred_block is " << bb_pre->get_block_id() << endl
//						 << "  for cur_block  " << b_cur->get_block_id() << endl;
				vector<Instruction*> bb_pre_statements = bb_pre->get_statements();
				if(bb_pre_statements.size() == 0){
					map<string, PreciseTypeVarStat*> cur_pre_VarT;
					compute_block_preoutput(bb_pre, cur_pre_VarT);

					//use livenss to filter;
					//added by haiyan
					filter_pre_vars_with_liveness(cur_pre_VarT, liveness);
					//ended by haiyan
					set_preInstsOut.insert(cur_pre_VarT);
					continue;
				}

				vector<Instruction*>::reverse_iterator bb_pre_statements_riter;
				bb_pre_statements_riter = bb_pre_statements.rbegin();
				map<string, PreciseTypeVarStat*> cur_pre_VarT;
				//cout << "for last instruction of preds-block " << endl;
				get_instOUT((*bb_pre_statements_riter), cur_pre_VarT, true);

				//added by haiyan
				filter_pre_vars_with_liveness(cur_pre_VarT, liveness);
				//ended by haiyan
				set_preInstsOut.insert(cur_pre_VarT);

//				cout << "#########Entry points of block  after liveness filter "
//						"" << b_cur->to_string() << endl;


			}
		}
	}






	merge_type_info(set_preInstsOut, preInstsOut);
}




void PreciseTypeInfer::merge_type_info(
		set <map <string, PreciseTypeVarStat*> >& input_set, // source
		map <string, PreciseTypeVarStat*>& output_set){      //dest
	set <map <string, PreciseTypeVarStat*> >::iterator it = input_set.begin();
	for(; it != input_set.end(); it++)
	{
		map <string, PreciseTypeVarStat*> cur_set = (*it);
		merge_set_TypeVarStat(cur_set, output_set);
	}

	if (0) {
		for (auto it = output_set.begin(); it != output_set.end(); it++) {
			PreciseTypeVarStat* var = it->second;
			cout << "	==> have type infor  for " << var->to_string() << endl;
		}

		cout << "#############End of the entry block fact printing ! " << endl;
	}
}



void PreciseTypeInfer::merge_set_TypeVarStat(
		map <string ,PreciseTypeVarStat*>& c_map,
		map <string ,PreciseTypeVarStat*>& o_map){
	map <string ,PreciseTypeVarStat*>::iterator c_it = c_map.begin();
	for(; c_it != c_map.end(); c_it++)
	{
		string name = c_it->first;
		//cout << "merging var  " << name << endl;
		map <string ,PreciseTypeVarStat*>::iterator o_it = o_map.find(name);
		if(o_it != o_map.end())
		{
			o_it->second->merge(c_it->second);
		}
		else
		{
			PreciseTypeVarStat* newst =  new PreciseTypeVarStat(*(c_it->second));
			o_map.insert(pair<string, PreciseTypeVarStat* >(
					name, newst));
		}

	}
}




void PreciseTypeInfer::get_instOUT(Instruction* inst_cur,
		map<string, PreciseTypeVarStat*>& set_inst_out,
		bool entry_point)
{
	//get the occupied space;
	map<Instruction*, map<string, PreciseTypeVarStat*> >::iterator map_iter;
	map_iter = map_precise_type.find(inst_cur);
	assert(map_iter != map_precise_type.end());

	Variable* defined_var = sail::get_defined_variable(inst_cur);
	bool is_redefined = false;
	if (defined_var != NULL) {
		is_redefined = true;

	}

	map<string, PreciseTypeVarStat*>::iterator it = map_iter->second.begin();
	for (; it != map_iter->second.end(); it++) {
		//Variable* v_cur = (*it)->get_var();
		string name = it->first;
		// if we can not find defined_var in the set
		if (is_redefined == false || name != defined_var->get_var_name()) {
			set_inst_out.insert(
					pair<string, PreciseTypeVarStat*>(name, it->second));
		}
	}

	//if(is_redefined == true)
	if (is_redefined == true) {
		// get type.
		type* new_type = NULL;
		//inference type only if it is not declared!
		if (defined_var->get_type() == type::get_type_from_name("[invalid]")
				|| defined_var->get_type()
						== type::get_type_from_name("Ljava/lang/Exception;")) {

			if (this->isFirst) { // first time rhs propagate to lhs;
				//based on type in the precisetypeinfer map
				new_type = get_cur_type(inst_cur);
			}

			else //second time should have lhs type, no matter how it comes from;
			{
				//already assigned in first round;
				new_type = defined_var->get_type();

				if (new_type->is_invalid_type()) {
					cerr << "FIXME !! " << endl;
					cerr << "variable is  " << defined_var->to_string() << endl;
					cerr << "instruction is " << inst_cur->to_string() << endl;
					assert(false);
				}
			}
		} else {

			new_type = defined_var->get_type();
		}

		PreciseTypeVarStat* new_type_stat =
				new PreciseTypeVarStat(defined_var, new_type);
		set_inst_out.insert(
				pair<string, PreciseTypeVarStat*>(defined_var->get_var_name(),
						new_type_stat));

	}

	//used to test
	if (entry_point) {

		if (0) {
			cout << "++++++++++++ after processing, the set_inst_out is :::::: "
					<< endl;
			for (map<string, PreciseTypeVarStat*>::iterator it =
					set_inst_out.begin(); it != set_inst_out.end(); it++) {
				cout << "PreciseTypeVarStat " << it->second->to_string()
						<< endl;
			}
			cout << " = = = = = end testing one instruction!! " << endl << endl;
		}

	}
}




/*******
 * if rhs type already exists, we directly use it, otherwise
 * get rhs type from the preciseTypeInfer;
 */
type* PreciseTypeInfer::get_cur_type(Instruction* inst_cur){
	type* cur_type = NULL;
	switch (inst_cur->get_instruction_id()) {
	case ASSIGNMENT: {
		Assignment* inst_assignment = static_cast<sail::Assignment*> (inst_cur);
		Symbol* sym_rhs = inst_assignment->get_rhs();
		if (sym_rhs->is_variable()) {
			Variable* v = static_cast<Variable *> (sym_rhs);
			cur_type = find_var_type_in_Inst(v, inst_cur);
		} else {
			Constant* c = static_cast<Constant *> (sym_rhs);
			cur_type = c->get_type();
		}
		break;
	}
	case FUNCTION_CALL: {
		FunctionCall* functionCall = static_cast<FunctionCall*> (inst_cur);
		cur_type = get_func_type(functionCall);
		break;
	}
	case CAST: {
		Cast* cast = static_cast<Cast*> (inst_cur);
		cur_type = cast->get_cast_type();
		break;
	}
	case BINOP: {
		Binop* binop = static_cast<Binop*> (inst_cur);
		cur_type = get_binop_type(binop);
		break;
	}
	case UNOP: {
		Unop* unop = static_cast<Unop*> (inst_cur);
		cur_type = get_unop_type(unop);
		break;
	}
	case ARRAY_REF_READ: {
		assert(false);
		break;
	}
	case FIELD_REF_READ: {
		assert(false);
		break;
	}
	case LOAD: {

		Load* load = static_cast<Load*> (inst_cur);
		Symbol* sym_rhs = load->get_rhs();
		//cout << "field name : " << load->get_field_name() << endl;
		if(sym_rhs->is_variable())
		{
			Variable* v = static_cast<Variable *> (sym_rhs);

			//haiyan added
			type* rhs_type = find_var_type_in_Inst(v, inst_cur);


			if(rhs_type->is_pointer_type())
			{
				type* inner_type =  rhs_type->get_inner_type();
				if(load->get_field_name() != "")
				{
					if(inner_type->is_record_type())
					{
						//cout << "f_name() : " << load->get_field_name() << endl;
						//cout << "in_type : " << inner_type->to_string() << endl;
						record_type* rt = static_cast<record_type*> (inner_type);
						record_info * ri = rt->get_field_from_name(load->get_field_name());

						if (ri == NULL) {
							cerr << "load inst : " << inst_cur->to_string() << endl;
							cerr << "load inst pp " << inst_cur->to_string(true) << endl;
							cerr << "f_name() : " << load->get_field_name()
									<< endl;
							cerr << "field in type ? : " << rt->to_string()
									<< endl;
							//cerr << "record_info->fname: " << ri->fname << endl;
						}
						assert(ri != NULL);
						//cout << "record_info->fname: " << ri->fname << endl;
						cur_type = ri->t;
						assert(cur_type != NULL);
					} else
					{
						assert(false);
					}
				}else{
					cur_type = inner_type;
				}
			}
			else
			{
				assert(false);
			}
		}else{
			assert(false);
		}
		break;
	}
	case ARRAY_REF_WRITE: {
		assert(false);
		break;
	}
	case FIELD_REF_WRITE:{
		assert(false);
		break;
	}
	case STORE:{
		assert(false);
		break;
	}
	case JUMP:{
		assert(false);
		break;
	}
	case SAIL_LABEL:{
		assert(false);
		break;
	}
	case FUNCTION_POINTER_CALL:{
		assert(false);
		break;
	}
	case LOOP_INVOCATION:{
		//cout << "LOOP_INVOCATION inst : " << inst_cur->to_string() << endl;
		assert(false);
		break;
	}
	case DROP_TEMPORARY:{
		assert(false);
		break;
	}
	case STATIC_ASSERT:{
		assert(false);
		break;
	}
	case ASSUME:{
		assert(false);
		break;
	}
	case ASSUME_SIZE:{
		assert(false);
		break;
	}
	case INSTANCEOF:{
		cur_type = il::get_integer_type();
		break;
	}
	case EXCEPTION_RETURN:{
		assert(false);
		break;
	}
	case ADDRESS_LABEL:{
		assert(false);
		break;
	}
	case ADDRESS_VAR:{
		assert(false);
		break;
	}
	case ADDRESS_STRING:{
		//cout << "addressString inst : " << inst_cur->to_string() << endl;
		AddressString* astr = static_cast<AddressString*> (inst_cur);
		//il::string_const_exp* str_exp =  astr->get_rhs();

		//"moniter enter" represents "monitor-enter" instruction in dalvik
		// bytecode. Haiyan hard code this part.
//		if(str_exp->get_string() == "monitor-enter" ||
//				str_exp->get_string() == "monitor-exit" )
//		{
//			cout << "address string monitor-enter !!" << endl;
//			Variable* v_lhs = astr->get_lhs();
//			cur_type = find_var_type_in_Inst(v_lhs, inst_cur);
//			break;
//		}
		if(!astr->get_lhs()->get_type()->is_invalid_type())
			cur_type = astr->get_lhs()->get_type();
		else
			cur_type = type::get_type_from_name("Ljava/lang/String;*");
		break;
	}
	case SAIL_ASSEMBLY:{
		assert(false);
		break;
	}
	case BRANCH:{
		assert(false);
		break;
	}
	default: {
		assert(false);
		break;
	}
	}
	if(cur_type == NULL)
	{
		cout << "instruction : " << inst_cur->to_string() << endl;
		assert(false);
	}
	return cur_type;
}




type* PreciseTypeInfer::find_var_type_in_Inst(Variable* var, Instruction* inst){


	map<Instruction*,map<string, PreciseTypeVarStat*> >::iterator  map_iter;
	map_iter = map_precise_type.find(inst);
	assert(map_iter != map_precise_type.end());

	map<string, PreciseTypeVarStat*>::iterator it =
			map_iter->second.find(var->get_var_name());

	if(it != map_iter->second.end())
	{
		return it->second->get_type();
	}

	//Otherwise, it should be predefined
	type* var_type = var->get_type();
	//type* invalid_type = type::get_type_from_name("[invalid]");
	//if(var_type != invalid_type){
	if(!var_type->is_invalid_type()){
		return var_type;
	}else
	{
		//Should find the variable's type, which has been defined previously
		cout << "inst " << inst->to_string() << endl;
		cout << "var  : " << var->to_string() << endl;
		cout << "var->get_var_name() " << var->get_var_name() << endl;
		cout << "inst lhs " << inst->get_lhs() << endl;
		cout << "inst lhs get_var_name " << inst->get_lhs()->get_var_name() << endl;
		if(var->get_var_name() == inst->get_lhs()->get_var_name()){
			return il::invalid_type::make();
		}

		cout << "var is_args : " << var->is_argument() << endl;
		cout << "inst : " << inst->to_string(false) << endl;
		cout << "inst pp " << inst->to_string(true) << endl;
		cout << "var type : " << var->get_type()->to_string() << endl;
		assert(false);
	}
}





void PreciseTypeInfer::get_typeset(Instruction* inst,
		set<Instruction*>& set_reaching, set<type*>& set_type){
	Variable* var = sail::get_defined_variable(inst);
//	Variable* var = inst->get_lhs();
	assert(var != NULL);

	// if it has no reaching and liveness
	//rhs type of inst
	if(set_reaching.size() == 0)
	{
		set_type.insert(get_cur_type(inst));
	}

	set<Instruction*>::iterator it_reaching = set_reaching.begin();
	for(; it_reaching != set_reaching.end(); it_reaching++)
	{
		map<Instruction*, map<string, PreciseTypeVarStat*> >::iterator  it =
				map_precise_type.find(*it_reaching);
		if(it == map_precise_type.end())
			continue; // assert false??
		map<string, PreciseTypeVarStat*>::iterator sec_iter =
				it->second.find(var->get_var_name());
		if(sec_iter != it->second.end())
		{
			set_type.insert(sec_iter->second->get_type());
		}

	}
}


void PreciseTypeInfer::get_reaching_points_inf(Variable* lhs,
		set<Instruction*>& set_reaching,
		map<Instruction*, type*>& reaching_points_info)
{


	//Variable* var = sail::get_defined_variable(inst);
	assert(lhs != NULL);

	set<Instruction*>::iterator it_reaching = set_reaching.begin();

	for(; it_reaching != set_reaching.end(); it_reaching++)
	{

		Instruction* reaching_inst = *it_reaching;

		map<Instruction*, map<string, PreciseTypeVarStat*> >::iterator  it =
				map_precise_type.find(reaching_inst);

		if(0)
			cout << "   == > reaching instruction " << reaching_inst->to_string() << endl;

		//didn't find
		if(it == map_precise_type.end())
			continue; // assert false??

		vector<Symbol*> syms;
		reaching_inst->get_in_use_symbols(syms);

		bool contain_define_var = false;
		for(int i = 0; i < (int) syms.size(); i++){
			if(syms.at(i)->is_variable()){
				Variable* var = static_cast<Variable*>(syms.at(i));
				if(var->get_var_name() == lhs->get_var_name())
					contain_define_var = true;
			}
		}

		if(contain_define_var){
			map<string, PreciseTypeVarStat*>::iterator sec_iter =
					it->second.find(lhs->get_var_name());

			if(sec_iter != it->second.end())
			{
				reaching_points_info[reaching_inst] = sec_iter->second->get_type();
				//set_type.insert(sec_iter->second->get_type());

			}

		}
		//	for(; sec_iter != it->second.end(); sec_iter++)
		//	{
		//		if((*sec_iter)->get_var()->get_var_name() == var->get_var_name())
		//		{
		//			set_type.insert((*sec_iter)->get_type());
		//		}
		//	}
	}
}



//pick the more precise one;
//(precise_type in the type already in inst's correspoinding variable)
type* PreciseTypeInfer::get_better_type(Variable* lhs,
		type* precise_type, //type in reaching inst;
		Instruction* inst, //reaching inst
		Function* f,
		bool& need_to_infer_null_pointer) {
	//if declared type better than precise_type, then return declared type, otherwise
	//return precise_type it self;



	if(precise_type->is_char_type()){
		return precise_type;
	}


	if(inst->get_instruction_id() == LOOP_INVOCATION)
		return precise_type;



	bool special_inst = false;

	if( inst->is_return_inst()
			|| (inst->get_instruction_id() == FUNCTION_CALL)
	)
		special_inst = true;


	if(precise_type->is_pointer_type()){

		if(!special_inst)
			return precise_type;


		pointer_type* pt = static_cast<pointer_type*>(precise_type);
		type* inner_type = pt->get_inner_type();

		if (!inner_type->is_record_type()){
			return precise_type;
		}

		record_type* inner_t = static_cast<record_type*>(inner_type);

		if (inner_t->name != JAVA_LANG_OBJECT)
			return precise_type;

		//so far, the precise_type should be object type;
		assert(inner_t->name == JAVA_LANG_OBJECT);
		return get_return_or_function_type_info(lhs, precise_type, inst,f);
	}


	if(!need_to_infer_null_pointer){

		return precise_type;
	}


	assert(need_to_infer_null_pointer);

	assert(precise_type == il::get_integer_type());

	cout << "Input type is integer type () !!!!!!! " << endl;
	if(special_inst){
		return get_return_or_function_type_info(lhs, precise_type, inst, f);
	}

	//not special inst;
	Variable* reaching_lhs = inst->get_lhs();
	il::type* reaching_lhs_t = NULL;

	if (reaching_lhs == NULL)
		return precise_type;


	if (inst->get_instruction_id() != STORE) {
		//cout << "Reaching point is " << inst->to_string() << endl;
		reaching_lhs_t = reaching_lhs->get_type();
		//cout << "Reaching_lhs_t is " << reaching_lhs_t->to_string() << endl;
		if(reaching_lhs_t->is_invalid_type())
			return precise_type;

		return reaching_lhs_t;

	} else {

		Store* s = static_cast<Store*>(inst);
		if (0) {
			cout << "store is " << s->to_string() << endl;
			cout << "lhs is " << s->get_lhs()->to_string() << endl;
			cout << "== lhs offset is " << s->get_offset() << endl;
			cout << "== lhs fieldname is " << s->get_field_name() << endl;
		}
		il::type* lhs_var_t = reaching_lhs->get_type();


		if(lhs_var_t->is_invalid_type())
			return precise_type;

		assert(lhs_var_t->is_pointer_type());
		il::pointer_type* lhs_pt = static_cast<il::pointer_type*>(lhs_var_t);
		if ((lhs_pt->get_inner_type()->is_record_type())) {
			il::record_type* rt =
					static_cast<il::record_type*>(lhs_pt->get_inner_type());


			record_info* record_info = rt->get_field_from_name(s->get_field_name());
			reaching_lhs_t = record_info->t;

			if (0) {
				cout << "TTTTTTTTTesting fields of record type begin" << endl;
				vector<il::record_info*>& records = rt->get_fields();
				il::record_info* info = NULL;
				for (int i = 0; i < records.size(); i++) {
					info = records.at(i);
					cout << "Fieldname " << info->fname << endl;
					cout << "Fieldoffset  " << info->offset << endl;
					cout << "Field type " << info->t->to_string() << endl;
				}
				cout << "TTTTTTTTTesting fields of record type end!!!!!!"
						<< endl;
			}

			assert(reaching_lhs_t != NULL);
			if(reaching_lhs_t->is_invalid_type())
				return precise_type;


			return reaching_lhs_t;

		}else{
			return precise_type;
		}
	}


	assert(false);

}

type* PreciseTypeInfer::get_return_or_function_type_info (
		Variable* define_var, type* orig_t,
		Instruction* reaching_point, Function* f) {
	if (reaching_point->is_return_inst()) {
		cout << " @!@!@! reaching point is [[[RETURN INSTRUCTION!!]]] " << endl;
		type* ret_t = f->get_identifier().get_function_signature()->ret_type;
		cout << "return type is " << ret_t->to_string() << endl;
		return ret_t;

	} else if (reaching_point->get_instruction_id() == FUNCTION_CALL) {
		//find the corresponding parameter's delcared type
		cout << " @!@!@! reaching point is {{{{{{{{FUNCTION CALL}}}}}}}}}"
				<<"\t" <<reaching_point->to_string() <<endl;
		FunctionCall* fc = static_cast<FunctionCall*>(reaching_point);

		vector<type *> arg_types =
				((function_type*) fc->get_signature())->get_arg_types();
		cout << "function sig " << fc->get_signature()->to_string() << endl;
		vector<Symbol*>* args = fc->get_arguments();

		assert(arg_types.size() == args->size());

		for (int i = 0; i < (int) args->size(); i++) {
			assert(args->at(i)->is_variable());
			Variable* var = (Variable*) (args->at(i));
			cout << "  ^^^^ argument name " << var->get_var_name() << endl;
			cout << " ^^^^ lhs name " << define_var->get_var_name() << endl;

			if (define_var->get_var_name() == var->get_var_name())
				return arg_types.at(i);
		}
		//shouldn't be here;
		return orig_t;

	}

}

void PreciseTypeInfer::print_out_mapping(){
	cout << "=======  PreciseTypeInfer::print_out_mapping  start ========" << endl;
	map<Instruction*, map<string, PreciseTypeVarStat*> >::iterator  map_iter;
	map_iter = map_precise_type.begin();
	for(; map_iter != map_precise_type.end(); map_iter++)
	{
		Instruction* inst = map_iter->first;
		cout << "first element: " << inst->to_string() <<
				"; get_instruction_id : " << map_iter->first->get_instruction_id();

		cout << "second element: " << endl;
		map<string, PreciseTypeVarStat*> ::iterator sec_iter = map_iter->second.begin();
		for(; sec_iter != map_iter->second.end(); sec_iter++)
		{
			cout << "first  :" << sec_iter->first << endl;
			cout <<  sec_iter->second->to_string() << endl;
		}
	}
	cout << "=======  PreciseTypeInfer::print_out_mapping  end ========" << endl;
}

void PreciseTypeInfer::print_out_precise_typeinfer_mapping(){
	cout << "=======  PreciseTypeInfer::print_out_mapping  start ========" << endl;
	for(int i = 0; i < (int)this->order_insts->size(); i ++){

		Instruction* cur_inst = order_insts->at(i);
		assert(cur_inst != NULL);
		if(map_precise_type.count(cur_inst) > 0){
			cout << "INS == " << cur_inst->to_string() << endl;

			auto it = map_precise_type[cur_inst].begin();
			for(; it != map_precise_type[cur_inst].end(); it ++){

				cout << it->second->to_string() << endl;
			}
		}
	}

	cout << "=======  PreciseTypeInfer::print_out_mapping  end ========" << endl;
}


type* PreciseTypeInfer::get_func_type(FunctionCall* functionCall){
	if(functionCall->is_allocator()){
		string s = functionCall->get_function_name();
		string delimiters = "_";
		size_t current;
		size_t next = -1;
		vector<string> vec_str;

		do
		{
			current = next + 1;
			next = s.find_first_of( delimiters, current );
			if(vec_str.size() == 2)
			{
				vec_str.push_back(s.substr( current, s.size() - current ));
			}else
			{
				string sub = s.substr( current, next - current );
				if(sub != "array" && vec_str.size() == 1)
				{
					vec_str.push_back(s.substr( current, s.size() - current ));
					break;
				}
				vec_str.push_back(sub);
			}

		}while (next != string::npos && vec_str.size() < 3);



		type* return_t = find_type_by_func_info(vec_str);

		return return_t;
	}
	type* temp_t = functionCall->get_signature();
	assert(temp_t->is_function_type());
	if(temp_t->is_function_type())
	{
		function_type* ft = static_cast<function_type*> (temp_t);
		if (0) {
			cout << "**** function_type is " << ft->to_string() << endl;
			cout << "**** return type is " << ft->get_return_type()->to_string()
					<< endl;
		}
		return ft->get_return_type();
	}
	assert(false);

}



type* PreciseTypeInfer::find_type_by_func_info(vector<string>& vec_str){
	string name;
	name = vec_str.at(vec_str.size()-1);

	int numStar = 0;
	while(name.at(0) == '[')
	{
		name = name.erase(0, 1);
		numStar++;
	}

	//I, J, F, D, B, S，Z
	if(name == "I"||name == "B"||name == "J"||name == "F"||name == "D"
			||name == "S"||name == "Z")
	{
		//v1 = alloc_array_B(v0)
		name = "int";
	}else if(name == "C")
	{
		name = "char";
	}else
	{
		//cout << "name1 : " << name << endl;
		/*
		 * append ';' if name doesn't have. exclude int
		 * for example, alloc_Lpolygon() and alloc_int()
		 */
		if(name.at(name.size()-1) != ';' && name != "int")
		{
			name = name + ";";
		}
		if(vec_str.at(0) == "alloc")
		{
			name = name + "*";
		}
	}
	if(vec_str.size() > 2)
	{
		name = name + "*";
	}

	while(numStar != 0)
	{
		name = name + "*";
		numStar--;
	}

	//cout << "name2 : " << name << endl;
	type* t = type::get_type_from_name(name);
	assert(t != NULL);
	return t;

}






type* PreciseTypeInfer::get_binop_type(Binop* binop){
	switch (binop->get_binop()) {
	case _PLUS:{
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_variable()) {
			Variable* v = static_cast<Variable *> (s1);
			return find_var_type_in_Inst(v, binop);
		} else {
			Constant* c = static_cast<Constant *> (s1);
			return c->get_type();
		}
	}
	case _MINUS:{
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_variable()) {
			Variable* v = static_cast<Variable *> (s1);
			return find_var_type_in_Inst(v, binop);
		} else {
			Constant* c = static_cast<Constant *> (s1);
			return c->get_type();
		}
	}
	case _MULTIPLY: {
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_variable()) {
			Variable* v = static_cast<Variable *> (s1);
			return find_var_type_in_Inst(v, binop);
		} else {
			Constant* c = static_cast<Constant *> (s1);
			return c->get_type();
		}
	}
	case _DIV:{
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_variable()) {
			Variable* v = static_cast<Variable *> (s1);
			return find_var_type_in_Inst(v, binop);
		} else {
			Constant* c = static_cast<Constant *> (s1);
			return c->get_type();
		}
	}
	case _MOD:{
		return il::get_integer_type();
	}
	case _LT:{
		return il::get_integer_type();
	}
	case _LEQ:{
		return il::get_integer_type();
	}
	case _GT:{
		return il::get_integer_type();
	}
	case _GEQ:{
		return il::get_integer_type();
	}
	case _EQ:{
		return il::get_integer_type();
	}
	case _NEQ:{
		return il::get_integer_type();
	}
	case _LEFT_SHIFT:{
		return il::get_integer_type();
	}
	case _RIGHT_SHIFT:{
		return il::get_integer_type();
	}
	case _REAL_DIV:{
		return il::get_integer_type();
	}
	case _BITWISE_OR:{
		return il::get_integer_type();
	}
	case _BITWISE_AND:{
		return il::get_integer_type();
	}
	case _BITWISE_XOR:{
		return il::get_integer_type();
	}
	case _LOGICAL_AND:{
		return il::get_integer_type();
	}
	case _LOGICAL_OR:{
		return il::get_integer_type();
	}
	case _LOGICAL_AND_NO_SHORTCIRCUIT:{
		return il::get_integer_type();
	}
	case _LOGICAL_OR_NO_SHORTCIRCUIT:{
		return il::get_integer_type();
	}
	case _POINTER_PLUS:{
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_variable()) {
			Variable* v = static_cast<Variable *> (s1);
			return find_var_type_in_Inst(v, binop);
		} else {
			Constant* c = static_cast<Constant *> (s1);
			return c->get_type();
		}
		break;
	}
	case _VEC_BINOP:{
		assert(false);
		break;
	}
	default:{
		assert(false);
		return NULL;
		break;
	}
	}
}



type* PreciseTypeInfer::get_unop_type(Unop* unop){
	switch (unop->get_unop()) {
	case _NEGATE:{
		return get_unop_type_helper(unop);
		break;
	}
	case _PREDECREMENT:{
		return get_unop_type_helper(unop);
		break;
	}
	case _PREINCREMENT:{
		return get_unop_type_helper(unop);
		break;
	}
	case _POSTDECREMENT:{
		return get_unop_type_helper(unop);
		break;
	}
	case _POSTINCREMENT:{
		return get_unop_type_helper(unop);
		break;
	}
	case _BITWISE_NOT:{
		return get_unop_type_helper(unop);
		break;
	}
	case _CONJUGATE:{
		/* complex conjugate */
		assert(false);
		break;
	}
	case _LOGICAL_NOT:{
		return get_unop_type_helper(unop);
		break;
	}
	case _VEC_UNOP:{
		assert(false);
		break;
	}
	default:{
		assert(false);
		return NULL;
		break;
	}
	}
}

type* PreciseTypeInfer::get_unop_type_helper(Unop* unop){
	Symbol* s = unop->get_operand();
	if (s->is_variable()) {
		Variable* v = static_cast<Variable *> (s);
		return find_var_type_in_Inst(v, unop);
	} else {
		Constant* c = static_cast<Constant *> (s);
		return c->get_type();
	}

}

}
