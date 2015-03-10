/*
 * Nil.cpp
 *
 *  Created on: Feb 9, 2010
 *      Author: tdillig
 */

#include "Nil.h"

Nil::Nil():VariableTerm(CNode::get_varmap().get_id("Nil"), ATTRIB_NIL)
{
	this->t = il::get_integer_type();
	this->apt = AP_NIL;
	this->specialization_type = AP_NIL;
	this->has_index_var = false;
	add_signedness_attribute();

}

Nil* Nil::make()
{
	Nil* nil = new Nil();
	return (Nil*) Term::get_term(nil);
}
string Nil::to_string()
{
	if(PRINT_AS_TERM) {
		return VariableTerm::to_string();
	}
	return "NIL";
}

Term* Nil::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	return this;
}
Term* Nil::substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	return this;
}

Nil::~Nil()
{

}
