#include "field_ref_expression.h"
#include "type.h"

namespace il
{

field_ref_expression::field_ref_expression()
{

}

field_ref_expression::field_ref_expression(expression* inner_exp,
		record_info* field_info, type* t, location loc)
{
	assert(inner_exp != NULL);
	assert(field_info!=NULL);
	assert(t!=NULL);
	this->inner_exp = inner_exp;
	this->field_info = field_info;
	this->t = t;
	this->loc = loc;
	this->node_type_id = FIELD_REF_EXP;
}

string field_ref_expression::to_string() const
{
	return (inner_exp->to_string() + "." + field_info->fname);
}


expression* field_ref_expression::get_inner_expression()
{
	return inner_exp;
}
record_info* field_ref_expression::get_field_info()
{
	return field_info;
}



field_ref_expression::~field_ref_expression()
{
}

}
