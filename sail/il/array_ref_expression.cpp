#include "array_ref_expression.h"

namespace il
{
array_ref_expression::array_ref_expression()
{

}

array_ref_expression::array_ref_expression(expression* array_exp,
		expression* index_exp, type* t, location loc)
{
	assert(array_exp!=NULL);
	assert(index_exp != NULL);
	assert(t!=NULL);
	this->array_exp = array_exp;
	this->index_exp = index_exp;
	this->t = t;
	this->loc = loc;
	this->node_type_id = ARRAY_REF_EXP;
}

string array_ref_expression::to_string() const
{
	return (array_exp->to_string() + "[" + index_exp->to_string() + "]");
}
expression* array_ref_expression::get_array_expression()
{
	return array_exp;
}
expression* array_ref_expression::get_index_expression()
{
	return index_exp;
}

array_ref_expression::~array_ref_expression()
{
}



}
