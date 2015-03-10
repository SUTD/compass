/*
 * AddressString.cpp
 *
 *  Created on: Sep 22, 2008
 *      Author: tdillig
 */

#include "AddressString.h"
#include "string_const_exp.h"
#include "Variable.h"
namespace sail{


AddressString::AddressString(Variable* v, il::string_const_exp* ce,
		il::node* original, int line) {
	this->v = v;
	this->ce = ce;
	this->original = original;
	this->inst_id = ADDRESS_STRING;
	this->line = line;

}

string AddressString::get_string()
{
	return ce->get_string();
}

string AddressString::to_string() const
{
	string res = v->to_string();
	res += " = &" + ce->to_string();
	return res;
}

string  AddressString::to_string(bool pretty_print) const
{
	if(!pretty_print) return to_string();
	string res = v->to_string(true);
	res += " = &" + ce->to_string();
	//haiyan added to print line num
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

Variable* AddressString::get_lhs()
{
	return v;
}

void AddressString::set_lhs(Variable* v)
{
	this->v = v;
}

il::string_const_exp* AddressString::get_rhs()
{
	return this->ce;
}

bool AddressString::is_removable()
{
	return v->is_removable_temp();
}


AddressString::~AddressString() {

}
}
