#include "throw_expression.h"
#include "type.h"

namespace il
{

throw_expression::throw_expression()
{

}

throw_expression::throw_expression(type *thrown_type,
                                   expression *full_throw_expr,
                                   type *t,
                                   location loc)
	: thrown_type(thrown_type),
	  full_throw_expr(full_throw_expr)
{
	this->t = t;
	this->loc = loc;
	this->node_type_id = THROW_EXP;
}

throw_expression::~throw_expression()
{
}

string throw_expression::to_string() const
{
	return "throw [" + thrown_type->to_string() + "] " + full_throw_expr->to_string();
}

type *throw_expression::get_thrown_type()
{
	return thrown_type;
}

expression *throw_expression::get_full_throw_expression()
{
	return full_throw_expr;
}



}
