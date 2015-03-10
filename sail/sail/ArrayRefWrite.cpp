/*
 * ArrayRefWrite.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "ArrayRefWrite.h"
#include "Variable.h"
#include "Symbol.h"
#include "node.h"
#include "assert.h"
#include "util.h"

namespace sail {

ArrayRefWrite::ArrayRefWrite(Variable* v1, Symbol* v2, Symbol* index,
		int offset, string name, il::node* original, int line) {
	this->v1 = v1;
	this->v2 = v2;
	this->index = index;
	this->original = original;
	this->inst_id = ARRAY_REF_WRITE;
	this->offset = offset;
	this->name = name;
	this->line = line;
}

int ArrayRefWrite::get_offset()
{
	return offset;
}
string ArrayRefWrite::get_field_name()
{
	return name;
}

string ArrayRefWrite::to_string() const
{
	string res = v1->to_string()
			+ "[" + index->to_string() + "]" ;
	if(name!="") res += "." + name;
	else if(offset != 0) res += ".off("+int_to_string(offset)+")";
	res += " = ";
	res += v2->to_string();
	return res;
}

string ArrayRefWrite::to_string(bool pretty_print) const
{
	string res = v1->to_string(pretty_print)
			+ "[" + index->to_string(pretty_print) + "]" ;
	if(name!="") res += "." + name;
	else if(offset != 0) res += ".off("+int_to_string(offset)+")";
	res += " = ";
	res += v2->to_string(pretty_print);
	return res;
}

Variable* ArrayRefWrite::get_lhs()
{
	return v1;
}
Symbol* ArrayRefWrite::get_rhs()
{
	return v2;
}
Symbol* ArrayRefWrite::get_index()
{
	return index;
}

bool ArrayRefWrite::is_removable()
{
	return false;
}



ArrayRefWrite::~ArrayRefWrite() {

}

}
