#include "addressof_expression.h"
#include "type.h"

namespace il
{

addressof_expression::addressof_expression()
{

}

addressof_expression::addressof_expression(expression* inner_exp,
		type* t, location loc)
{
	this->inner_exp = inner_exp;
	if(t == NULL) t = il::pointer_type::make(inner_exp->get_type(), "");
	this->t = t;
	this->loc = loc;
	this->node_type_id = ADDRESSOF_EXP;
}

string addressof_expression::to_string() const
{
	return ("&" + inner_exp->to_string());
}

expression* addressof_expression::get_inner_expression()
{
	return inner_exp;
}

addressof_expression::~addressof_expression()
{
}



}
