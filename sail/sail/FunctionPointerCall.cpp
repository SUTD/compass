/*
 * FunctionPointerCall.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "FunctionPointerCall.h"
#include "Variable.h"
#include "Symbol.h"
#include "node.h"
#include "Function.h"
#include<assert.h>

namespace sail {

FunctionPointerCall::FunctionPointerCall(Variable* ret, Variable* fn_ptr,
		vector<Symbol*>* args, il::node* original, int line)
{
	this->ret = ret;
	this->fn_ptr = fn_ptr;
	this->args = args;
	assert(original!=NULL);
	this->original = original;
	this->inst_id = FUNCTION_POINTER_CALL;
	this->line = line;

}

string FunctionPointerCall::to_string() const
{
	string res ="";
	if(ret!=NULL) res+= ret->to_string() + " = ";
	res += fn_ptr->to_string() + "(";
	for(int i=0; i<(int)args->size(); i++)
	{
		res += (*args)[i]->to_string();
		if(i!= (int) args->size()-1) res += ", ";
	}
	res += ")";
	return res;
}


string FunctionPointerCall::to_string(bool pretty_print) const
{
	string res ="";
	if(ret!=NULL) res+= ret->to_string() + " = ";
	res += fn_ptr->to_string() + "(";
	for(int i=0; i<(int)args->size(); i++)
	{
		res += (*args)[i]->to_string(pretty_print);
		if(i!= (int) args->size()-1) res += ", ";
	}
	res += ")";
	return res;
}

bool FunctionPointerCall::has_return()
{
	return ret!=NULL;
}

Variable* FunctionPointerCall::get_return_variable()
{
	return ret;
}

Variable* FunctionPointerCall::get_lhs()
{
	return ret;
}

void FunctionPointerCall::set_lhs(Variable* v)
{
	this->ret = v;
}

Variable* FunctionPointerCall::get_function_pointer()
{
	return fn_ptr;
}

vector<Symbol*>* FunctionPointerCall::get_arguments()
{
	return args;
}

bool FunctionPointerCall::is_removable()
{
	return false;
}



FunctionPointerCall::~FunctionPointerCall() {
	delete args;
}

}
