/*
 * AddressLabel.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "AddressLabel.h"
#include "Variable.h"
#include "node.h"
#include "function_address_expression.h"
#include <assert.h>

namespace sail {

AddressLabel::AddressLabel(Variable* v, string label, bool is_function,
		il::node* original, int line) {
	this->v = v;
	this->label=label;
	this->is_function = is_function;
	assert(original!=NULL);
	this->original = original;
	this->inst_id = ADDRESS_LABEL;
	this->line = line;
}

string AddressLabel::to_string() const
{
	string res = v->to_string() + " = &" + label;
	return res;
}
string AddressLabel::to_string(bool pretty_print) const
{
	if(!pretty_print) return to_string();
	string res = v->to_string(true) + " = &" + label;
	return res;
}

Variable* AddressLabel::get_lhs()
{
	return v;
}

void AddressLabel::set_lhs(Variable* v)
{
	this->v = v;
}

string AddressLabel::get_label()
{
	return label;
}
bool AddressLabel::is_function_label()
{
	return is_function;
}

il::type* AddressLabel::get_signature()
{
	if(this->original->node_type_id != FUNCTION_ADDRESS_EXP)
		return NULL;
	il::function_address_expression* fae = (il::function_address_expression*)
			original;
	return fae->get_function_signature();
}

bool AddressLabel::is_removable()
{
	return v->is_removable_temp();
}


AddressLabel::~AddressLabel() {

}

}
