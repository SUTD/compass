/*
 * InvariantEnforcer.cpp
 *
 *  Created on: Sep 20, 2009
 *      Author: tdillig
 */

#include "InvariantEnforcer.h"
#include "AccessPath.h"
#include "MemoryLocation.h"
#include "Edge.h"
#include "IndexVariable.h"
#include "Variable.h"
#include "FunctionValue.h"
#include "ConstraintGenerator.h"
#include "ConstantValue.h"
#include "MemoryGraph.h"
#include "compass_assert.h"
#include "Instantiator.h"
#include "SummaryGraph.h"

InvariantEnforcer::InvariantEnforcer() {
	this->counter = 0;

}

void InvariantEnforcer::clear()
{
	this->counter = 0;
}

void InvariantEnforcer::enforce_existence_and_uniqueness(
		MemoryLocation* source, int offset, Constraint stmt_guard,
		MemoryGraph* mg)
{

	check_invariants(source, offset, mg);
	set<Edge*>* edges = source->get_successors(offset);
	bool is_precise = true;
	set<Edge*>::iterator it = edges->begin();
	for(; it!= edges->end(); it++)
	{
		Edge* e = *it;
		Constraint edge_c = e->get_constraint();
		if(!edge_c.is_precise()) {
			is_precise = false;
			break;
		}
	}

	if(is_precise) return;

	//cout << "STATEMENT GUARD: " << stmt_guard << endl;



	AccessPath* source_ap = source->get_access_path(offset);
	AccessPath* index = source_ap->find_outermost_index_var(true);

	AccessPath* d;

	if(index == NULL) {
		d = Variable::make_disjointness_var(counter,
					il::get_unsigned_integer_type());
	}
	else {
		d= FunctionValue::make_disjoint(counter, index,
				il::get_unsigned_integer_type());
	}

	it= edges->begin();
	int i=0;
	int num_edges = edges->size();
	for(; it!= edges->end(); it++, i++)
	{
		Edge* e = *it;
		Constraint orig_edge_c = e->get_constraint();


		Constraint edge_c = orig_edge_c;
		edge_c.assume(stmt_guard);


		AccessPath* target_ap = e->get_target_ap();
		Constraint disjoint_c = get_disjointness_constraint(d,i, num_edges);




		Constraint target_only;
		Constraint target_c = get_target_constraint(edge_c, source_ap, target_ap,
				target_only);


		Constraint new_edge_c = disjoint_c & target_c;

		// Register this with the constraint solver
		if(!disjoint_c.is_true()) {
			Constraint::add_quantified_axiom(disjoint_c, edge_c);
		}
		else
			if(!target_only.is_true())
		{
			Constraint::add_quantified_axiom(target_only, edge_c);
		}

		new_edge_c &= stmt_guard;
		e->set_constraint(new_edge_c);

	}

	counter++;



}

void InvariantEnforcer::enforce_existence_and_uniqueness(Instantiator& inst,
		AccessPath* summary_source, Constraint sum_update_c,
		set<Edge*>& sum_edges,
		map<Constraint, Constraint> & uniqueness_to_summary)
{

	assert_context("Enforcing existence and uniqueness on: " +
			summary_source->to_string());


	map<Edge*, Constraint> inst_constraints;
	bool is_precise = true;
	set<Edge*>::iterator it = sum_edges.begin();
	for(; it!= sum_edges.end(); it++)
	{
		Edge* e = *it;


		AccessPath* source_ap = e->get_source_ap()->strip_deref();
		AccessPath* target_ap = e->get_target_ap()->strip_deref();


		Constraint edge_c = e->get_constraint();
		Constraint inst_edge_c = inst.instantiate_constraint(edge_c);
		inst_edge_c.assume(inst.stmt_guard);

		inst_constraints[e] = inst_edge_c;
		if(!edge_c.is_precise()) {
			is_precise = false;
		}
	}



	Constraint inst_update_c = inst.instantiate_constraint(sum_update_c);
	inst_update_c.assume(inst.stmt_guard);



	if(is_precise) {
		inst.inst_update_c[summary_source] = inst_update_c;
		set<Edge*>::iterator it = sum_edges.begin();
		for(; it!= sum_edges.end(); it++) {
			Edge* e = *it;
			inst.edge_to_inst_constraints[e] = inst_constraints[e];
		}
		return;
	}





	AccessPath* index = summary_source->find_outermost_index_var(true);

	AccessPath* d;

	if(index == NULL) {
		d = Variable::make_disjointness_var(counter,
					il::get_unsigned_integer_type());
	}
	else {
		d= FunctionValue::make_disjoint(counter, index,
				il::get_unsigned_integer_type());
	}

	it= sum_edges.begin();
	int i=0;
	int num_edges = sum_edges.size();

	if(!inst_update_c.valid()) num_edges++;

	for(; it!= sum_edges.end(); it++, i++)
	{
		Edge* e = *it;
		Constraint inst_edge_c = inst_constraints[e];


		AccessPath* target_ap = e->get_target_ap();
		Constraint disjoint_c = get_disjointness_constraint(d,i, num_edges);


		Constraint target_only;
		Constraint target_c = get_target_constraint(inst_edge_c, summary_source,
				target_ap, target_only);



		Constraint new_edge_c = disjoint_c & target_c;

		// Register this with the constraint solver
		if(!disjoint_c.is_true()) {
			Constraint::add_quantified_axiom(disjoint_c, inst_edge_c);
#ifdef ENABLE_USER_INTERACTION
			{
				Constraint orig_edge_c = inst.sg->get_original_edge_c(e);

				Constraint my_c = inst.instantiate_constraint(
						orig_edge_c, true);

				my_c = IndexVarManager::convert_index_vars_to_instantiation_vars(my_c);
				my_c = DisjointVarManager::
						replace_disjointness_constraints(my_c, inst.sg);
				uniqueness_to_summary[disjoint_c] = my_c;
				cout << "Add Mapping #1: " <<
						disjoint_c << " -> " << my_c << endl;

			}
#endif
		}
		else
			if(!target_only.is_true())
		{
			Constraint::add_quantified_axiom(target_only, inst_edge_c);
		}

		c_assert(inst.edge_to_inst_constraints.count(e) == 0);
		inst.edge_to_inst_constraints[e] = new_edge_c;


	}

	if(num_edges > sum_edges.size()) {
		Constraint disjoint_c = get_disjointness_constraint(d,i, num_edges);
		if(!disjoint_c.is_true()) {
			Constraint::add_quantified_axiom(disjoint_c, inst_update_c);
			//cout << "Update cond:" << endl;
			//cout << "Adding axiom:" << disjoint_c << " ---> " << inst_update_c << endl;
#ifdef ENABLE_USER_INTERACTION
			{
				Constraint orig_update_c =
						inst.sg->get_original_update_c(summary_source);
				Constraint my_c = inst.instantiate_constraint(
						orig_update_c, true);
				my_c = IndexVarManager::
						convert_index_vars_to_instantiation_vars(my_c);
				my_c = DisjointVarManager::
							replace_disjointness_constraints(my_c, inst.sg);
				uniqueness_to_summary[!disjoint_c] = my_c;

				cout << "Add Mapping #2: " <<
						!disjoint_c << " -> " << my_c << endl;

			}
#endif
		}
		inst.inst_update_c[summary_source] = !disjoint_c;

	}
	else {
		inst.inst_update_c[summary_source] = Constraint(true);
	}

	counter++;

}


Constraint InvariantEnforcer::get_disjointness_constraint(
		AccessPath* disjointness_term,
		int edge_index, int num_edges)
{
	Constraint disjoint_c;

	if(num_edges == 1)
	{
		disjoint_c = Constraint(true);
	}

	else if(num_edges == 2 && edge_index == 0)
	{
		disjoint_c = ConstraintGenerator::get_eqz_constraint(disjointness_term);
	}
	else if(num_edges == 2 && edge_index == 1)
	{
		disjoint_c = ConstraintGenerator::get_neqz_constraint(disjointness_term);
	}


	else if(edge_index == num_edges-1)
	{
		ConstantValue* c = ConstantValue::make(edge_index);
		disjoint_c =
				ConstraintGenerator::get_geq_constraint(disjointness_term, c);
	}

	else {
		ConstantValue* c = ConstantValue::make(edge_index);
		disjoint_c =
				ConstraintGenerator::get_eq_constraint(disjointness_term, c);
	}
	return disjoint_c;
}

Constraint InvariantEnforcer::get_target_constraint(Constraint& edge_c,
		AccessPath* source_ap,
		AccessPath* target_ap, Constraint & target_eq_only)
{

	Constraint target_c(true);


	/*
	 * If the target has an index variable we need to make sure
	 * it is a function of the source index.
	 */
	vector<IndexVariable*> index_vars;
	target_ap->get_nested_index_vars(index_vars);

	//AccessPath* target_index_ap = NULL;// = target_ap->find_outermost_index_var(false);
	//if(index_vars.size()>0) target_index_ap = index_vars[0];
	//if(target_index_ap != NULL)
	for(unsigned int i=0; i < index_vars.size(); i++)
	{
		IndexVariable* target_index = index_vars[i];
		if(target_index->is_source())
			target_index = IndexVariable::make_target(target_index);
		else target_index = IndexVariable::make_inst_target(target_index);

		AccessPath* source_index = source_ap->find_outermost_index_var(true);
		AccessPath* t;
		if(source_index == NULL) {
			t = Variable::make_target_var(counter);

		}
		else {
			t = FunctionValue::make_target(counter, source_index);
		}



		edge_c.replace_term(target_index->to_term(), t->to_term());
		target_c &= ConstraintGenerator::get_eq_constraint(target_index, t);
		target_eq_only &= target_c;
	}

	/*
	 * If there are shared index variables, we need to preserve them.
	 */
	set<IndexVariable*> source_indices;
	set<IndexVariable*> target_indices;
	set<IndexVariable*> intersection;
	source_ap->get_nested_index_vars(source_indices);
	target_ap->get_nested_index_vars(target_indices);
	set_intersection(source_indices.begin(), source_indices.end(),
			target_indices.begin(), target_indices.end(),
			insert_iterator<set<IndexVariable*> >(intersection,
					intersection.begin()));

	Constraint preserve_c;
	set<IndexVariable*>::iterator it = intersection.begin();
	for(; it!= intersection.end(); it++)
	{
		IndexVariable* source_index = *it;
		IndexVariable* target_index = IndexVariable::make_target(source_index);
		Constraint c = ConstraintGenerator::get_eq_constraint(source_index,
				target_index);
		preserve_c &= c;
	}
	Constraint res = target_c & preserve_c;
	return res;



}

InvariantEnforcer::~InvariantEnforcer() {
}
