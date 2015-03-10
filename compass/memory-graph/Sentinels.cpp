/*
 * Sentinels.cpp
 *
 *  Created on: Oct 26, 2009
 *      Author: tdillig
 */

#include "Sentinels.h"
#include "access-path.h"
#include "MemoryLocation.h"
#include "MemoryGraph.h"
#include "ConstraintGenerator.h"
#include "compass_assert.h"
#include "il/type.h"

Sentinels::Sentinels() {


}

bool Sentinels::has_sentinel(AccessPath* ap)
{

	bool is_array = false;
	il::type* t = ap->get_type();
	while(t->is_array_type()){
		t = t->get_inner_type();
		is_array = true;
	}
	while(ap->get_ap_type() == AP_ARRAYREF)
		ap = ap->get_inner();

	if(t->is_char_type())
	{
		if(is_array) return true;
		if(ap->get_ap_type() != AP_DEREF) return false;
		AccessPath* parent_ap = ap->strip_deref();
		return (parent_ap->get_type()->is_pointer_type());
	}

	return false;

}
long int Sentinels::get_sentinel(AccessPath* ap)
{
	return 0;
}

void Sentinels::add_sentinel_assumption(AccessPath* length_ap, MemoryGraph* mg)
{
	c_assert(length_ap->is_length_function());
	FunctionValue* length = (FunctionValue*) length_ap;
	// This is really the inner of the array
	AccessPath* array = length->get_arguments()[0];
	long int sentinel = get_sentinel(array);

	int elem_size = (array->get_type()->get_size() == 0 ?
			1 : (array->get_type()->get_size()/8));

	AccessPath* last_elem = ArrayRef::make(array,
			length, array->get_type(), elem_size);


	AccessPath* first_elem = ArrayRef::make(array,
			ConstantValue::make(0), array->get_type(),
			elem_size);


	Constraint imp1 = ConstraintGenerator::get_eq_constraint(last_elem,
			ConstantValue::make(sentinel));
	Constraint::set_background_knowledge(imp1);

	AccessPath* size = FieldSelection::make_size(array);


	cout << "SIZE: " << size->to_string() << endl;
	set<pair<AccessPath*, Constraint> > vs;
	mg->get_value_set(size, vs);
	Constraint len_lt_size(false);
	set<pair<AccessPath*, Constraint> >::iterator it = vs.begin();
	for(; it!= vs.end(); it++) {
		AccessPath* size_ap = it->first;
		set<IndexVariable*> indices;
		size_ap->get_nested_index_vars(indices);
		c_assert(indices.size() <= 1);
		if (indices.size() == 1) {
			size_ap = size_ap->replace(*indices.begin(),
					ConstantValue::make(0));
		}

		cout << "Size: " << size_ap->to_string() <<  " under: " << it->second << endl;
		Constraint c = ConstraintGenerator::get_lt_constraint(length, size_ap);


		c &= it->second;
		len_lt_size |= c;

	}

	if(len_lt_size.unsat()) {
		len_lt_size = ConstraintGenerator::get_lt_constraint(length, size);
	}



//	Constraint len_lt_size = ConstraintGenerator::get_lt_constraint(length,
	//		size);

	Constraint::set_background_knowledge(len_lt_size);

	cout << "Adding sentinel assumption: " << len_lt_size << endl;



}

Constraint Sentinels::get_eq_sentinel_constraint(AccessPath* ap, bool is_loop)
{
	assert_context("Getting equal to sentinel constraint: " +
			AccessPath::safe_string(ap));
	long int sentinel = get_sentinel(ap);
	c_assert(ap->get_ap_type() == AP_ARRAYREF);
	ArrayRef* ar = (ArrayRef*) ap;

	ArrayRef* ar_zero = ArrayRef::make(ar->get_inner(), ConstantValue::make(0),
			ar->get_type(), ar->get_elem_size());

	AccessPath* length = FunctionValue::make_length(ar);
	Constraint res = ConstraintGenerator::get_leqz_constraint(length);

	AccessPath* total_length = FunctionValue::make_length(ar_zero);
	AccessPath* last_elem = ArrayRef::make(ar->get_inner(),
			total_length, ar->get_type(),
			ar->get_elem_size());


	Constraint imp1 = ConstraintGenerator::get_eq_constraint(last_elem,
			ConstantValue::make(sentinel));
	Constraint::set_background_knowledge(imp1);

	AccessPath* size = FieldSelection::make_size(ar->get_inner());
	Constraint len_lt_size = ConstraintGenerator::get_lt_constraint(total_length,
			size);

	Constraint::set_background_knowledge(len_lt_size);

	return res;

}

Sentinels::~Sentinels() {

}
