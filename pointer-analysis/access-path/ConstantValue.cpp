/*
 * ConstantValue.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "ConstantValue.h"
#include "ConstantTerm.h"
#include "Term.h"
#include "util.h"

ConstantValue* ConstantValue::make(long int value)
{
	ConstantValue* cv = new ConstantValue(value);
	return (ConstantValue*)ConstantTerm::get_term(cv);
}

ConstantValue::ConstantValue(long int value):ConstantTerm(value)
{
	this->t = il::get_integer_type();
	this->has_index_var = false;
	this->apt = AP_CONSTANT;
	this->specialization_type = AP_CONSTANT;
}


Term* ConstantValue::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	return this;
}

Term* ConstantValue::substitute(Term* (*sub_func)(Term* t, void* data),
		void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	return this;
}


ConstantValue::~ConstantValue() {
}
