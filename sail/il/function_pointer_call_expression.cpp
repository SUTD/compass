#include "function_pointer_call_expression.h"

namespace il
{

function_pointer_call_expression::function_pointer_call_expression()
{

}

function_pointer_call_expression::function_pointer_call_expression(
		expression* fn_ptr, vector<expression*> args, type* t, location loc)
{
	assert(fn_ptr!=NULL);
	assert(t!=NULL);
	this->fn_ptr = fn_ptr;
	this->args = args;
	this->t = t;
	this->loc = loc;
	this->node_type_id = FUNCTION_PTR_CALL_EXP;
}

string function_pointer_call_expression::to_string() const
{
	string res = fn_ptr->to_string() + "(";
	for(int i=0; i<(int)args.size(); i++){
		assert(args[i]!=NULL);
		res += args[i]->to_string();
		if(i != (int)args.size()-1) res += ",";
	}
	res+= ")";
	return res;
}
expression* function_pointer_call_expression::get_function_pointer()
{
	return fn_ptr;
}
vector<expression*>& function_pointer_call_expression::get_arguments()
{
	return args;
}

bool function_pointer_call_expression::has_return()
{
	type * fn_type = fn_ptr->get_type();
	assert(fn_type->is_pointer_type());
	pointer_type* _fn_type = (pointer_type*)fn_type;
	type* sig = _fn_type->get_deref_type();
	assert(sig->is_function_type());
	function_type *_sig = (function_type*)sig;
	return _sig->get_return_type() != NULL;
}


function_pointer_call_expression::~function_pointer_call_expression()
{
}

}
