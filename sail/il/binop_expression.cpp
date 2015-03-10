#include "binop_expression.h"

namespace il
{

binop_expression::binop_expression()
{

}

binop_expression::binop_expression(expression* exp1, expression* exp2,
		binop_type op, type* t, location loc)
{
	assert(exp1 != NULL);
	assert(exp2!=NULL);
	this->exp1 = exp1;
	this->exp2 = exp2;
	this->op = op;
	this->t = t;
	this->loc = loc;
	this->node_type_id = BINOP_EXP;
}

string binop_expression:: to_string() const
{
	string res = exp1->to_string();
	res+= binop_to_string(op);
	res+= (exp2->to_string());
	return res;
}
binop_type binop_expression::get_operator()
{
	return op;
}
expression* binop_expression::get_first_operand()
{
	return exp1;
}
expression* binop_expression::get_second_operand()
{
	return exp2;
}

binop_expression::~binop_expression()
{
}

bool binop_expression::is_vector_binop()
{
	return (op==_VEC_BINOP);
}

void binop_expression::set_first_operand(expression* op1)
{
	this->exp1 = op1;
}
void binop_expression::set_second_operand(expression* op2)
{
	this->exp2 = op2;
}

string binop_expression::binop_to_string(binop_type op)
{
	switch(op)
	{
	case _PLUS:
	case _POINTER_PLUS:
		return "+";
	case _MINUS:
		return "-";
	case _MULTIPLY:
		return "*";
	case _DIV:
	case _REAL_DIV:
		return "/";
	case _MOD:
		return "%";
	case _LT:
		return "<";
	case _LEQ:
		return "<=";
	case _GT:
		return ">";
	case _GEQ:
		return ">=";
	case _EQ:
		return "==";
	case _NEQ:
		return "!=";
	case _LEFT_SHIFT:
		return "<<";
	case _RIGHT_SHIFT:
		return ">>";
	case _BITWISE_OR:
		return "|";
	case _BITWISE_AND:
		return "&";
	case _BITWISE_XOR:
		return "^";
	case _LOGICAL_AND:
		return "&&";
	case _LOGICAL_OR:
		return "||";
	case _LOGICAL_AND_NO_SHORTCIRCUIT:
		return "&& (No shortcircuit)";
	case _LOGICAL_OR_NO_SHORTCIRCUIT:
		return "|| (No shortcircuit)";
	case _VEC_BINOP:
		return "VEC_BINOP";
	default:
		return "UNKNOWN_BINOP";

	}
}

bool binop_expression::is_predicate_binop(il::binop_type bt)
{
	switch(bt)
	{
		case il::_LT:
		case il::_LEQ:
		case il::_GT:
		case il::_GEQ:
		case il::_EQ:
		case il::_NEQ:
		case _LOGICAL_AND:
		case _LOGICAL_OR:
		case _LOGICAL_AND_NO_SHORTCIRCUIT:
		case _LOGICAL_OR_NO_SHORTCIRCUIT:
			return true;
		default:
			return false;
	}
}



}
