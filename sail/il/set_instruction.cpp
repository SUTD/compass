#include "set_instruction.h"
#include "expression.h"

namespace il
{

set_instruction::set_instruction()
{

}


set_instruction::set_instruction(expression* lvalue, expression* rhs,
		location loc, bool declaration)
{
	assert(lvalue!=NULL);
	assert(lvalue->is_expression());
	assert(rhs!=NULL);
	assert(rhs->is_expression());
	this->lvalue = lvalue;
	this->rhs = rhs;
	this->loc = loc;
	this->is_declaration = declaration;
	this->node_type_id = SET_INSTRUCTION;


	//if(to_string().find("foo::x = 0;") != string::npos) {
		//assert(false);
	//}

}
expression* set_instruction::get_lvalue()
{
	return lvalue;
}
expression* set_instruction::get_rhs()
{
	return rhs;
}
void set_instruction::print()
{
	cout << "Set instruction" <<endl;
}

bool set_instruction::is_declaration_inst()
{
	return is_declaration;
}


set_instruction::~set_instruction()
{
}

string set_instruction::to_string() const
{
	string res = lvalue->to_string() + " = " + rhs->to_string();
	res += "; "; /* DO NOT CHANGE THIS! Otherwise, modify_expression will break */
	return res;
}

ostream& operator <<(ostream &os, const set_instruction &obj)
{
      os  << obj.to_string();
      return os;
}

}
