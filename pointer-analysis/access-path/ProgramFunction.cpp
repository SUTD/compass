/*
 * ProgramFunction.cpp
 *
 *  Created on: Feb 19, 2010
 *      Author: isil
 */

#include "ProgramFunction.h"
#include "ConstantValue.h"
#include "compass_assert.h"

map<string, int> ProgramFunction::function_name_to_id;
int ProgramFunction::counter = 0;

ProgramFunction::ProgramFunction(const string& function_name,
		il::type* sig):
	FunctionTerm(CNode::get_varmap().get_id("program_function"),
			ConstantValue::make(get_id(function_name, sig))->to_term(), true,
			ATTRIB_PROGRAM_FUNCTION)
{
	assert_context("Making program function: " + function_name);
	c_assert(sig != NULL);
	this->function_name = function_name;
	this->signature = sig;
	this->t = sig;
	this->has_index_var = false;
	this->apt = AP_PROGRAM_FUNCTION;
	this->specialization_type = AP_PROGRAM_FUNCTION;
	add_signedness_attribute();
}


ProgramFunction* ProgramFunction::make(const string& function_name,
		il::type* sig)
{
	ProgramFunction* pf = new ProgramFunction(function_name, sig);
	ProgramFunction* res=  (ProgramFunction*) FunctionTerm::get_term(pf);
	return res;
}
const string& ProgramFunction::get_function_name() const
{
	return function_name;
}

il::type* ProgramFunction::get_signature()
{
	return this->signature;
}

string ProgramFunction::to_string()
{
	if(PRINT_AS_TERM) {
		return FunctionTerm::to_string();
	}
	return function_name + "<" + signature->to_string() + ">";
}
Term* ProgramFunction::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	return this;
}
Term* ProgramFunction::substitute(Term* (*sub_func)(Term* t, void* data),
			void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	return this;
}

int ProgramFunction::get_id(const string& fn, il::type* sig)
{
	string key = fn;
	key+= sig->to_string();
	if(function_name_to_id.count(key) > 0) {
		return function_name_to_id[key];
	}
	function_name_to_id[key] = counter;
	return counter++;
}

void ProgramFunction::clear()
{
	function_name_to_id.clear();
	counter = 0;
}

ProgramFunction::~ProgramFunction() {

}
