#include "variable_declaration.h"
#include "variable.h"

#include "expression.h"

namespace il
{

variable_declaration::variable_declaration()
{

}

variable_declaration::variable_declaration(variable * v,
		namespace_context ns, expression *init,
		location loc):
		var(v), ns(ns), init_exp(init)
{
	this->loc = loc;
	this->node_type_id = VARIABLE_DECL;
	if(init_exp) assert(init_exp->is_expression());
}

variable_declaration::~variable_declaration()
{
	cout << "Deleting variable declaration: " << to_string() << endl;
}

variable* variable_declaration::get_variable()
{
	return var;
}

const namespace_context& variable_declaration::get_namespace() const
{
	return ns;
}

bool variable_declaration::has_initializer()
{
	return init_exp != NULL;
}
void variable_declaration::set_initializer(expression* init)
{
	init_exp = init;
}

expression *variable_declaration::get_initializer()
{
	return init_exp;
}
void variable_declaration::print()
{
	cout << "vardecl: ";
	var->print();
	loc.print();
}

string variable_declaration::to_string() const
{
	string res = ns.to_string();
	res += var->get_type()->to_string();
	res += " ";
	res += var->to_string();
	if(init_exp != NULL) res+=" = " + init_exp->to_string();
	return res;

}

ostream& operator <<(ostream &os, const variable_declaration &obj)
{
      os  << obj.to_string();
      return os;
}

bool variable_declaration::is_fun_decl()
{
	return false;
}

bool variable_declaration::is_var_decl()
{
	return true;
}



}


