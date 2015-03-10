/*
 * IndexVarManager.cpp
 *
 *  Created on: Oct 16, 2008
 *      Author: isil
 */

#include "IndexVarManager.h"


#include "Constraint.h"
#include "util.h"
#include "MemoryLocation.h"
#include "AccessPath.h"
#include "Variable.h"
#include "ArithmeticValue.h"
#include "FunctionValue.h"
#include "ConstantValue.h"
#include "FieldSelection.h"
#include "ConstraintGenerator.h"
#include "IndexVariable.h"
#include "Term.h"
#include "ArrayRef.h"
#include "IterationCounter.h"
#include "compass_assert.h"

IndexVarManager::IndexVarManager() {
	dummy_index = IndexVariable::make_source();
}

IndexVariable* IndexVarManager::get_dummy_index()
{
	return dummy_index;
}



IndexVariable* IndexVarManager::get_new_source_index(AccessPath* rep,
		bool is_unsigned)
{
	set<IndexVariable*> index_vars;
	rep->get_nested_index_vars(index_vars);
	set<IndexVariable*>::iterator it = index_vars.begin();
	for(; it!= index_vars.end(); it++)
	{
		rep = rep->replace(*it, dummy_index);
	}
	if(ap_to_index_map.count(rep) != 0)
	{

		return ap_to_index_map[rep];
	}
	IndexVariable* iv = IndexVariable::make_source();
	ap_to_index_map[rep] = iv;
	return iv;
}




Constraint IndexVarManager::increment_index_var(Constraint orig_c,
		IndexVariable* index_var, AccessPath* inc_value)
{
	IndexVariable* temp_var = IndexVariable::make_free();
	orig_c.replace_term(index_var, temp_var);
	AccessPath* av = ArithmeticValue::make_plus(temp_var, inc_value);
	Constraint new_c = ConstraintGenerator::get_eq_constraint(av, index_var);
	Constraint final_c = orig_c & new_c;
	final_c.eliminate_evar(temp_var);
	return final_c;
}

bool IndexVarManager::is_index_term(Term* t)
{
	return(t->get_specialization() == AP_INDEX);

}

bool IndexVarManager::is_counter_term(Term* t, bool parametric)
{
	if(t->get_specialization() != AP_COUNTER) return false;
	IterationCounter* ic = (IterationCounter*) AccessPath::to_ap(t);
	if(parametric) return ic->is_parametric();
	return ic->is_last();

}

/*
 * Changes all occurences of source indices in this access path
 * to target indices.
 */
AccessPath* IndexVarManager::rename_source_to_target(AccessPath* ap)
{
	set<IndexVariable*> indices;
	ap->get_nested_index_vars(indices);

	map<AccessPath*, AccessPath*> subs;
	set<IndexVariable*>::iterator it = indices.begin();
	for(; it!= indices.end(); it++)
	{
		IndexVariable* iv= *it;
		if(!iv->is_source()) continue;
		IndexVariable* target_iv = IndexVariable::make_target(iv);
		subs[iv] = target_iv;
	}

	return ap->replace(subs);

}

void IndexVarManager::remove_unused_index_vars(Constraint & c,
			AccessPath* source_ap, AccessPath* target_ap)
{


	set<Term*> terms;
	c.get_terms(terms, false);
	set<IndexVariable*> source_vars, target_vars;
	source_ap->get_nested_index_vars(source_vars);

	target_ap->get_nested_index_vars(target_vars);


	set<Term*> used_vars;
	set<IndexVariable*>::iterator it = source_vars.begin();
	for(; it!= source_vars.end(); it++){
		used_vars.insert(*it);
	}

	for(it=target_vars.begin(); it!=target_vars.end(); it++){
		IndexVariable* target_var = IndexVariable::make_target(*it);
		used_vars.insert(target_var);
	}



	/*
	 * Eliminate unused index vars and iteration counters
	 */
	set<IterationCounter*> counters;
	target_ap->get_counters(counters, true);
	set<Term*>::iterator it2 = terms.begin();

	for(;it2!=terms.end();it2++){

		Term* t = *it2;
		bool eliminate_counter = is_counter_term(t, true);

		if(eliminate_counter)
		{

			IterationCounter* ic = (IterationCounter*) t;
			if(ic->get_num_total_contexts() == 0) {
				eliminate_counter = false;

			}
		}


		if(eliminate_counter) {
			IterationCounter* ic = (IterationCounter*) AccessPath::to_ap(t);
			if(counters.count(ic) == 0) {

				c.eliminate_evar((VariableTerm*)t);
			}
			continue;
		}

		if(!is_index_term(t)) {
			continue;
		}
		IndexVariable* iv = (IndexVariable*) t;


		if(!iv->is_source() && !iv->is_target()){
			continue;
		}

		if(used_vars.count(t)==0) {
			c.eliminate_evar((VariableTerm*)t);
		}
	}

	/*
	 * Eliminate pos functions if the source is not an ADT.
	 */
	/*if(source_ap->find_representative()->get_ap_type() != AP_ADT)
	{
		set<Term*>::iterator it2 = terms.begin();
		for(;it2!=terms.end();it2++){

			Term* t = *it2;
			if(!t->is_specialized()) continue;
			AccessPath* ap_t = AccessPath::to_ap(t);
			if(ap_t->get_ap_type() == AP_FUNCTION){
				FunctionValue* fv = (FunctionValue*) ap_t;
				if(fv->is_adt_pos_selector()) {
					ConstraintGenerator::eliminate_free_var(c, fv);
				}
			}
		}
	}*/




}

/*
 * Removes all k's  from loops.
 */
void IndexVarManager::remove_parametric_loop_vars(Constraint & c)
{
	set<Term*> free_vars;
	c.get_free_variables(free_vars);
	set<Term*>::iterator it2 = free_vars.begin();
	for(;it2!=free_vars.end();it2++){
		Term* t = *it2;
		if(t->get_specialization() != AP_COUNTER) continue;
		IterationCounter* ic = (IterationCounter*) t;
		if(ic->is_parametric()) c.eliminate_evar(ic);
	}
}



void IndexVarManager::rename_source_to_target_index(Constraint& c)
{
	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_source()) {
			IndexVariable* target = IndexVariable::make_target(iv);
			c.replace_term(iv, target);
		}
	}
}

void IndexVarManager::rename_source_to_fresh_index(Constraint& c)
{
	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_source()) {
			IndexVariable* target = IndexVariable::make_free();
			c.replace_term(iv, target);
		}
	}
}

void IndexVarManager::rename_inst_source_to_inst_target(Constraint& c)
{
	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_inst_source()) {
			IndexVariable* target = IndexVariable::make_inst_target(iv);
			c.replace_term(iv, target);
		}
	}
}

AccessPath* IndexVarManager::refresh_index_vars(AccessPath* ap, Constraint& c)
{
	set<IndexVariable*> index_vars;
	ap->get_nested_index_vars(index_vars, true);

	map<AccessPath*, AccessPath*> subs;
	map<Term*, Term*> sub_terms;
	set<IndexVariable*>::iterator it = index_vars.begin();
	for(; it != index_vars.end(); it++) {
		IndexVariable* old_source = *it;
		if(!old_source->is_source()) continue;
		IndexVariable* old_target = IndexVariable::make_target(old_source);
		IndexVariable* new_source = IndexVariable::make_source();
		IndexVariable* new_target = IndexVariable::make_target(new_source);
		subs[old_source] = new_source;
		subs[old_target] = new_target;
		sub_terms[old_source] = new_source;
		sub_terms[old_target] = new_target;
	}

	c.replace_terms(sub_terms);
	return ap->replace(subs);
}



void IndexVarManager::rename_target_to_source_index(Constraint& c)
{

	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_target()) {
			IndexVariable* source = IndexVariable::make_source(iv);
			c.replace_term(iv, source);
		}
	}

}

void IndexVarManager::rename_target_to_fresh_index(Constraint& c)
{

	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_target()) {
			IndexVariable* source = IndexVariable::make_free();
			c.replace_term(iv, source);
		}
	}

}



IndexVariable* IndexVarManager::get_source_index_var(MemoryLocation* loc)
{
	IndexVariable* index = loc->get_representative_access_path()->get_index_var();
	return index;
}

IndexVariable* IndexVarManager::get_target_index_var(MemoryLocation* loc)
{
	IndexVariable* index = loc->get_representative_access_path()->get_index_var();
	return IndexVariable::make_target(index);

}

void IndexVarManager::rename_index_vars(Constraint& c)
{
	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_source())
			c.eliminate_evar(iv);

	}
	it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_target()) {
			IndexVariable* new_iv = IndexVariable::make_source(iv);
			c.replace_term(iv, new_iv);
		}
	}
}

void IndexVarManager::rename_inst_index_vars(Constraint& c)
{
	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_inst_source())
			c.eliminate_evar(iv);

	}
	it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_inst_target()) {
			IndexVariable* new_iv = IndexVariable::make_inst_source(iv);
			c.replace_term(iv, new_iv);
		}
	}
}

void IndexVarManager::eliminate_target_vars(Constraint& c)
{
	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_target())
			c.eliminate_evar(iv);
	}
}

/*
 * Removes all source and target variables from the constraint.
 */
void IndexVarManager::remove_all_index_vars(Constraint& c)
{
	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_source() || iv->is_target()){
			c.eliminate_evar(iv);
		}
	}
}

void IndexVarManager::eliminate_sum_index_vars(Constraint& c)
{
	set<Term*> vars;
	c.get_free_variables(vars);
	set<Term*>::iterator it = vars.begin();
	for(; it!= vars.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_inst_source() || iv->is_inst_target()){
			c.eliminate_evar(iv);
		}
	}
}


void IndexVarManager::eliminate_index_vars(Constraint& c,
		set<IndexVariable*> index_vars)
{

	for(set<IndexVariable*>::iterator it = index_vars.begin();
			it!= index_vars.end(); it++)
	{
		IndexVariable* iv = *it;
		c.eliminate_evar(iv);
	}

}

void IndexVarManager::eliminate_source_vars(Constraint& c)
{
	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_source()){
			c.eliminate_evar(iv);
		}
	}
}

void IndexVarManager::eliminate_fresh_vars(Constraint& c)
{
	set<Term*> names;
	c.get_free_variables(names);
	set<Term*>::iterator it = names.begin();
	for(; it!= names.end(); it++) {
		Term* t = *it;
		if(!is_index_term(t)) continue;
		IndexVariable* iv = (IndexVariable*) t;
		if(iv->is_free()){
			c.eliminate_evar(iv);
		}
	}
}

AccessPath* IndexVarManager::convert_index_vars_to_instantiation_vars(
		AccessPath* ap)
{
	set<IndexVariable*> index_vars;
	map<AccessPath*, AccessPath*> replacements;
	ap->get_nested_index_vars(index_vars);
	set<IndexVariable*>::iterator it = index_vars.begin();
	for(; it!= index_vars.end(); it++)
	{
		IndexVariable* iv = *it;
		IndexVariable* inst_iv = IndexVariable::make_inst_source(iv);
		replacements[iv] = inst_iv;
	}
	return ap->replace(replacements);

}



Constraint IndexVarManager::convert_index_vars_to_instantiation_vars(
		Constraint c)
{
	set<IndexVariable*> indices;
	get_index_vars(c, indices);
	map<Term*, Term*> replacements;
	set<IndexVariable*>::iterator it = indices.begin();
	for(; it!= indices.end(); it++)
	{

		IndexVariable* iv = *it;
		if(iv->is_source())
		{
			IndexVariable* inst_iv = IndexVariable::make_inst_source(iv);
			replacements[iv] = inst_iv;
		}
		else {
			if(!iv->is_target()) {
				continue;
				assert_context("Unexpected index var: " +
						AccessPath::safe_string(iv) + " in constraint: " +
						c.to_string());
				c_assert(false);
			}
			IndexVariable* inst_iv = IndexVariable::make_inst_target(iv);
			replacements[iv] = inst_iv;

		}
	}

	c.replace_terms(replacements);
	return c;

}

void IndexVarManager::get_sum_index_vars(AccessPath* ap, set<IndexVariable*>&
			sum_indices)
{
	set<IndexVariable*> indices;
	ap->get_nested_index_vars(indices);
	set<IndexVariable*>::iterator it = indices.begin();
	for(; it!= indices.end(); it++)
	{
		IndexVariable* iv = *it;
		if(iv->is_inst_source() || iv->is_inst_target()) {
			sum_indices.insert(iv);
		}
	}
}

void IndexVarManager::get_index_vars(Constraint c,
		set<IndexVariable*>& index_vars)
{
	assert_context("Getting index vars from constraint: "+ c.to_string());
	set<Term*> terms;
	c.get_terms(terms, true);
	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
 		if(t->get_specialization() == AP_INDEX)
		{
			AccessPath* ap = AccessPath::to_ap(t);
			c_assert(ap != NULL && ap->get_ap_type() == AP_INDEX);
			index_vars.insert((IndexVariable*) ap);
		}
	}
}


Term* inc_index_fn(Term* t, void* _base_to_index)
{
	map<AccessPath*, AccessPath*>* base_to_index =
			(map<AccessPath*, AccessPath*>*)_base_to_index;
	if(!t->is_specialized()) return t;
	AccessPath* ap = AccessPath::to_ap(t);

	if(base_to_index->count(ap) > 0)
	{
		AccessPath* l = (*base_to_index)[ap];
		il::type* t = ap->get_type();
		c_assert(t->is_pointer_type());
		int ap_size = t->get_inner_type()->get_size()/8;
		c_assert(ap_size!=0);
		return ArithmeticValue::make_plus(ap,
				ArithmeticValue::make_times(l,
						ConstantValue::make(ap_size)))->to_term();
	}

	if(ap->is_size_field_ap())
	{
		AccessPath* inner = ap->get_inner();
		if(inner->get_ap_type() == AP_ARRAYREF)
		{
			ArrayRef* ar = (ArrayRef*)inner;
			AccessPath* ar_inner = ar->get_inner()->strip_deref();
			if(base_to_index->count(ar_inner) == 0) return t;
			AccessPath* inc_amount = (*base_to_index)[ar_inner];

			inc_amount =
					ArithmeticValue::make_times(inc_amount,
							ConstantValue::make(ar->get_elem_size()));
			AccessPath* res = ArithmeticValue::make_minus(ap, inc_amount);
			return res->to_term();
		}
		else {
			AccessPath* inner = ap->get_inner()->strip_deref();
			if(base_to_index->count(inner) == 0) return t;
			AccessPath* inc_amount = (*base_to_index)[inner];
			AccessPath* res = ArithmeticValue::make_minus(ap, inc_amount);
			return res->to_term();

		}

	}
	if(ap->get_ap_type() == AP_FUNCTION)
	{
		FunctionValue* fv = (FunctionValue*) ap;

		if(fv->is_length())
		{
			AccessPath* inner = fv->get_arguments()[0];

			if(base_to_index->count(inner->strip_deref()) == 0) {
				return t;
			}
			AccessPath* inc_amount = (*base_to_index)[inner->strip_deref()];
			Term* res =
						ArithmeticValue::make_minus(fv, inc_amount)->to_term();

			return res;
			/*
			if(inner->get_ap_type() == AP_ARRAYREF)
			{
				ArrayRef* ar = (ArrayRef*) inner;
				AccessPath* ar_inner = ar->get_inner();
				AccessPath* key = ar_inner->strip_deref();
				if(base_to_index->count(key) == 0) {
					return t;
				}
				AccessPath* inc_amount = (*base_to_index)[key];
				inc_amount =
					ArithmeticValue::make_times(inc_amount, ar->get_elem_size());
				Term* res =
						ArithmeticValue::make_minus(fv, inc_amount)->to_term();

				return res;
			}*/


		}
	}



	if(ap->get_ap_type() != AP_ARRAYREF) return t;
	ArrayRef* ar = (ArrayRef*) ap;

	AccessPath* inner = ar->get_inner();
	AccessPath* inner_stripped = inner->strip_deref();
	if(base_to_index->count(inner_stripped) == 0) return t;
	AccessPath* inc_amount = (*base_to_index)[inner_stripped];
	AccessPath* index_exp = ar->get_index_expression();
	AccessPath* val = ArithmeticValue::make_plus(index_exp, inc_amount);
	AccessPath* res = ar->replace(index_exp, val);

	return res->to_term();
}

AccessPath* IndexVarManager::set_summary_indices_to_zero(AccessPath* ap)
{
	set<IndexVariable*> sum_indices;
	IndexVarManager::get_sum_index_vars(ap, sum_indices);
	map<AccessPath*, AccessPath*> reps;
	ConstantValue* zero = ConstantValue::make(0);
	set<IndexVariable*>::iterator it = sum_indices.begin();
	for(; it!= sum_indices.end(); it++) {
		IndexVariable* iv = *it;
		reps[iv] = zero;
	}
	return ap->replace(reps);
}

void IndexVarManager::increment_index(Constraint& c, map<AccessPath*, AccessPath*>&
		base_to_index)
{
	c.replace_terms(inc_index_fn, &base_to_index);
}


Constraint IndexVarManager::get_outer_indices_zero_constraint(AccessPath* ap)
{
	Constraint res;
	while(ap!=NULL && ap->get_ap_type() != AP_DEREF) {
		IndexVariable* iv = ap->get_index_var();
		if(iv!=NULL)  {
			IndexVariable* target = IndexVariable::make_target(iv);
			res &= ConstraintGenerator::get_eqz_constraint(target);
		}
		ap = ap->get_inner();
	}
	return res;

}

Constraint IndexVarManager::get_source_eq_target_constraint(AccessPath* ap)
{
	Constraint res(true);
	set<IndexVariable*> indices;
	ap->get_nested_index_vars(indices);
	set<IndexVariable*>::iterator it = indices.begin();
	for(; it!= indices.end(); it++)
	{
		IndexVariable* iv = *it;
		IndexVariable* target = IndexVariable::make_target(iv);
		res &=ConstraintGenerator::get_eq_constraint(iv, target);
	}
	return res;
}

IndexVarManager::~IndexVarManager() {
}
