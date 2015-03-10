/*
 * Address.cpp
 *
 *  Created on: Sep 27, 2008
 *      Author: tdillig
 */

#include "Address.h"
#include "util.h"
#include "ConstraintGenerator.h"
#include "Constraint.h"
#include "ConstantValue.h"
#include "compass_assert.h"

map<AccessPath*, int> Address::addr_to_id;
int Address::counter = 0;


Address* Address::make(AccessPath* inner)
{
	Address* new_adr = new Address(inner);
	return (Address*) FunctionTerm::get_term(new_adr);
}


Address::Address(AccessPath* inner):
	FunctionTerm(CNode::get_varmap().get_id("addr"),
			ConstantValue::make(get_id(inner))->to_term(),
			true, ATTRIB_ADDRESS)
{
	c_assert(inner != NULL);
	this->inner = inner;
	this->t = il::pointer_type::make(inner->get_type(), "");
	has_index_var = inner->contains_index_var();
	this->specialization_type = AP_ADDRESS;
	this->apt = AP_ADDRESS;

	Constraint::set_gtz_attribute(this->to_term());

}

Term* Address::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	Term* inner_t = inner->to_term()->substitute(subs);
	AccessPath* new_inner = AccessPath::to_ap(inner_t);
	if(inner == new_inner) return this;
	return Address::make(new_inner);
}

Term* Address::substitute(Term* (*sub_func)(Term* t, void* data), void* my_data)
{
	Term* new_t = (*sub_func)(this, my_data);
	if(new_t != this) {
		return new_t;
	}
	Term* inner_t = inner->to_term()->substitute(sub_func, my_data);
	AccessPath* new_inner = AccessPath::to_ap(inner_t);
	if(inner == new_inner) return this;
	return Address::make(new_inner);
}


AccessPath* Address::get_inner()
{
	return inner;
}



string Address::to_string()
{
	if(PRINT_AS_TERM) return FunctionTerm::to_string();
	return "&" + inner->to_string();
}

bool Address::operator==(const Term& _other)
{
	Term& other = (Term&)_other;
	if(!other.is_specialized()) return false;
	AccessPath* ap = AccessPath::to_ap(&other);
	if(ap->get_ap_type() != AP_ADDRESS) return false;
	Address* other_address = (Address*) ap;
	bool res =  inner == other_address->inner;
	return res;
}


int Address::get_id(AccessPath* ap)
{
	if(ap->get_ap_type() == AP_ADDRESS) {
		//c_assert(false);
	}
	//if(!ap->is_base())
		//cerr << "WARNING: Non-base ap encountered: " << ap->to_string() << endl;

	if(addr_to_id.count(ap) > 0) return addr_to_id[ap];
	addr_to_id[ap] = counter;
	return counter++;
}

void Address::clear()
{
	addr_to_id.clear();
	counter = 0;
}

Address::~Address() {
}
