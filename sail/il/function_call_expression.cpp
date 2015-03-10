#include <sstream>
using namespace std;

#include "function_call_expression.h"

namespace il
{

function_call_expression::function_call_expression() {}

function_call_expression::function_call_expression(
		string fn_name,  namespace_context ns,
		type *fn_signature, vector<expression *> &args,
		type *t, bool is_no_return, bool is_alloc, bool is_operator_new,
		bool is_dealloc,
		bool is_constructor, bool is_destructor, expression *vtable_lookup_expr,
		long int vtable_index, location loc)
	: fn_name(fn_name), ns(ns),
	  fn_signature(fn_signature),
	  args(args),
	  is_no_return(is_no_return),
	  is_alloc(is_alloc), operator_new(is_operator_new),
	  is_dealloc(is_dealloc),
	  constructor(is_constructor),
	  destructor(is_destructor),
	  vtable_lookup_expr(vtable_lookup_expr),
	  vtable_index(vtable_index)
{
	assert(fn_signature != NULL);
	assert(t!=NULL);
	this->t = t;
	this->loc = loc;
	this->node_type_id = FUNCTION_CALL_EXP;

}

bool function_call_expression::is_operator_new()
{
	return operator_new;
}

expression *function_call_expression::get_vtable_lookup_expr()
{
	return vtable_lookup_expr;
}

int function_call_expression::get_vtable_index()
{
	return vtable_index;
}

function_call_expression::~function_call_expression()
{
}

string function_call_expression::to_string() const
{
	stringstream res;
	

	if(ns.to_string() != "") {
		res << ns.to_string();
		res << "::";
	}
	res << fn_name;
	
	if (is_call_to_virtual())
		res << "[@ " << vtable_lookup_expr->to_string() << ", " << vtable_index << " ]";
	
	res << "(";
	for (unsigned int i = 0; i < args.size(); i++) {

		assert(args[i] != NULL);
		res << args[i]->to_string();
		if (i != args.size() - 1)
			res << ", ";
	}
	res << ")";
	
	return res.str();
}

string function_call_expression::get_function()
{
	return fn_name;
}
const namespace_context & function_call_expression::get_namespace()
{
	return ns;
}

bool function_call_expression::has_return()
{
	assert(fn_signature->is_function_type());
	function_type* mysig = (function_type*) fn_signature;
	return(mysig->get_return_type()!=NULL);
}

bool function_call_expression::is_exit_function()
{
	return this->is_no_return;
}

type* function_call_expression::get_signature()
{
	return fn_signature;
}
vector<expression*>& function_call_expression::get_arguments()
{
	return args;
}

bool function_call_expression::is_allocator()
{
	return is_alloc;
}

bool function_call_expression::is_deallocator()
{
	return is_dealloc;
}

bool function_call_expression::is_constructor()
{
	return constructor;
}

bool function_call_expression::is_destructor()
{
	return destructor;
}

bool function_call_expression::is_call_to_virtual() const
{
	return vtable_lookup_expr != NULL;
}


}
