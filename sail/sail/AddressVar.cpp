/*
 * AddressVar.cpp
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 */

#include "AddressVar.h"
#include "Variable.h"
#include "Symbol.h"
#include "util.h"
#include "node.h"
#include <assert.h>

namespace sail {

AddressVar::AddressVar(Variable* v, Symbol* s, il::node* original, int line) {
	this->v = v;
	this->s = s;
	this->original = original;
	this->inst_id = ADDRESS_VAR;
	this->line = line;

}

string AddressVar::to_string() const
{
	string res = v->to_string();
	res += " = & (" + s->to_string() + ")";
	return res;
}

string AddressVar::to_string(bool pretty_print) const
{
	if(!pretty_print) return to_string();
	string res = v->to_string(true);
	res += " = & (" + s->to_string(true) + "(";
	return res;
}


Variable* AddressVar::get_lhs()
{
	return v;
}

void AddressVar::set_lhs(Variable* v)
{
	this->v = v;
}


Symbol* AddressVar::get_rhs()
{
	return s;
}

bool AddressVar::is_removable()
{
	return v->is_removable_temp();
}



AddressVar::~AddressVar() {

}

}
