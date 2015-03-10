#include "deref_expression.h"
#include "type.h"

namespace il
{

deref_expression::deref_expression()
{

}

deref_expression::deref_expression(expression* inner_exp, type* t, location loc)
{
	this->inner_exp = inner_exp;
	this->t = t;
	this->loc = loc;
	this->node_type_id = DEREF_EXP;
}
expression* deref_expression::get_inner_expression()
{
	return inner_exp;
}

string deref_expression::to_string() const
{
	return ("*(" + inner_exp->to_string() + ")");
}

deref_expression::~deref_expression()
{
}

}
