/*
 * Instruction.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "Instruction.h"
#include "sail.h"

#define RECHABLE true

namespace sail {

Instruction::Instruction() {
	original = NULL;
	line = -1;

	//added by haiyan
	inside_static_assert = false;
	return_inst = false;

	if(RECHABLE)
		reachable = true;

	sum_for_alloc = false;
}

il::node* Instruction::get_original_node()
{
	return original;
}


instruction_type Instruction::get_instruction_id()
{
	return inst_id;
}

bool Instruction::is_save_instruction()
{
	return(this->inst_id == ADDRESS_LABEL ||
			this->inst_id == ADDRESS_VAR ||
			this->inst_id == ARRAY_REF_READ ||
			this->inst_id == ASSIGNMENT ||
			this->inst_id == BINOP ||
			this->inst_id == CAST ||
			this->inst_id == FIELD_REF_READ ||
			this->inst_id == FUNCTION_CALL ||
			this->inst_id == FUNCTION_POINTER_CALL ||
			this->inst_id == LOAD ||
			this->inst_id == UNOP ||
			this->inst_id == INSTANCEOF //added by haiyan
			);
}



bool Instruction::is_removable()
{
	return false;
}


void Instruction::set_lhs(sail::Variable* lhs){
	switch(get_instruction_id())
		{
		case ADDRESS_VAR:
		{
			AddressVar* av = (AddressVar*) this;
			av->set_lhs(lhs);

		}break;

		case ASSIGNMENT:
		{
			Assignment* a = (Assignment*) this;

			a->set_lhs(lhs);
		}break;

		case BINOP:
		{
			Binop* b = (Binop*) this;
			b->set_lhs(lhs);
		}break;
		case UNOP:
		{
			Unop* u = (Unop*) this;
			u->set_lhs(lhs);
		}break;
		case CAST:
		{
			Cast* c = (Cast*) this;

			c->set_lhs(lhs);
		}break;

		case FUNCTION_CALL:
		{
			FunctionCall* fc = (FunctionCall*) this;
			fc->set_lhs(lhs);
		}break;
		case LOAD:
		{
			Load* l = (Load*) this;

			l->set_lhs(lhs);
		}break;

		case STORE:
		{
			Store* s = (Store*) this;

			if(s->get_lhs()->is_variable()){
				s->set_lhs(lhs);
			}

		}break;
		case ADDRESS_STRING:{
			AddressString* as = (AddressString*) this;
			as->set_lhs(lhs);
		}break;
		case INSTANCEOF:{
			InstanceOf* io = (InstanceOf*) this;
			io->set_lhs(lhs);
		}break;

		default:{
			cout << "inst instruction id :: "<< get_instruction_id() << endl;
			assert(false);
		}

		}

}

sail::Variable* Instruction::get_lhs(){

	switch(get_instruction_id())
	{
	case ADDRESS_VAR:
	{
		AddressVar* av = (AddressVar*) this;
		return av->get_lhs();
	}

	case ASSIGNMENT:
	{
		Assignment* a = (Assignment*) this;

		return a->get_lhs();
	}

	case BINOP:
	{
		Binop* b = (Binop*) this;
		return b->get_lhs();
	}
	case UNOP:
	{
		Unop* u = (Unop*) this;
		return u->get_lhs();
	}
	case CAST:
	{
		Cast* c = (Cast*) this;

		return c->get_lhs();
	}

	case FUNCTION_CALL:
	{
		FunctionCall* fc = (FunctionCall*) this;
		return fc->get_lhs();
	}
	case LOAD:
	{
		Load* l = (Load*) this;

		return l->get_lhs();
	}

	case STORE:
	{
		Store* s = (Store*) this;

		if(s->get_lhs()->is_variable())
			return (sail::Variable*)s->get_lhs();
		return NULL;
	}
	case ADDRESS_STRING:{
		AddressString* as = (AddressString*) this;
		string rhs_str = as->get_string();
		if((rhs_str != "monitor-enter")
				&&(rhs_str != "monitor-exit"))
			return as->get_lhs();
		else
			return NULL;
	}
	case INSTANCEOF:{
		InstanceOf* io = (InstanceOf*) this;
		return io->get_lhs();
	}

	default:{
		cout << "inst instruction id :: "<< get_instruction_id() << endl;
		return NULL;
	}

	}


}

/***
 * give it a new variable; do not delete the original one, because it may be shared by other instruction;
 */

bool Instruction::replace_in_use_symbol_name(const string& orig_name,
		const string& new_name)
{

	bool local_var = false;
	if(orig_name != new_name)
		local_var = true;

	switch(get_instruction_id())
	{

	case ADDRESS_VAR:
	{
		//AddressVar* av = (AddressVar*) this;
		return false;

	}
	case ARRAY_REF_READ:
	{
		//ArrayRefRead* ar = (ArrayRefRead*) this;
		return false;
	}
	case ARRAY_REF_WRITE:
	{
		//ArrayRefWrite* ar = (ArrayRefWrite*) this;
		return false;
	}
	case ASSIGNMENT:
	{
		Assignment* a = (Assignment*) this;
		if (a->get_rhs()->is_variable()) {
			Variable* var = (Variable*) (a->get_rhs());
			if (var->get_var_name() == orig_name) {

				Variable* new_var = var->clone();

				new_var->set_name(new_name);
				if((local_var)&&(var->is_argument()))
					new_var->set_as_local();

				a->set_rhs(new_var);
				return true;
			}
		}
		return false;
	}
	case ASSUME:
	{
		//Assume* a=  (Assume*) this;
		return false;
	}
	case ASSUME_SIZE:
	{
		//AssumeSize* as = (AssumeSize*) this;
		return false;
	}
	case BINOP:
	{
		Binop* b = (Binop*) this;

		bool flag = false;
		if(b->get_first_operand()->is_variable()){
			Variable* rhs1 = (Variable*)(b->get_first_operand());
			if(rhs1->get_var_name() == orig_name){
				Variable* new_rhs1 = rhs1->clone();
				new_rhs1->set_name(new_name);
				if((local_var)&&(rhs1->is_argument()))
					new_rhs1->set_as_local();

				b->set_rhs1(new_rhs1);

				flag = true;
			}
		}

		if (b->get_second_operand()->is_variable()) {
			Variable* rhs2 = (Variable*) (b->get_second_operand());
			if (rhs2->get_var_name() == orig_name) {

				Variable* new_rhs2 = rhs2->clone();

				new_rhs2->set_name(new_name);

				if((local_var)&&(rhs2->is_argument()))
					new_rhs2->set_as_local();

				b->set_rhs2(new_rhs2);

				flag = true;
			}
		}

		return flag;
	}
	case UNOP:
	{
		Unop* u = (Unop*) this;
		if (u->get_operand()->is_variable()) {

			Variable* rhs = (Variable*) (u->get_operand());
			if (rhs->get_var_name() == orig_name){

				Variable* new_rhs = rhs->clone();

				new_rhs->set_name(new_name);

				if((local_var)&&(rhs->is_argument()))
					new_rhs->set_as_local();

				u->set_rhs(new_rhs);
				return true;
			}
		}
		return false;
	}
	case CAST:
	{
		Cast* c = (Cast*) this;

		if(c->get_rhs()->is_variable()){
			Variable* rhs = (Variable*)(c->get_rhs());
			if(rhs->get_var_name() == orig_name){

				Variable* new_rhs = rhs->clone();
				new_rhs->set_name(new_name);

				if((local_var)&&(rhs->is_argument()))
					new_rhs->set_as_local();

				c->set_rhs(new_rhs);
				return true;
			}
		}
		return false;
	}
	case FIELD_REF_READ:
	{
		//FieldRefRead* fs = (FieldRefRead*) this;
		return false;
	}
	case FIELD_REF_WRITE:
	{
		//FieldRefWrite* f = (FieldRefWrite*) this;
		return false;
	}
	case FUNCTION_CALL:
	{
		FunctionCall* fc = (FunctionCall*) this;
		vector<Symbol*>* args = fc->get_arguments();

		bool flag = false;
		for(unsigned int i=0; i<args->size(); i++)
		{
			if(args->at(i)->is_variable()){
				Variable* arg = (Variable*)(args->at(i));
				if(arg->get_var_name() == orig_name){
					Variable* new_arg = arg->clone();
					new_arg->set_name(new_name);

					if((local_var)&&(arg->is_argument()))
						new_arg->set_as_local();

					fc->set_rhs(new_arg, i);

					flag = true;
				}
			}
		}

		return flag;
	}
	case FUNCTION_POINTER_CALL:
	{
		//FunctionPointerCall* fc = (FunctionPointerCall*) this;

		return false;
	}
	case LOAD:
	{
		Load* l = (Load*) this;
		if(l->get_rhs()->is_variable()){
			Variable* rhs = (Variable*)(l->get_rhs());
			if(rhs->get_var_name() == orig_name){
				Variable* new_rhs = rhs->clone();
				new_rhs->set_name(new_name);

				if((local_var)&&(rhs->is_argument()))
					new_rhs->set_as_local();

				l->set_rhs(new_rhs);

				return true;
			}
		}
		return false;
	}
	case STATIC_ASSERT:
	{
		//StaticAssert* s = (StaticAssert*) this;
		return false;
	}
	case STORE:
	{
		Store* s = (Store*) this;
		bool flag = false;
		if(s->get_lhs()->is_variable()){
			Variable* lhs = (Variable*)(s->get_lhs());
			if(lhs->get_var_name() == orig_name){

				Variable* new_lhs = lhs->clone();
				new_lhs->set_name(new_name);

				if((local_var)&&(lhs->is_argument()))
					new_lhs->set_as_local();

				s->set_lhs(new_lhs);

				flag = true;
			}
		}

		if(s->get_rhs()->is_variable()){
			Variable* rhs = (Variable*)(s->get_rhs());
			if(rhs->get_var_name() == orig_name){

				Variable* new_rhs = rhs->clone();

				new_rhs->set_name(new_name);

				if((local_var)&&(rhs->is_argument()))
					new_rhs->set_as_local();

				s->set_rhs(new_rhs);

				flag = true;
			}
		}

		return flag;
	}
	case ADDRESS_STRING:{
		AddressString* as = (AddressString*) this;
		if((as->get_string() != "monitor-enter")
				&&(as->get_string() != "monitor-exit"))
			return false;

		//this case, vi = "monitor-sth" is a use register;
		if(as->get_lhs()->get_var_name() == orig_name){

			Variable* new_lhs = as->get_lhs()->clone();
			new_lhs->set_name(new_name);
			if((local_var)&&(as->get_lhs()->is_argument()))
				new_lhs->set_as_local();

			as->set_lhs(new_lhs);
			return true;
		}

		return false;
	}
	default:
		return false;
	}
}

void Instruction::get_in_use_symbols(vector<Symbol*>& syms){
	switch(get_instruction_id())
	{

	case ADDRESS_VAR:
	{
		AddressVar* av = (AddressVar*) this;
		if(av->get_rhs()->is_variable())
			syms.push_back(av->get_rhs());
		return;
	}
	case ARRAY_REF_READ:
	{
		ArrayRefRead* ar = (ArrayRefRead*) this;
		if(ar->get_rhs()->is_variable())
			syms.push_back(ar->get_rhs());
		if(ar->get_index()->is_variable())
			syms.push_back(ar->get_index());
		return;
	}
	case ARRAY_REF_WRITE:
	{
		ArrayRefWrite* ar = (ArrayRefWrite*) this;
		if(ar->get_rhs()->is_variable())
			syms.push_back(ar->get_rhs());
		return;
	}
	case ASSIGNMENT:
	{
		Assignment* a = (Assignment*) this;
		if(a->get_rhs()->is_variable())
			syms.push_back(a->get_rhs());
		return;
	}
	case ASSUME:
	{
		Assume* a=  (Assume*) this;
		if(a->get_predicate()->is_variable())
			syms.push_back(a->get_predicate());
		return;
	}
	case ASSUME_SIZE:
	{
		AssumeSize* as = (AssumeSize*) this;
		if(as->get_buffer()->is_variable())
			syms.push_back(as->get_buffer());
		if(as->get_size()->is_variable())
			syms.push_back(as->get_size());
		return;
	}
	case BINOP:
	{
		Binop* b = (Binop*) this;
		if(b->get_first_operand()->is_variable())
			syms.push_back(b->get_first_operand());
		if(b->get_second_operand()->is_variable())
			syms.push_back(b->get_second_operand());
		return;
	}
	case UNOP:
	{
		Unop* u = (Unop*) this;
		if(u->get_operand()->is_variable())
			syms.push_back(u->get_operand());
		return;
	}
	case CAST:
	{
		Cast* c = (Cast*) this;
		if(c->get_rhs()->is_variable())
			syms.push_back(c->get_rhs());
		return;
	}
	case FIELD_REF_READ:
	{
		FieldRefRead* fs = (FieldRefRead*) this;
		if(fs->get_rhs()->is_variable())
			syms.push_back(fs->get_rhs());
		return;
	}
	case FIELD_REF_WRITE:
	{
		FieldRefWrite* f = (FieldRefWrite*) this;
		if(f->get_rhs()->is_variable())
			syms.push_back(f->get_rhs());
		return;
	}
	case FUNCTION_CALL:
	{
		FunctionCall* fc = (FunctionCall*) this;
		vector<Symbol*>* args = fc->get_arguments();
		for(unsigned int i=0; i<args->size(); i++)
		{
			if((*args)[i]->is_variable())
				syms.push_back((*args)[i]);
		}
		return;
	}
	case FUNCTION_POINTER_CALL:
	{
		FunctionPointerCall* fc = (FunctionPointerCall*) this;
		vector<Symbol*>* args = fc->get_arguments();
		for(unsigned int i=0; i<args->size(); i++)
		{
			if((*args)[i]->is_variable())
				syms.push_back((*args)[i]);
		}
		return;
	}
	case LOAD:
	{
		Load* l = (Load*) this;
		if(l->get_rhs()->is_variable())
			syms.push_back(l->get_rhs());
		return;
	}
	case STATIC_ASSERT:
	{
		StaticAssert* s = (StaticAssert*) this;
		if(s->get_predicate()->is_variable())
			syms.push_back(s->get_predicate());
		return;
	}
	case STORE:
	{
		Store* s = (Store*) this;
		if(s->get_lhs()->is_variable())
			syms.push_back(s->get_lhs());
		if(s->get_rhs()->is_variable())
			syms.push_back(s->get_rhs());
		return;
	}

	case ADDRESS_STRING:{ //special case
		AddressString* as = (AddressString*) this;
		if((as->get_string() != "monitor-enter")
				&&(as->get_string() != "monitor-exit"))
			return;

		//this case, vi = "monitor-sth" is a use register;
		syms.push_back(as->get_lhs());
		return;
	}

	default:
		return;


	}
}

void Instruction::get_rhs_symbols(vector<Symbol*>& syms){
	switch(get_instruction_id())
	{

	case ADDRESS_VAR:
	{
		AddressVar* av = (AddressVar*) this;
		syms.push_back(av->get_rhs());
		return;
	}
	case ARRAY_REF_READ:
	{
		ArrayRefRead* ar = (ArrayRefRead*) this;
		syms.push_back(ar->get_rhs());
		syms.push_back(ar->get_index());
		return;
	}
	case ARRAY_REF_WRITE:
	{
		ArrayRefWrite* ar = (ArrayRefWrite*) this;
		syms.push_back(ar->get_rhs());
		return;
	}
	case ASSIGNMENT:
	{
		Assignment* a = (Assignment*) this;
		syms.push_back(a->get_rhs());
		return;
	}
	case ASSUME:
	{
		Assume* a=  (Assume*) this;
		syms.push_back(a->get_predicate());
		return;
	}
	case ASSUME_SIZE:
	{
		AssumeSize* as = (AssumeSize*) this;
		syms.push_back(as->get_buffer());
		syms.push_back(as->get_size());
		return;
	}
	case BINOP:
	{
		Binop* b = (Binop*) this;
		syms.push_back(b->get_first_operand());
		syms.push_back(b->get_second_operand());
		return;
	}
	case UNOP:
	{
		Unop* u = (Unop*) this;
		syms.push_back(u->get_operand());
		return;
	}
	case CAST:
	{
		Cast* c = (Cast*) this;
		syms.push_back(c->get_rhs());
		return;
	}
	case FIELD_REF_READ:
	{
		FieldRefRead* fs = (FieldRefRead*) this;
		syms.push_back(fs->get_rhs());
		return;
	}
	case FIELD_REF_WRITE:
	{
		FieldRefWrite* f = (FieldRefWrite*) this;
		syms.push_back(f->get_rhs());
		return;
	}
	case FUNCTION_CALL:
	{
		FunctionCall* fc = (FunctionCall*) this;
		vector<Symbol*>* args = fc->get_arguments();
		for(unsigned int i=0; i<args->size(); i++)
		{
			syms.push_back((*args)[i]);
		}
		return;
	}
	case FUNCTION_POINTER_CALL:
	{
		FunctionPointerCall* fc = (FunctionPointerCall*) this;
		vector<Symbol*>* args = fc->get_arguments();
		for(unsigned int i=0; i<args->size(); i++)
		{
			syms.push_back((*args)[i]);
		}
		return;
	}
	case LOAD:
	{
		Load* l = (Load*) this;
		syms.push_back(l->get_rhs());
		return;
	}
	case STATIC_ASSERT:
	{
		StaticAssert* s = (StaticAssert*) this;
		syms.push_back(s->get_predicate());
		return;
	}
	case STORE:
	{
		Store* s = (Store*) this;
		syms.push_back(s->get_rhs());
		return;
	}
	case ADDRESS_STRING: //for the main purpose of "vi = "moniter-enter" /"monitor-exit"
	{

		AddressString* as = (AddressString*) this;
		if((as->get_string() == "monitor-enter")||
				(as->get_string() == "monitor-exit"))
			syms.push_back(as->get_lhs());
		return;
	}
	default:
		return;


	}
}
void Instruction::get_rhs_symbols(set<Symbol*>& syms)
{
	switch(get_instruction_id())
	{
	case ADDRESS_VAR:
	{
		AddressVar* av = (AddressVar*) this;
		syms.insert(av->get_rhs());
		return;
	}
	case ARRAY_REF_READ:
	{
		ArrayRefRead* ar = (ArrayRefRead*) this;
		syms.insert(ar->get_rhs());
		syms.insert(ar->get_index());
		return;
	}
	case ARRAY_REF_WRITE:
	{
		ArrayRefWrite* ar = (ArrayRefWrite*) this;
		syms.insert(ar->get_rhs());
		return;
	}
	case ASSIGNMENT:
	{
		Assignment* a = (Assignment*) this;
		syms.insert(a->get_rhs());
		return;
	}
	case ASSUME:
	{
		Assume* a=  (Assume*) this;
		syms.insert(a->get_predicate());
		return;
	}
	case ASSUME_SIZE:
	{
		AssumeSize* as = (AssumeSize*) this;
		syms.insert(as->get_buffer());
		syms.insert(as->get_size());
		return;
	}
	case BINOP:
	{
		Binop* b = (Binop*) this;
		syms.insert(b->get_first_operand());
		syms.insert(b->get_second_operand());
		return;
	}
	case UNOP:
	{
		Unop* u = (Unop*) this;
		syms.insert(u->get_operand());
		return;
	}
	case CAST:
	{
		Cast* c = (Cast*) this;
		syms.insert(c->get_rhs());
		return;
	}
	case FIELD_REF_READ:
	{
		FieldRefRead* fs = (FieldRefRead*) this;
		syms.insert(fs->get_rhs());
		return;
	}
	case FIELD_REF_WRITE:
	{
		FieldRefWrite* f = (FieldRefWrite*) this;
		syms.insert(f->get_rhs());
		return;
	}
	case FUNCTION_CALL:
	{
		FunctionCall* fc = (FunctionCall*) this;
		vector<Symbol*>* args = fc->get_arguments();
		for(unsigned int i=0; i<args->size(); i++)
		{
			syms.insert((*args)[i]);
		}
		return;
	}
	case FUNCTION_POINTER_CALL:
	{
		FunctionPointerCall* fc = (FunctionPointerCall*) this;
		vector<Symbol*>* args = fc->get_arguments();
		for(unsigned int i=0; i<args->size(); i++)
		{
			syms.insert((*args)[i]);
		}
		return;
	}
	case LOAD:
	{
		Load* l = (Load*) this;
		syms.insert(l->get_rhs());
		return;
	}
	case STATIC_ASSERT:
	{
		StaticAssert* s = (StaticAssert*) this;
		syms.insert(s->get_predicate());
		return;
	}
	case STORE:
	{
		Store* s = (Store*) this;
		syms.insert(s->get_rhs());
		return;
	}
	case ADDRESS_STRING: //for the main purpose of "vi = "moniter-enter" /"monitor-exit"
	{

		AddressString* as = (AddressString*) this;

		if((as->get_string() == "monitor-enter")||
				(as->get_string() == "monitor-exit"))
			syms.insert(as->get_lhs());
		return;
	}
	default:
		return;


	}
}

void Instruction::get_symbols(set<Symbol*>& syms)
{
	switch(get_instruction_id())
	{
	case ADDRESS_LABEL:
	{
		AddressLabel* al = (AddressLabel*) this;
		syms.insert(al->get_lhs());
		return;
	}
	case ADDRESS_STRING:
	{
		AddressString* as = (AddressString*) this;
		syms.insert(as->get_lhs());
	}
	case ADDRESS_VAR:
	{
		AddressVar* av = (AddressVar*) this;
		syms.insert(av->get_lhs());
		syms.insert(av->get_rhs());
		return;
	}
	case ARRAY_REF_READ:
	{
		ArrayRefRead* ar = (ArrayRefRead*) this;
		syms.insert(ar->get_lhs());
		syms.insert(ar->get_rhs());
		syms.insert(ar->get_index());
		return;
	}
	case ARRAY_REF_WRITE:
	{
		ArrayRefWrite* ar = (ArrayRefWrite*) this;
		syms.insert(ar->get_rhs());
		syms.insert(ar->get_lhs());
		syms.insert(ar->get_index());
		return;
	}
	case ASSIGNMENT:
	{
		Assignment* a = (Assignment*) this;
		syms.insert(a->get_lhs());
		syms.insert(a->get_rhs());
		return;
	}
	case ASSUME:
	{
		Assume* a=  (Assume*) this;
		syms.insert(a->get_predicate());
		return;
	}
	case ASSUME_SIZE:
	{
		AssumeSize* as = (AssumeSize*) this;
		syms.insert(as->get_buffer());
		syms.insert(as->get_size());
		return;
	}
	case BINOP:
	{
		Binop* b = (Binop*) this;
		syms.insert(b->get_lhs());
		syms.insert(b->get_first_operand());
		syms.insert(b->get_second_operand());
		return;
	}
	case UNOP:
	{
		Unop* u = (Unop*) this;
		syms.insert(u->get_lhs());
		syms.insert(u->get_operand());
		return;
	}
	case CAST:
	{
		Cast* c = (Cast*) this;
		syms.insert(c->get_lhs());
		syms.insert(c->get_rhs());
		return;
	}
	case FIELD_REF_READ:
	{
		FieldRefRead* fs = (FieldRefRead*) this;
		syms.insert(fs->get_lhs());
		syms.insert(fs->get_rhs());
		return;
	}
	case FIELD_REF_WRITE:
	{
		FieldRefWrite* f = (FieldRefWrite*) this;
		syms.insert(f->get_lhs());
		syms.insert(f->get_rhs());
		return;
	}
	case FUNCTION_CALL:
	{
		FunctionCall* fc = (FunctionCall*) this;
		syms.insert(fc->get_return_variable());
		vector<Symbol*>* args = fc->get_arguments();
		for(unsigned int i=0; i<args->size(); i++)
		{
			syms.insert((*args)[i]);
		}
		return;
	}
	case FUNCTION_POINTER_CALL:
	{
		FunctionPointerCall* fc = (FunctionPointerCall*) this;
		syms.insert(fc->get_return_variable());
		vector<Symbol*>* args = fc->get_arguments();
		for(unsigned int i=0; i<args->size(); i++)
		{
			syms.insert((*args)[i]);
		}
		return;
	}
	case LOAD:
	{
		Load* l = (Load*) this;
		syms.insert(l->get_lhs());
		syms.insert(l->get_rhs());
		return;
	}
	case STATIC_ASSERT:
	{
		StaticAssert* s = (StaticAssert*) this;
		syms.insert(s->get_predicate());
		return;
	}
	case STORE:
	{
		Store* s = (Store*) this;
		syms.insert(s->get_lhs());
		syms.insert(s->get_rhs());
		return;
	}
	default:
		return;


	}
}

Instruction::~Instruction()
{

}

}
