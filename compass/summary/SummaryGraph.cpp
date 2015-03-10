/*
 * SummaryGraph.cpp
 *
 *  Created on: Oct 18, 2008
 *      Author: tdillig
 */

#include "MemoryGraph.h"
#include "AccessPath.h"
#include "SummaryGraph.h"
#include "MemoryLocation.h"
#include "Edge.h"
#include "util.h"
#include "access-path.h"
#include "SummaryClosure.h"
#include "sail/SummaryUnit.h"
#include "sail/Function.h"
#include "sail/SuperBlock.h"
#include "BlockExecutionCounter.h"
#include "Callgraph.h"
#include "MemoryAnalysis.h"
#include "compass_assert.h"
#include "util.h"
#include "sail/Variable.h"
#include "GlobalAnalysisState.h"
#include "MemoryLeakTrace.h"
#include "BasicBlock.h"
#include "LoopAbstractor.h"
#include "LoopInvariantChecker.h"

#define DEBUG false

/*
 * Abstract loops by giving fresh names to all values afterwards?
 */
#define ABSTRACT_LOOPS false


/*
 * Constructor for functions
 */
SummaryGraph::SummaryGraph(MemoryGraph* mg, Constraint return_cond,
		sail::Function* su)
{
	assert_context("Constructing summary graph for " +
			su->get_identifier().to_string());
	if(DEBUG) {
		cout << "Generating summary for: " << su->get_identifier().to_string()
				<< endl;
	}
	mg->ma.push_dotty_level("Generating Summary (function)");
	is_init_fn = su->is_init_function();


	int start = clock();

	this->mg = mg;
	this->sum_t = SUM_FUNCTION;
	this->su =  su;
	this->defining_class = su->get_defining_class();
	this->sum_id = su->get_identifier().to_string();
	this->sum_short_id = su->get_identifier().get_function_name();
	this->return_cond = return_cond;
	this->is_loop = false;
	this->sum_call_id = call_id(su->get_identifier());


	construct_reachable_graph();
	check_leaks();
	this->error_traces = mg->error_traces;

	collect_aps_in_error_traces();

	Constraint::disable_background();
	mg->ma.add_dotty("Reachable Graph", to_dotty());
	update_memory_graph();
	compute_update_conditions();
	eliminate_unobservables();

	mg->ma.add_dotty("After eliminating unobservables", to_dotty());

	collect_loop_termination_constraints();
	collect_access_paths_to_instantiate();
	Constraint::clear_background();

	eliminate_unused_iteration_counters();

	mg->ma.add_dotty("After eliminating unused iteration counters", to_dotty());


	check_summary_invariants(this);

	int end = clock();
	int time= end-start;
	double t = (((((double)time) /
			((double)CLOCKS_PER_SEC) )));
	//cout << "Summary generation time (function): " << t << endl;
	mg->ma.add_dotty("Final", to_dotty());
	mg->ma.pop_dotty_level();

#ifdef ENABLE_USER_INTERACTION
	uniqueness_to_sum_c = mg->uniqueness_to_sum_c;

	cout << "-----------UNIQUENESS TO SUM MAP----------" << endl;
	for(map<Constraint, Constraint>::iterator it = uniqueness_to_sum_c.begin();
			it!= uniqueness_to_sum_c.end(); it++)
	{
		Constraint c1 = it->first;
		Constraint c2 = it->second;
		cout << c1 << "->" << c2 << endl;

	}

	cout << "-----------END UNIQUENESS TO SUM MAP----------" << endl;



#endif /*ENABLE_USER_INTERACTION */

}

void SummaryGraph::collect_aps_in_error_traces()
{
#ifndef ENABLE_USER_INTERACTION
	return;
#endif


	set<ErrorTrace*>::iterator it = error_traces.begin();
	for(; it != error_traces.end(); it++)
	{
		ErrorTrace* et = *it;
		Constraint fail_c = et->get_failure_cond();
		cout << "fail c:" << fail_c << endl;
		set<Term*> terms;
		fail_c.get_terms(terms, true);
		set<Term*>::iterator it2 = terms.begin();
		for(; it2 != terms.end(); it2++)
		{
			Term* t = *it2;
			AccessPath* ap = AccessPath::to_ap(t);
			cout << "AP: " << ap->to_string() << endl;
			reachable_aps.insert(ap);
		}
	}
#ifdef ENABLE_USER_INTERACTION
	uniqueness_to_sum_c = mg->uniqueness_to_sum_c;
#endif /*ENABLE_USER_INTERACTION */
}



/*
 * Constructor for loops
 */
SummaryGraph::SummaryGraph(MemoryGraph* mg, Constraint return_cond,
		Constraint continuation_cond,
		sail::SuperBlock* su)
{
	cout << "******* STARTING LOOP SG ********** " << endl;
	mg->ma.push_dotty_level("Generating Summary (loop)");
	is_init_fn = false;

	int start = clock();
	if(DEBUG) {
		cout << "Generating summary for: " << su->get_identifier() << endl;
	}

	this->mg = mg;
	this->sum_t = SUM_LOOP;
	this->su =  su;
	this->defining_class = NULL;
	this->sum_id = su->get_identifier().to_string();
	this->sum_short_id = su->get_identifier().get_function_name();
	this->return_cond = return_cond;
	this->loop_continuation_cond = continuation_cond;
	this->error_traces = mg->error_traces;
	this->is_loop = true;
	this->sum_call_id = call_id(su->get_identifier());

	if(ABSTRACT_LOOPS) LoopInvariantChecker(*this, mg);
	construct_reachable_graph();
	check_leaks();
	this->error_traces = mg->error_traces;


	Constraint::disable_background();
	compute_update_conditions();
	mg->ma.add_dotty("Reachable Graph", to_dotty());
	if(!ABSTRACT_LOOPS)
		SummaryClosure(*this);
	else LoopAbstractor(*this);
	mg->ma.add_dotty("After Closure", to_dotty());
	//compute_update_conditions();

	collect_loop_termination_constraints();
	collect_access_paths_to_instantiate();
	update_memory_graph();


	set<ErrorTrace*>::iterator it = error_traces.begin();
	for(; it!= error_traces.end(); it++) {
		ErrorTrace* et = *it;
		Constraint new_c = this->include_counter_dependecies(et->get_failure_cond(),
				this->loop_termination_constraints);
		et->set_failure_cond(new_c);
	}


	int end = clock();
	int time= end-start;
	double t = (((((double)time) /
			((double)CLOCKS_PER_SEC) )));
	//cout << "Summary generation time (loop): " << t << endl;

	check_summary_invariants(this);

#ifdef ENABLE_USER_INTERACTION
	{
		postprocess_imprecise_values();
		postprocess_bracketing_constraints();
		original_update_conditions = update_conditions;
	}
#endif
	mg->ma.add_dotty("Final", to_dotty());
	mg->ma.pop_dotty_level();

	cout << "******* ENDING LOOP SG ********** " << endl;

}

void SummaryGraph::get_summary_entry_points(
		set<pair<MemoryLocation*, int> >& entry_points)

{
	set<Edge*, CompareTimestamp>::iterator it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* source_loc = e->get_source_loc();
		int source_offset = e->get_source_offset();
		set<Edge*> preds;
		get_predecessors(source_loc, source_offset, preds);
		if(preds.size() == 0) {
			entry_points.insert(make_pair(source_loc, source_offset));
		}

	}
}

void SummaryGraph::postprocess_bracketing_constraints()
{

	set<pair<MemoryLocation*, int> > entry_points;
	get_summary_entry_points(entry_points);

	queue<pair<MemoryLocation*, int> > worklist;
	for(set<pair<MemoryLocation*, int> >::iterator it = entry_points.begin();
			it != entry_points.end(); it++)
	{
		worklist.push(*it);
	}

	while(worklist.size() > 0) {
		pair<MemoryLocation*, int> cur = worklist.front();
		worklist.pop();

		MemoryLocation* loc = cur.first;
		int offset = cur.second;
		AccessPath* cur_ap = loc->get_access_path(offset);
		set<Edge*> succs;
		get_successors(loc, offset, succs);

		bool replace = false;
		for(set<Edge*>::iterator it = succs.begin(); it!= succs.end(); it++)
		{
			Edge* e = *it;
			if(!e->get_constraint().is_precise()) {
				replace = true;
				break;
			}
		}

		/*
		 * Make a new unmodeled value representing value after the loop
		 */
		if(replace) {
			AccessPath* source_ap = loc->get_access_path(offset);
			il::type* target_type = (*succs.begin())->get_target_ap()->get_type();
			string pp = "Value of " + source_ap->to_string() +
					" after the loop";
			DisplayTag dt(pp, mg->line, mg->file);
			AccessPath* imprecise_ap;
			IndexVariable* source_iv = source_ap->find_outermost_index_var(true);
			IndexVariable* target_iv = NULL;
			if(source_iv == NULL)
				imprecise_ap = UnmodeledValue::make_imprecise(
						target_type, dt);
			else{
				IndexVariable* target_iv_temp = IndexVariable::make_target();
				target_iv = IndexVariable::make_inst_source(target_iv_temp);

				imprecise_ap = UnmodeledValue::make_imprecise(
										target_type, target_iv, dt);
			}
			this->aps_to_instantiate.insert(imprecise_ap);
			cout << "--- Adding to aps to instantiate: " << imprecise_ap->to_string() << endl;

			MemoryLocation* new_loc = mg->get_location(imprecise_ap->add_deref());

			/*
			 * first, propagate constraints to targets
			 */
			set<Edge*> target_outgoing_edges;

			for(set<Edge*>::iterator it = succs.begin(); it!= succs.end(); it++)
			{
				Edge* e = *it;
				Constraint c = e->get_constraint();
				MemoryLocation* target_loc = e->get_target_loc();
				int target_offset = e->get_target_offset();
				mg->ivm.rename_inst_index_vars(c);
				set<Edge*> current_succs;
				get_successors(target_loc, target_offset, current_succs);
				for(set<Edge*>::iterator it =  current_succs.begin();
						it != current_succs.end(); it++)
				{
					Edge* cur_e = *it;
					target_outgoing_edges.insert(cur_e);
					Constraint cur_c = cur_e->get_constraint();
					cur_c &= c;
					cur_e->set_constraint(cur_c);
				}
			}

			/*
			 * Delete all edges from source to targets
			 */
			for(set<Edge*>::iterator it = succs.begin(); it!= succs.end(); it++)
			{
				Edge* e = *it;
				remove_edge(e);
			}
			Constraint new_c;
			if(update_conditions.count(cur_ap) > 0 &&
					update_conditions[cur_ap].is_precise()){
				new_c = update_conditions[cur_ap];
			}
			else {
				update_conditions[cur_ap] = Constraint(true);
			}
			if(source_iv != NULL) {
				IndexVariable* target_iv_target =
						IndexVariable::make_inst_target(target_iv);
				Constraint c(source_iv->to_term(),
						target_iv_target->to_term(), ATOM_EQ);
				new_c = c;
			}
			add_edge(loc, offset, new_loc, 0, new_c);
#ifdef ENABLE_USER_INTERACTION
			original_update_conditions[cur_ap] = update_conditions[cur_ap];
#endif


			/*
			 * Remove all edges outgoing to targets
			 */
			for(set<Edge*>::iterator it = target_outgoing_edges.begin();
					it!= target_outgoing_edges.end(); it++)
			{
				Edge* e = *it;
				remove_edge(e);
			}
			/*
			 * Add edges from unmodeled to target's targets
			 */
			for(set<Edge*>::iterator it = target_outgoing_edges.begin();
					it!= target_outgoing_edges.end(); it++)
			{
				Edge* e = *it;
				AccessPath* source_ap = e->get_source_ap();
				set<IndexVariable*> index_vars;
				source_ap->get_nested_index_vars(index_vars);


				Constraint c = e->get_constraint();


				MemoryLocation* target_loc = e->get_target_loc();
				IndexVariable* old_source_iv = e->get_source_ap()->find_outermost_index_var(true);
				if(old_source_iv != NULL && target_iv != NULL)
					c.replace_term(old_source_iv->to_term(), target_iv->to_term());
				IndexVarManager::eliminate_index_vars(c, index_vars);

				int target_offset = e->get_target_offset();
				add_edge(new_loc, 0, target_loc, target_offset, c);

			}

			/*
			 * Enqueue new location
			 */
			worklist.push(make_pair(new_loc, 0));
		}
		else
		{
			for(set<Edge*>::iterator it = succs.begin(); it!= succs.end(); it++)
			{
				Edge* e = *it;
				MemoryLocation* target_loc = e->get_target_loc();
				int target_offset = e->get_target_offset();
				worklist.push(make_pair(target_loc, target_offset));
			}
		}

	}


}


void SummaryGraph::postprocess_imprecise_values()
{

	map<AccessPath*, AccessPath*> replacements;
	map<Term*, Term*> constraint_replacements;

	//
	// Build the replacements map
	//
	set<Edge*, CompareTimestamp>::iterator it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* e = *it;
		AccessPath* target_ap_drf = e->get_target_ap();
		AccessPath* target_ap = target_ap_drf->strip_deref();
		if(!target_ap->contains_unmodeled_value()) continue;
		AccessPath* source_ap = e->get_source_ap();
		if(source_ap->contains_index_var()) continue;
		string description = "Value of " + source_ap->to_string() + " after loop";

		int line =
				(static_cast<sail::SuperBlock*>(su))->get_last_instruction()->line;
		string file = su->get_identifier().get_file_id().get_filename();
		DisplayTag dt(description, line, file);
		UnmodeledValue* uv = UnmodeledValue::make(target_ap->get_type(),
				UNM_IMPRECISE, dt);
		replacements[target_ap] = uv;
		constraint_replacements[target_ap->to_term()] = uv->to_term();

		Constraint bg = ConstraintGenerator::get_eq_constraint(uv, target_ap);
		this->return_cond &= bg;
	}

	//
	 // Now, do the replacements
	 //

	// edges
	 for(it = edges.begin(); it != edges.end(); it++)
	 {
		 Edge* e = *it;
		 e->get_source_loc()->replace(replacements);
		 e->get_target_loc()->replace(replacements);
		 e->get_constraint().replace_terms(constraint_replacements);
	 }

	 //
	  // aps_to_instantiate
	  //
	 set<AccessPath*> new_aps_to_inst;
	 for(set<AccessPath*>::iterator it = aps_to_instantiate.begin();
			 it!= aps_to_instantiate.end(); it++)
	 {
		 AccessPath* ap = *it;
		 AccessPath* new_ap = ap->replace(replacements);
		 new_aps_to_inst.insert(new_ap);
	 }

	 aps_to_instantiate = new_aps_to_inst;

	 //
	  // update conditions
	  //
	 map<AccessPath*, Constraint> new_update_c;
	 for(map<AccessPath*, Constraint>::iterator it = update_conditions.begin();
			 it != update_conditions.end(); it++)
	 {
		 AccessPath* ap = it->first;
		 Constraint c = it->second;
		 AccessPath* new_ap = ap->replace(replacements);
		 c.replace_terms(constraint_replacements);
		 new_update_c[new_ap] = c;
	 }
	 update_conditions = new_update_c;

	 //
	  // error traces
	  //
	 for(set<ErrorTrace*>::iterator it = error_traces.begin();
			 it!= error_traces.end(); it++)
	 {
		 ErrorTrace* e = *it;
		 Constraint new_c = e->get_failure_cond();
	     new_c.replace_terms(constraint_replacements);
		 e->set_failure_cond(new_c);
	 }


#ifdef ENABLE_USER_INTERACTION

	 for(map<Edge*, Constraint>::iterator it = original_edge_c.begin();
			 it!= original_edge_c.end(); it++)
	 {
		 Constraint c = it->second;
		 c.replace_terms(constraint_replacements);
		 it->second = c;
	 }

	 map<AccessPath*, Constraint> new_orig_update;
	 for(map<AccessPath*, Constraint>::iterator it = original_update_conditions.begin();
			 it != original_update_conditions.end(); it++)
	 {
		 AccessPath* ap = it->first;
		Constraint c = it->second;
		AccessPath* new_ap = ap->replace(replacements);
		c.replace_terms(constraint_replacements);
		new_orig_update[new_ap] = c;
	 }

	 this->original_update_conditions = new_orig_update;

	 for(map<Constraint, Constraint>::iterator it = uniqueness_to_sum_c.begin();
			 it!= uniqueness_to_sum_c.end(); it++)
	 {
		 Constraint c = it->second;
		 c.replace_terms(constraint_replacements);
		 it->second = c;
	 }
#endif

}


il::record_type* SummaryGraph::get_defining_class()
{
	return this->defining_class;
}

void SummaryGraph::compute_update_conditions()
{
	set<Edge*>::iterator it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* e = *it;
		AccessPath* source = e->get_source_ap();
		if(update_conditions.count(source) > 0) continue;
		if(source->get_ap_type() == AP_VARIABLE) {
			Variable* v= (Variable*) source;
			if(v->is_exit_variable()) {
				update_conditions[source] = Constraint(true);
				continue;
			}
		}

		set<Edge*> succs;
		get_successors(e->get_source_loc(), e->get_source_offset(), succs);
		Constraint update_c(false);
		set<Edge*>::iterator it2 = succs.begin();
		for(; it2!= succs.end(); it2++)
		{
			Edge* succ_e = *it2;
			Constraint c = succ_e->get_constraint();
			IndexVarManager::eliminate_target_vars(c);
			update_c |= c;
		}
		update_c.sat();
		update_c.valid();
		update_conditions[source] = update_c;
	}

	if(DEBUG) print_update_conditions();



}

void SummaryGraph::print_update_conditions()
{
	cout << "=======UPDATE CONDITIONS============" << endl;
	map<AccessPath*, Constraint>::iterator it = update_conditions.begin();
	for(; it!= update_conditions.end(); it++)
	{
		cout << "\t " << it->first->to_string() << " -> " << it->second
				<< endl;
	}
	cout << "=======+++++++++++++++============" << endl;

}

void SummaryGraph::eliminate_unused_iteration_counters()
{
	set<IterationCounter*> counters;
	set<Edge*>::iterator it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* e = *it;
		AccessPath* s = e->get_source_ap();
		AccessPath* t = e->get_target_ap();
		Constraint c = e->get_constraint();
		s->get_counters(counters, false);
		t->get_counters(counters, false);

		SummaryClosure::get_termination_vars(c, counters);

	}

	/*set<ErrorTrace*>::iterator it2 = error_traces.begin();
	for(; it2!= error_traces.end(); it2++) {
		ErrorTrace* et = *it2;
		SummaryClosure::get_termination_vars(et->get_failure_cond(), counters);
	}*/

	return_cond = return_cond.nc();
	SummaryClosure::get_termination_vars(return_cond.nc(), counters);

	set<IterationCounter*> all_counters;
	set<Constraint>::iterator it3 = loop_termination_constraints.begin();
	for(; it3!= loop_termination_constraints.end(); it3++)
	{
		Constraint c = *it3;
		c = c.nc();
		SummaryClosure::get_termination_vars(c, all_counters);
	}

	set<AccessPath*> to_eliminate;
	to_eliminate.insert(all_counters.begin(), all_counters.end());
	set<IterationCounter*>::iterator it4 = counters.begin();
	for(; it4!= counters.end(); it4++){
		IterationCounter* cur = *it4;
		to_eliminate.erase(cur);
	}

	if(to_eliminate.size() == 0) return;

	// Now eliminate the useless counters

	it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* e = *it;

		Constraint c = e->get_constraint();
		ConstraintGenerator::eliminate_evars(c, to_eliminate);
		e->set_constraint(c);

	}

	ConstraintGenerator::eliminate_evars(return_cond, to_eliminate);

	set<Constraint> new_constraints;
	it3 = loop_termination_constraints.begin();
	for(; it3!= loop_termination_constraints.end(); it3++)
	{
		Constraint c = *it3;
		ConstraintGenerator::eliminate_evars(c, to_eliminate);
		c.sat();
		new_constraints.insert(c);

	}

	loop_termination_constraints = new_constraints;





}


/*
 * If error traces contain N's, we want to explicitly add constraints
 * we know about N.
 */
Constraint SummaryGraph::include_counter_dependecies(Constraint c, set<Constraint>&
		counter_info)
{

	c.propagate_background();
	c.sat();

	Constraint res = c;
	set<Term*> terms;
	c.get_terms(terms, true);

	set<Term*> counters;
	set<Term*>::iterator it = terms.begin();
	for(; it!=terms.end(); it++)
	{
		Term* t = *it;

		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		if(ap->get_ap_type() == AP_COUNTER) {
			IterationCounter* ic = (IterationCounter*) ap;
			if(!ic->is_parametric()) {
				counters.insert(t);
			}

		}
	}


	//set<Constraint>::iterator it2 = mg->loop_termination_constraints.begin();
	set<Constraint>::iterator it2 = counter_info.begin();
	for(; it2!= counter_info.end(); it2++)
	{
		Constraint cur = *it2;
		if(cur.contains_term(counters)) {
			res&=cur;
		}
	}
#ifdef ENABLE_USER_INTERACTION
	{
		IndexVarManager::eliminate_source_vars(res);
		IndexVarManager::eliminate_target_vars(res);
		IndexVarManager::eliminate_sum_index_vars(res);
	}
#endif

	return res;
}


const set<ErrorTrace*> & SummaryGraph::get_error_traces() const
{
	return error_traces;
}


string SummaryGraph::info_to_string()
{
	string res = "{{ Return Condition: " + return_cond.to_string() + "\\n ";
	res+="Loop Counters:\\n";
	set<Constraint>::iterator it2 =
			loop_termination_constraints.begin();
	for(; it2 != loop_termination_constraints.end(); it2++)
	{
		res+=it2->to_string()+ "\\n";
	}

	res += "Error trace summary:\\n";
	set<ErrorTrace*>::iterator it = error_traces.begin();
	for(; it!= error_traces.end(); it++)
	{
		ErrorTrace* et = *it;
		res+=  et->to_string() + "\\n";
	}
	res+="}}";
	return  MemoryAnalysis::escape_dotty_string(res);

}



Constraint SummaryGraph::get_constraint(AccessPath* source, AccessPath* target)
{
	MemoryLocation* source_loc = mg->get_location(source);
	int source_offset = source_loc->find_offset(source);

	set<Edge*> edges;
	get_successors(source_loc, source_offset, edges);

	set<Edge*>::iterator it = edges.begin();
	for(; it!= edges.end(); it++) {
		Edge* e = *it;
		if(e->get_target_ap() == target) return e->get_constraint();
	}
	return Constraint(false);
}

Constraint SummaryGraph::get_update_condition(AccessPath* source)
{
	c_assert(update_conditions.count(source) > 0);
	return update_conditions[source];
}

bool SummaryGraph::is_init_fn_summary()
{
	return is_init_fn;
}


void SummaryGraph::construct_reachable_graph()
{


	MemoryLocation* ret_loc = NULL;
	/*
	 * For functions, only memory locations reachable from
	 * interface objects should be included in the summary.
	 */
	map<AccessPath*, MemoryLocation*>::iterator it = mg->ap_to_locs.begin();
	for(; it!= mg->ap_to_locs.end(); it++)
	{
		MemoryLocation* loc = it->second;
		if(loc->is_interface_object()) {
		//	cout << "************ Interface object: " <<
			//		loc->get_representative_access_path()->to_string() << endl;
			if(loc->get_access_path(0)->get_ap_type() == AP_VARIABLE) {
				Variable* v = (Variable*) loc->get_access_path(0);
				if(v->is_return_var()) ret_loc = loc;
			}
			entry_points.insert(loc);
			construct_reachable_graph(loc);
		}
	}
#ifdef ENABLE_USER_INTERACTION
	return;
#endif

	/*
	 * The remainder must be a heuristic to improve performance on
	 * some benchmarks...
	 */

	if(ret_loc == NULL){
		reachable_aps.clear();
	}
	else {
		set<AccessPath*> ret_aps;

		set<Edge*>* edges = ret_loc->get_successors(0);
		set<Edge*>::iterator it = edges->begin();
		for(; it!= edges->end(); it++)
		{
			Edge* e = *it;
			ret_aps.insert(e->get_target_ap()->strip_deref());
			ConstraintGenerator::get_nested_memory_aps(e->get_constraint(),
					ret_aps);
		}

		set<AccessPath*> intersection;
		set_intersection(reachable_aps.begin(), reachable_aps.end(),
				ret_aps.begin(), ret_aps.end(),
				insert_iterator<set<AccessPath*> >(intersection,
						intersection.begin()));
		reachable_aps.clear();
		reachable_aps.insert(intersection.begin(), intersection.end());

	}



	/*{
		cout << "Relevant aps: " << endl;
		set<AccessPath*>::iterator it = reachable_aps.begin();
		for(; it!= reachable_aps.end(); it++) {
			cout << "\t " << (*it)->to_string() << endl;
		}
	}*/

}

void SummaryGraph::construct_reachable_graph(MemoryLocation* loc)
{
	if(reachable_objects.count(loc) > 0) return;
	reachable_objects.insert(loc);

	AccessPath* rep = loc->get_representative_access_path();
	if(rep->is_size_field_ap()) return;


	map<int, set<Edge*> * >::iterator it=
			loc->get_successor_map().begin();


	for(; it!= loc->get_successor_map().end(); it++)
	{


		set<Edge*>::iterator it2 = it->second->begin();
		for(; it2 != it->second->end(); it2++)
		{
			Edge* cur_e = *it2;
			construct_reachable_graph(cur_e->get_target_loc());




			Constraint non_default_c = cur_e->get_non_default_constraint();
			if(non_default_c.sat()){
				cur_e->set_constraint(non_default_c);
				edges.insert(cur_e);
				/*if(cur_e->get_source_ap()->get_ap_type() == AP_VARIABLE)
				{
					Variable* v = (Variable*)cur_e->get_source_ap();
						if(v->is_return_var())
							reachable_aps.insert(cur_e->get_target_ap()->strip_deref());

				}*/
				reachable_aps.insert(cur_e->get_source_ap()->strip_deref());
				reachable_aps.insert(cur_e->get_target_ap()->strip_deref());

			}
		}
	}


}


void SummaryGraph::check_leaks()
{

	assert_context("Checking memory leaks.");
	if(!GlobalAnalysisState::check_memory_leaks())
		return;


	/*
	 * Uniquify the locations -- there might be "junk" left over
	 * from upgrading to arrays or ADT's
	 */
	set<AccessPath*> to_skip;
	map<AccessPath*, MemoryLocation*>::iterator it = mg->ap_to_locs.begin();
	for(; it!= mg->ap_to_locs.end(); it++)
	{
		AccessPath* rep  = it->first;
		if(rep->get_ap_type() == AP_ARRAYREF || rep->get_ap_type() == AP_ADT)
		{
			AccessPath* inner = rep->get_inner();
			if(mg->ap_to_locs.count(inner) > 0)
				//cout << "DUPLICATE FOUND!!! " << rep->to_string() << " and " <<
				//inner->to_string() << endl;
				//cout << "Memory location 1: "<< it->second->to_string(false) << endl;
				//cout << "Memory location 2:" << mg->ap_to_locs[inner]->to_string(false) << endl;
				to_skip.insert(inner);
		}
	}


	/*
	 * Anything that was allocated, but is no longer reachable is leaked.
	 */
	 it = mg->ap_to_locs.begin();
	for(; it!= mg->ap_to_locs.end(); it++)
	{
		AccessPath* ap = it->first;
		if(to_skip.count(ap) > 0) continue;
		MemoryLocation* loc = it->second;
		AccessPath* rep = loc->get_representative_access_path();
		if(rep->get_base() == NULL) continue;
		if(rep->get_base()->get_ap_type() != AP_ALLOC) continue;
		if(rep->get_num_derefs() != 1) continue;


		// Does not escape; so if it's not deleted, we report an error!
		if(mg->is_top_level_function ||
				reachable_objects.count(loc) == 0)
		{

			// Check if it is possible that this location was not deleted
			Constraint not_deleted_c = mg->get_not_deleted_constraint(loc);
			if(not_deleted_c.unsat()) continue;
			Alloc* a = (Alloc*) rep->get_base();
			int line = a->get_alloc_id().get_original_allocation_site().line;
			string file = a->get_alloc_id().get_original_allocation_site().file;

			MemoryLeakTrace* mlt = MemoryLeakTrace::make(a, a->to_string(),
					line, file, not_deleted_c,
					mg->ma.get_cur_iteration_counter());

			if(mlt == NULL) continue;

			mg->error_traces.insert(mlt);

		}

	}


	/*
	 * For locations that escape, compute the constraint
	 * under which they don't escape.
	 */
	set<MemoryLocation*>::iterator it2= reachable_objects.begin();
	for(; it2!= reachable_objects.end(); it2++)
	{
		MemoryLocation* loc = *it2;
		AccessPath* rep = loc->get_representative_access_path();
		if(rep->get_base() == NULL) continue;
		if(rep->get_base()->get_ap_type() != AP_ALLOC) continue;
		if(rep->get_num_derefs() != 1) continue;

		Constraint escape_c(false);

		map<int, AccessPath*>::iterator it3 = loc->get_access_path_map().begin();
		for(; it3 != loc->get_access_path_map().end(); it3++)
		{
			int offset = it3->first;
			if(it3->first < 0) continue;
			set<Edge*> preds;
			get_predecessors(loc, offset, preds);
			set<Edge*>::iterator pred_it = preds.begin();
			for(; pred_it != preds.end(); pred_it++)
			{
				Edge* e = *pred_it;
				Constraint c = e->get_constraint();
				IndexVarManager::eliminate_source_vars(c);
				IndexVarManager::eliminate_target_vars(c);
				escape_c |= c;
			}

		}



		Constraint not_escape_c = !escape_c;
		Constraint not_deleted_c = mg->get_not_deleted_constraint(loc);
		Constraint leak_c = not_deleted_c & not_escape_c;
		if(leak_c.unsat()) continue;
		Alloc* a = (Alloc*) rep->get_base();
		int line = a->get_alloc_id().get_original_allocation_site().line;
		string file = a->get_alloc_id().get_original_allocation_site().file;

		MemoryLeakTrace* mlt = MemoryLeakTrace::make(a, a->to_string(),
				line, file, leak_c,
				mg->ma.get_cur_iteration_counter());
		if(mlt == NULL) continue;
		mg->error_traces.insert(mlt);
	}
}

/*
 * Collects the set of atomic access paths that need to be instantiated
 * in the calling context and replaces index variables
 * with summary index variables to avoid naming collisions.
 */

void SummaryGraph::collect_access_paths_to_instantiate()
{
	set<Edge*>::iterator it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* e = *it;
		AccessPath* _source_ap = e->get_source_ap();
		AccessPath* _target_ap = e->get_target_ap();
		Constraint _c = e->get_constraint();

		AccessPath* source_ap = IndexVarManager::
				convert_index_vars_to_instantiation_vars(_source_ap);
		AccessPath* target_ap = IndexVarManager::
				convert_index_vars_to_instantiation_vars(_target_ap);
		Constraint c= IndexVarManager::
				convert_index_vars_to_instantiation_vars(_c);
		AccessPath* source_addr = source_ap->strip_deref();
		AccessPath* target_addr = target_ap->strip_deref();
		if(DEBUG)
		{
			cout << "SOURCE AP: " << source_ap->to_string() << endl;
			cout << "SUMMARY GRAPH AP TO INSTANTIATE: " <<
			source_addr->to_string() << endl;
		}
		aps_to_instantiate.insert(source_addr);
		aps_to_instantiate.insert(target_addr);

		collect_access_paths(c, aps_to_instantiate);
		e->set_source(source_ap);
		e->set_target(target_ap);
		e->set_constraint(c);

	}

	collect_access_paths(return_cond, aps_to_instantiate);

	/*{
		set<ErrorTrace*>::iterator it = error_traces.begin();
		for(; it!= error_traces.end(); it++)
		{
			ErrorTrace* et = *it;
			et->get_access_paths_to_instantiate(aps_to_instantiate);
		}
	}*/

	set<Constraint>::iterator it2 = loop_termination_constraints.begin();
	for(; it2!= loop_termination_constraints.end(); it2++)
	{
		collect_access_paths(*it2, aps_to_instantiate);
	}

	map<AccessPath*, Constraint> new_update_conditions;
	map<AccessPath*, Constraint>::iterator it3 = update_conditions.begin();
	for(; it3!= update_conditions.end(); it3++)
	{
		AccessPath* ap = it3->first;
		Constraint c = it3->second;

		AccessPath* new_ap = IndexVarManager::
				convert_index_vars_to_instantiation_vars(ap);
		Constraint new_c= IndexVarManager::
				convert_index_vars_to_instantiation_vars(c);

		new_update_conditions[new_ap] = new_c;

	}
	update_conditions = new_update_conditions;

#ifdef ENABLE_USER_INTERACTION
	{
		map<Edge*, Constraint>::iterator it = original_edge_c.begin();
		for(; it != original_edge_c.end(); it++)
		{
			collect_access_paths(it->second, aps_to_instantiate);
		}

		map<AccessPath*, Constraint>::iterator it2 =
				original_update_conditions.begin();
		for(; it2!= original_update_conditions.end(); it2++)
		{
			collect_access_paths(it2->second, aps_to_instantiate);
		}

	}
#endif /*ENABLE_USER_INTERACTION */

	if(DEBUG) {
		print_update_conditions();
	}



}

bool SummaryGraph::is_function_summary()
{
	return !is_loop;
}
bool SummaryGraph::is_loop_summary()
{
	return is_loop;
}

void SummaryGraph::collect_access_paths(Constraint c, set<AccessPath*>& aps)
{

	set<Term*> terms;
	c.get_terms(terms, false);
	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		collect_access_paths(t, aps);

	}
}



void SummaryGraph::collect_access_paths(Term* t, set<AccessPath*>& aps)
{
	assert_context("Collecting access paths from " + t->to_string());
	switch(t->get_term_type())
	{
	case CONSTANT_TERM:
	{
		c_assert(t->is_specialized());
		aps.insert(AccessPath::to_ap(t));
		break;
	}
	case VARIABLE_TERM:
	case FUNCTION_TERM:
	{
		if(!t->is_specialized()) cout << "Term is: " << t->to_string() << endl;
		c_assert(t->is_specialized());

		int apt = t->get_specialization();
		AccessPath* ap = AccessPath::to_ap(t);
		if(apt == AP_VARIABLE) aps.insert(ap);
		if(apt == AP_ADDRESS) aps.insert(ap);
		if(apt == AP_UNMODELED) aps.insert(ap);
		if(apt == AP_DEREF) aps.insert(ap);
		if(apt == AP_COUNTER) aps.insert(ap);
		if(apt == AP_FIELD) aps.insert(ap);
		if(apt == AP_ARRAYREF) aps.insert(ap);
		if(apt == AP_ALLOC) aps.insert(ap);
		if(apt == AP_ADT) aps.insert(ap);
		if(apt == AP_FUNCTION) {
			aps.insert(ap);
		}
		if(apt == AP_STRING) aps.insert(ap);
		if(apt == AP_PROGRAM_FUNCTION) aps.insert(ap);


		break;
	}
	case ARITHMETIC_TERM:
	{
		ArithmeticTerm* at = (ArithmeticTerm*) t;
		const map<Term*, long int>& elems = at->get_elems();
		map<Term*, long int>::const_iterator it = elems.begin();
		for(; it!= elems.end(); it++)
		{
			Term* cur = it->first;
			collect_access_paths(cur, aps);
		}
		break;
	}
	default:
		c_assert(false);

	}
}

/*
 * We refine edge constraints in two ways:
 * 1) We refine edge constraints to a location L (not originally accessible in
 * the calling context) by assuming the disjunction of
 * the incoming edge constraints to L. This is useful if an unobservable
 * variable appears in the incoming edge constraints to L and the elimination
 * causes this edge constraint to become imprecise. This transformation
 * is correct because future referents to L in the calling context must
 * access it through one of the referents to L in the summary.
 *
 * 2) If the disjunction of the incoming edges and the return condition
 * imply that a location is non-null, we set its attribute to be >0.
 * This is very important in many cases, e.g.,
 * int* t = malloc(...);
 * if(!t) exit();
 * return t;
 *
 * Here, we eliminate the allocation, but we still need to know that the
 * return value is non-null.
 */


void SummaryGraph::refine_edge_constraints()
{
	vector<MemoryLocation*> order;
	set<MemoryLocation*> visited;
	set<MemoryLocation*>::iterator loc_it = entry_points.begin();
	for(; loc_it != entry_points.end(); loc_it++)
	{
		MemoryLocation* loc = *loc_it;
		get_postordering(loc, order, visited);
	}

	for(int i= 0; i< (int)order.size(); i++)
	{

		MemoryLocation* target_loc = order[i];
		AccessPath* ap = target_loc->get_representative_access_path();
		AccessPath* base = ap->get_base();
		if(base == NULL) continue;
		// If the base is a variable, this location may be accessible
		// directly in the calling context
		if(base->get_ap_type() == AP_VARIABLE) continue;


		Constraint pred_constraints(false);
		map<int, AccessPath*>::iterator it =
				target_loc->get_access_path_map().begin();
		for(; it != target_loc->get_access_path_map().end(); it++)
		{
			int offset = it->first;

			set<Edge*> preds;
			get_predecessors(target_loc, offset, preds);
			set<Edge*>::iterator it2 = preds.begin();
			Constraint cur_pred_c(false);
			for(; it2!= preds.end(); it2++)
			{
				Edge* cur_e = *it2;
				Constraint cur_c = cur_e->get_constraint();
				IndexVarManager::eliminate_source_vars(cur_c);
				IndexVarManager::eliminate_target_vars(cur_c);
				cur_pred_c |= cur_c;
			}
			// If the return condition implies an
			// allocation is non-null, add an attribute.
			AccessPath* target_stripped =
					target_loc->get_access_path(offset)->strip_deref();

			if(target_stripped->get_type()->is_pointer_type()
					&& target_stripped->get_base()->get_ap_type() != AP_VARIABLE) {
				Constraint null_c =
						ConstraintGenerator::get_eqz_constraint(target_stripped);
				null_c &= cur_pred_c;
				Constraint query_c = null_c & return_cond;
				if(query_c.unsat_discard()) {
					Constraint::set_gtz_attribute(target_stripped->to_term());
				}
			}


			pred_constraints |= cur_pred_c;

		}




		if(pred_constraints.nc().valid()) continue;

		it = target_loc->get_access_path_map().begin();
		for(; it != target_loc->get_access_path_map().end(); it++)
		{
			int target_offset = it->first;
			set<Edge*> succs;
			get_successors(target_loc, target_offset, succs);

			set<Edge*>::iterator it2 = succs.begin();
			for(; it2!=succs.end(); it2++)
			{
				Edge* cur_e = *it2;
				if(cur_e->get_source_offset() == DELETED_OFFSET) continue;
				Constraint cur_c = cur_e->get_constraint();
				cur_c.assume(pred_constraints);
				cur_e->set_constraint(cur_c);


			}
		}


	}
}


void SummaryGraph::get_postordering(MemoryLocation* loc,
		vector<MemoryLocation*> & order,
		set<MemoryLocation*>& processed)
{
	if(processed.count(loc) > 0) return;
	processed.insert(loc);

	map<int, AccessPath*>::iterator it2 = loc->get_access_path_map().begin();
	for(; it2 != loc->get_access_path_map().end(); it2++)
	{
		int offset = it2->first;
		set<Edge*> succs;
		get_successors(loc, offset, succs);
		set<Edge*>::iterator it = succs.begin();
		for(; it!= succs.end(); it++)
		{
			Edge* e = *it;
			get_postordering(e->get_target_loc(), order,
					processed);
		}

	}
	order.push_back(loc);




}

void SummaryGraph::eliminate_unobservables()
{
	cout << "ELIMINATING UNOBSERVABLES!!!" << endl;
	set<ErrorTrace*>::iterator it2 = error_traces.begin();
	for(; it2!= error_traces.end(); it2++)
	{
		ErrorTrace* e = *it2;
		Constraint error_c = e->get_failure_cond();

#ifndef ENABLE_USER_INTERACTION
		{
			error_c = include_counter_dependecies(error_c,
				mg->loop_termination_constraints);
		}
#endif
		error_c.sat();

		Constraint new_error_c;
#ifdef ENABLE_USER_INTERACTION
			new_error_c =  error_c;
#endif
#ifndef ENABLE_USER_INTERACTION
			new_error_c = eliminate_unobservables(error_c, false, false);
#endif
		e->set_failure_cond(new_error_c);
	}


	refine_edge_constraints();

	set<Edge*>::iterator it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* e = *it;


		Constraint edge_c = e->get_constraint();
		Constraint new_edge_c = eliminate_unobservables(edge_c, true, true);
	//	bool res = new_edge_c.sc().implies(edge_c.nc());
		//assert(res);
#ifdef ENABLE_USER_INTERACTION
		original_edge_c[e] = edge_c;
#endif
		e->set_constraint(new_edge_c);

	}



	map<AccessPath*, Constraint>::iterator it3 = update_conditions.begin();
	for(; it3!= update_conditions.end(); it3++)
	{
		Constraint c = it3->second;
		Constraint new_c = eliminate_unobservables(c, true, true);
#ifdef ENABLE_USER_INTERACTION
		{

			cout << "******** adding to orig_update_c ******* " <<
					it3->first->to_string() << endl;
			original_update_conditions[
		IndexVarManager::convert_index_vars_to_instantiation_vars(it3->first)] = c;
		}
#endif
		it3->second = new_c;
	}
#ifndef ENABLE_USER_INTERACTION
	{
		return_cond = eliminate_unobservables(return_cond, true, true);
	}
#endif



}

Constraint SummaryGraph::get_original_edge_c(Edge* e)
{
#ifdef ENABLE_USER_INTERACTION
		if(original_edge_c.count(e) == 0){
			assert_context("Getting orig constraint for " + e->to_string() +
					"address: " + int_to_string((long int)e));
			c_assert(false);
		}
		return original_edge_c[e];
#endif /*ENABLE_USER_INTERACTION */
	assert(false);
}

Constraint SummaryGraph::get_original_update_c(AccessPath* ap)
{
#ifdef ENABLE_USER_INTERACTION
		cout << "Update c for " << ap->to_string() << endl;
		cout << "AP address: " << (long int)ap << endl;

		map<AccessPath*, Constraint>::iterator it =
				original_update_conditions.begin();
		for(; it != original_update_conditions.end(); it++) {
			cout << it->first->to_string() << "--> " <<
					it->second << endl;
			cout << "AP address: " << (long int)it->first << endl;

		}


		if(original_update_conditions.count(ap) == 0){
			assert_context("Cannot find original update condition for: "
					+ ap->to_string());
			c_assert(false);
		}
		return original_update_conditions[ap];
#endif /*ENABLE_USER_INTERACTION */
	assert(false);
}

Constraint SummaryGraph::get_constraint_for_disjointness_leaf(Constraint c)
{
#ifdef ENABLE_USER_INTERACTION
		assert(uniqueness_to_sum_c.count(c) > 0);
		return uniqueness_to_sum_c[c];
#endif /*ENABLE_USER_INTERACTION */
	assert(false);
}

map<Constraint, Constraint> & SummaryGraph::get_disjointness_map()
{
#ifdef ENABLE_USER_INTERACTION
		return uniqueness_to_sum_c;
#endif /*ENABLE_USER_INTERACTION */
	assert(false);
}

Constraint SummaryGraph::eliminate_unobservables(Constraint c,
		bool eliminate_counters, bool keep_reachable_unobservables)
{



	Constraint orig_c = c;



	c.propagate_background();


	c.sat();
	c.valid();




	if(keep_reachable_unobservables) {
		ConstraintGenerator::eliminate_unobservables(c, is_loop, reachable_aps,
			eliminate_counters);

	}
	else {
		set<AccessPath*> empty;
		ConstraintGenerator::eliminate_unobservables(c, is_loop, empty,
			eliminate_counters);
	}






	ConstraintGenerator::eliminate_disjointness_terms(c);


	ConstraintGenerator::eliminate_background_target_variables(c);

	c.sat();
	c.valid();

	return c;
}

void SummaryGraph::collect_loop_termination_constraints()
{

	set<Constraint>::iterator it = mg->loop_termination_constraints.begin();
	for(; it!= mg->loop_termination_constraints.end(); it++)
	{
		Constraint c = *it;
		c = eliminate_unobservables(c, true, false);
		if(c.nc().valid()) continue;
		loop_termination_constraints.insert(c);
	}

}


/*
 * Remove all reachable memory location from the memory graph and
 * claim their ownership.
 */
void SummaryGraph::update_memory_graph()
{
	set<Edge*>::iterator it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* e = *it;
		mg->relinquish_ownership(e);
	}
}


Edge* SummaryGraph::get_existing_edge(MemoryLocation* source, int source_offset,
		MemoryLocation* target, int target_offset)
{

	set<Edge*>* succs = source->get_successors(source_offset);
	if (succs == NULL)
		return NULL;
	set<Edge*>::iterator it = succs->begin();
	for (; it != succs->end(); it++) {
		Edge* e = *it;
		if(edges.count(e) == 0) continue;
 		if (e->get_target_loc() == target && e->get_target_offset()
				== target_offset) {
			return e;

 		}
	}
	return NULL;
}


void SummaryGraph::clear_edges(MemoryLocation* source_loc, int source_offset)
{
	set<Edge*> succs;
	get_successors(source_loc, source_offset, succs);
	set<Edge*>::iterator it = succs.begin();
	for(; it!= succs.end(); it++) {
		Edge* e = *it;
		edges.erase(e);
	}

}

/*
 * Adds an edge between the given memory locations
 */
Edge* SummaryGraph::add_edge(MemoryLocation* source_loc, int source_offset,
		MemoryLocation* target_loc, int target_offset,
		Constraint c, int timestamp)
{

	AccessPath* target_ap = target_loc->get_access_path(target_offset);

	AccessPath* def_target =
		source_loc->get_default_target(source_offset);

	/*
	 * Determine if there is already an existing edge between source and target
	 */
	Edge* existing = get_existing_edge(source_loc,  source_offset,
			target_loc, target_offset);


	int ts = (timestamp == -1 ? mg->edge_counter++ : timestamp);
	Edge* e = new Edge(mg->ivm, ts, c, source_loc, target_loc,
			source_offset, target_offset, target_ap == def_target, 0);

	// Default edges aren't allowed in the summary
	if(e->is_default_edge()) {
		return NULL;
	}

	Constraint non_default_c = e->get_non_default_constraint();
	e->set_constraint(non_default_c);


	if(existing != NULL) {
 		bool edge_exists= (e->get_constraint().nc().
 				implies(existing->get_constraint().nc()) &&
 			existing->get_constraint().sc().implies(e->get_constraint().sc()));
		if(!edge_exists)
			existing->or_constraint(mg->ivm, e->get_constraint(), 0);
		else existing = NULL;
		delete e;
		e = existing;
	}
	else  {
		edges.insert(e);

	}
	return e;

}



void  SummaryGraph::get_successors(MemoryLocation* loc, int offset,
		set<Edge*>& successors)
{
	if(!loc->has_field_at_offset(offset)) return;
	set<Edge*>* succs = loc->get_successors(offset);

	set<Edge*>::iterator it = succs->begin();
	for(; it!= succs->end(); it++)
	{
		Edge* e = *it;
		if(edges.count(e) == 0) continue;
		successors.insert(e);
	}
}
void  SummaryGraph::get_predecessors(MemoryLocation* loc, int offset,
		set<Edge*>& preds)
{
	if(!loc->has_field_at_offset(offset)) return;
	set<Edge*>* p = loc->get_predecessors(offset);
	set<Edge*>::iterator it = p->begin();
	for(; it!= p->end(); it++)
	{
		Edge* e = *it;
		if(edges.count(e) == 0) continue;
		preds.insert(e);
	}
}
/*
 * Gives the guarded value set for loc&offset
 * combination. The result of get_value_set is *always* in terms of
 * source variables, both for the constraint as well as the access paths.
 */
void SummaryGraph::get_value_set(AccessPath* ap,
		set<pair<AccessPath*, Constraint> >& values,
		Constraint c)
{
	MemoryLocation* loc = mg->get_location(ap);
	int offset = loc->find_offset(ap);
	set<Edge*>* targets = loc->get_successors(offset);
	set<Edge*>::const_iterator it = targets->begin();
	for(; it!=targets->end(); it++)
	{
		Edge* e = *it;
		if(edges.count(e) == 0) continue;
		MemoryLocation* target = e->get_target_loc();
		int target_offset = e->get_target_offset();
		AccessPath* target_ap  = target->get_access_path(target_offset);
		target_ap = target_ap->strip_deref();
		AccessPath* res = target_ap;
		Constraint target_c = e->get_constraint();
		mg->ivm.rename_index_vars(target_c);
		Constraint sat_c = target_c & c;
		if(!sat_c.sat()) continue;
		values.insert(pair<AccessPath*, Constraint>(res, target_c));
	}
}


void SummaryGraph::remove_edge(Edge* e)
{
	edges.erase(e);
}

string SummaryGraph::get_sum_id()
{
	return sum_id;
}

call_id SummaryGraph::get_call_id()
{
	return sum_call_id;
}

string SummaryGraph::get_sum_short_id()
{
	return sum_short_id;
}

Constraint SummaryGraph::get_loop_termination_constraints()
{
	Constraint res;
	set<Constraint>::iterator it = loop_termination_constraints.begin();
	for(; it!= loop_termination_constraints.end(); it++) {
		Constraint cur = *it;
		res &= cur;
	}

	return res;
}

Constraint SummaryGraph::get_return_cond()
{
	return return_cond;
}

Identifier  SummaryGraph::get_identifier()
{
	return su->get_identifier();
}


const set<Constraint> &  SummaryGraph::get_loop_counter_relations()
{
	return loop_termination_constraints;
}


string SummaryGraph::to_dotty()
{
	set<MemoryLocation*> printed;

	string res;
	res = "digraph G { rankdir=LR;\n";
	res+= "ranksep=1.2;\n";
	res+= "nodesep=0.9;\n";

	res+=string("node_info") + " [shape = record]";
	res+="[port=e][height=0.1][label= \"";

	res+=info_to_string();
	res+= "\"]\n";


	set<Edge*, CompareTimestamp>::iterator it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* e = *it;
		MemoryLocation* loc = e->get_source_loc();
		if(printed.count(loc) == 0){
			printed.insert(loc);
			res+=string("node")+int_to_string((long int)loc) + " [shape = record]";
			res+="[port=e][height=0.1][label= \"";
			res+=loc->to_dotty();
			res+= "\"]\n";
		}
		loc = e->get_target_loc();
		if(printed.count(loc) == 0){
			printed.insert(loc);
			res+=string("node")+int_to_string((long int)loc) + " [shape = record]";
			res+="[port=e][height=0.1][label= \"";
			res+=loc->to_dotty();
			res+= "\"]\n";
		}

		Constraint edge_c = e->get_constraint();
		string s = edge_c.to_string();
		s += "[" + int_to_string(e->get_time_stamp()) + "]";
		s+= e->has_default_target() ? "(d)" : "";
		res+= "\"node" + int_to_string((long int)e->get_source_loc())+"\"";
		if(e->get_source_offset()>=0 )
			res+=":off_s" + int_to_string(e->get_source_offset());
		else //res+=":off_size_s";
			res+=":off_s_minus" + int_to_string(-e->get_source_offset());
		res +=" -> ";
		res+= "\"node" + int_to_string((long int)e->get_target_loc()) + "\"";
		if(e->get_target_offset() >=0)
			res+=":off_e" + int_to_string(e->get_target_offset());
		else //res+=":off_size_e";
			res+=":off_e_minus" + int_to_string(-e->get_target_offset());
		res += " [label = \"";
		res+= s;
		res+="\"]\n";




	}
	res+="}\n";\
	return res;
}


string SummaryGraph::to_dotty(const string& filter)
{
	/*
	 * Find all edges whose source contains "filter".
	 */
	set<Edge*> to_print;

	set<Edge*, CompareTimestamp>::iterator it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* e = *it;
		AccessPath* source_ap = e->get_source_ap();
		if(source_ap->to_string().find(filter)!=string::npos)
			to_print.insert(e);
	}

	set<Edge*> worklist = to_print;
	while(worklist.size() > 0)
	{
		Edge* e = *worklist.begin();
		worklist.erase(e);

		to_print.insert(e);
		set<Edge*> succs;
		get_successors(e->get_source_loc(), e->get_source_offset(), succs);
		set<Edge*>::iterator it = succs.begin();
		for(; it != succs.end(); it++)
		{
			Edge* cur_suc = *it;
			if(to_print.count(cur_suc) > 0) continue;
			worklist.insert(cur_suc);
		}
	}

	set<MemoryLocation*> printed;

	string res;
	res = "digraph G { rankdir=LR;\n";
	res+= "ranksep=1.2;\n";
	res+= "nodesep=0.7;\n";

	/*res+=string("node_info") + " [shape = record]";
	res+="[port=e][height=0.1][label= \"";

	res+=info_to_string();
	res+= "\"]\n";
	*/

	set<Edge*>::iterator it2 = to_print.begin();
	for(; it2!= to_print.end(); it2++)
	{
		Edge* e = *it2;
		MemoryLocation* loc = e->get_source_loc();
		if(printed.count(loc) == 0){
			printed.insert(loc);
			res+=string("node")+int_to_string((long int)loc) + " [shape = record]";
			res+="[port=e][height=0.1][label= \"";
			res+=loc->to_dotty();
			res+= "\"]\n";
		}
		loc = e->get_target_loc();
		if(printed.count(loc) == 0){
			printed.insert(loc);
			res+=string("node")+int_to_string((long int)loc) + " [shape = record]";
			res+="[port=e][height=0.1][label= \"";
			res+=loc->to_dotty();
			res+= "\"]\n";
		}

		Constraint edge_c = e->get_constraint();
		string s = edge_c.to_string();
		s += "[" + int_to_string(e->get_time_stamp()) + "]";
		s+= e->has_default_target() ? "(d)" : "";
		res+= "\"node" + int_to_string((long int)e->get_source_loc())+"\"";
		if(e->get_source_offset()>=0 )
			res+=":off_s" + int_to_string(e->get_source_offset());
		else //res+=":off_size_s";
			res+=":off_s_minus" + int_to_string(-e->get_source_offset());
		res +=" -> ";
		res+= "\"node" + int_to_string((long int)e->get_target_loc()) + "\"";
		if(e->get_target_offset() >=0)
			res+=":off_e" + int_to_string(e->get_target_offset());
		else //res+=":off_size_e";
			res+=":off_e_minus" + int_to_string(-e->get_target_offset());
		res += " [label = \"";
		res+= s;
		res+="\"]\n";




	}
	res+="}\n";\
	return res;

}
