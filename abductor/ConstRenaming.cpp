/*
 * ConstRenaming.cpp
 *
 *  Created on: Nov 27, 2012
 *      Author: boyang
 */

#include "ConstRenaming.h"
#include "Cfg.h"
#include "Block.h"
#include "BasicBlock.h"
#include "SuperBlock.h"
#include "Assignment.h"
#include "Function.h"
#include "CfgEdge.h"
#include "Constant.h"
#include "sail/Variable.h"
#include "VariableTerm.h"
#include "ConstantTerm.h"

#include <queue>
#include <set>

#include "sail.h"
#include "il.h"
#define DEBUG false

using namespace sail;
using namespace il;

ConstRenaming::ConstRenaming() {
	// TODO Auto-generated constructor stub
}

ConstRenaming::ConstRenaming(Function* f){
	//cout << "=======  before function  renaming =======" << endl;
	//cout << f->to_string() << endl;

	name_counter = 0;
	temp_name_counter = 0;

	function_declaration* declare = f->get_original_declaration();
	var_prefix = declare->get_name()+"::";

	set<Block*, sail::CompareBlock> worklist;
	worklist.insert(f->get_entry_block());
	while(worklist.size() > 0)
	{
		Block* b_cur = *(worklist.begin());
		worklist.erase(b_cur);
		if(b_cur->is_basicblock())
		{
			BasicBlock* bb_cur = static_cast<BasicBlock*>(b_cur);
			//cout << "bb_cur before : " << bb_cur->to_string() << endl;
			Fill_map_by_block(bb_cur);
			//cout << "bb_cur after : " << bb_cur->to_string() << endl;
		}else
		{
			SuperBlock* bb_cur = static_cast<SuperBlock*>(b_cur);
			worklist.insert(bb_cur->get_entry_block());
		}
		set<CfgEdge*>& set_Edges = b_cur->get_successors();
		set<CfgEdge*>::iterator iter_Edges = set_Edges.begin();
		for(; iter_Edges != set_Edges.end(); iter_Edges++)
		{
			worklist.insert((*iter_Edges)->get_target());
		}
	}

	compute_knowlege();

	if(DEBUG)
	{
		cout << "======== print map ======== " << endl;
		cout << "prefix : " << this->var_prefix << endl;
		print_map();
		cout << "=======  after function  renaming =======" << endl;
	}
}


ConstRenaming::~ConstRenaming() {
	// TODO Auto-generated destructor stub
}



void ConstRenaming::Fill_map_by_block(BasicBlock* bb_cur){
	vector<Instruction*> vector_inst =  bb_cur->get_statements();
	vector<Instruction*>::iterator iter_inst = vector_inst.begin();
	Instruction* preinst = NULL;
	for (;iter_inst != vector_inst.end(); iter_inst++)
	{
		//cout << "instruction  : " << (*iter_inst)->to_string() << endl;
		cons_inst_renaming(bb_cur, *iter_inst, preinst);
		preinst = *iter_inst;
	}
}



void ConstRenaming::compute_knowlege(){
	this->knowlege = Constraint(true);
	map<int , Variable*>::iterator map_iter = this->int_var_map.begin();
	for(; map_iter != this->int_var_map.end(); map_iter++)
	{
		string str_term = this->var_prefix + map_iter->second->get_var_name();
		Term* term_var = VariableTerm::make(str_term);
		Term * term_cons = ConstantTerm::make(map_iter->first);
		Constraint temp_const(term_var, term_cons, ATOM_EQ);
		this->knowlege = knowlege & temp_const;
	}
	//cout << "knowlege : " << knowlege.to_string() << endl;
}




Constraint ConstRenaming::get_knowledge(){
	return this->knowlege;
}


void ConstRenaming::insert_to_map(int i_insert){
	//if i_insert and i_insert+-1  is not in the map
	if(int_var_map.find(i_insert) == int_var_map.end()
			&& int_var_map.find(i_insert+1) == int_var_map.end()
			&& int_var_map.find(i_insert-1) == int_var_map.end()){
		stringstream ss;//create a stringstream
		ss << name_counter;//add number to the stream
		string name = "$cn" + ss.str();
		name_counter++;
		Variable* new_var = new Variable(name, il::get_integer_type());
		new_var->set_name(name);
		this->int_var_map.insert(pair<int, Variable*>(i_insert,new_var));
	}
}




Variable* ConstRenaming::get_variable_of_con(int i_insert, int& i_case){
	map<int , Variable*>::iterator map_iter = int_var_map.find(i_insert);
	Variable* v_return = NULL;
	if(map_iter != int_var_map.end()){
		v_return = map_iter->second;
		i_case = 1;
	}

	map_iter = int_var_map.find(i_insert-1);
	if(map_iter != int_var_map.end()){
		v_return = map_iter->second;
		i_case = 2;
	}

	map_iter = int_var_map.find(i_insert+1);
	if(map_iter != int_var_map.end()){
		v_return = map_iter->second;
		i_case = 3;
	}
	assert(v_return != NULL);
	return v_return;
}



void ConstRenaming::cons_inst_renaming(BasicBlock* bb_cur,
		Instruction* inst_cur,  Instruction* preInst){
	switch (inst_cur->get_instruction_id()) {
	case ASSIGNMENT: {
		Assignment* inst_assignment = static_cast<sail::Assignment*> (inst_cur);
		Symbol* sym_rhs = inst_assignment->get_rhs();
		if (sym_rhs->is_constant()){
			Constant* c = static_cast<Constant *> (sym_rhs);
			int i_c = c->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			//cout << "bb_cur 11111" << bb_cur->to_string() << endl;
			assignment_renaming(bb_cur, v_cn ,inst_assignment, i_case);
			//cout << "bb_cur 22222" << bb_cur->to_string() << endl;
		}
		break;
	}
	case FUNCTION_CALL: {
		break;
	}
	case CAST: {
		break;
	}
	case BINOP: {
		Binop* binop = static_cast<Binop*> (inst_cur);
		//cout << "bb_cur 11111" << bb_cur->to_string() << endl;
		cons_of_binop_renaming(bb_cur, binop, preInst);
		//cout << "bb_cur 22222" << bb_cur->to_string() << endl;
		break;
	}
	case UNOP: {
		break;
	}
	case ARRAY_REF_READ: {
		break;
	}
	case FIELD_REF_READ: {
		assert(false);
		break;
	}
	case LOAD: {
		break;
	}
	case ARRAY_REF_WRITE: {
		break;
	}
	case FIELD_REF_WRITE:{
		break;
	}
	case STORE:{
		break;
	}
	case JUMP:{
		break;
	}
	case SAIL_LABEL:{
		break;
	}
	case FUNCTION_POINTER_CALL:{
		break;
	}
	case LOOP_INVOCATION:{
		break;
	}
	case DROP_TEMPORARY:{
		break;
	}
	case STATIC_ASSERT:{
		break;
	}
	case ASSUME:{
		break;
	}
	case ASSUME_SIZE:{
		break;
	}
	case INSTANCEOF:{
		break;
	}
	case EXCEPTION_RETURN:{
		break;
	}
	case ADDRESS_LABEL:{
		break;
	}
	case ADDRESS_VAR:{
		break;
	}
	case ADDRESS_STRING:{
		break;
	}
	case SAIL_ASSEMBLY:{
		break;
	}
	case BRANCH:{
		break;
	}
	default: {
		break;
	}
	}
}

void ConstRenaming::assignment_renaming(BasicBlock* bb_cur, Variable* v_cn,
		Assignment* inst_assignment, int i_case){
	switch (i_case) {
	case 1:{
		//==
		inst_assignment->set_rhs(v_cn);
		break;
	}
	case 2:{
		//-1 exists
		Variable* new_var = new Variable("__temp$", il::get_integer_type());
		rename_unique_temp(new_var);
		Constant* con1 = new Constant(1, true, 32);
		Binop* new_binop = new  Binop(new_var, v_cn, con1, _PLUS, NULL, -1);
		inst_assignment->set_rhs(new_var);
		bb_cur->add_statement_before(new_binop, inst_assignment);
		break;
	}
	case 3:{
		//+1 exists
		Variable* new_var = new Variable("__temp$", il::get_integer_type());
		rename_unique_temp(new_var);
		Constant* con1 = new Constant(1, true, 32);
		Binop* new_binop = new  Binop(new_var, v_cn, con1, _MINUS, NULL, -1);
		inst_assignment->set_rhs(new_var);
		bb_cur->add_statement_before(new_binop, inst_assignment);
		break;
	}
	default:{
		assert(false);
	}
	}
}

void ConstRenaming::binop_renaming_rhs1(BasicBlock* bb_cur, Variable* v_cn,
		Binop* inst_binop, int i_case){
	switch (i_case) {
	case 1:{
		//==
		inst_binop->set_rhs1(v_cn);
		break;
	}
	case 2:{
		//-1 exists
		Variable* new_var = new Variable("__temp$", il::get_integer_type());
		rename_unique_temp(new_var);
		Constant* con1 = new Constant(1, true, 32);
		Binop* new_binop = new Binop(new_var, v_cn, con1, _PLUS, NULL, -1);
		inst_binop->set_rhs1(new_var);
		bb_cur->add_statement_before(new_binop, inst_binop);
		break;
	}
	case 3:{
		//+1 exists
		Variable* new_var = new Variable("__temp$", il::get_integer_type());
		rename_unique_temp(new_var);
		Constant* con1 = new Constant(1, true, 32);
		Binop* new_binop = new Binop(new_var, v_cn, con1, _MINUS, NULL, -1);
		inst_binop->set_rhs1(new_var);
		bb_cur->add_statement_before(new_binop, inst_binop);
		break;
	}
	default:{
		assert(false);
	}
	}
}



void ConstRenaming::binop_renaming_rhs2(BasicBlock* bb_cur, Variable* v_cn,
		Binop* inst_binop, int i_case){
	switch (i_case) {
	case 1:{
		//==
		inst_binop->set_rhs2(v_cn);
		break;
	}
	case 2:{
		//-1 exists
		Variable* new_var = new Variable("__temp$", il::get_integer_type());
		rename_unique_temp(new_var);
		Constant* con1 = new Constant(1, true, 32);
		Binop* new_binop = new Binop(new_var, v_cn, con1, _PLUS, NULL, -1);
		inst_binop->set_rhs2(new_var);
		bb_cur->add_statement_before(new_binop, inst_binop);
		break;
	}
	case 3:{
		//+1 exists
		Variable* new_var = new Variable("__temp$", il::get_integer_type());
		rename_unique_temp(new_var);
		Constant* con1 = new Constant(1, true, 32);
		Binop* new_binop = new Binop(new_var, v_cn, con1, _MINUS, NULL, -1);
		inst_binop->set_rhs2(new_var);
		bb_cur->add_statement_before(new_binop, inst_binop);
		break;
	}
	default:{
		assert(false);
	}
	}
}



void ConstRenaming::rename_unique_temp(Variable* var)
{
	stringstream ss;//create a stringstream
	ss << temp_name_counter;//add number to the stream
	string name = "__temp$c" + ss.str();
	temp_name_counter++;
	var->set_name(name);
}


void ConstRenaming::cons_of_binop_renaming(BasicBlock* bb_cur, Binop* binop,
		Instruction* preInst){
	switch (binop->get_binop()) {
	case _PLUS:{
		break;
	}
	case _MINUS:{
		break;
	}
	case _MULTIPLY: {
		break;
	}
	case _DIV:{
		break;
	}
	case _MOD:{
		break;
	}
	case _LT:{
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_constant()) {
			Constant* c1 = static_cast<Constant*> (s1);
			int i_c = c1->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs1(bb_cur, v_cn ,binop, i_case);
		}
		Symbol* s2 = binop->get_second_operand();
		if (s2->is_constant()) {
			Constant* c2 = static_cast<Constant*> (s2);
			int i_c = c2->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs2(bb_cur, v_cn ,binop, i_case);
		}
		break;
	}
	case _LEQ:{
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_constant()) {
			Constant* c1 = static_cast<Constant*> (s1);
			int i_c = c1->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs1(bb_cur, v_cn ,binop, i_case);
		}
		Symbol* s2 = binop->get_second_operand();
		if (s2->is_constant()) {
			Constant* c2 = static_cast<Constant*> (s2);
			int i_c = c2->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs2(bb_cur, v_cn ,binop, i_case);
		}
	}
	case _GT:{
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_constant()) {
			Constant* c1 = static_cast<Constant*> (s1);
			int i_c = c1->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs1(bb_cur, v_cn ,binop, i_case);
		}
		Symbol* s2 = binop->get_second_operand();
		if (s2->is_constant()) {
			Constant* c2 = static_cast<Constant*> (s2);
			int i_c = c2->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs2(bb_cur, v_cn ,binop, i_case);
		}
	}
	case _GEQ:{
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_constant()) {
			Constant* c1 = static_cast<Constant*> (s1);
			int i_c = c1->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs1(bb_cur, v_cn ,binop, i_case);
		}
		Symbol* s2 = binop->get_second_operand();
		if (s2->is_constant()) {
			Constant* c2 = static_cast<Constant*> (s2);
			int i_c = c2->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs2(bb_cur, v_cn ,binop, i_case);
		}
	}
	case _EQ:{
		// do not rename if it's mod operation
		if(preInst != NULL)
		{
			if(preInst->get_instruction_id() == BINOP)
			{
				Binop* prebinop = static_cast<Binop*> (preInst);
				if(prebinop->get_binop() == _MOD)
					break;
			}
		}
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_constant()) {
			Constant* c1 = static_cast<Constant*> (s1);
			int i_c = c1->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs1(bb_cur, v_cn ,binop, i_case);
		}
		Symbol* s2 = binop->get_second_operand();
		if (s2->is_constant()) {
			Constant* c2 = static_cast<Constant*> (s2);
			int i_c = c2->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs2(bb_cur, v_cn ,binop, i_case);
		}
	}
	case _NEQ:{
		// do not rename if it's mod operation
		if(preInst != NULL)
		{
			if(preInst->get_instruction_id() == BINOP)
			{
				Binop* prebinop = static_cast<Binop*> (preInst);
				if(prebinop->get_binop() == _MOD)
					break;
			}
		}
		Symbol* s1 = binop->get_first_operand();
		if (s1->is_constant()) {
			Constant* c1 = static_cast<Constant*> (s1);
			int i_c = c1->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs1(bb_cur, v_cn ,binop, i_case);
		}
		Symbol* s2 = binop->get_second_operand();
		if (s2->is_constant()) {
			Constant* c2 = static_cast<Constant*> (s2);
			int i_c = c2->get_integer();
			insert_to_map(i_c);
			int i_case = 0;
			Variable* v_cn = get_variable_of_con(i_c, i_case);
			binop_renaming_rhs2(bb_cur, v_cn ,binop, i_case);
		}
	}
	case _LEFT_SHIFT:{
		break;
	}
	case _RIGHT_SHIFT:{
		break;
	}
	case _REAL_DIV:{
		break;
	}
	case _BITWISE_OR:{
		break;
	}
	case _BITWISE_AND:{
		break;
	}
	case _LOGICAL_AND:{
		break;
	}
	case _LOGICAL_OR:{
		break;
	}
	case _LOGICAL_AND_NO_SHORTCIRCUIT:{
		break;
	}
	case _LOGICAL_OR_NO_SHORTCIRCUIT:{
		break;
	}
	case _POINTER_PLUS:{
		break;
	}
	case _VEC_BINOP:{
		break;
	}
	default:{
		break;
	}
	}
}



void ConstRenaming::print_map(){
	map<int , Variable*>::iterator map_iter = this->int_var_map.begin();
	for(; map_iter != this->int_var_map.end(); map_iter++)
	{
		cout << "Constant : " << map_iter->first;
		cout << "\t Variable : " << map_iter->second->to_string() << endl;
	}
}


