/*
 * modify_expression.cpp
 *
 *  Created on: Jun 27, 2008
 *      Author: isil
 */

#include "modify_expression.h"
#include "set_instruction.h"

namespace il {

modify_expression::modify_expression()
{

}

modify_expression::modify_expression(set_instruction* s, bool is_initalizing, location loc)
	: s(s),
	  is_initalizing(is_initalizing)
{
	assert(s!=NULL);
	this->t = s->get_lvalue()->get_type();
	this->loc = loc;
	this->node_type_id = MODIFY_EXP;
}

string modify_expression::to_string() const
{
	string res = "";
	string stmt_str = s->to_string();
	res = stmt_str.substr(0, stmt_str.size()-2);
	return res;

}

set_instruction* modify_expression::get_set_instruction()
{
	return s;
}



modify_expression::~modify_expression() {

}

bool modify_expression::is_initalizing_exp()
{
	return is_initalizing;
}

}
