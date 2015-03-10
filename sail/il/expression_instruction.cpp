#include "expression_instruction.h"
#include "assert.h"
#include "expression.h"

namespace il
{

expression_instruction::expression_instruction()
{

}

expression_instruction::expression_instruction(expression* exp)
{
	assert(exp!=NULL);
	this->exp = exp;
	this->loc = exp->get_location();
	this->node_type_id = EXPR_INSTRUCTION;
}

string expression_instruction::to_string() const
{
	return exp->to_string() + "; ";
}
void expression_instruction::print()
{
	cout << this->to_string() << endl;
}

expression* expression_instruction::get_expression()
{
	return exp;
}



expression_instruction::~expression_instruction()
{
}

}
