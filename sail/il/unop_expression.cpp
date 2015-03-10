#include "unop_expression.h"

namespace il
{
unop_expression::unop_expression()
{

}
unop_expression::unop_expression(expression* inner_exp, unop_type unop,
		type* t, location loc)
{
	assert(inner_exp != NULL);
	assert(t!=NULL);
	this->inner_exp = inner_exp;
	this->unop = unop;
	this->t = t;
	this->loc = loc;
	this->node_type_id = UNOP_EXP;
}

string unop_expression::to_string() const
{
	string res = "";
	bool needs_paren = false;
	if(is_postop()) res+=inner_exp->to_string();
	res += unop_to_string(unop, &needs_paren);
	if(needs_paren) res+=")";
	if(!is_postop()) res+= inner_exp->to_string();
	return res;

}
expression* unop_expression::get_operand()
{
	return inner_exp;
}
unop_type unop_expression::get_operator()
{
	return unop;
}
bool unop_expression::is_vector_unop()
{
	return (unop == _VEC_UNOP);
}

bool unop_expression::is_postop() const
{
	return (unop == _POSTDECREMENT || unop == _POSTINCREMENT);
}


string unop_expression::unop_to_string(unop_type op,
		bool* need_close_paren = NULL)
{
	if(need_close_paren!=NULL) *need_close_paren = false;
	switch(op)
	{
		case _NEGATE:
			return "-";
		case _CONJUGATE:
		{
			if(need_close_paren!=NULL){
					*need_close_paren = true;
					return "conjugate(";
			}
			return "conjugate ";

		}
		case _PREDECREMENT:
		case _POSTDECREMENT:
			return "--";
		case _PREINCREMENT:
		case _POSTINCREMENT:
			return "++";
		case _BITWISE_NOT:
			return "~";
		case _LOGICAL_NOT:
			return "!";
		case _VEC_UNOP:
			return "VEC UNOP";
		default:
			return "UNKNOWN UNOP";

	}
}

bool unop_expression::has_side_effect()
{
	return (unop == _PREDECREMENT || unop == _PREINCREMENT ||
			unop == _POSTDECREMENT || unop == _POSTINCREMENT);
}

unop_expression::~unop_expression()
{
}


}
