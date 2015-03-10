/*
 * DropTemporary.cpp
 *
 *  Created on: Oct 6, 2008
 *      Author: tdillig
 */

#include "DropVariable.h"
#include "Variable.h"

namespace sail{

	DropVariable::DropVariable() {
		this->var = NULL;
		this->temp = false;
		this->original = NULL;
		this->inst_id = DROP_TEMPORARY;

	}
	DropVariable::DropVariable(Variable* var, bool temp) {
		this->var = var;
		this->temp = temp;
		this->original = NULL;
		this->inst_id = DROP_TEMPORARY;

	}

	Variable* DropVariable::get_var()
	{
		return var;
	}

	bool DropVariable::is_temp()
	{
		return temp;
	}

	bool DropVariable::is_removable()
	{
		return false;
	}

	string DropVariable::to_string() const
	{
		string res = "drop_ref(" + var->to_string() + ")" ;
		return res;
	}
	string DropVariable::to_string(bool pp) const
	{
		if(pp) return "";
		string res = "drop_ref(" + var->to_string(false) + ")" ;
		return res;

	}

	DropVariable::~DropVariable() {

	}
}
