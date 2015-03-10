/*
 * FieldRefWrite.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "FieldRefWrite.h"
#include "Variable.h"
#include "node.h"
#include "type.h"
#include "expression.h"
#include <assert.h>
#include "util.h"

namespace sail {

FieldRefWrite::FieldRefWrite(Variable* v1, Symbol* v2, int offset,
		il::node* original, int line) {
	this->v1 = v1;
	this->v2 = v2;
	this->offset = offset;
	this->original = original;
	this->inst_id = FIELD_REF_WRITE;

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

FieldRefWrite::FieldRefWrite(Variable* v1, Symbol* v2,
		il::record_info* ri, il::node* original, int line)
{
	this->v1 = v1;
	this->v2 = v2;
	this->offset = ri->offset;
	this->field_name = ri->fname;
	this->original = original;
	this->inst_id = FIELD_REF_WRITE;
	this->line =line;
}

FieldRefWrite::FieldRefWrite(Variable* v1, Symbol* v2,
			string & name, int offset, il::node* original, int line)
{
	this->v1 = v1;
	this->v2 = v2;
	this->offset = offset;
	this->field_name = name;
	this->original = original;
	this->inst_id = FIELD_REF_WRITE;
	this->line = line;
}

string FieldRefWrite::get_field_name()
{
	return field_name;
}

bool FieldRefWrite::is_removable()
{
	return false;
}

string FieldRefWrite::to_string() const
{
	string res = v1->to_string() + "." + field_name;
	res += " = " + v2->to_string();
	return res;
}

string FieldRefWrite::to_string(bool pretty_print) const
{
	string res = v1->to_string( pretty_print) + "." + field_name;
	res += " = " + v2->to_string( pretty_print);
	return res;
}

Variable* FieldRefWrite::get_lhs()
{
	return v1;
}

Symbol* FieldRefWrite::get_rhs()
{
	return v2;
}

int FieldRefWrite::get_offset()
{
	return offset;
}



FieldRefWrite::~FieldRefWrite() {

}

}
