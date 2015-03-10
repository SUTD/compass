/*
 * IndexVariable.cpp
 *
 *  Created on: Sep 6, 2009
 *      Author: isil
 */

#include "IndexVariable.h"
#include "util.h"
#include "compass_assert.h"

int IndexVariable::index_counter = 0;

IndexVariable::IndexVariable(ap_attribute attrib):
	VariableTerm(CNode::get_varmap().get_id("var_"+int_to_string(attrib) + "_" +
			int_to_string(++index_counter)),
			attrib)
{
	index = index_counter;
	this->specialization_type = AP_INDEX;
	this->apt = AP_INDEX;
	this->has_index_var = true;
	this->t = il::get_integer_type();
}
IndexVariable::IndexVariable(int index, ap_attribute attrib):
	VariableTerm(CNode::get_varmap().get_id("var_"+int_to_string(attrib) +
			"_" + int_to_string(index)),
			attrib)
{
	this->index = index;
	this->specialization_type = AP_INDEX;
	this->apt = AP_INDEX;
	this->has_index_var = true;
	this->t = il::get_integer_type();
}


int IndexVariable::get_index()
{
	return index;
}

IndexVariable* IndexVariable::make_source()
{
	IndexVariable* iv = new IndexVariable(ATTRIB_SOURCE_INDEX);
	return (IndexVariable*)VariableTerm::get_term(iv);
}
IndexVariable* IndexVariable::make_target()
{
	IndexVariable* iv = new IndexVariable(ATTRIB_TARGET_INDEX);
	return (IndexVariable*)VariableTerm::get_term(iv);
}

IndexVariable* IndexVariable::make_free()
{
	IndexVariable* iv = new IndexVariable(ATTRIB_FREE_INDEX);
	return (IndexVariable*)VariableTerm::get_term(iv);
}
IndexVariable* IndexVariable::make_source(IndexVariable* other)
{
	IndexVariable* iv = new IndexVariable(other->index, ATTRIB_SOURCE_INDEX);
	return (IndexVariable*)VariableTerm::get_term(iv);
}
IndexVariable* IndexVariable::make_target(IndexVariable* other)
{
	IndexVariable* iv = new IndexVariable(other->index, ATTRIB_TARGET_INDEX);
	return (IndexVariable*)VariableTerm::get_term(iv);
}

IndexVariable* IndexVariable::make_inst_source(IndexVariable* other)
{
	IndexVariable* iv = new IndexVariable(other->index, ATTRIB_INST_SOURCE_INDEX);
	return (IndexVariable*)VariableTerm::get_term(iv);
}
IndexVariable* IndexVariable::make_inst_target(IndexVariable* other)
{
	IndexVariable* iv = new IndexVariable(other->index, ATTRIB_INST_TARGET_INDEX);
	return (IndexVariable*)VariableTerm::get_term(iv);
}
IndexVariable* IndexVariable::make_free(IndexVariable* other)
{
	IndexVariable* iv = new IndexVariable(other->index, ATTRIB_FREE_INDEX);
	return (IndexVariable*)VariableTerm::get_term(iv);
}

bool IndexVariable::is_source()
{
	return this->get_id_attribute() == ATTRIB_SOURCE_INDEX;
}
bool IndexVariable::is_target()
{
	return this->get_id_attribute() == ATTRIB_TARGET_INDEX;
}
bool IndexVariable::is_free()
{
	return this->get_id_attribute() == ATTRIB_FREE_INDEX;
}
bool IndexVariable::is_inst_source()
{
	return this->get_id_attribute() == ATTRIB_INST_SOURCE_INDEX;
}

bool IndexVariable::is_inst_target()
{
	return this->get_id_attribute() == ATTRIB_INST_TARGET_INDEX;
}

string IndexVariable::to_string()
{
	if(PRINT_AS_TERM) {
		return VariableTerm::to_string();
	}

	string prefix;
	switch(this->get_id_attribute())
	{
	case ATTRIB_SOURCE_INDEX:
		prefix = "i";
		break;
	case ATTRIB_TARGET_INDEX:
		prefix = "j";
		break;
	case ATTRIB_FREE_INDEX:
		prefix = "f";
		break;
	case ATTRIB_INST_SOURCE_INDEX:
		prefix = "s_i";
		break;
	case ATTRIB_INST_TARGET_INDEX:
		prefix = "s_j";
		break;
	default:
		c_assert(false);

	}
	return prefix + int_to_string(index);
}

void IndexVariable::clear()
{
	index_counter = 0;
}

Term* IndexVariable::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	return this;
}

Term* IndexVariable::substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	return this;
}


IndexVariable::~IndexVariable() {

}
