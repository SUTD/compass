/*
 * Deref.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "Deref.h"
#include "util.h"
#include "compass_assert.h"

Deref* Deref::make(AccessPath* inner, il::type* t)
{
	assert_context("Making deref of " + safe_string(inner) );



	Deref* d = new Deref(inner, t);
	Term* res =   FunctionTerm::get_term(d);
	//cout << "Deref " << ((Deref*)res)->to_string() << endl;
 	return (Deref*) res;
}

Deref::Deref(AccessPath* inner, il::type* t): FunctionTerm(
		CNode::get_varmap().get_id("drf"), inner->to_term(), false) {
	if(t!=NULL) this->t = t;
	else{
		il::type* inner_type = inner->get_type();
		c_assert(inner_type != NULL);
		while(inner_type->is_array_type()) {
			inner_type = inner_type->get_inner_type();
		}
		if(inner_type->is_pointer_type()){
			this->t = ((il::pointer_type*)inner_type)->get_deref_type();
		}
		else {
			this->t = il::get_integer_type();
		}
	}
	this->has_index_var = inner->contains_index_var();
	this->specialization_type = AP_DEREF;
	this->apt = AP_DEREF;
	this->inner = inner;

	add_signedness_attribute();

}

string Deref::to_string()
{
	if(PRINT_AS_TERM)
	{
		return FunctionTerm::to_string();
	}
	string res = "*(" + inner->to_string() +")";
	return res;
}
AccessPath* Deref::get_inner()
{
	return inner;
}

Term* Deref::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	Term* inner_t = inner->to_term()->substitute(subs);
	AccessPath* new_inner= AccessPath::to_ap(inner_t);
	if(new_inner == inner) return this;
	return Deref::make(new_inner, t);
}


Term* Deref::substitute(Term* (*sub_func)(Term* t, void* data), void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	Term* inner_t = inner->to_term()->substitute(sub_func, my_data);
	AccessPath* new_inner= AccessPath::to_ap(inner_t);
	if(new_inner == inner) return this;
	return Deref::make(new_inner, t);
}


Deref::~Deref() {
}
