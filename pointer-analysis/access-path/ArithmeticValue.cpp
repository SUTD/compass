/*
 * ArithmeticValue.cpp
 *
 *  Created on: Sep 24, 2008
 *      Author: tdillig
 */

#include "ArithmeticValue.h"
#include "util.h"
#include "il/type.h"
#include "ConstantValue.h"
#include "UnmodeledValue.h"
#include "FunctionValue.h"
#include "compass_assert.h"


AccessPath* ArithmeticValue::make_plus(AccessPath* ap1, AccessPath* ap2)
{
	assert_context("Making plus: " + safe_string(ap1) + "+" +
			safe_string(ap2));
	ArithmeticValue* av = new ArithmeticValue(ap1, 1, ap2, 1, il::get_integer_type());
	c_assert(av->get_ap_type() == AP_ARITHMETIC);
	AccessPath* res = canonicalize(av);
 	Term* t = ArithmeticTerm::get_term(res->to_term());
	c_assert(t->is_specialized());
	//cout << "AccessPath::to_ap(t)" <<  AccessPath::to_ap(t)->to_string() << endl;
	return AccessPath::to_ap(t);

}
AccessPath* ArithmeticValue::make_minus(AccessPath* ap1, AccessPath* ap2)
{
	assert_context("Making minus: " + safe_string(ap1) + "-" +
			safe_string(ap2));
	ArithmeticValue* av = new ArithmeticValue(ap1, 1, ap2, -1, il::get_integer_type());
	AccessPath* res = canonicalize(av);
	Term* t = ArithmeticTerm::get_term(res->to_term());
	c_assert(t->is_specialized());

	AccessPath* ret_ap = AccessPath::to_ap(t);
	cout<< "first operand ap type " << ap1->get_ap_type() << endl;
	cout << "second operand ap type " << ap2->get_ap_type() << endl;
	cout << "created ArithmeticValue ap type is " << ret_ap->get_ap_type() << endl;
	return ret_ap;
	//return AccessPath::to_ap(t);
}
AccessPath* ArithmeticValue::make_times(AccessPath* ap1, AccessPath* ap2)
{
	assert_context("Making times: " +safe_string(ap1) + "*" +
			safe_string(ap2));
	/*
	 * Check if either ap1 or ap2 are constants
	 */
	AccessPath* ap = NULL;
	long int constant = 0;
	if(ap1->get_ap_type() == AP_CONSTANT) {
		ConstantValue* cv = (ConstantValue*) ap1;
		ap = ap2;
		constant = cv->get_constant();
	}
	else if(ap2->get_ap_type() == AP_CONSTANT) {
		ConstantValue* cv = (ConstantValue*) ap2;
		ap = ap1;
		constant = cv->get_constant();
	}

	/*
	 * If either one is a constant, we make an arithmetic value,
	 * otherwise we represent the multiplication with an uninterpreted
	 * function
	 */
	if(ap!= NULL) {
		ArithmeticValue* av = new ArithmeticValue(ap, constant, il::get_integer_type());
		AccessPath* res = canonicalize(av);
		Term* t = ArithmeticTerm::get_term(res->to_term());
		c_assert(t->is_specialized());
		return AccessPath::to_ap(t);
	}

	else {
		return FunctionValue::_make_times(ap1, ap2);
	}
}

AccessPath* ArithmeticValue::make_division(AccessPath* ap1, AccessPath* ap2)
{

	/*
	 * If ap2 is not a constant, make an uninterpreted function
	 */
	if(ap2->get_ap_type() != AP_CONSTANT) {
		return FunctionValue::_make_division(ap1, ap2);
	}

	ConstantValue* cv = (ConstantValue*) ap2;
	AccessPath* ap = ap1;
	long int constant = cv->get_constant();


	switch(ap->get_ap_type())
	{
	case AP_CONSTANT:
	{
		ConstantValue* cv = (ConstantValue*) ap;
		if(constant == 0)
			return NULL;
		return ConstantValue::make(cv->get_constant()/constant);
	}
	case AP_ARITHMETIC:
	{
		ArithmeticValue* av = (ArithmeticValue*) ap;
		long int gcd = av->get_gcd(true);

		//added by haiyan
		if(constant == 0)
			return NULL;

		if(gcd%constant == 0) {
			const map<Term*, long int>& elems = av->get_elems();
			map<Term*, long int> new_elems;
			map<Term*, long int>::const_iterator it = elems.begin();
			for(; it!= elems.end(); it++)
			{
				Term* t = it->first;
				long int c= it->second;
				c/= constant;
				new_elems[t] = c;
			}
			long int new_constant = av->get_constant()/constant;
			return ArithmeticValue::make(new_elems, new_constant);

		}
	}
	default:
		return FunctionValue::_make_division(ap1, ap2);
	}

}

AccessPath* ArithmeticValue::make(const map<Term*, long int>& elems,
		long int constant)
{
	ArithmeticValue* av = new ArithmeticValue(elems, constant, il::get_integer_type());
	AccessPath* res = canonicalize(av);
	Term* t = ArithmeticTerm::get_term(res->to_term());
	c_assert(t->is_specialized());
	return AccessPath::to_ap(t);
}

AccessPath* ArithmeticValue::make_field_offset_plus(AccessPath* ap,
		const string& field_name, int field_offset, il::type* field_type)
{
	if(ap->get_ap_type() == AP_ARITHMETIC)
	{
		ArithmeticValue* av = (ArithmeticValue*) ap;
		if(!av->is_structured_ptr_arithmetic()) {
			return ArithmeticValue::make_plus(ap, ConstantValue::make(field_offset));
		}

		long int new_constant = field_offset + av->get_constant();
		vector<offset> new_offsets;
		new_offsets = av->get_offsets();
		new_offsets.push_back(offset(field_name, field_offset, field_type));

		ArithmeticValue* new_av = new ArithmeticValue(av->get_base_ptr(),
				new_offsets, av->get_elems(), new_constant,
				il::pointer_type::make(field_type, ""));

		AccessPath* res = canonicalize(new_av);
		Term* t = ArithmeticTerm::get_term(res->to_term());
		c_assert(t->is_specialized());
		return AccessPath::to_ap(t);

	}

	vector<offset> offsets;
	offsets.push_back(offset(field_name, field_offset, field_type));
	map<Term*, long int> elems;
	elems[ap->to_term()] = 1;
	long int constant = field_offset;

	ArithmeticValue* new_av = new ArithmeticValue(ap, offsets, elems, constant,
			il::pointer_type::make(field_type, ""));
	AccessPath* res = canonicalize(new_av);
	Term* t = ArithmeticTerm::get_term(res->to_term());
	c_assert(t->is_specialized());
	return AccessPath::to_ap(t);

}
AccessPath* ArithmeticValue::make_index_offset_plus(AccessPath* ap,
		long int elem_size, AccessPath* index, il::type* type)
{
	if(ap->get_ap_type() == AP_ARITHMETIC)
	{
		ArithmeticValue* av = (ArithmeticValue*) ap;
		if(!av->is_structured_ptr_arithmetic()) {
			AccessPath* times_av =
					ArithmeticValue::make_times(index, ConstantValue::make(
							elem_size));

			return ArithmeticValue::make_plus(ap,
					times_av);
		}


		vector<offset> new_offsets;
		new_offsets = av->get_offsets();
		new_offsets.push_back(offset(elem_size, index));
		map<Term*, long int> new_elems = av->get_elems();
		new_elems[index->to_term()] += elem_size;

		ArithmeticValue* new_av = new ArithmeticValue(av->get_base_ptr(),
				new_offsets, new_elems, av->get_constant(), il::get_integer_type());

		AccessPath* res = canonicalize(new_av);
		Term* t = ArithmeticTerm::get_term(res->to_term());
		c_assert(t->is_specialized());
		return AccessPath::to_ap(t);

	}

	vector<offset> offsets;
	offsets.push_back(offset(elem_size, index));
	map<Term*, long int> elems;
	elems[ap->to_term()] = 1;
	elems[index->to_term()] += elem_size;

	ArithmeticValue* new_av = new ArithmeticValue(ap, offsets, elems, 0, type);
	//ArithmeticValue* new_av = new ArithmeticValue(ap, offsets, elems, 0,
	//		il::get_integer_type());
	AccessPath* res = canonicalize(new_av);
	Term* t = ArithmeticTerm::get_term(res->to_term());
	c_assert(t->is_specialized());
	return AccessPath::to_ap(t);
}

AccessPath* ArithmeticValue::make_structured_pointer_arithmetic(
		AccessPath* base_ptr, vector<offset>& offsets)
{
	map<Term*, long int> elems;
	elems[base_ptr->to_term()] = 1;
	long int constant = 0;
	for(unsigned int i=0; i<offsets.size(); i++)
	{
		offset o = offsets[i];
		if(o.is_field_offset()) {
			constant += o.field_offset;
		}
		else {
			AccessPath* index = o.index;
			elems[index->to_term()] += o.elem_size;
		}
	}


	il::type* field_type = NULL;
	for(int i=offsets.size()-1; i>=0; i--)
	{
		il::type* t = offsets[i].field_type;
		if(t != NULL) {
			field_type = t;
			break;
		}
	}
	if(field_type == NULL) field_type = base_ptr->get_type();


	il::type* av_type= il::pointer_type::make(field_type, "");
	ArithmeticValue* av = new ArithmeticValue(base_ptr, offsets, elems,
			constant, av_type);
	AccessPath* res = canonicalize(av);
	Term* t = ArithmeticTerm::get_term(res->to_term());
	c_assert(t->is_specialized());
	return AccessPath::to_ap(t);
}

bool ArithmeticValue::is_structured_ptr_arithmetic()
{
	return base_ptr != NULL;
}

const vector<offset>& ArithmeticValue::get_offsets()
{
	return offsets;
}


AccessPath* ArithmeticValue::get_base_ptr()
{
	return base_ptr;
}

AccessPath* ArithmeticValue::canonicalize(ArithmeticValue* av)
{
	if(av->get_elems().size() == 0) {
		long int constant = av->get_constant();
		delete av;
		AccessPath* ap = ConstantValue::make(constant);
		return ap;
	}
	if(av->get_constant() == 0 && av->get_elems().size() == 1 &&
			(av->get_elems().begin())->second ==1)
	{
		Term* t = av->get_elems().begin()->first;
		switch(t->get_term_type())
		{
		case VARIABLE_TERM:
		case FUNCTION_TERM:
		{
			delete av;
			if(!t->is_specialized()){
				cerr << "Term: " << t->to_string() << endl;
			}
			c_assert(t->is_specialized());
			return AccessPath::to_ap(t);
		}
		default:
			c_assert(false);
		}
	}

	return av;


}

ArithmeticValue::ArithmeticValue(AccessPath* base_ptr, vector<offset>& offsets,
		const map<Term*, long int>& elems, long int constant, il::type* t):
			ArithmeticTerm(elems, constant)
{
	this->base_ptr = base_ptr;
	this->offsets = offsets;
	this->t = t;
	this->specialization_type = AP_ARITHMETIC;
	this->apt = AP_ARITHMETIC;
	this->has_index_var = base_ptr->contains_index_var();

	if(has_index_var) return;
	for(unsigned int i=0; i<offsets.size(); i++)
	{
		offset o = offsets[i];
		if(!o.is_index_offset()) continue;
		AccessPath* index = o.index;
		if(index->get_ap_type() == AP_INDEX) has_index_var = true;

	}
}
ArithmeticValue::ArithmeticValue(AccessPath* ap1, long int c1,
		AccessPath* ap2, long int c2, il::type* t):
			ArithmeticTerm(ap1->to_term(), c1, ap2->to_term(), c2)
{
	this->base_ptr = NULL;
	this->t = t;
	this->specialization_type = AP_ARITHMETIC;
	this->apt = AP_ARITHMETIC;
	this->has_index_var = ap1->contains_index_var() || ap2->contains_index_var();
}
ArithmeticValue::ArithmeticValue(AccessPath* ap, long int c, il::type* t):
	ArithmeticTerm(ap->to_term(), c)
{
	this->base_ptr = NULL;
	this->t = t;
	this->specialization_type = AP_ARITHMETIC;
	this->apt = AP_ARITHMETIC;
	this->has_index_var = ap->contains_index_var();

}

ArithmeticValue::ArithmeticValue(const map<Term*, long int>& elems,
		long int constant, il::type* t): ArithmeticTerm(elems, constant)
{
	this->base_ptr = NULL;
	this->t = t;
	this->specialization_type = AP_ARITHMETIC;
	this->apt = AP_ARITHMETIC;
	has_index_var = false;
	map<Term*, long int>::const_iterator it = elems.begin();
	for(; it!= elems.end(); it++)
	{
		Term* cur = it->first;
		if(!cur->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(cur);
		if(ap->contains_index_var()) {
			has_index_var = true;
			break;
		}
	}


}



long int ArithmeticValue::get_coefficient(AccessPath* term)
{
	const map<Term*, long int>& elems = this->get_elems();
	if(elems.count(term->to_term()) == 0) return 0;
	return elems.find(term->to_term())->second;
}

AccessPath* ArithmeticValue::get_inner()
{
	if(base_ptr != NULL) return base_ptr;

	const map<Term*, long int>& elems = get_elems();
	if(elems.size() > 2) return NULL; // This can't be pointer arithmetic
	if(elems.size() == 1) {
		Term* t = elems.begin()->first;
		long int coef = elems.begin()->second;
		if(coef != 1) return NULL;
		if(!t->is_specialized()) return NULL;
		return AccessPath::to_ap(t);
	}

	else {
		map<Term*, long int>::const_iterator it= elems.begin();
		Term* t1 = it->first;
		long int c1 = it->second;
		it++;
		Term* t2 = it->first;
		long int c2= it->second;
		if(t1->get_specialization() == AP_INDEX)
		{
			if(c2 != 1) return NULL;
			if(!t2->is_specialized()) return NULL;
			return AccessPath::to_ap(t2);
		}
		else if(t2->get_specialization() == AP_INDEX)
		{
			if(c1 != 1) return NULL;
			if(!t1->is_specialized()) return NULL;
			return AccessPath::to_ap(t1);
		}
		else return NULL;
	}
}


/*
 * Is the given term one of terms of this arithmetic value?
 */
bool ArithmeticValue::contains_term(AccessPath* term)
{
	const map<Term*, long int>& elems = this->get_elems();
	return(elems.count(term->to_term()) != 0);
}


Term* ArithmeticValue::strip_term(Term*& t){
	cout << "this arithmeticValue is " << this->to_string() << endl;
	cout << "strip t is " << t->to_string() << endl;
	if(!contains_term(AccessPath::to_ap(t))){
		cout << "Arithmeticvalue don't have term " << t->to_string() << endl;
		return this->to_term();
	}

	ArithmeticTerm* at = (ArithmeticTerm*) this;

	map<Term*, long int>::iterator it = at->get_eles().begin();
	for(; it != at->get_eles().end(); it++ ){
		if(it->first == t){
			Term* cur_term = at->subtract(it->first);
			return cur_term;
		}
	}
	assert(false);
}

map<Term*, long int> ArithmeticValue::elems_to_terms(
		map<AccessPath* ,long int>& elems)
{
	map<Term*, long int> res;
	map<AccessPath*, long int>::iterator it = elems.begin();
	for(; it!= elems.end(); it++)
	{
		AccessPath* ap = it->first;
		long int c = it->second;
		res[ap->to_term()] = c;
	}
	return res;
}

Term* ArithmeticValue::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];

	if(base_ptr == NULL)
	{
		const map<Term*, long int>& elems = this->get_elems();
		bool changed = false;
		map<Term*, long int> new_elems;
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			long int c = it->second;
			Term* new_t = t->substitute(subs);
			if(t!= new_t) changed = true;
			new_elems[new_t] += c;

		}

		if(!changed) return this;
		AccessPath* new_av = ArithmeticValue::make(new_elems, this->get_constant());
		return new_av->to_term();
	}
	else {
		bool changed = false;
		Term* new_base_ptr_t = base_ptr->to_term()->substitute(subs);
		c_assert(new_base_ptr_t->is_specialized());
		AccessPath* new_base_ptr = AccessPath::to_ap(new_base_ptr_t);
		if(new_base_ptr != base_ptr) changed = true;
		vector<offset> new_offsets;

		for(unsigned int i=0; i<offsets.size(); i++)
		{
			offset o = offsets[i];
			if(o.is_field_offset()) {
				new_offsets.push_back(o);
				continue;
			}
			AccessPath* index = o.index;
			Term* new_index_t = index->to_term()->substitute(subs);
			c_assert(new_index_t->is_specialized());
			AccessPath* new_index = AccessPath::to_ap(new_index_t);
			if(new_index != index) changed = true;
			new_offsets.push_back(offset(o.elem_size, new_index));

		}

		if(!changed) return this;
		AccessPath* new_av = ArithmeticValue::make_structured_pointer_arithmetic
				(new_base_ptr, new_offsets);
		return new_av->to_term();
	}

}


Term* ArithmeticValue::substitute(Term* (*sub_func)(Term* t, void* data),
		void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}

	if(base_ptr == NULL)
	{
		const map<Term*, long int>& elems = this->get_elems();
		bool changed = false;
		map<Term*, long int> new_elems;
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* t = it->first;
			long int c = it->second;
			Term* new_t = t->substitute(sub_func, my_data);
			if(t!= new_t) changed = true;
			new_elems[new_t] = c;

		}

		if(!changed) return this;
		AccessPath* new_av = ArithmeticValue::make(new_elems, this->get_constant());
		return new_av->to_term();
	}
	else {
		bool changed = false;
		Term* new_base_ptr_t = base_ptr->to_term()->substitute(sub_func, my_data);
		c_assert(new_base_ptr_t->is_specialized());
		AccessPath* new_base_ptr = AccessPath::to_ap(new_base_ptr_t);
		if(new_base_ptr != base_ptr) changed = true;
		vector<offset> new_offsets;

		for(unsigned int i=0; i<offsets.size(); i++)
		{
			offset o = offsets[i];
			if(o.is_field_offset()) {
				new_offsets.push_back(o);
				continue;
			}
			AccessPath* index = o.index;
			Term* new_index_t = index->to_term()->substitute(sub_func, my_data);
			c_assert(new_index_t->is_specialized());
			AccessPath* new_index = AccessPath::to_ap(new_index_t);
			if(new_index != index) changed = true;
			new_offsets.push_back(offset(o.elem_size, new_index));

		}

		if(!changed) return this;
		AccessPath* new_av = ArithmeticValue::make_structured_pointer_arithmetic
				(new_base_ptr, new_offsets);
		return new_av->to_term();
	}

}



bool ArithmeticValue::operator==(const Term& _other)
{
	Term* other = &((Term&)_other);
	if(other->get_specialization() != AP_ARITHMETIC) return false;
	ArithmeticValue* av = (ArithmeticValue*) AccessPath::to_ap(other);
	if(av->base_ptr != base_ptr) return false;
	if(base_ptr == NULL) return ArithmeticTerm::operator==(_other);
	if(offsets.size() != av->offsets.size()) return false;
	for(unsigned int i=0; i<offsets.size(); i++)
	{
		if(!(offsets[i] == av->offsets[i])) return false;
	}

	return true;

}
string ArithmeticValue::to_string()
{
	if(PRINT_AS_TERM) {
		return ArithmeticTerm::to_string();
	}
	if(base_ptr == NULL) {
		return  ArithmeticTerm::to_string();
	}

	string res = base_ptr->to_string();
	for(unsigned int i=0; i<offsets.size(); i++)
	{
		res += "+";
		offset& o = offsets[i];
		if(o.is_field_offset()) {
			res += int_to_string(o.field_offset);
		}
		else {
			res += int_to_string(o.elem_size) + "*" + o.index->to_string();
		}
	}
	return res;
}




ArithmeticValue::~ArithmeticValue()
{
}

//--------------------------------------------------------------
