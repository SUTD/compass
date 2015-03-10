#include "function_declaration.h"

#include "block.h"
#include "variable_declaration.h"
#include "function_declaration.h"

namespace il
{

function_declaration::function_declaration()
{
}

function_declaration::function_declaration(string name,
	                                       namespace_context ns,
	                                       qualifier_type qt,
	                                       type *ret_type,
	                                       vector<variable_declaration *> &args,
	                                       block *body,
	                                       bool is_alloc,
	                                       bool is_dealloc,
	                                       bool constructor,
	                                       bool destructor,
	                                       bool is_inline,
	                                       bool is_vararg,
	                                       bool is_member_function,
	                                       bool is_static_member_function,
	                                       bool is_virtual,
	                                       bool is_abstract,
	                                       function_type *sig)
	: name(name),
	  ns(ns),
	  arguments(args),
	  member_function(is_member_function),
	  static_member_function(is_static_member_function),
	  is_virtual(is_virtual), is_abstract(is_abstract)
{
	this->body = body;
	this->is_alloc = is_alloc;
	this->is_dealloc = is_dealloc;
	this->constructor = constructor;
	this->destructor = destructor;
	this->is_inline = is_inline;
	this->vararg = is_vararg;
	this->ret_type = ret_type;
	this->qt = qt;
	this->signature = sig;
	this->node_type_id = FUNCTION_DECL;
	if(destructor)
	{
		vector<type*> arg_t;
		assert(sig->get_arg_types().size() >=1);
		arg_t.push_back(sig->get_arg_types()[0]);
		signature = function_type::make(sig->get_return_type(), arg_t,
				sig->is_vararg);
	}

	/*
	 * If constructor/destructor,
	 */
}

string function_declaration::get_name()
{
	return name;
}

int function_declaration::get_num_args()
{
	return arguments.size();
}

bool function_declaration::is_method()
{
	return member_function;
}

bool function_declaration::is_virtual_method()
{
	return is_virtual;
}

bool function_declaration::is_abstract_virtual_method()
{
	return is_abstract;
}


bool function_declaration::is_static_method()
{
	return static_member_function;
}



const vector<variable_declaration*> & function_declaration::get_arguments()
{
	return arguments;
}

type* function_declaration::get_ret_type()
{
	return ret_type;
}

const namespace_context & function_declaration::get_namespace()
{
	return ns;
}

function_type* function_declaration::get_signature()
{
	return this->signature;
}

block* function_declaration::get_body()
{
	return body;
}

bool function_declaration::is_allocator()
{
	return is_alloc;
}


bool function_declaration::is_deallocator()
{
	return is_dealloc;
}

bool function_declaration::is_constructor()
{
	return constructor;
}

bool function_declaration::is_destructor()
{
	return destructor;
}

bool function_declaration::is_vararg()
{
	return vararg;
}

bool function_declaration::is_inlined()
{
	return is_inline;
}

void function_declaration::set_body(block *b)
{
	body = b;
}

function_declaration::~function_declaration()
{
}

void function_declaration::print()
{
	cout << "function_Decl:" << name << endl;
}

string function_declaration::to_string() const
{

	string res = qt_to_string(qt);
	
	if (static_member_function)
		res += "class-static ";
	
	if (is_virtual)
		res += "virtual ";
	
	if (ret_type == NULL) res += "void";
	else res += ret_type->to_string();

	res += + " " + ns.to_string()  + name + "(";

	for(unsigned int i = 0; i < arguments.size(); i++)
	{
		if(i!=0)
			res += ", ";
		res += arguments[i]->to_string();
	}

	if (vararg) res += ", ...";

	res += ")";
	res += body ? "\n" + body->to_string() : ";";
	return res;
}

bool function_declaration::is_fun_decl()
{
	return true;
}
bool function_declaration::is_var_decl()
{
	return false;
}

ostream& operator <<(ostream &os, const function_declaration &obj)
{
      os << obj.to_string();
      return os;
}



}
