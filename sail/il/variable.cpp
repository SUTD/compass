
#include "variable.h"
#include "block.h"
#include "variable_declaration.h"
#include "util.h"

namespace il
{

int variable::unmodeled_counter = 0;

#define UNMODELED_GCC_EXP "$unmodeled_gcc_exp"

string qt_to_string(qualifier_type qt)
{


	string res = "";
	if(qt & EXTERN) res+= "extern ";
	if(qt & VOLATILE) res+= "volatile ";
	if(qt & STATIC) res+= "static ";
	if(qt & REGISTER) res+= "register ";
	if(qt & CONSTANT) res+= "const ";
	if(qt & PRIVATE) res+= "private ";
	if(qt & PUBLIC) res+= "public ";
	if(qt & PROTECTED) res+= "protected ";


	return res;
}

variable* variable::get_unmodeled_var()
{
	string name = UNMODELED_GCC_EXP + int_to_string(unmodeled_counter++);
	namespace_context ns;
	variable* res = new variable(name, ns, NULL, LOCAL, NONE, NULL,
			il::get_integer_type(), -1);
	return res;
}

variable::variable()
{
	this->scope = NULL;
}
variable::variable(string name, namespace_context ns,
		block* scope, scope_type st, qualifier_type qt,
		variable_declaration* decl, type* t, int arg_num):t(t)
{
	this->name = name;
	this->ns = ns;
	this->scope = scope;
	this->decl = decl;
	this->qt = qt;
	this->st = st;
	this->arg_num = arg_num;
}
bool variable::is_global()
{
	return (st == GLOBAL);
}
bool variable::is_local()
{
	return (st == LOCAL);
}
bool variable::is_argument()
{
	return (st == ARGUMENT);
}
block *variable::get_scope()
{
	return scope;
}

scope_type variable::get_scope_type()
{
	return st;
}

string variable::get_name()
{
	return name;
}

const namespace_context& variable::get_namespace() const
{
	return ns;
}

void variable::print()
{
	if(st == GLOBAL) cout << "Global(";
	else if(st==ARGUMENT) cout << "Arg(";
	else{
		cout << "Local(";
	}
	cout << name;

	cout << ")  ";
	cout << " type: ";

}

string variable::to_string() const
{

	string res = ns.to_string() + name;
    return res;

}

ostream& operator <<(ostream &os, const variable &obj)
{
	os << obj.to_string();
    return os;
}

bool variable::operator==(const variable & other)
{
	return (other.name == this->name && other.ns == this->ns
			&& other.scope == this->scope);
}

void variable::set_scope(block *b)
{
	scope = b;
	if(scope != NULL) st = LOCAL;
}


variable::~variable()
{
}

int variable::get_arg_num()
{
	return arg_num;
}


}
