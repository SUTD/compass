/*
 * FieldRefRead.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "FieldRefRead.h"
#include "Variable.h"
#include "node.h"
#include "type.h"
#include "expression.h"
#include <assert.h>

namespace sail {

FieldRefRead::FieldRefRead(Variable* v1, Variable* v2,
		int offset, il::node* original, int line)
{
	this->v1 = v1;
	this->v2 = v2;
	this->offset = offset;
	assert(original != NULL);
	this->original = original;
	this->inst_id = FIELD_REF_READ;

	il::node* n = v2->get_original_node();
	il::expression* e = (il::expression*) n;
	e->assert_expression();
	il::type* t = e->get_type();
	assert(t->is_record_type());
	il::record_type* rt = (il::record_type*) t;
	il::record_info* ri = rt->get_field_from_offset(offset);
	assert(ri != NULL);
	this->field_name = ri->fname;
	this->line = line;

}

FieldRefRead::FieldRefRead(Variable* v1, Variable* v2, il::record_info* ri,
		il::node* original, int line)
{
	this->v1 = v1;
	this->v2 = v2;
	this->offset = ri->offset;
	this->field_name = ri->fname;
	assert(original != NULL);
	this->original = original;
	this->inst_id = FIELD_REF_READ;
	this->line = line;
}

FieldRefRead::FieldRefRead(Variable* v1, Variable* v2, int offset,
		string field_name, il::node* original, int line)
{
	this->v1 = v1;
	this->v2 = v2;
	this->offset = offset;
	this->field_name = field_name;
	assert(original != NULL);
	this->original = original;
	this->inst_id = FIELD_REF_READ;
	this->line = line;
}

string FieldRefRead::get_field_name()
{
	return field_name;
}

string FieldRefRead::to_string() const
{
	string res = v1->to_string();
	res += " = " + v2->to_string() + "." + field_name;
	return res;
}

string FieldRefRead::to_string(bool pretty_print) const
{
	string res = v1->to_string(pretty_print);
	res += " = " + v2->to_string(pretty_print) + "." + field_name;
	return res;
}

bool FieldRefRead::is_removable()
{
	return v1->is_removable_temp();
}

Variable* FieldRefRead::get_lhs()
{
	return v1;
}
void FieldRefRead::set_lhs(Variable* v)
{
	this->v1 = v;
}

Variable* FieldRefRead::get_rhs()
{
	return v2;
}

int FieldRefRead::get_offset()
{
	return offset;
}


FieldRefRead::~FieldRefRead() {

}

}
