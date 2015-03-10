#include "cast_expression.h"
#include "type.h"

namespace il
{

cast_expression::cast_expression()
{

}

cast_expression::cast_expression(expression* exp, type* t, location loc)
{
	assert(!t->is_void_type());
	this->exp = exp;
	this->t = t;
	this->loc = loc;
	this->node_type_id = CAST_EXP;
}

expression* cast_expression::get_inner_expression()
{
	return exp;
}

string cast_expression::to_string() const
{
	string res = "(";
	res += t->to_string() + ")";
	res+= exp->to_string();
	return res;
}




cast_expression::~cast_expression()
{
}

}
