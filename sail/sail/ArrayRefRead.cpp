/*
 * ArrayRefRead.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "ArrayRefRead.h"
#include "Variable.h"
#include "Symbol.h"
#include "node.h"
#include "assert.h"
#include "util.h"

namespace sail {

ArrayRefRead::ArrayRefRead(Variable* v1, Variable* v2,
		Symbol* index, int offset, string name, il::node* original, int line) {
	this->v1 = v1;
	this->v2 = v2;
	this->index = index;
	assert(original != NULL);
	this->original = original;
	this->inst_id = ARRAY_REF_READ;
	this->offset = offset;
	this->name = name;
	this->line = line;
}

string ArrayRefRead::to_string() const
{
	string res = v1->to_string() + " = ";
	res += v2->to_string() + "[";
	res += index->to_string() + "]";
	if(offset == 0 && name == "") return res;
	if(name != "") return res + "." +name;
	return (res + ".off(" + int_to_string(offset) + ")");
}
string ArrayRefRead::to_string(bool pretty_print) const
{
	if(!pretty_print) return to_string();
	string res = v1->to_string(true) + " = ";
	res += v2->to_string(true) + "[";
	res += index->to_string(true) + "]";
	if(offset == 0 && name == "") return res;
	if(name != "") return res + "." +name;
	return (res + ".off(" + int_to_string(offset) + ")");

}

int ArrayRefRead::get_offset()
{
	return offset;
}
string ArrayRefRead::get_field_name()
{
	return name;
}

Variable* ArrayRefRead::get_lhs()
{
	return v1;
}

void ArrayRefRead::set_lhs(Variable* v)
{
	this->v1 = v;
}

Variable* ArrayRefRead::get_rhs()
{
	return v2;
}
Symbol* ArrayRefRead::get_index()
{
	return index;
}

bool ArrayRefRead::is_removable()
{
	return v1->is_removable_temp();
}


ArrayRefRead::~ArrayRefRead() {

}

}
