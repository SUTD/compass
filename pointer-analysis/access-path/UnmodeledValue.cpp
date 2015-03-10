/*
 * UnmodeledValue.cpp
 *
 *  Created on: Sep 6, 2009
 *      Author: tdillig
 */

#include "UnmodeledValue.h"
#include "util.h"
#include "ConstantValue.h"
#include "compass_assert.h"

#define UNMODELED_PREFIX "unmodeled"

int UnmodeledValue::unmodeled_ap_counter = 0;

UnmodeledValue* UnmodeledValue::make_imprecise(il::type* t,
		const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(t, UNM_IMPRECISE, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}

UnmodeledValue* UnmodeledValue::make_imprecise(il::type* t,
		AccessPath* index_var, const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(t, index_var, UNM_IMPRECISE, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}



UnmodeledValue* UnmodeledValue::make_unknown_fn_return(il::type* t,
		const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(t, UNM_UNKNOWN_FUNCTION, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}

UnmodeledValue* UnmodeledValue::make_unknown_fn_return(il::type* t,
		AccessPath* index_var, const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(t, index_var,
			UNM_UNKNOWN_FUNCTION, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}


UnmodeledValue* UnmodeledValue::make_invalid(il::type* t, const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(t, UNM_INVALID, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}


UnmodeledValue* UnmodeledValue::make_invalid(il::type* t,
		AccessPath* index_var, const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(t, index_var, UNM_INVALID, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}

UnmodeledValue* UnmodeledValue::make_uninit(il::type* t,
		const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(t, UNM_UNINIT, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}

UnmodeledValue* UnmodeledValue::make_uninit(il::type* t,
		AccessPath* index_var, const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(t, index_var, UNM_UNINIT, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}

UnmodeledValue* UnmodeledValue::make(il::type* t, unmodeled_type kind,
		const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(t, kind, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}

UnmodeledValue* UnmodeledValue::make(il::type* t, AccessPath* index_var,
		unmodeled_type kind, const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(t, index_var, kind, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}

UnmodeledValue* UnmodeledValue::make(int id, il::type* t, AccessPath* iv,
		unmodeled_type kind, const DisplayTag & dt)
{
	UnmodeledValue* uv = new UnmodeledValue(id, t, iv, kind, dt);
	return (UnmodeledValue*) FunctionTerm::get_term(uv);
}


UnmodeledValue::UnmodeledValue(il::type* t, unmodeled_type kind,
		const DisplayTag & dt):
	FunctionTerm(CNode::get_varmap().get_id(
			UNMODELED_PREFIX + int_to_string(unmodeled_ap_counter)),
		ConstantValue::make(0), false, ATTRIB_UNMODELED),dt(dt)
{
	this->t = t;
	this->specialization_type = AP_UNMODELED;
	this->apt = AP_UNMODELED;
	this->index_var= NULL;
	this->has_index_var = false;
	this->kind = kind;
	this->id = unmodeled_ap_counter++;
	add_signedness_attribute();

}


UnmodeledValue::UnmodeledValue(il::type* t, AccessPath* index_var,
		unmodeled_type kind, const DisplayTag& dt):
	FunctionTerm(CNode::get_varmap().get_id(
			UNMODELED_PREFIX + int_to_string(unmodeled_ap_counter)),
		index_var->to_term(), false, ATTRIB_UNMODELED),dt(dt)
{
	this->t = t;
	this->specialization_type = AP_UNMODELED;
	this->apt = AP_UNMODELED;
	this->index_var= index_var;
	this->has_index_var = (index_var != NULL);
	this->kind = kind;
	this->id = unmodeled_ap_counter++;
	add_signedness_attribute();

}

UnmodeledValue::UnmodeledValue(int id, il::type* t, AccessPath* index_var,
		unmodeled_type kind, const DisplayTag& dt):
		FunctionTerm(CNode::get_varmap().get_id(UNMODELED_PREFIX +
				int_to_string(id)),
			index_var->to_term(), false, ATTRIB_UNMODELED),dt(dt)
{
	this->t = t;
	this->specialization_type = AP_UNMODELED;
	this->apt = AP_UNMODELED;
	this->index_var= index_var;
	this->has_index_var = (index_var != NULL);
	this->kind = kind;
	this->id = id;
	add_signedness_attribute();
}

string UnmodeledValue::to_string()
{
	if(PRINT_AS_TERM) {
		return FunctionTerm::to_string();
	}

	string res;

	if(get_attribute() == TERM_ATTRIB_GTZ) {

		res += "+";
	}

	if(kind == UNM_IMPRECISE) res += "imprecise";
	else if(kind == UNM_UNINIT) res += "uninit";
	else if(kind == UNM_INVALID) res += "invalid";
	else if(kind == UNM_UNKNOWN_FUNCTION) res += "unknown";
	else c_assert(false);

	res +=  int_to_string(id);

	if(kind == UNM_IMPRECISE && dt.get_description()!= "") {
		res = dt.get_description();
	}

	if(index_var!=NULL) {
		res += "(" + index_var->to_string() + ")";
	}


	//res += "(t: " + t->to_string() + ", kind: " + int_to_string(kind) + ")";


	return res;
}

void UnmodeledValue::clear()
{
	unmodeled_ap_counter = 0;
}

Term* UnmodeledValue::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	if(index_var == NULL) return this;
	Term* index_t = index_var->to_term()->substitute(subs);
	AccessPath* index_ap = AccessPath::to_ap(index_t);
	if(index_ap == index_var) return this;
	return UnmodeledValue::make(id, t, index_ap, kind, dt);
}

Term* UnmodeledValue::substitute(Term* (*sub_func)(Term* t, void* data),
				void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	if(index_var == NULL) return this;
	Term* index_t = index_var->to_term()->substitute(sub_func, my_data);
	AccessPath* index_ap = AccessPath::to_ap(index_t);
	if(index_ap == index_var) return this;
	return UnmodeledValue::make(id, t, index_ap, kind, dt);

}

bool UnmodeledValue::operator==(const Term& _other)
{
	Term& other = (Term&) _other;
	if(other.get_specialization() != AP_UNMODELED) return false;
	UnmodeledValue* u = (UnmodeledValue*) AccessPath::to_ap(&other);
	if(u->kind != kind ) return false;
	if(this->index_var != u->index_var) return false;
	return *u->t == *t && u->id == id && this->dt == u->dt;
}
bool UnmodeledValue::is_imprecise()
{
	return kind == UNM_IMPRECISE;
}

bool UnmodeledValue::is_unknown_func_return()
{
	return kind == UNM_UNKNOWN_FUNCTION;
}
bool UnmodeledValue::is_uninitialized()
{
	return kind == UNM_UNINIT;
}
bool UnmodeledValue::is_constant_deref()
{
	return kind == UNM_INVALID;
}
unmodeled_type UnmodeledValue::get_kind()
{
 return kind;
}

IndexVariable* UnmodeledValue::get_index_var()
{
	if(index_var == NULL) return NULL;
	c_assert(index_var->get_ap_type() == AP_INDEX);
	return (IndexVariable*)index_var;
}

int UnmodeledValue::get_unmodeled_id()
{
	return id;
}

AccessPath* UnmodeledValue::get_index_expression()
{
	return index_var;
}

const DisplayTag& UnmodeledValue::get_display_tag()
{
	return dt;
}

UnmodeledValue::~UnmodeledValue() {

}
