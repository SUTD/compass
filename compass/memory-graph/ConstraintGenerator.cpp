/*
 * ConstraintGenerator.cpp
 *
 *  Created on: Oct 16, 2008
 *      Author: tdillig
 */

#include "ConstraintGenerator.h"
#include "Constraint.h"

#include "MemoryGraph.h"
#include "MemoryLocation.h"
#include "access-path.h"
#include "sail/Variable.h"
#include "sail/Constant.h"
#include "il/type.h"
#include "FieldSelection.h"
#include "ConstantValue.h"
#include "Edge.h"
#include "util.h"
#include <iostream>
#include "Address.h"
#include "FunctionValue.h"
#include "Alloc.h"
#include "Type.h"
#include "il/binop_expression.h"
#include "FunctionValue.h"
#include "sail/SummaryUnit.h"
#include "MemoryAnalysis.h"
#include "compass_assert.h"

#define CONVERT_NULL_TERMINATOR_TO_ARITHMETIC true

ConstraintGenerator::ConstraintGenerator(MemoryGraph & mg):mg(mg) {


}

Constraint ConstraintGenerator::get_eqz_constraint(AccessPath* ap)
{
	ConstantValue* constant = ConstantValue::make(0);
	Constraint c(ap->to_term(), constant, ATOM_EQ);
	return c;
}

Constraint ConstraintGenerator::get_neqz_constraint(AccessPath* ap)
{
	ConstantValue* constant = ConstantValue::make(0);
	Constraint c(ap->to_term(), constant, ATOM_NEQ);
	return c;
}

Constraint ConstraintGenerator::get_gtz_constraint(AccessPath* ap)
{
	ConstantValue* constant = ConstantValue::make(0);
	Constraint c(ap->to_term(), constant, ATOM_GT);
	return c;
}

Constraint ConstraintGenerator::get_geqz_constraint(AccessPath* ap)
{
	ConstantValue* constant = ConstantValue::make(0);
	Constraint c(ap->to_term(), constant, ATOM_GEQ);
	return c;
}


Constraint ConstraintGenerator::get_ltz_constraint(AccessPath* ap)
{
	ConstantValue* constant = ConstantValue::make(0);
	Constraint c(ap->to_term(), constant, ATOM_LT);
	return c;
}


Constraint ConstraintGenerator::get_leqz_constraint(AccessPath* ap)
{
	ConstantValue* constant = ConstantValue::make(0);
	Constraint c(ap->to_term(), constant, ATOM_LEQ);
	return c;
}


Constraint ConstraintGenerator::get_leq_constraint(AccessPath* ap1,
		AccessPath* ap2)
{
	return Constraint(ap1->to_term(), ap2->to_term(), ATOM_LEQ);
}
Constraint ConstraintGenerator::get_lt_constraint(AccessPath* ap1,
		AccessPath* ap2)
{
	return Constraint(ap1->to_term(), ap2->to_term(), ATOM_LT);
}
Constraint ConstraintGenerator::get_gt_constraint(AccessPath* ap1,
		AccessPath* ap2)
{
	return Constraint(ap1->to_term(), ap2->to_term(), ATOM_GT);
}


Constraint ConstraintGenerator::get_eq_constraint(AccessPath* ap1,
		AccessPath* ap2)
{
	return Constraint(ap1->to_term(), ap2->to_term(), ATOM_EQ);
}

Constraint ConstraintGenerator::get_neq_constraint(AccessPath* ap1,
		AccessPath* ap2)
{
	return !Constraint(ap1->to_term(), ap2->to_term(), ATOM_EQ);
}

Constraint ConstraintGenerator::get_geq_constraint(AccessPath* ap1,
		AccessPath* ap2)
{
	return Constraint(ap1->to_term(), ap2->to_term(), ATOM_GEQ);
}

Constraint ConstraintGenerator::get_mod_constraint(AccessPath* ap1, long int c)
{
	ConstantValue* cv = ConstantValue::make(c);
	return Constraint(ap1->to_term(), cv->to_term(), ATOM_MOD);
}


/*
 * sum = ap1 + ap2
 */
Constraint ConstraintGenerator::get_sum_constraint(AccessPath* sum,
		AccessPath* ap1, AccessPath* ap2)
{
	AccessPath* plus_ap = ArithmeticValue::make_plus(ap1, ap2);
	return Constraint(sum->to_term(), plus_ap->to_term(), ATOM_EQ);
}

Constraint ConstraintGenerator::get_eq_to_value_set_constraint(AccessPath* v,
		const set<pair<AccessPath*, Constraint> >& vs, bool eliminate_counters)
{
	Constraint res(false);
	set<pair<AccessPath*, Constraint> >::const_iterator it = vs.begin();
	for(; it!= vs.end(); it++)
	{
		AccessPath* val = it->first;
		Constraint val_c = it->second;
		//if(val->get_ap_type() == AP_NIL) continue;
		Constraint val_eq_v;
		if(val->get_ap_type() != AP_NIL) {
			val_eq_v = get_eq_constraint(val, v);
		}
		else {
			val_eq_v =
					ConstraintGenerator::get_neqz_constraint(FunctionValue::make_is_nil(v));
		}
		Constraint cur = val_eq_v & val_c;
		IndexVarManager::eliminate_source_vars(cur);
		if(eliminate_counters)
		{
			set<IterationCounter*> counters;
			val->get_counters(counters, true);
			set<VariableTerm*> counter_terms;
			counter_terms.insert(counters.begin(), counters.end());
			cur.eliminate_evars(counter_terms);
		}
		res |=cur;

	}
	return res;
}


Constraint ConstraintGenerator::get_neqz_constraint_from_value_set(
		sail::Symbol* s)
{
	set<pair<AccessPath*, Constraint> > values;
	mg.get_value_set(s, values);
	Constraint res(false);
	set<pair<AccessPath*, Constraint> >::iterator it = values.begin();

	for(; it!=values.end(); it++)
	{
		AccessPath* ap = it->first;
		Constraint c = it->second;
		Constraint neqz_ap = get_neqz_constraint(ap);

		Constraint cur_c = neqz_ap & c;
		res = res | cur_c;
	}
	return res;
}


bool ConstraintGenerator::contains_access_path(Constraint c, set<AccessPath*>& ap)
{

	set<AccessPath*>::iterator it = ap.begin();
	for(; it!= ap.end(); it++){
		AccessPath* cur = *it;
		if(contains_access_path(c, cur)) return true;
	}
	return false;
}




bool ConstraintGenerator::contains_access_path(Constraint c, AccessPath* ap)
{
	Term* ap_t = ap->to_term();
	return c.contains_term(ap_t);
}

/*
 * ap1 binop ap2
 */
Constraint ConstraintGenerator::get_constraint_from_binop(AccessPath* ap1,
		AccessPath* ap2, il::binop_type binop)
{

	assert_context("Making constraint from binop: " + AccessPath::safe_string(ap1) +
			il::binop_expression::binop_to_string(binop) +
			AccessPath::safe_string(ap2));
	switch(binop)
	{
	case il::_LT:
	{
		return Constraint(ap1->to_term(), ap2->to_term(), ATOM_LT);
	}
	case il::_LEQ:
	{

		return Constraint(ap1->to_term(), ap2->to_term(), ATOM_LEQ);
	}
	case il::_GT:
	{
		return Constraint(ap1->to_term(), ap2->to_term(), ATOM_GT);
	}
	case il::_GEQ:
	{
		return Constraint(ap1->to_term(), ap2->to_term(), ATOM_GEQ);
	}
	case il::_EQ:
	{
		return Constraint(ap1->to_term(), ap2->to_term(), ATOM_EQ);
	}
	case il::_NEQ:
	{
		return Constraint(ap1->to_term(), ap2->to_term(), ATOM_NEQ);
	}
	case il::_LOGICAL_AND:
	case il::_LOGICAL_AND_NO_SHORTCIRCUIT:
	{
		Constraint c1 = ConstraintGenerator::get_neqz_constraint(ap1);
		Constraint c2 = ConstraintGenerator::get_neqz_constraint(ap2);
		return c1 & c2;
	}
	case il::_LOGICAL_OR:
	case il::_LOGICAL_OR_NO_SHORTCIRCUIT:
	{
		Constraint c1 = ConstraintGenerator::get_neqz_constraint(ap1);
		Constraint c2 = ConstraintGenerator::get_neqz_constraint(ap2);
		return c1 | c2;
	}

	default:
		c_assert(false);
	}

}

Constraint ConstraintGenerator::get_subtype_of_constraint(AccessPath* ap1,
		AccessPath* ap2)
{
	AccessPath* fv = FunctionValue::make_subtype(ap1, ap2);
	ConstantValue* cv = ConstantValue::make(1);
	return ConstraintGenerator::get_eq_constraint(fv, cv);
}


inline Constraint ConstraintGenerator::make_constraint_from_binop(
		AccessPath* ap1, AccessPath* ap2, il::binop_type bt,
		Constraint& to_and)
{

	assert_context("Making constraint from binop: " +
			AccessPath::safe_string(ap1) +
			il::binop_expression::binop_to_string(bt) +
			AccessPath::safe_string(ap2));

	if(!CONVERT_NULL_TERMINATOR_TO_ARITHMETIC) {
		return get_constraint_from_binop(ap1, ap2, bt);

	}

	if(bt != il::_EQ && bt != il::_NEQ) {
		return get_constraint_from_binop(ap1, ap2, bt);
	}

	AccessPath* _constant = NULL;
	/*
	 * First make sure the constant is guaranteed to be the second argument.
	 */
	if(ap1->get_ap_type() == AP_CONSTANT)
	{
			_constant = ap1;
			ap1 = ap2;
			ap2 = _constant;

	}
	else if(ap2->get_ap_type() == AP_CONSTANT)
	{
		_constant = ap2;
	}



	// If neither is a constant character or ap1 doesn't have a sentinel
	// make a regular binop.
	if(_constant == NULL || !mg.sentinels.has_sentinel(ap1)){
		return get_constraint_from_binop(ap1, ap2, bt);
	}

	/*
	 * 	If the constant is not the sentinel, also make
	 * a regular binop
	 */
	ConstantValue* constant = (ConstantValue*) _constant;
	long int sentinel = mg.sentinels.get_sentinel(ap1);
	if( sentinel !=constant->get_constant()){
		return get_constraint_from_binop(ap1, ap2, bt);
	}


	ap_type apt = ap1->get_ap_type();

	/*
	 * We should upgrade ap1 to be an array.
	 */
	if(apt != AP_ARRAYREF){
		MemoryLocation* loc = mg.get_location(ap1);
		int offset = loc->find_offset(ap1);
		il::type* t = ap1->get_type();
		c_assert(!t->is_array_type());
		c_assert(!t->is_void_type());
		mg.change_to_array_loc(loc, t);
		ap1 = loc->get_access_path(offset);
		c_assert(ap1->get_ap_type() == AP_ARRAYREF);
		ap1 = ArrayRef::make(ap1->get_inner(),
				ConstantValue::make(0), t->get_size()/8);

	}

	Constraint res = mg.sentinels.get_eq_sentinel_constraint(ap1,
			mg.ma.get_su()->is_superblock());
	if(bt == il::_NEQ) res = !res;
	return res;
}





/*
 * Yields the constraint under which the given binop is true and false
 * respectively.
 * ap_c1 and ap_c2 are the incoming constraints on the edges to _ap1 and _ap2
 * respectively -- we assume the index variables on the incoming edges are
 * named j, so we need to rename them.
 */
pair<Constraint, Constraint> ConstraintGenerator::get_constraint_from_pred_binop
	(AccessPath* _ap1, Constraint ap_c1, AccessPath* _ap2, Constraint ap_c2,
			il::binop_type bt)
{
	/*cout << "Getting constraint from binop: " << _ap1->to_string() << " op " <<
		_ap2->to_string() << " ap_c1: " << ap_c1.to_string() << " ap_c2: "
		<< ap_c2.to_string() << endl; */

	AccessPath* ap1 = _ap1->push_address_through();
	AccessPath* ap2 = _ap2->push_address_through();


	mg.ivm.rename_index_vars(ap_c1);
	mg.ivm.rename_index_vars(ap_c2);



	set<IndexVariable*> ap1_indices;
	ap1->get_nested_index_vars(ap1_indices);
	set<IndexVariable*> ap2_indices;
	ap2->get_nested_index_vars(ap2_indices);

	/*
	 * Don't eliminate the i's associated with the size field.
	 */
	IndexVariable* size_index1 = NULL;
	IndexVariable* size_index2 =NULL;
	if(ap1->is_size_field_ap()) {
		size_index1 = ap1->find_outermost_index_var(true);
	}
	if(ap2->is_size_field_ap()) {
		size_index2 = ap2->find_outermost_index_var(true);
	}

	/*
	 * Stores replaced iteration counters that need to be eliminated.
	 */
	set<VariableTerm*> to_eliminate;


	map<Term*, Term*> ap1_index_map;
	map<Term*, Term*> ap2_index_map;
	set<IndexVariable*>::iterator it = ap1_indices.begin();
	for(; it!= ap1_indices.end(); it++)
	{
		IndexVariable* old_var = *it;
		if(old_var == size_index1) continue;
		IndexVariable* new_var = IndexVariable::make_free();
		ap1_index_map[old_var] = new_var;
		ap1 = ap1->replace(old_var, new_var);
	}

	set<IterationCounter*> counters_c1;
	ap1->get_counters(counters_c1, true);
	set<IterationCounter*>::iterator it_counter1 = counters_c1.begin();
	for(; it_counter1 != counters_c1.end(); it_counter1++)
	{
		IterationCounter* ic = *it_counter1;
		Variable* temp = Variable::make_temp(ic->get_type());
		ap1_index_map[ic] = temp;
		ap1 = ap1->replace(ic, temp);
		to_eliminate.insert(temp);
	}

	ap_c1.replace_terms(ap1_index_map);

	it = ap2_indices.begin();
	for(; it!= ap2_indices.end(); it++)
	{
		IndexVariable* old_var = *it;
		if(old_var == size_index2) continue;
		IndexVariable* new_var = IndexVariable::make_free();
		ap2_index_map[old_var] = new_var;
		ap2 = ap2->replace(old_var, new_var);
	}

	set<IterationCounter*> counters_c2;
	ap2->get_counters(counters_c2, true);
	set<IterationCounter*>::iterator it_counter2 = counters_c2.begin();
	for(; it_counter2 != counters_c2.end(); it_counter2++)
	{
		IterationCounter* ic = *it_counter2;
		Variable* temp = Variable::make_temp(ic->get_type());
		ap2_index_map[ic] = temp;
		ap2 = ap2->replace(ic, temp);
		to_eliminate.insert(temp);
	}

	ap_c2.replace_terms(ap2_index_map);


	Constraint to_and;
	Constraint res = this->make_constraint_from_binop(ap1, ap2, bt, to_and);



	ConstantValue* zero = ConstantValue::make(0);
	if(size_index1 != NULL) {
		res.replace_term(size_index1, zero);
	}
	if(size_index2 != NULL) {
		res.replace_term(size_index2, zero);
	}

	/*cout << "Res: " << res.to_string()<< endl;
	cout << "ap c1: " << ap_c1.to_string() << endl;
	cout << "ap c2: " << ap_c2.to_string() << endl;
	cout << "to and : " << to_and.to_string() << endl; */

	Constraint true_c = res & ap_c1 & ap_c2 & to_and;

	mg.ivm.eliminate_fresh_vars(true_c);
	true_c.eliminate_evars(to_eliminate);

	mg.ivm.rename_source_to_target_index(true_c);
	Constraint false_c = !res & ap_c1 & ap_c2 & to_and;
	mg.ivm.eliminate_fresh_vars(false_c);
	false_c.eliminate_evars(to_eliminate);
	mg.ivm.rename_source_to_target_index(false_c);


	return pair<Constraint, Constraint>(true_c, false_c);
}



/*
 * Eliminates the existentially quantified access path ap from c.
 */
void ConstraintGenerator::eliminate_evar(Constraint& c, AccessPath* ap)
{
	Term* ap_t = ap->to_term();

	if(ap_t->get_term_type() == VARIABLE_TERM)
	{
		c.eliminate_evar((VariableTerm*)ap_t);
		return;
	}

	Variable* v = Variable::make_temp(ap->get_type());
	c.replace_term(ap->to_term(), v);
	c.eliminate_evar(v);

}

void ConstraintGenerator::eliminate_uvar(Constraint& c, AccessPath* ap)
{
	Constraint cc =!c;
	eliminate_evar(cc, ap);
	c = !cc;
}

/*
 * Eliminates the access path ap from c.
 */
void ConstraintGenerator::eliminate_free_var(Constraint& c, AccessPath* ap)
{


	assert_context("Eliminating free var "+
			AccessPath::safe_string(ap) + " from: " +
			c.to_string());
	Term* ap_t = ap->to_term();
	if(ap_t->get_term_type() == VARIABLE_TERM)
	{
		c.eliminate_free_var((VariableTerm*)ap_t);
	}
	else {
		Variable* v = Variable::make_temp(ap->get_type());
		c.replace_term(ap->to_term(), v);
		c.eliminate_free_var(v);
		c_assert(!c.contains_term(v));
	}

}

void ConstraintGenerator::eliminate_evars(Constraint& c, set<AccessPath*>& aps)
{
	set<AccessPath*>::iterator it = aps.begin();
	for(; it!=aps.end(); it++)
	{
		AccessPath* ap = *it;
		eliminate_evar(c, ap);
	}
}
void ConstraintGenerator::eliminate_free_vars(Constraint& c, set<AccessPath*>& aps)
{
	set<AccessPath*>::iterator it = aps.begin();
	for(; it!=aps.end(); it++)
	{
		AccessPath* ap = *it;
		eliminate_free_var(c, ap);
	}
}

void ConstraintGenerator::eliminate_background_target_variables(Constraint& c)
{
	set<Term*> terms;
	c.get_terms(terms, true);

	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		bool eliminate = false;
		if(ap->get_ap_type() == AP_VARIABLE)
		{
			Variable* v = (Variable*) ap;
			if(v->is_background_target_var()){
				eliminate = true;
			}
		}
		else if(ap->get_ap_type() == AP_FUNCTION)
		{
			FunctionValue* fv = (FunctionValue*) ap;
			if(fv->is_target()) eliminate = true;
		}
		if(eliminate) {
			eliminate_evar(c, ap);
		}
	}
}

void ConstraintGenerator::eliminate_disjointness_terms(Constraint& c)
{
	set<Term*> terms;
	c.get_terms(terms, true);

	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		bool eliminate = false;
		if(ap->get_ap_type() == AP_VARIABLE)
		{
			Variable* v = (Variable*) ap;
			if(v->is_disjointness_var()){
				eliminate = true;
			}
		}
		else if(ap->get_ap_type() == AP_FUNCTION)
		{
			FunctionValue* fv = (FunctionValue*) ap;
			if(fv->is_disjoint()) eliminate = true;
		}
		if(eliminate) {
			eliminate_free_var(c, ap);
		}
	}
}

void ConstraintGenerator::eliminate_unobservables(Constraint & c, bool is_loop,
		set<AccessPath*>& excluded_aps, bool eliminate_counters)
{

	Constraint old_c = c;
 	set<Term*> terms;
	c.get_terms(terms, true);
	set<AccessPath*> unobservables;



	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		ap->get_unobservable_aps(unobservables, is_loop, excluded_aps,
				eliminate_counters);
	}


	{
		set<AccessPath*>::iterator it = unobservables.begin();
		for(; it!=unobservables.end(); it++)
		{
			AccessPath* ap = *it;
			eliminate_free_var(c, ap);

		}
	}

	//assert(c.sc().implies(old_c.nc()));
}

void ConstraintGenerator::get_nested_aps(Constraint c, set<AccessPath*> & aps)
{
	set<Term*> terms;
	c.get_terms(terms, false);
	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		aps.insert(AccessPath::to_ap(t));
	}
}

void ConstraintGenerator::get_nested_memory_aps(Constraint c,
		set<AccessPath*>& aps)
{
	set<Term*> terms;
	c.get_terms(terms, false);
	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		ap->get_nested_memory_aps(aps);
	}

}

bool ConstraintGenerator::disjoint(set<Constraint>& constraints)
{
	set<Constraint>::iterator it = constraints.begin();
	for(; it!= constraints.end(); it++)
	{
		Constraint c1 = *it;
		set<Constraint>::iterator it2 = it;
		it2++;
		for(; it2!= constraints.end(); it2++)
		{

			Constraint c2 = *it2;
			Constraint res = c1 & c2;
			if(res.sat()) return false;
		}
	}
	return true;
}

ConstraintGenerator::~ConstraintGenerator() {

}
