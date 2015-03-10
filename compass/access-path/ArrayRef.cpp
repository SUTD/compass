/*
 * ArrayRef.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "ArrayRef.h"
#include "util.h"
#include "IndexVariable.h"
#include "Constraint.h"
#include "ConstraintGenerator.h"
#include "ArithmeticValue.h"
#include "compass_assert.h"

ArrayRef* ArrayRef::make(AccessPath* inner, AccessPath* index_var,
		il::type* t, int elem_size)
{
	assert_context("Making array ref: " + safe_string(inner) + " type: " +
			(t ? t->to_string() : "null") +
			" elem size: " + int_to_string(elem_size));
	c_assert(index_var != NULL);
	ArrayRef* ar = new ArrayRef(inner, index_var, false, t, elem_size);
	ArrayRef* res= (ArrayRef*) FunctionTerm::get_term(ar);
	return res;
}

ArrayRef* ArrayRef::make_stringconst(AccessPath* inner, AccessPath* index_var,
		il::type* t)
{
	ArrayRef* ar = new ArrayRef(inner, index_var, true, t, 1);
	return (ArrayRef*) FunctionTerm::get_term(ar);
}

ArrayRef* ArrayRef::make(AccessPath* inner, AccessPath* index_var, int elem_size)
{
	ArrayRef* ar = new ArrayRef(inner, index_var, false, elem_size);
	return (ArrayRef*) FunctionTerm::get_term(ar);
}

bool ArrayRef::is_string_constant()
{
	if(inner->get_ap_type() != AP_VARIABLE) return false;
	return ((Variable*) inner)->is_string_const();
}
ArrayRef::ArrayRef(AccessPath* inner, AccessPath* index_var,
		bool is_string_const, il::type* t, int elem_size):
	FunctionTerm(CNode::get_varmap().get_id("array_" +int_to_string(elem_size)),
			inner->to_term(),
			index_var->to_term(), false)
{

	c_assert(index_var != NULL);

	while(inner->get_ap_type() == AP_ARRAYREF)
	{
		ArrayRef* ar = (ArrayRef*)inner;
		index_var = ArithmeticValue::make_plus(index_var,
				ar->get_index_expression());
		inner = ar->get_inner();
	}
	c_assert(inner->get_ap_type() != AP_ARRAYREF);
	this->inner = inner;
	this->index_var = index_var;
	this->t = t;
	this->has_index_var = true;
	this->specialization_type =AP_ARRAYREF;
	this->apt = AP_ARRAYREF;
	this->is_string_const = is_string_const;
	c_assert(elem_size != 0);
	this->elem_size = elem_size;

	AccessPath* ap = this;
	c_assert(ap->t != NULL);
	c_assert(get_type() != NULL);

	add_signedness_attribute();
}

ArrayRef::ArrayRef(AccessPath* inner, AccessPath* index_var,
		bool is_string_const, int elem_size):
	FunctionTerm(CNode::get_varmap().get_id("array_"+int_to_string(elem_size)),
			inner->to_term(),
			index_var->to_term(), false)
{

	c_assert(index_var != NULL);
	while(inner->get_ap_type() == AP_ARRAYREF)
	{
		ArrayRef* ar = (ArrayRef*)inner;
		index_var = ArithmeticValue::make_plus(index_var,
				ar->get_index_expression());
		inner = ar->get_inner();
	}
	c_assert(inner->get_ap_type() != AP_ARRAYREF);
	this->inner = inner;
	this->index_var = index_var;
	this->has_index_var = true;

	il::type* inner_type = inner->get_type();
	if(inner_type->is_array_type()) {
		il::array_type* at = (il::array_type*) inner_type;
		inner_type = at->get_elem_type();
	}
	t=inner_type;
	this->specialization_type =AP_ARRAYREF;
	this->apt = AP_ARRAYREF;
	this->is_string_const = is_string_const;
	c_assert(elem_size != 0);
	this->elem_size = elem_size;
	AccessPath* ap = this;
	c_assert(ap->t != NULL);
	c_assert(get_type() != NULL);
	add_signedness_attribute();
}



AccessPath* ArrayRef::get_index_expression()
{
	return index_var;
}
Term* ArrayRef::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	Term* new_inner_t = inner->to_term()->substitute(subs);
	AccessPath* new_inner = AccessPath::to_ap(new_inner_t);
	Term* new_index_t = index_var->to_term()->substitute(subs);
	AccessPath* new_index = AccessPath::to_ap(new_index_t);
	if(new_index == NULL)
	{
		cout << "old index: " << index_var->to_string() << endl;
		map<Term*, Term*>::iterator it = subs.begin();
		for(; it != subs.end(); it++)
		{
			cout << it->first->to_string() << " -> " <<
					(it->second == NULL? "null" : it->second->to_string()) << endl;

		}
	}
	if(new_inner == inner && new_index == index_var) return this;
	return ArrayRef::make(new_inner, new_index, t, get_elem_size());
}


Term* ArrayRef::substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	Term* new_inner_t = inner->to_term()->substitute(sub_func, my_data);
	AccessPath* new_inner = AccessPath::to_ap(new_inner_t);
	Term* new_index_t = index_var->to_term()->substitute(sub_func, my_data);
	AccessPath* new_index = AccessPath::to_ap(new_index_t);
	if(new_inner == inner && new_index == index_var) return this;
	return ArrayRef::make(new_inner, new_index, t, get_elem_size());
}

int ArrayRef::get_elem_size()
{
	return elem_size;
}


string ArrayRef::to_string()
{
	if(PRINT_AS_TERM) {
		return FunctionTerm::to_string();
	}
	string res = inner->to_string();
	res += "[" + index_var->to_string()+ "]";
	return res;
}



AccessPath* ArrayRef::get_inner()
{
	return inner;
}


IndexVariable* ArrayRef::get_index_var()
{
	c_assert(index_var->get_ap_type() == AP_INDEX);
	return (IndexVariable*)index_var;
}



ArrayRef::~ArrayRef()
{
}
