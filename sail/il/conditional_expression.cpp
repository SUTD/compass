#include "conditional_expression.h"
#include "type.h"

namespace il
{

conditional_expression::conditional_expression()
{

}

conditional_expression::conditional_expression(expression* cond,
		expression* then_clause, expression* else_clause, type* t, location loc)
{
	assert(cond!=NULL);
	assert(then_clause != NULL);
	assert(else_clause != NULL);
	assert(t!=NULL);
	this->cond= cond;
	this->then_clause = then_clause;
	this->else_clause = else_clause;
	this->t = t;
	this->loc = loc;
	this->node_type_id = CONDITIONAL_EXP;
}

string conditional_expression::to_string() const
{
	string res = cond->to_string() + " ? ";
	res+= then_clause->to_string() + " : ";
	res+= else_clause->to_string();
	return res;
}
expression* conditional_expression::get_conditional()
{
	return cond;
}
expression* conditional_expression::get_then_clause()
{
	return then_clause;
}
expression* conditional_expression::get_else_clause()
{
	return else_clause;
}


conditional_expression::~conditional_expression()
{
}

}
