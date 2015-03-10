/*
 * TypeConstant.cpp
 *
 *  Created on: Apr 22, 2010
 *      Author: isil
 */

#include "TypeConstant.h"
#include "util.h"
#include "il/type.h"
#include "Constraint.h"
#include "ConstraintGenerator.h"

map<string, int> TypeConstant::type_to_id;
int TypeConstant::counter = 0;
set<TypeConstant*> TypeConstant::added_axioms;

TypeConstant::TypeConstant(il::type* t):
	FunctionTerm(CNode::get_varmap().get_id("type_constant"),
		ConstantValue::make(get_id(t))->to_term(), true,
		ATTRIB_TYPE_CONSTANT)
{
	this->type_constant = t;
	this->t = il::get_integer_type();
	this->has_index_var = false;
	this->apt = AP_TYPE_CONSTANT;
	this->specialization_type = AP_TYPE_CONSTANT;
	add_signedness_attribute();
}

TypeConstant* TypeConstant::make(il::type* t)
{
	TypeConstant* tc = new TypeConstant(t);
	TypeConstant* res=  (TypeConstant*) TypeConstant::get_term(tc);
	add_subtype_axioms(res);
	return res;
}

il::type* TypeConstant::get_type_constant() const
{
	return type_constant;
}

string TypeConstant::to_string()
{
	if(PRINT_AS_TERM) {
		FunctionTerm::to_string();
	}
	return type_constant->to_string();
}

Term* TypeConstant::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	return this;
}

Term* TypeConstant::substitute(Term* (*sub_func)(Term* t, void* data),
			void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	return this;
}

int TypeConstant::get_id(il::type* t)
{
	string key = t->to_string();
	if(type_to_id.count(key) > 0) {
		return type_to_id[key];
	}
	type_to_id[key] = counter;
	return counter++;
}

void TypeConstant::clear()
{
	type_to_id.clear();
	counter = 0;
	added_axioms.clear();
}

void TypeConstant::add_all_axioms()
{
	set<TypeConstant*>::iterator it = added_axioms.begin();
	for(; it!= added_axioms.end(); it++) {
		TypeConstant* tc = *it;
		add_subtype_axioms_internal(tc);
	}
}

void TypeConstant::add_subtype_axioms_internal(TypeConstant* tc)
{
	il::type* t = tc->get_type_constant();

	if(!t->is_record_type()) return;
	il::record_type* rt = (il::record_type*) t;

	set<il::record_type*> subclasses;
	rt->get_transitive_subclasses(subclasses);


	set<il::record_type*>::iterator it = subclasses.begin();
	for(; it!= subclasses.end(); it++) {
		il::record_type* cur = *it;
		TypeConstant* cur_subtype = TypeConstant::make(cur);
		Constraint subtype_c =
				ConstraintGenerator::get_subtype_of_constraint(cur_subtype, tc);

		Constraint::set_background_knowledge(subtype_c);


	}
}


void TypeConstant::add_subtype_axioms(TypeConstant* tc)
{
	if(added_axioms.count(tc) > 0) return;
	added_axioms.insert(tc);
	add_subtype_axioms_internal(tc);


}


TypeConstant::~TypeConstant()
{

}
