/*
 * FieldSelection.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "FieldSelection.h"
#include "util.h"
#include "MemoryGraph.h"
#include "Deref.h"
#include "compass_assert.h"
#include "ArrayRef.h"
#include "ConstantValue.h"



#define FIELD_PREFIX "field"

#define GET_FUN_ID(offset) (CNode::get_varmap().get_id(FIELD_PREFIX + \
		(offset<0 ? ("_minus_" + int_to_string(-offset)) :\
				int_to_string(offset))+"_"+int_to_string(t->get_size())))

FieldSelection* FieldSelection::make(const string& field_name,
		int field_offset, il::type* t, AccessPath* inner,
		AccessPath* index_var)
{
	assert_context("Making field selection: " + safe_string(inner) +
			" field name: " + field_name + " offset: " +
			int_to_string(field_offset));



	/*if(inner->get_ap_type() == AP_DEREF &&
			inner->get_inner()->get_ap_type() == AP_ADT && field_offset < 0) {
		AccessPath* inner_inner = inner->get_inner();
		inner_inner = inner_inner->get_inner();
		inner = inner_inner->add_deref();
	}*/


	if(field_offset == SIZE_OFFSET)
	{


		if(inner->get_ap_type() == AP_ARRAYREF) {
			ArrayRef* ar = (ArrayRef*) inner;
			AccessPath* i = ar->get_index_expression();
			if(i->get_ap_type() != AP_INDEX) {
				inner = ArrayRef::make(ar->get_inner(),
						ConstantValue::make(0), ar->get_type(),
						ar->get_elem_size());
			}
		}
	}


	if(field_offset < 0) {
		inner= inner->get_representative_for_fake_field();
	}


	c_assert(t != NULL);
	FieldSelection* res;
	if(index_var == NULL)
		res = new FieldSelection(field_name, field_offset, inner, t);
	else
		res = new FieldSelection(field_name, field_offset, inner, index_var, t);

	//delete me
//	c_assert(res->to_string().find("x.x") == string::npos);
	//delete me end
	return (FieldSelection*) FunctionTerm::get_term(res);
}


FieldSelection* FieldSelection::make_anonymous(int field_offset,
		il::type* t, AccessPath* inner)
{
	c_assert(t != NULL);
	FieldSelection* res = new FieldSelection(field_offset, inner, t);
	return (FieldSelection*) FunctionTerm::get_term(res);
}

FieldSelection* FieldSelection::make(const string & field_name,
		int field_offset,  AccessPath* inner,
		AccessPath* index_var)
{
	FieldSelection* res;
	il::type* t = make_type(inner, field_offset);
	if(index_var == NULL)
		res = new FieldSelection(field_name, field_offset, inner,  t);
	else
		res = new FieldSelection(field_name, field_offset, inner, index_var,
				t);
	return (FieldSelection*) FunctionTerm::get_term(res);
}

FieldSelection::FieldSelection(const string & field_name, int field_offset,
		AccessPath* inner, il::type* t):
			FunctionTerm(GET_FUN_ID(field_offset), inner->to_term(), false)
{
	this->t = t;
	this->specialization_type = AP_FIELD;
	this->apt = AP_FIELD;
	this->field_name = field_name;
	this->field_offset = field_offset;
	this->inner = inner;
	this->index_var = NULL;
	this->has_index_var = inner->contains_index_var();
	this->anonymous = false;

	add_signedness_attribute();
}
FieldSelection::FieldSelection(const string & field_name, int field_offset,
		AccessPath* inner, AccessPath* index_var,
		il::type* t):
			FunctionTerm(GET_FUN_ID(field_offset), inner->to_term(),
					index_var->to_term(), false)
{
	this->t = t;
	this->specialization_type = AP_FIELD;
	this->apt = AP_FIELD;
	this->field_name = field_name;
	this->field_offset = field_offset;
	this->inner = inner;
	this->index_var = index_var;
	this->has_index_var = true;
	this->anonymous = false;

	add_signedness_attribute();
}

FieldSelection::FieldSelection(int field_offset,
		AccessPath* inner, il::type* t):
			FunctionTerm(GET_FUN_ID(field_offset), inner->to_term(), false)
{
	this->t = t;
	this->specialization_type = AP_FIELD;
	this->apt = AP_FIELD;
	this->field_name = "anon_" + int_to_string(field_offset);
	this->field_offset = field_offset;
	this->inner = inner;
	this->index_var = NULL;
	this->has_index_var = inner->contains_index_var();
	this->anonymous = true;

	add_signedness_attribute();
}

bool FieldSelection::is_anonymous()
{
	return this->anonymous;
}



AccessPath* FieldSelection::get_index_expression()
{
	return index_var;
}
Term* FieldSelection::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	Term* inner_t = inner->to_term()->substitute(subs);
	AccessPath* new_inner = AccessPath::to_ap(inner_t);
	if(index_var == NULL)
	{
		if(inner == new_inner) return this;
		return FieldSelection::make(field_name, field_offset, t, new_inner);
	}
	else {
		Term* index_t = index_var->to_term()->substitute(subs);
		AccessPath* new_index = AccessPath::to_ap(index_t);
		if(inner == new_inner && new_index == index_var) return this;
		return FieldSelection::make(field_name, field_offset, t, new_inner,
				new_index);
	}
}

Term* FieldSelection::substitute(Term* (*sub_func)(Term* t, void* data),
			void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	Term* inner_t = inner->to_term()->substitute(sub_func, my_data);
	AccessPath* new_inner = AccessPath::to_ap(inner_t);
	if(index_var == NULL)
	{
		if(inner == new_inner) return this;
		return FieldSelection::make(field_name, field_offset, t, new_inner);
	}
	else {
		Term* index_t = index_var->to_term()->substitute(sub_func, my_data);
		AccessPath* new_index = AccessPath::to_ap(index_t);
		if(inner == new_inner && new_index == index_var) return this;
		return FieldSelection::make(field_name, field_offset, t, new_inner,
				new_index);
	}
}

FieldSelection* FieldSelection::make_size(AccessPath* inner)
{


	return FieldSelection::make(SIZE_FIELD_NAME, SIZE_OFFSET,
			il::get_unsigned_integer_type(), inner->find_representative());
}

FieldSelection* FieldSelection::make_rtti(AccessPath* inner)
{
	return FieldSelection::make(RTTI_FIELD_NAME, RTTI_OFFSET,
			il::get_integer_type(), inner->find_representative());
}

FieldSelection* FieldSelection::make_deleted(AccessPath* inner)
{
//	if(inner->get_ap_type() == AP_ARRAYREF) inner = inner->get_inner();
	return FieldSelection::make(DELETED_FIELD_NAME, DELETED_OFFSET,
			il::get_integer_type(), inner->find_representative());
}

il::type* FieldSelection::make_type(AccessPath* inner, int field_offset)
{
	il::type* inner_t = inner->get_type();
	if(!inner_t->is_record_type())
	{
		return il::get_integer_type();
	}
	il::record_type* rt = (il::record_type*) inner_t;
	il::record_info* ri = rt->get_field_from_offset(field_offset*8);
	if(ri==NULL){
		cerr << "Cannot find field at offset " << field_offset << " in struct "
		<< *rt << endl;
		vector<il::record_info*>& info = rt->get_fields();
		cout << "Num fields: " << info.size() << endl;
		for(unsigned int i=0; i < info.size(); i++) {
			cout << info[i]->offset << endl;
		}

		c_assert(false);
	}
	c_assert(ri->t != NULL);
	return ri->t;
}

bool FieldSelection::is_size_field()
{
	return field_offset == SIZE_OFFSET && field_name == SIZE_FIELD_NAME;
}


bool FieldSelection::is_rtti_field()
{
	bool res= field_offset == RTTI_OFFSET && field_name == RTTI_FIELD_NAME;
	return res;
}

bool FieldSelection::is_deleted_field()
{
	return( field_offset == DELETED_OFFSET && field_name == DELETED_FIELD_NAME);
}



string FieldSelection::to_string()
{
	if(PRINT_AS_TERM) {
		return FunctionTerm::to_string();
	}
	string res;
	if(inner->get_ap_type() == AP_DEREF)
	{
		Deref* d = (Deref*) inner;
		res = d->get_inner()->to_string() + "->";
	}
	else {
		res = inner->to_string() + ".";
	}

	res += field_name;
	if(index_var != NULL)
	{
		res += "(" + index_var->to_string() + ")";
	}
	return res;

}

AccessPath* FieldSelection::get_inner()
{
	return inner;
}
IndexVariable* FieldSelection::get_index_var()
{
	if(index_var == NULL) return NULL;
	c_assert(index_var->get_ap_type() == AP_INDEX);
	return (IndexVariable*)index_var;
}




int FieldSelection::get_field_offset()
{
	return field_offset;
}
string FieldSelection::get_field_name()
{
	return field_name;
}


FieldSelection::~FieldSelection() {

}
