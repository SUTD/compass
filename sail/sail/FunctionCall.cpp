/*
 * FunctionCall.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "FunctionCall.h"
#include "Function.h"
#include "Symbol.h"
#include "type.h"
#include "node.h"
#include "function_call_expression.h"
#include <assert.h>
#include "Variable.h"
#include "function_declaration.h"

namespace sail {

FunctionCall::FunctionCall(Variable* ret, string fn_name,
		il::namespace_context ns,
		il::type* fn_signature,
		vector<Symbol*>* args, bool virtual_dispatch,
		bool is_allocator, bool is_constructor,
		bool is_super,
		il::node* original, int line)
{

	this->virtual_dispatch = virtual_dispatch;
	this->is_allocator_flag = is_allocator;
	this->is_constructor_flag = is_constructor;
	this->is_super_flag = is_super;

	this->ret = ret;
	this->fn_name = fn_name;
	this->ns = ns;

	this->fn_signature = fn_signature;
	this->args = args;
	

	this->original = original;
	this->inst_id = FUNCTION_CALL;
	this->line = line;
}

FunctionCall::FunctionCall(Variable* ret, string fn_name, il::namespace_context ns,
			il::type* fn_signature, vector<Symbol*>* args,
			bool virtual_dispatch, bool is_allocator, bool is_constructor,
			bool is_super, int line){
		this->virtual_dispatch = virtual_dispatch;
		this->is_allocator_flag = is_allocator;
		this->is_constructor_flag = is_constructor;
		this->is_super_flag = is_super;

		this->ret = ret;
		this->fn_name = fn_name;
		this->ns = ns;

		this->fn_signature = fn_signature;
		this->args = args;
		this->original = NULL;
		this->inst_id = FUNCTION_CALL;
		this->line = line;

}

bool FunctionCall::is_virtual_call()
{
	return virtual_dispatch;
}

il::method_info* FunctionCall::find_matching_method(il::record_type* t,
		const string& name, il::type* _signature)
{


	assert(_signature->is_function_type());
	il::function_type* signature = static_cast<il::function_type*>(_signature);

	signature = signature->get_method_signature();


	const map<string, il::method_info*>& methods = t->get_member_functions();
	auto it = methods.begin();
	for(; it!= methods.end(); it++)
	{
		string cur_name = it->first;
		//cout << "******* cur name: " << cur_name << endl;
		il::function_type* ft = it->second->get_signature();

		if(name != cur_name) continue;
		if(it->second->static_method) continue;

	//	cout << "****** cur sig: " << ft->to_string() << endl;
		ft = ft->get_method_signature();
	//	cout << "****** cur sig 2: " << ft->to_string() << endl;
	//	cout << "signature" << signature->to_string() << endl;


		if(ft == signature){
		//	cout << "FOUND METHOD!!!" << endl;
			return it->second;
		}

	}

	return NULL;

}

void FunctionCall::get_virtual_call_targets(set<call_id>& targets)
{
	assert(virtual_dispatch);


	// This call should have at least one argument
	assert(this->args->size() >=1);


	// The first argument should be a pointer to a record type.
	Symbol* first_arg = (*args)[0];
	il::type* t = first_arg->get_type();
	if(!t->is_pointer_type()) return;
	assert(t->is_pointer_type());
	il::type* _class_t = t->get_inner_type();
	if(!_class_t->is_record_type()) return;

	assert(_class_t->is_record_type());
	il::record_type* class_t = static_cast<il::record_type*>(_class_t);


	set<il::record_type*> subclasses;
	class_t->get_transitive_subclasses(subclasses);

	set<il::record_type*>::iterator it = subclasses.begin();

	//cout << "Getting all virtual targets for : " << class_t->to_string() << endl;
	for(; it!= subclasses.end(); it++) {
		il::record_type* cur = *it;
		//cout << "SUBCLASS: " << cur->to_string() << endl;
		//cout << "looking for method " << fn_name << "sig: " <<
		//		fn_signature->to_string() << endl;
		il::method_info* mi = find_matching_method(cur, fn_name,
				fn_signature);

		if(mi == NULL) continue;
		if(mi->abstract_method) continue;
		//cout << "mi added to call id: " << mi->name << " sig: " <<
		//		*mi->get_signature() << "ns: " << mi->ns.to_string()<< endl;
		call_id cid(mi->name, mi->ns, mi->get_signature(),
				LANG_CPP);
		targets.insert(cid);
	}


}


string FunctionCall::to_string() const
{
	return to_string(false);
}

string FunctionCall::to_string(bool pretty_print) const
{
	string res = "";
	//haiyan added 5.3
	if((!pretty_print)&&(ret != NULL)) res += ret->to_string()+ " = ";
	else if(ret!=NULL) res += ret->to_string(true) + " = ";
	res += get_namespace().to_string();
	res+= (virtual_dispatch? "@" : "") + fn_name + "(";
	for(int i=0; i<(int)args->size(); i++)
	{
		res += (*args)[i]->to_string(pretty_print);
		if(i!= (int) args->size()-1) res += ", ";
	}
	res += ")";
	if(is_exit_function()) res+="<exit fn>";
	//haiyan added to print line num.
	if(pretty_print)
	{
	    string linenum;
	    if((this->line != -1)&&(this->line != 0))
	    {
		stringstream ss;
		ss << this->line;
		linenum = ss.str();
		res +=  "( line :" + linenum + " )";
	    }
	}
	return res;
}

string FunctionCall::get_function_name()
{
	return fn_name;
}
il::namespace_context FunctionCall::get_namespace() const
{
	return ns;

}


il::type* FunctionCall::get_signature()
{
	return fn_signature;
}

vector<Symbol*>* FunctionCall::get_arguments()
{
	return args;
}

bool FunctionCall::has_return()
{
	return ret != NULL;
}

bool FunctionCall::is_exit_function() const
{
	/*
	 * Built-in expresssions are never exit functions
	 */
	if(original == NULL || original->node_type_id != FUNCTION_CALL_EXP)
		return false;
	il::function_call_expression* orig_call =
			(il::function_call_expression*) original;
	return orig_call->is_exit_function();
}

bool FunctionCall::is_allocator() const
{
	return is_allocator_flag;

}

bool FunctionCall::is_super() const
{
	return is_super_flag;

}


bool FunctionCall::is_operator_new() const
{
	if(original == NULL || original->node_type_id != FUNCTION_CALL_EXP)
		return false;
	il::function_call_expression* orig_call =
			(il::function_call_expression*) original;
	return orig_call->is_operator_new();
}

bool FunctionCall::is_deallocator() const
{
	if(original == NULL || original->node_type_id != FUNCTION_CALL_EXP)
		return false;
	il::function_call_expression* orig_call =
			(il::function_call_expression*) original;
	return orig_call->is_deallocator();
}


bool FunctionCall::is_constructor()
{
	return is_constructor_flag;


}


bool FunctionCall::is_destructor()
{
	if(original == NULL || original->node_type_id != FUNCTION_CALL_EXP)
		return false;
	il::function_call_expression* orig_call =
			(il::function_call_expression*) original;
	return orig_call->is_destructor();
}

Variable* FunctionCall::get_return_variable()
{
	return ret;
}

Variable* FunctionCall::get_lhs()
{
	return ret;
}

void FunctionCall::set_lhs(Variable* v)
{
	this->ret = v;
}


void FunctionCall::set_rhs(Symbol* s, int pos)
{
	this->args->at(pos) = s;
}


bool FunctionCall::is_removable()
{
	return false;
}




FunctionCall::~FunctionCall() {
	delete args;
}

}
