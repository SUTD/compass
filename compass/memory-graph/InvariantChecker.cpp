/*
 * InvariantChecker.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: isil
 */

#include "InvariantChecker.h"
#include "MemoryGraph.h"
#include "MemoryLocation.h"
#include "Edge.h"
#include "compass_assert.h"
#include "Variable.h"
#include "SummaryGraph.h"
#include "sail/SummaryUnit.h"
#include "MemoryAnalysis.h"
#include "FieldSelection.h"


#define HARD_ASSERT false

InvariantChecker::InvariantChecker()
{
}

bool InvariantChecker::is_redundant_rtti_field(MemoryLocation* loc, int offset)
{
	if(offset != RTTI_OFFSET) return false;
	AccessPath* rep = loc->get_representative_access_path();
	il::type* t = rep->get_type();
	if(!t->is_record_type()) return false;
	il::record_type* rt = (il::record_type*) t;
	return (!rt->has_virtual_methods());
}

void InvariantChecker::check_soundness_invariants(MemoryLocation* loc,
		int offset, MemoryGraph* mg)
{


	/*
	 * Special edge case: If this offset is associated with the rtti field
	 * and the type has no virtual methods, then the rtti may be legitimately
	 * uninitialized because we don't to look it up.
	 */
	if(is_redundant_rtti_field(loc, offset)) return;


	AccessPath* ap = loc->get_access_path(offset);
	/*
	 * Soundness invariant: Necessary conditions on outgoing points-to edges
	 * should or up to true, and sufficient conditions should be mutually
	 * exclusive.
	 */

	set<Edge*>* edges = loc->get_successors(offset);

	check_sc_mutually_exclusive(ap, edges, Constraint());


	Constraint nc_disjunction(false);
	set<Edge*>::iterator it = edges->begin();
	for(; it!= edges->end(); it++) {
		Edge* e = *it;
		nc_disjunction |= e->get_constraint().nc();

	}

	IndexVarManager::eliminate_target_vars(nc_disjunction);
	IndexVarManager::eliminate_source_vars(nc_disjunction);
	IndexVarManager::remove_parametric_loop_vars(nc_disjunction);

	set<Term*> terms;
	nc_disjunction.get_terms(terms, true);

	set<AccessPath*> to_eliminate;
	set<Term*>::iterator it2 = terms.begin();
	for(; it2 != terms.end(); it2++) {
		Term* t= *it2;

		if(t->get_specialization() == AP_PROGRAM_FUNCTION) {
			to_eliminate.insert(AccessPath::to_ap(t));
		}

		if(t->get_specialization() == AP_TYPE_CONSTANT) {
			to_eliminate.insert(AccessPath::to_ap(t));
		}

		if(t->get_term_type()== VARIABLE_TERM) {
			VariableTerm* vt= (VariableTerm*) t;
			if(vt->get_id_attribute() == ATTRIB_TARGET)
				to_eliminate.insert(AccessPath::to_ap(t));
		}

		else if(t->get_term_type()== FUNCTION_TERM) {
			FunctionTerm* vt= (FunctionTerm*) t;
			if(vt->get_id_attribute() == ATTRIB_TARGET ||
					vt->get_id_attribute() == ATTRIB_ADT_POS)
				to_eliminate.insert(AccessPath::to_ap(t));
		}


	}

	ConstraintGenerator::eliminate_evars(nc_disjunction, to_eliminate);

	nc_disjunction = nc_disjunction.nc();
	if(!nc_disjunction.valid_discard()) {
		nc_disjunction.valid();
		assert_context("Soundness violation detected on edges from " +
				ap->to_string() + " : Disjunction of necessary"
				"conditions is not valid: " + nc_disjunction.to_string()) ;
		if(HARD_ASSERT) c_assert(false);
		else c_warn(false);
	}


}


void InvariantChecker::_check_invariants(MemoryLocation* loc, int offset,
		MemoryGraph* mg)
{
	/*
	 * Special edge case: If this offset is associated with the rtti field
	 * and the type has no virtual methods, then the rtti may be legitimately
	 * uninitialized because we don't to look it up.
	 */
	if(is_redundant_rtti_field(loc, offset)) return;

	if(mg->ma.get_current_stmt_guard().unsat_discard()) return;

	if(!loc->has_field_at_offset(offset)) return;
	AccessPath* ap = loc->get_access_path(offset);

	/*
	 * There should be at least one outgoing edge.
	 */
	set<Edge*>* edges = loc->get_successors(offset);
	if(edges->size() == 0) {
		assert_context("No outgoing edges from " + ap->to_string());
		if(HARD_ASSERT) c_assert(false);
		else c_warn(false);
	}


	/*
	 * We don't check invariants on temporary variables because
	 * they are handled specially.
	 */
	if(ap->get_base()->get_ap_type() == AP_VARIABLE) {
		Variable* v = (Variable*) ap->get_base();
		if(v->is_sail_temporary()){
			return;
		}
		if(v->is_summary_temp()) {
			return;
		}
	}
	/*
	 * Check that target is not a temporary because this is illegal
	 */

	set<Edge*>::iterator it = edges->begin();
	for(; it!= edges->end(); it++) {
		Edge* e = *it;
		check_target_restrictions(e);
	}


	/*
	 * Check that sufficent conditions are pairwise contradictory and necessary
	 * conditions or to true.
	 */
	check_soundness_invariants(loc, offset, mg);






}

bool InvariantChecker::check_target_restrictions(Edge* e)
{
	AccessPath* target_ap = e->get_target_ap();
	set<AccessPath*> aps;
	target_ap->get_nested_memory_aps(aps);
	set<AccessPath*>::iterator it = aps.begin();
	for(; it!= aps.end(); it++) {
		AccessPath* cur = *it;
		AccessPath* base = cur->get_base();
		if(base == NULL) continue;

		if(base->get_ap_type() == AP_VARIABLE) {
			Variable* v = (Variable*) base;
 			if( (v->is_sail_temporary() && !v->is_anonymous()) || v->is_summary_temp()){
				assert_context("Target of edge contains illegal access path: " +
						target_ap->to_string());
				if(HARD_ASSERT) c_assert(false);
				else c_warn(false);
				return false;
			}



		}

	}
	return true;

}

bool InvariantChecker::check_sc_mutually_exclusive(AccessPath* ap,
		set<Edge*>* edges, Constraint bg)
{
	set<Edge*>::iterator it = edges->begin();
	for(; it != edges->end(); it++) {
		Edge* e = *it;
		Constraint c = e->get_constraint();
		Constraint sc = c.sc();

		set<Edge*>::iterator it2 = it;
		it2++;
		for(; it2!= edges->end(); it2++) {
			Edge* e2 = *it2;
			Constraint c2 = e2->get_constraint();
			Constraint sc2 = c2.sc();

			Constraint pair_c = sc & sc2 & bg;

			if(pair_c.sat_discard()) {
				assert_context("Soundness violation detected on edges from " +
						ap->to_string() + " : Pairwise conjunction of sufficient"
								" conditions is satisfiable for edges: " +
								e->to_string() + " and " + e2->to_string() + "\n"
								+ " Constraints: " + e->get_constraint().to_string()
								+ " and " + e2->get_constraint().to_string());
				if(HARD_ASSERT) c_assert(false);
				else c_warn(false);
				return false;
			}

		}
	}
	return true;
}

void InvariantChecker::compute_transitive_edge_constraints(
		SummaryGraph* sg, map<AccessPath*, Constraint>& pred_constraints)
{
	vector<MemoryLocation*> order;
	set<MemoryLocation*> visited;
	set<MemoryLocation*>::iterator loc_it = sg->entry_points.begin();
	for(; loc_it != sg->entry_points.end(); loc_it++)
	{
		MemoryLocation* loc = *loc_it;
		sg->get_postordering(loc, order, visited);
	}

	for(int i= order.size()-1; i>=0; i--)
	{

		MemoryLocation* target_loc = order[i];

		AccessPath* ap = target_loc->get_representative_access_path();

		AccessPath* base = ap->get_base();
		if(base == NULL) continue;
		// If the base is a variable, this location may be accessible
		// directly in the calling context
		if(base->get_ap_type() == AP_VARIABLE) continue;

		map<int, AccessPath*>::iterator it =
				target_loc->get_access_path_map().begin();
		for(; it != target_loc->get_access_path_map().end(); it++)
		{
			int offset = it->first;
			AccessPath* cur_ap = target_loc->get_access_path(offset);

			set<Edge*> succs;
			sg->get_successors(target_loc, offset, succs);
			if(succs.size() == 0) continue;

			set<Edge*> preds;
			sg->get_predecessors(target_loc, offset, preds);


			Constraint pred_c;
			if(preds.size() != 0) {

				set<Edge*>::iterator it2 = preds.begin();
				pred_c = Constraint(false);
				for(; it2!= preds.end(); it2++)
				{
					Edge* cur_e = *it2;
					AccessPath* cur_source = cur_e->get_source_ap();
					Constraint cur_c = pred_constraints[cur_source];
					IndexVarManager::eliminate_source_vars(cur_c);
					IndexVarManager::eliminate_target_vars(cur_c);
					pred_c |= (cur_c & cur_e->get_constraint());
				}

			}
			pred_constraints[cur_ap] = pred_c;


			}

	}
}

void InvariantChecker::_check_summary_invariants(SummaryGraph* sg)
{

	assert_context("Checking summary invariants for " +
			sg->su->get_identifier().to_string());

	map<AccessPath*, Constraint> pred_constraints;
	compute_transitive_edge_constraints(sg, pred_constraints);


	set<AccessPath*> checked_locations;
	Constraint bg = sg->get_loop_termination_constraints() & sg->get_return_cond();
	set<Edge*>::iterator it =  sg->edges.begin();
	for(; it != sg->edges.end(); it++)
	{
		Edge* e = *it;
		AccessPath* source_ap = e->get_source_ap();

		check_target_restrictions(e);


		if(source_ap->get_base()->is_exit_variable()) continue;
		if(checked_locations.count(source_ap) > 0) continue;

		MemoryLocation* loc = e->get_source_loc();
		int offset = e->get_source_offset();

		set<Edge*> succs;
		sg->get_successors(loc, offset, succs);

		Constraint pred_c = pred_constraints[source_ap];


		check_sc_mutually_exclusive(source_ap, &succs, bg & pred_c );

	}
}

InvariantChecker::~InvariantChecker()
{
}
