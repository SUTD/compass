/*
 * AbstractDataStructure.cpp
 *
 *  Created on: Feb 1, 2010
 *      Author: isil
 */

#include "AbstractDataStructure.h"
#include "FunctionValue.h"
#include "compass_assert.h"




AbstractDataStructure::AbstractDataStructure(AccessPath* inner,
		AccessPath* index_var, il::type* key_type, il::type* value_type,
		il::type* t):FunctionTerm(
				CNode::get_varmap().get_id("adt"),
				inner->to_term(), index_var->to_term(), false)
{

	assert_context("Making abstract data structure with inner: " + inner->to_string() +
			" index var: " + index_var->to_string() + " key type: " +
			key_type->to_string() + " value type: " + value_type->to_string());
	/*
	 * We don't allow nesting of ADT's without a pointer derefernce.
	 */
	//if(inner->find_representative()->get_ap_type() == AP_ADT) {
		//c_assert(false);
	//}

	c_assert(t->is_abstract_data_type());
	this->inner = inner;
	this->index_var = index_var;
	this->t = t;
	this->key_type = key_type;
	this->value_type = value_type;
	il::type_attribute_kind tk = t->get_attribute();
	if(tk == il::POSITION_DEPENDENT_ADT) this->adt_type = POSITION_DEPENDENT;
	else if(tk == il::SINGLE_VALUED_ADT) this->adt_type = SINGLE_VALUED;
	else this->adt_type = MULTI_VALUED;
	this->has_index_var = true;
	this->specialization_type = AP_ADT;
	this->apt = AP_ADT;
	add_signedness_attribute();

}


AbstractDataStructure* AbstractDataStructure::make(AccessPath* inner,
		AccessPath* index_var,il::type* key_type, il::type* value_type,
		il::type* t)
{
	c_assert(index_var != NULL);
	c_assert(inner != NULL);
	AbstractDataStructure* abs = new AbstractDataStructure(inner, index_var,
			key_type, value_type, t);
	AbstractDataStructure* res= (AbstractDataStructure*)
			FunctionTerm::get_term(abs);
	return res;
}

string AbstractDataStructure::to_string()
{
	if(PRINT_AS_TERM) {
		return FunctionTerm::to_string();
	}
	return "(" + inner->to_string() +
			+ "{" + key_type->to_string() + ", " + value_type->to_string() +
			"})<" + index_var->to_string() + ">";
}
AccessPath* AbstractDataStructure::get_inner()
{
	return inner;
}
IndexVariable* AbstractDataStructure::get_index_var()
{
	c_assert(index_var->get_ap_type() == AP_INDEX);
	return (IndexVariable*) index_var;
}
AccessPath* AbstractDataStructure::get_index_expression()
{
	return index_var;
}

il::type* AbstractDataStructure::get_key_type()
{
	return key_type;
}
il::type* AbstractDataStructure::get_value_type()
{
	return value_type;
}

bool AbstractDataStructure::operator==(const Term& _other)
{
	 Term& other = (Term&) _other;
	if(!other.is_specialized()) return false;
	AccessPath* other_ap = AccessPath::to_ap(&other);
	if(other_ap->get_ap_type() != AP_ADT) return false;



	AbstractDataStructure* o_adt = static_cast<AbstractDataStructure*>(other_ap);
	bool res = o_adt->inner == inner && o_adt->index_var == index_var &&
			o_adt->adt_type == adt_type &&
			o_adt->key_type->to_string() == key_type->to_string() &&
			o_adt->value_type->to_string() == value_type->to_string();

	if(other.to_string() == to_string() && !res)
	{

		cout <<"other adt type: " << o_adt->adt_type <<endl;
		cout <<"mine: " <<  adt_type <<  endl;

		cout <<"other key type: " << o_adt->key_type->to_string() << " addr: "
				<< o_adt->key_type << endl;
		cout <<"mine: " <<  key_type->to_string() << " addr: " << key_type << endl;


		cout <<"other value type: " << o_adt->value_type->to_string() << " addr: "
				<< o_adt->value_type << endl;
		cout <<"mine: " <<  value_type->to_string() << " addr: " << value_type << endl;



		cout <<"inner: " << o_adt->inner->to_string() << " addr: "
				<< o_adt->inner << endl;
		cout <<"mine: " <<  inner->to_string() << " addr: " << inner << endl;

		cout << "KEy eq: " << (*o_adt->key_type == *key_type) << endl;

		c_assert(false);
	}

	return res;

}

Term* AbstractDataStructure::substitute(map<Term*, Term*>& subs)
{
	if(subs.count(this) > 0) return subs[this];
	Term* new_inner_t = inner->to_term()->substitute(subs);
	AccessPath* new_inner = AccessPath::to_ap(new_inner_t);
	Term* new_index_t = index_var->to_term()->substitute(subs);
	AccessPath* new_index = AccessPath::to_ap(new_index_t);
	if(new_inner == inner && new_index == index_var) return this;
	return AbstractDataStructure::make(new_inner, new_index, key_type,
			value_type, t);
}
Term* AbstractDataStructure::substitute(Term* (*sub_func)(Term* t, void* data),
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
	return AbstractDataStructure::make(new_inner, new_index, key_type,
			value_type, t);
}
bool AbstractDataStructure::is_position_dependent()
{
	return adt_type == POSITION_DEPENDENT;
}
bool AbstractDataStructure::is_value_dependent()
{
	return adt_type != POSITION_DEPENDENT;
}
bool AbstractDataStructure::is_single_valued()
{
	return adt_type == SINGLE_VALUED;
}
bool AbstractDataStructure::is_multi_valued()
{
	return adt_type == MULTI_VALUED;
}

AccessPath* AbstractDataStructure::get_index_from_key(AccessPath* key)
{
	// For position-dependent data structures, the key
	// is the index to the data structure.
	if(adt_type == POSITION_DEPENDENT) return key;

	/*
	 * For single-valued data structures, the position is an invertible
	 * function of the key.
	 */
	if(adt_type == SINGLE_VALUED) {
		return FunctionValue::make_adt_position_selector(inner->strip_deref(),
				key);

	}

	/*
	 * TODO: Extend to multi-valued data structures.
	 */
	c_assert(false);

}

AbstractDataStructure::~AbstractDataStructure()
{

}
