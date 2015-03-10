/*
 * StringLiteral.cpp
 *
 *  Created on: Feb 3, 2010
 *      Author: isil
 */

#include "StringLiteral.h"
#include "il/type.h"
#include "ConstantValue.h"

map<string, int> StringLiteral::string_to_id;
int StringLiteral::counter = 0;


StringLiteral::StringLiteral(const string& s):
	FunctionTerm(CNode::get_varmap().get_id("str_const"),
		ConstantValue::make(get_id(s))->to_term(), true,
		ATTRIB_STRING_CONST)
{
	this->t = il::pointer_type::make(il::get_char_type(), "");
	this->s = s;
	this->has_index_var = false;
	this->apt = AP_STRING;
	this->specialization_type = AP_STRING;
	add_signedness_attribute();
}

StringLiteral* StringLiteral::make(const string& string_literal)
{
	StringLiteral* s =new StringLiteral(string_literal);
	return (StringLiteral*) FunctionTerm::get_term(s);
}
const string& StringLiteral::get_string_constant()
{
	return s;
}
string StringLiteral::to_string()
{
	if(PRINT_AS_TERM) {
		return FunctionTerm::to_string();
	}
	return "string(\\\"" + s + "\\\")";
	//return FunctionTerm::to_string();
}
Term* StringLiteral::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	return this;
}
Term* StringLiteral::substitute(Term* (*sub_func)(Term* t, void* data),
			void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	return this;
}

int StringLiteral::get_id(const string& s)
{
	if(string_to_id.count(s) > 0) return string_to_id[s];
	string_to_id[s] = counter;
	return counter++;
}

void StringLiteral::clear()
{
	string_to_id.clear();
	counter = 0;
}

StringLiteral::~StringLiteral()
{

}
