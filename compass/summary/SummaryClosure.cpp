/*
 * SummaryClosure.cpp
 *
 *  Created on: Mar 9, 2009
 *      Author: isil
 */

#include "SummaryClosure.h"
#include "SummaryGraph.h"
#include "MemoryLocation.h"
#include "access-path.h"
#include "MemoryGraph.h"
#include <algorithm>
#include "util.h"
#include "sail/SummaryUnit.h"
#include "ErrorTrace.h"
#include "Variable.h"
#include "MemoryAnalysis.h"
#include "ConstraintGenerator.h"
#include "BlockExecutionCounter.h"
#include "IndexVariable.h"
#include "sail/BasicBlock.h"
#include "sail/CfgEdge.h"
#include "sail/Symbol.h"
#include "Instantiator.h"
#include "sail/SuperBlock.h"
#include "compass_assert.h"
#include "GlobalAnalysisState.h"

#define DEBUG false
#define INVALID_COUNTER -1

int generalization::counter;

generalization::generalization()
{

}

AccessPath* generalization::get_generalization(gen_kind_t kind, int counter_id)
{
	map<AccessPath*, AccessPath*> replacements;
	set<IterationCounter*> counters;
	IterationCounter* new_counter;
	if(kind == PARAMETRIC) {
		new_counter = IterationCounter::make_parametric(counter_id);
		parametric->get_counters(counters, true);
	}
	else {
		new_counter = IterationCounter::make_last(counter_id);
		last->get_counters(counters, false);
	}
	set<IterationCounter*>::iterator it = counters.begin();
	for(; it!= counters.end(); it++) {
		IterationCounter* v = *it;
		replacements[v] =new_counter;
	}

	if(kind == PARAMETRIC)
		return parametric->replace(replacements);
	else
		return last->replace(replacements);


}

generalization::generalization(AccessPath* base,
		AccessPath* ap, Constraint c,  int cur_counter)
{

	this->base = base;
	c.sat();
	c.valid();
	constraint = c;
	AccessPath* parametric_var= IterationCounter::make_parametric(cur_counter);
	AccessPath* termination_var = ArithmeticValue::make_minus(
			IterationCounter::make_last(cur_counter), ConstantValue::make(1));

	if(ap->get_ap_type() == AP_UNMODELED)
	{
		UnmodeledValue* uv = static_cast<UnmodeledValue*>(ap);
		parametric = UnmodeledValue::make_imprecise(ap->get_type(),
				parametric_var, uv->get_display_tag());
		last = parametric->replace(parametric_var, termination_var);
		increment = 0;
		return;
	}
	ArithmeticValue* av = (ArithmeticValue*) ap;
	long int constant = av->get_constant();
	map<Term*, long int> param_terms;
	param_terms[base->to_term()] += 1;
	param_terms[parametric_var->to_term()] += constant;
	parametric = ArithmeticValue::make(param_terms, 0);


	map<Term*, long int> last_it_terms;
	last_it_terms[base->to_term()] += 1;
	last_it_terms[termination_var->to_term()] += constant;
	last = ArithmeticValue::make(last_it_terms, 0);


	increment = constant;





}



long int generalization::get_increment()
{
	return increment;
}

string generalization::to_string()
{
	string res = parametric->to_string() + "( "+
		last->to_string() + ") \\ " + constraint.to_string();
	return res;
}

// -----------------------------------------------

generalization_set::generalization_set()
{

}

void generalization_set::clear()
{
	generalizations.clear();
}



bool generalization_set::add_generalization(AccessPath* base,
		AccessPath* ap, Constraint c,  int cur_counter)
{
	bool res = true;
	AccessPath* av = ArithmeticValue::make_minus(ap, base);
	if(av->get_ap_type() != AP_CONSTANT) {
		DisplayTag dt(base->to_string() + "@" +
				int_to_string(cur_counter), -1, "<loop1>");
		ap = UnmodeledValue::make_imprecise(ap->get_type(), dt);
		res = false;
	}

	generalization* g = new generalization(base, ap, c, cur_counter);
	generalizations.insert(g);
	return res;
}

void generalization_set::add_generalization(generalization* g)
{
	generalizations.insert(g);
}



string generalization_set::to_string()
{

	string res = "[";
	set<generalization*>::iterator it = generalizations.begin();
	unsigned int i = 0;
	for(; it!=generalizations.end(); it++, i++ )
	{
		generalization* g = *it;
		res += g->to_string();
		if(i != generalizations.size()-1) res += ", ";

	}
	res += "]";
	return res;
}

int generalization_set::size()
{
	return generalizations.size();
}

generalization_set::~generalization_set()
{
	clear();
}

//----------------------------------------------------------------

SummaryClosure::SummaryClosure(SummaryGraph& sg,
		vector<string>* visuals):sg(sg)
{
	assert_context("Doing summary closure for " +
			sg.su->get_identifier().to_string());
	generalization::counter = 0;
	this->visuals = visuals;

	collect_summary_locs();
	pointer_index_counter = 0;
	preprocess_pointer_arithmetic();
	compute_generalizations();




	sg.eliminate_unobservables();
	Constraint::clear_background();
	sg.mg->ma.add_dotty("After eliminating unobservables: ", sg.to_dotty());

	if(DEBUG) {
		print_linear_scalars();
		print_non_generalizable_aps();
	}


	compute_termination_constraint();
	eliminate_non_linear_aps();

	sg.mg->ma.add_dotty("After eliminating non-linear dependencies: ", sg.to_dotty());

	generalize_constraints();
	sg.mg->ma.add_dotty("After generalizing constraints: ", sg.to_dotty());
	generalize_targets();
	sg.mg->ma.add_dotty("After generalizing targets: ", sg.to_dotty());
	close_graph();

	generalize_error_traces();
	sg.mg->ma.add_dotty("After closing: ", sg.to_dotty());
	eliminate_iteration_counters();
	sg.mg->ma.add_dotty("After eliminating counters: ", sg.to_dotty());
	postprocess_pointer_arithmetic();
	collect_used_counters();

	if(DEBUG) {
		print_used_counters();
	}
	minimize_iteration_counters();
	uniquify_error_traces();
	sg.loop_termination_constraints.insert(termination_cond);
}


void SummaryClosure::uniquify_error_traces()
{
	set<ErrorTrace*> new_traces;
	set<ErrorTrace*>::iterator it = sg.error_traces.begin();
	for(; it!= sg.error_traces.end(); it++)
	{
		ErrorTrace* et = *it;
		ErrorTrace* new_et = et->set_counter(0);
		new_traces.insert(new_et);
	}
	sg.error_traces = new_traces;
}

void SummaryClosure::minimize_iteration_counters()
{
	set<VariableTerm*> terms;
	for(int i=0; i<= sg.mg->ma.get_bec()->get_max_counter(); i++)
	{
		IterationCounter* ic = IterationCounter::make_last(i);
		if(counters.count(ic) > 0) continue;
		terms.insert(ic);
	}

	termination_cond.eliminate_evars(terms);
	termination_cond.sat();


	if(counters.size() != 1) return;
	IterationCounter* ic = IterationCounter::make_last(*counters.begin());
	Constraint c = termination_cond;
	c.assume(ConstraintGenerator::get_neq_constraint(ic,
			ConstantValue::make(0)));

	Constraint loop_entry_c = !termination_cond;
	loop_entry_c.replace_term(ic, ConstantValue::make(0));

	loop_entry_c.sat();

	Term* eq_term = c.find_equality(ic->to_term());
	termination_cond = termination_cond.nc();
	if(eq_term == NULL) return;
	if(!eq_term->is_specialized()) return;
	AccessPath* eq_ap = AccessPath::to_ap(eq_term);

	set<Edge*>::iterator it = sg.edges.begin();
	map<Edge*, AccessPath*> new_targets;
	for(; it!= sg.edges.end(); it++)
	{
		Edge* e = *it;
		Constraint c= e->get_constraint();

		c.replace_term(ic, eq_ap->to_term());
		c &= loop_entry_c;
		c.sat();
		c.valid();


		AccessPath* target = e->get_target_ap();
		AccessPath* new_target = target->replace(ic, eq_ap);
		e->set_constraint(c);

		if(new_target == target) continue;
		new_targets[e] = new_target;





	}

	map<Edge*, AccessPath*>::iterator it3 = new_targets.begin();
	for(; it3!= new_targets.end(); it3++)
	{
		Edge* old_e = it3->first;
		AccessPath* new_target = it3->second;

		MemoryLocation* source_loc = old_e->get_source_loc();
		int source_offset = old_e->get_source_offset();
		MemoryLocation* target_loc = sg.mg->get_location(new_target);
		int target_offset = target_loc->find_offset(new_target);
		int timestamp = old_e->get_time_stamp();


		sg.remove_edge(old_e);
		sg.add_edge(source_loc, source_offset, target_loc,
				target_offset, old_e->get_constraint(), timestamp);
	}

	set<ErrorTrace*>::iterator it2 = sg.error_traces.begin();
	for(; it2!= sg.error_traces.end(); it2++)
	{
		ErrorTrace* et = *it2;
		Constraint failure_c = et->get_failure_cond();




		if(failure_c.contains_term(ic->to_term())) {
			failure_c.replace_term(ic, eq_ap->to_term());
			failure_c &= loop_entry_c;
			failure_c.sat();
			failure_c.valid();
			et->set_failure_cond(failure_c);
		}


	}

	termination_cond = Constraint(true);

}

void SummaryClosure::collect_used_counters()
{
	set<Edge*>::iterator it = sg.edges.begin();
	for(; it!= sg.edges.end(); it++)
	{
		Edge* e = *it;
		e->get_source_ap()->get_counters(counters, false);
		e->get_target_ap()->get_counters(counters, false);
		get_termination_vars(e->get_constraint(), counters);
	}

	set<ErrorTrace*>::iterator it2 = sg.error_traces.begin();
	for(; it2!= sg.error_traces.end(); it2++) {
		ErrorTrace* et = *it2;
		get_termination_vars(et->get_failure_cond(), counters);
	}

}

inline Variable* SummaryClosure::get_pointer_index()
{
	string name = "l" + int_to_string(pointer_index_counter++);
	Variable* v= Variable::make_temp(name);
	return v;
}

/*
 * Makes pointer arithmetic explicit by introducing integer variables
 * so that we can treat pointer arithmetic and explicit index based
 * side effects more uniformly. For instance, if there is a side
 * effect a++, this is turned into a points to (*a)[i] under the constraint
 * i=l_a and l_a points to *(l_a + 1).
 */
void SummaryClosure::preprocess_pointer_arithmetic()
{

	assert_context("Preprocessing pointer arithmetic.");
	set<Edge*>::iterator it = sg.edges.begin();
	for(; it!=sg.edges.end(); it++)
	{
		Edge* se = *it;

		/*
		 * If edge does not go to the default target, there cannot be pointer
		 * arithmetic.
		 */
		if(!se->has_default_target() ||
				se->get_source_ap()->find_outermost_index_var(true)!= NULL) {
			continue;
		}

		Constraint edge_c = se->get_constraint();
		IndexVariable* source_index_var =
				se->get_target_ap()->find_outermost_index_var(false);
		c_assert(source_index_var != NULL);
		IndexVariable* target_index_var =
				IndexVariable::make_target(source_index_var);
		/*
		 * Figure out all the possible increments
		 */
		map<Term*, Constraint>  equalities;
		edge_c.get_disjunctive_equalities(target_index_var,equalities);

		/*
		 * Introduce variables l_i for each index variable i
		 * and stipulate i = l_i
		 */
		Variable* fake_index = get_pointer_index();
		pointer_indices.insert(fake_index);
		base_to_index[se->get_source_ap()] = fake_index;

		AccessPath* source_sub = ArithmeticValue::make_minus(source_index_var,
				fake_index);
		AccessPath* target_sub = ArithmeticValue::make_minus(target_index_var,
						fake_index);
		pointer_arithmetic_subs[source_index_var] = source_sub->to_term();
		pointer_arithmetic_subs[target_index_var] = target_sub->to_term();




		MemoryLocation* new_loc = sg.mg->get_location(fake_index);
		locs.insert(new_loc);
		if(equalities.size() > 0) {
		Constraint edge_c = se->get_constraint();
		edge_c.assume(sg.loop_continuation_cond);
		se->set_constraint(edge_c);
			map<Term*, Constraint>::iterator it2 = equalities.begin();
			for(; it2!=equalities.end(); it2++)
			{
				Term* _cur_val = it2->first;
				c_assert(_cur_val->is_specialized());
				AccessPath* cur_val = AccessPath::to_ap(_cur_val);



				Constraint c = it2->second;
				AccessPath* av = ArithmeticValue::make_plus(fake_index, cur_val);

				AccessPath* target_ap = av->add_deref();
				MemoryLocation* target_loc = sg.mg->get_location(target_ap);
				locs.insert(target_loc);
				Edge* e = sg.add_edge(new_loc, 0, target_loc, 0, c);
				c_assert(e != NULL);
				e->get_block_ids().clear();
				e->get_block_ids().insert(se->get_block_ids().begin(),
						se->get_block_ids().end());

			}
		}
		else {

			DisplayTag dt("imprecise loop pointer arithmetic", this->sg.mg->line,
					this->sg.mg->file);
			AccessPath* target_ap = UnmodeledValue::make_imprecise(
					il::get_integer_type(), dt);
			target_ap = ArithmeticValue::make_plus(fake_index,
					target_ap);
			target_ap = target_ap->add_deref();
			MemoryLocation* target_loc = sg.mg->get_location(target_ap);
			locs.insert(target_loc);
			Edge* e = sg.add_edge(new_loc, 0, target_loc, 0,
					Constraint(true));
			c_assert(e != NULL);
			e->get_block_ids().clear();
			e->get_block_ids().insert(se->get_block_ids().begin(),
				se->get_block_ids().end());

		}
	}

	if(DEBUG) print_base_to_index();
	preprocess_pointer_arithmetic_in_constraints();

}

void SummaryClosure::postprocess_pointer_arithmetic()
{
	assert_context("Post-processing pointer arithmetic");
	map<Term*, Term*> replacements;
	map<AccessPath*, AccessPath*> ap_replacements;
	AccessPath* zero = ConstantValue::make(0);
	set<AccessPath*>::iterator it = pointer_indices.begin();
	for(; it!= pointer_indices.end(); it++)
	{
		replacements[(*it)->to_term()] = zero->to_term();
		ap_replacements[*it] = zero;
	}


	sg.return_cond.replace_terms(replacements);
	termination_cond.replace_terms(replacements);
	sg.loop_continuation_cond.replace_terms(replacements);

	set<Edge*>::iterator it2 = sg.edges.begin();
	for(; it2!= sg.edges.end(); it2++)
	{
		Edge* se = *it2;
		MemoryLocation* source_loc = se->get_source_loc();
		MemoryLocation* target_loc = se->get_target_loc();
		int source_offset = se->get_source_offset();
		int target_offset = se->get_target_offset();
		AccessPath* source_ap = se->get_source_ap();
		AccessPath* target_ap = se->get_target_ap();
		source_ap = source_ap->replace(ap_replacements);
		target_ap = target_ap->replace(ap_replacements);
		source_loc->set_access_path(source_ap, source_offset);
		target_loc->set_access_path(target_ap, target_offset);


		Constraint c = se->get_constraint();
		c.replace_terms(replacements);
		se->set_constraint(c);
	}

	set<ErrorTrace*>::iterator it3 = sg.error_traces.begin();
	for(; it3!=sg.error_traces.end(); it3++)
	{
		ErrorTrace* et = *it3;
		Constraint c = et->get_failure_cond();
		c.replace_terms(replacements);
		et->set_failure_cond(c);
	}

	set<Edge*> to_delete;
	it = pointer_indices.begin();
	for(; it!= pointer_indices.end(); it++)
	{
		MemoryLocation* loc = sg.mg->get_location(*it);
		set<Edge*> succs;
		sg.get_successors(loc, 0, succs);
		to_delete.insert(succs.begin(), succs.end());

	}

	it2 = to_delete.begin();
	for(; it2!= to_delete.end(); it2++)
	{
		Edge* se = *it2;
		sg.remove_edge(se);
	}



}

void SummaryClosure::preprocess_pointer_arithmetic(Constraint & c)
{
	IndexVarManager::increment_index(c, base_to_index);
}

void SummaryClosure::preprocess_pointer_arithmetic_in_constraints()
{

	/*
	 * Fix constraint in return condition
	 */

	preprocess_pointer_arithmetic(sg.return_cond);
	preprocess_pointer_arithmetic(sg.loop_continuation_cond);
	/*
	 * Fix constraint in summary edges
	 */
	set<Edge*>::iterator edge_it = sg.edges.begin();
	for(; edge_it != sg.edges.end(); edge_it++)
	{
		Edge* e = *edge_it;
		Constraint sum_c = e->get_constraint();
		sum_c.replace_terms(pointer_arithmetic_subs);
		preprocess_pointer_arithmetic(sum_c);
		e->set_constraint(sum_c);
	}

	/*
	 * Fix constraint in static assert failure conditions
	 */
	set<ErrorTrace*>::iterator error_it = sg.error_traces.begin();
	for(; error_it!= sg.error_traces.end(); error_it++)
	{
		ErrorTrace* et = *error_it;
		Constraint failure_c = et->get_failure_cond();
		preprocess_pointer_arithmetic(failure_c);
		et->set_failure_cond(failure_c);
	}

}


Constraint SummaryClosure::get_original_constraint(generalization* g)
{
	AccessPath* base = g->base;
	AccessPath* gen = g->parametric;
	set<IterationCounter*> counters;
	gen->get_counters(counters, true);
	map<AccessPath*, AccessPath*> replacements;

	AccessPath* one = ConstantValue::make(1);
	set<IterationCounter*>::iterator it = counters.begin();
	for(; it!= counters.end(); it++) {
		replacements[*it] = one;
	}

	AccessPath* target = gen->replace(replacements);
	target = target->add_deref();
	Constraint c = sg.get_constraint(base, target);

	// This must be an imprecise generalization-- we should
	// leave it alone.
	if(c.is_false()) {
		return g->constraint;
	}
	return c;
}


void SummaryClosure::compute_termination_constraint()
{
	assert_context("Computing termination constraint");
	termination_cond = compute_generalized_termination_cond();

	termination_cond &= sg.mg->ma.get_bec()->get_counter_relations();
}

bool SummaryClosure::contains_termination_var(AccessPath* ap)
{
	set<IterationCounter*> counters;
	ap->get_counters(counters, false);
	return counters.size() > 0;
}

Constraint SummaryClosure::get_last_iteration_constraint(Constraint break_cond,
		sail::Block* exit_pred_block)
{


	Constraint cur_cont_c = break_cond;

	int counter_id =
		sg.mg->ma.get_bec()->get_iteration_counter(exit_pred_block->get_block_id());

	/*
	 * We assume the statement guard because the break cond also
	 * stipulates the statement guard at the exit block, which
	 * we isn't really part of the current break condition.
	 */
	cur_cont_c.assume(sg.mg->ma.get_stmt_guard(exit_pred_block, false));
	Constraint cont_c_no_stmt_g = !cur_cont_c;
	preprocess_pointer_arithmetic(cont_c_no_stmt_g);


	/*
	 * last_cond_stmt_g is the constraint that needs to hold
	 * during the last iteration.
	 */
	Constraint last_cond_no_stmt_g = generalize_constraint(cont_c_no_stmt_g,
					LAST, counter_id);
	ConstraintGenerator::eliminate_unobservables(last_cond_no_stmt_g, true,
			sg.reachable_aps, true);
	last_cond_no_stmt_g = increment_iteration_counters(last_cond_no_stmt_g,
			ConstantValue::make(1));

	/*
	 * Prev cond is the condition that holds on the iteration before the last.
	 */
	Constraint prev_cond =  increment_iteration_counters(last_cond_no_stmt_g,
				ConstantValue::make(1));
	Constraint simp_prev_cond = prev_cond;
	simp_prev_cond.assume(last_cond_no_stmt_g);

	/*
	 * The last iteration condition must imply something about the
	 * iteration before the previous iteration. For instance,
	 * N-1<size implies N-2< size. However, note that this is not
	 * equivalent to checking that last_cond_no_stmt_g implies
	 * prev_cond because it might only imply it under some additional
	 * constraint; hence we check if the prev cond
	 * can be simplified with respect to last_cond_no_stmt_g.
	 */
	if(simp_prev_cond != prev_cond)
	{
		/*
		 * simp_prev_cond is the part that is not very useful, and we
		 * want to obtain the part of the prev_cond that implies
		 * something about previous iterations.
		 */
		prev_cond.assume(simp_prev_cond);
		last_cond_no_stmt_g = increment_iteration_counters(prev_cond,
				ConstantValue::make(-1));

		IterationCounter* cur_ic = IterationCounter::make_last(counter_id);
		Constraint no_last_cond = ConstraintGenerator::get_eq_constraint(cur_ic,
				ConstantValue::make(1));
		Constraint last_cond = last_cond_no_stmt_g &
				sg.mg->ma.get_stmt_guard(exit_pred_block, false);
		return (last_cond | no_last_cond);

	}

	// The last iteration condition doesn't say something useful.
	else return Constraint(true);
}

Constraint SummaryClosure::compute_generalized_termination_cond()
{
	sail::BasicBlock* exit_block = sg.su->get_exit_block();
	set<sail::CfgEdge*>& preds = exit_block->get_predecessors();
	set<sail::CfgEdge*>::iterator it = preds.begin();
	Constraint term_cond(false);
	for(; it!= preds.end(); it++)
	{
		sail::CfgEdge* edge = *it;
		sail::Block* source_b = edge->get_source();
		sail::Symbol* cond = edge->get_cond();

		sail::Block* pred_b = source_b;
		if(cond == NULL) {
			sail::Block* pred = edge->get_source();
			if(pred->get_predecessors().size() == 1)
			{
				edge = *pred->get_predecessors().begin();
				cond = edge->get_cond();
				pred_b = edge->get_source();
			}
			if(cond == NULL) return Constraint();
		}

		Constraint orig_term_c = sg.mg->get_neqz_constraint(cond);
		Constraint cur_term_c = orig_term_c;
		preprocess_pointer_arithmetic(cur_term_c);
		int counter_id =
			sg.mg->ma.get_bec()->get_iteration_counter(source_b->get_block_id());
		IterationCounter* cur_ic = IterationCounter::make_last(counter_id);
		Constraint gen_term = generalize_constraint(cur_term_c, LAST, counter_id);

		Constraint last_it_constraint =
				get_last_iteration_constraint(orig_term_c, pred_b);


		gen_term &= last_it_constraint;


		if(sg.mg->ma.get_bec()->get_max_counter() < 2) {
			 term_cond |= gen_term;
			 continue;
		 }

		// If counter id is 0, we can related N0 to N1
		if(counter_id == 0)
		{
			IterationCounter* next_id = IterationCounter::make_last(counter_id+1);
			Constraint c = ConstraintGenerator::get_sum_constraint(cur_ic, next_id,
					ConstantValue::make(1));

			gen_term &= c;

		}
		else {
			IterationCounter* prev = IterationCounter::make_last(counter_id-1);
			Constraint c = ConstraintGenerator::get_eq_constraint(cur_ic, prev);
			gen_term &= c;
		}
		term_cond |= gen_term;
	}

	Constraint res = sg.eliminate_unobservables(term_cond, true, false);
	res.sat();
	res.valid();
	return res;

}


void SummaryClosure::collect_summary_locs()
{
	set<Edge*>::iterator it = sg.edges.begin();
	for(; it!= sg.edges.end(); it++)
	{
		Edge* e = *it;
		locs.insert(e->get_source_loc());
		locs.insert(e->get_target_loc());
	}
}

int SummaryClosure::get_iteration_counter(Edge* e)
{
	set<int>& block_ids = e->get_block_ids();
	set<int>::iterator it = block_ids.begin();
	int counter_id = INVALID_COUNTER;
	for(; it!= block_ids.end(); it++)
	{
		int cur_counter = sg.mg->ma.get_bec()->get_iteration_counter(*it);
		if(cur_counter == -1) return INVALID_COUNTER;
		if(counter_id == INVALID_COUNTER){
			counter_id = cur_counter;
		}
		else if(counter_id != cur_counter) return INVALID_COUNTER;
	}
	return counter_id;

}

void SummaryClosure::compute_generalizations()
{

	assert_context("Computing generalizations.");
	/*
	 * First compute all sources.
	 */
	{
		set<Edge*>::iterator it = sg.edges.begin();
		for(; it!= sg.edges.end(); it++)
		{
			Edge* e = *it;
			all_sources.insert(e->get_source_ap());
		}
	}


	set<MemoryLocation*>::iterator it = locs.begin();
	for(; it!= locs.end(); it++)
	{
		MemoryLocation* loc = *it;
		map<int, set<Edge*>* >& succs = loc->get_successor_map();
		map<int, set<Edge*>* >::iterator it2 = succs.begin();


		for(; it2!=succs.end(); it2++)
		{
			int offset = it2->first;
			AccessPath* source = loc->get_access_path(offset);

			set<Edge*> edges;
			sg.get_successors(loc, offset, edges);
			if(edges.size() == 0) continue;
			set<Edge*>::iterator it3 = edges.begin();
			map<Edge*, Constraint> edge_constraints;
			bool compute_gen = false;
			// The purpose of incremented is to determine if this value
			// is always incremented
			bool incremented = true;
			for(; it3!= edges.end(); it3++)
			{
				Edge* e = *it3;
				AccessPath* source = e->get_source_ap();

				// This must be due to ptr arithmetic, which is
				// handled specially
				if(e->has_default_target()) continue;
				if(source->is_deleted_field()){
					continue;
				}


				compute_gen =true;
				AccessPath* target = e->get_target_ap();
				AccessPath* target_stripped = target->strip_deref();






				if((target != Deref::make(target_stripped, target->get_type()) ||
						target_stripped->get_ap_type() != AP_ARITHMETIC)
						|| !target_stripped->contains_nested_access_path(source)
					) {
					non_generalizable_aps.insert(source);
					break;
				}

				AccessPath* delta = ArithmeticValue::make_minus(target_stripped, source);
				Constraint delta_geqz = ConstraintGenerator::get_geqz_constraint(delta);
				if(!delta_geqz.valid_discard()) {
					incremented = false;
				}

				Constraint edge_c = e->get_constraint();


				edge_c.assume(sg.loop_continuation_cond);
				ConstraintGenerator::eliminate_free_vars(edge_c,
						all_sources);
				edge_constraints[e] = edge_c;
			}
			if(!compute_gen || non_generalizable_aps.count(source) > 0){
				continue;
			}

			generalization_set& gs = generalization_map[source];
			generalized_aps.insert(source);

			set<Constraint> constraints;
			Constraint or_c(false);


			map<Edge*, Constraint>::iterator c_it =
					edge_constraints.begin();
			for(; c_it!= edge_constraints.end(); c_it++) {
				Constraint c = c_it->second;

				//IndexVarManager::eliminate_source_vars(c);
				//IndexVarManager::eliminate_target_vars(c);

				constraints.insert(c);
				or_c |= c;
			}



			if(!or_c.valid() || !ConstraintGenerator::disjoint(constraints)) {
				//non_generalizable_aps.insert(source);
				il::type* t = source->get_type();
				if(incremented) {
					if(this->pointer_indices.count(source)) {
						t = il::get_unsigned_integer_type();
					}
				}
				string pp = "Cannot generalize " + source->to_string() + " in loop";
				DisplayTag dt(pp, sg.mg->line, sg.mg->file);
				AccessPath* imprecise = UnmodeledValue::make_imprecise(t, dt);
				gs.add_generalization(source, imprecise ,
						Constraint(true), INVALID_COUNTER);


				continue;

			}



			/*
			 * If we got here, the source is recursively defined and
			 * may be generalizable.
			 */

			c_it = edge_constraints.begin();
			for(; c_it!= edge_constraints.end(); c_it++)
			{
				Edge* e = c_it->first;
				AccessPath* gen = e->get_target_ap()->strip_deref();
				AccessPath* av= ArithmeticValue::make_minus(gen,
						e->get_source_ap());


				Constraint c = c_it->second;
				if(av->get_ap_type() != AP_CONSTANT ||
						get_iteration_counter(e) == INVALID_COUNTER)
				{
					string pp = "Cannot generalize " + source->to_string() +
							" in loop";
					DisplayTag dt(pp, sg.mg->line, sg.mg->file);

					gen = UnmodeledValue::make_imprecise(gen->get_type(), dt);

				}
				c = sg.eliminate_unobservables(c, true, false);


				bool generalized = gs.add_generalization(source, gen, c,
						get_iteration_counter(e));
				if(!generalized) {
					non_generalizable_aps.insert(source);
				}

			}



		}
	}

}


void SummaryClosure::print_non_generalizable_aps()
{
	cout << "-----Begin non-generalizable access paths: " << endl;
	set<AccessPath*>::iterator it = this->non_generalizable_aps.begin();
	for(; it!= this->non_generalizable_aps.end(); it++)
	{
		AccessPath* ap = *it;
		cout << "\t " << ap->to_string() << endl;
	}
	cout << "-----End non-generalizable access paths" << endl;
}


void SummaryClosure::print_linear_scalars()
{
	cout << "******GENERALIZABLE ACCESS PATHS********" << endl;
	map<AccessPath*, generalization_set >::iterator it =
		generalization_map.begin();
	for(; it!= generalization_map.end(); it++)
	{
		AccessPath* ap = it->first;
		cout << "\t " << ap->to_string() << ": " <<
			it->second.to_string() << endl;
	}
	cout << "******END GENERALIZABLE ACCESS PATHS********" << endl;
}










/*
 * Closing the summary graph G means the following:
 * If there is an edge from X to *Y in G as
 * well as an edge from Y to *Z, then add the edge
 * X to *Z. The NC for the edge constraint is the
 * conjunction of the necessary condition for the individual
 * edge constraints, but the sufficient condition is false.
 */
void SummaryClosure::close_graph()
{
	assert_context("Doing graph closure");
	prepare_graph();
	set<Edge*, CompareTimestamp> ordered_edges;
	ordered_edges = sg.edges;
	int i=0;
	while(!ordered_edges.empty())
	{
		i++;
		if(i > 100) {
			assert_context("Graph closure not terminating!!!");
			c_assert(false);
			return;
		}
		Edge* e = *ordered_edges.begin();


		ordered_edges.erase(e);
		MemoryLocation* source_loc = e->get_source_loc();
		int source_offset = e->get_source_offset();
		AccessPath* source_ap = source_loc->get_access_path(source_offset);

		MemoryLocation* target_loc = e->get_target_loc();
		int target_offset = e->get_target_offset();
		AccessPath* target_ap = target_loc->get_access_path(target_offset);

		set<pair<AccessPath*, Constraint > > instantiations;
		bool res = get_closed_targets(target_ap, e->get_constraint(),
				instantiations);

		/*
		 * We need to make the target imprecise!
		 */
		if(!res) {
			set<Edge*> succs;
			sg.get_successors(source_loc, source_offset, succs);
			if(succs.size() == 1) {
				Edge* s = *succs.begin();
				if(s->get_target_ap()->get_ap_type() == AP_UNMODELED) {
					continue;
				}
			}
			sg.clear_edges(source_loc, source_offset);
			string pp = "Imprecise in loop " + source_loc->to_string() +
										" in loop";
			DisplayTag dt(pp, sg.mg->line, sg.mg->file);
			AccessPath* imprecise_ap = UnmodeledValue::make_imprecise(
					source_ap->get_type(), dt);
			MemoryLocation* new_loc = sg.mg->get_location(imprecise_ap);
			Edge* e = sg.add_edge(source_loc, source_offset, new_loc, 0,
					Constraint(true));
			if(e==NULL) continue;
			enqueue_dependencies(source_ap, imprecise_ap, ordered_edges, e);
			continue;
		}

		if(DEBUG){
			cout << "+++++++++++++++++" << endl;
			cout << "Source: " << source_ap->to_string() << endl;
			cout << "Target: " << target_ap->to_string() << endl;
			cout << "Closure of targets: " << endl;
			set<pair<AccessPath*, Constraint > >::iterator it = instantiations.begin();
			for(; it!= instantiations.end(); it++)
			{
				cout << "\t " << it->first->to_string() << " under " <<
					it->second.to_string() << endl;
			}
			cout << "+++++++++++++++++" << endl;
		}


		if(instantiations.size() == 0) continue;

		/*
		 * Add the transitive edge and enqueue all dependencies
		 * Note that if we ever add a transitive edge the sufficient condition
		 * becomes false because we don't take iteration number
		 * into account when closing the graph.
		 */
		Constraint edge_c = e->get_constraint();
		edge_c = Constraint(edge_c, Constraint(false));
		ConstraintGenerator::eliminate_free_vars(edge_c, non_generalizable_aps);


		bool edge_added = false;

		set<pair<AccessPath*, Constraint > >::iterator it = instantiations.begin();
		for(; it!= instantiations.end(); it++)
		{
			AccessPath* new_target = it->first;
			Constraint cur_c = it->second & edge_c;
			MemoryLocation* new_t_loc = sg.mg->get_location(new_target);
			int new_t_offset = new_t_loc->find_offset(new_target);


			if(cur_c.unsat()) continue;
			Edge* new_e = sg.add_edge(source_loc, source_offset,
					new_t_loc, new_t_offset, cur_c);
			edge_added = true;
			if(new_e == NULL) continue;



			// Enqueue dependencies
			/*if(target_aps_to_edges.count(source_ap) > 0)
			{

				set<Edge*>& dependencies=target_aps_to_edges[source_ap];
				if(DEBUG)
				{
					cout << "-----DEPENDENCIES-----" << endl;
					set<Edge*>::iterator it = dependencies.begin();
					for(; it!=dependencies.end(); it++)
					{
						cout << "\t " << (*it)->to_string() << endl;
					}
					cout << "-----END DEPENDENCIES-----" << endl;
				}

				ordered_edges.insert(dependencies.begin(), dependencies.end());

			}

			// Now update the target_aps_to_edges map
			AccessPath* stripped_target = new_target->strip_deref();
			set<AccessPath*> target_mem_aps;
			stripped_target->get_nested_memory_aps(target_mem_aps);
			set<AccessPath*>::iterator it = target_mem_aps.begin();
			for(; it!= target_mem_aps.end(); it++)
			{
				this->target_aps_to_edges[*it].insert(new_e);
			}*/
			enqueue_dependencies(source_ap, new_target, ordered_edges, new_e);
		}



		/*
		 * If we actually added an edge, we have to make the sufficient condition
		 * false because we don't differentiate between different recursive calls.
		 */
		if(edge_added) {
			e->set_constraint(edge_c);
		}




	}
}

void SummaryClosure::enqueue_dependencies(AccessPath* source_ap,
		AccessPath* new_target, set<Edge*,CompareTimestamp>& ordered_edges,
		Edge* new_e)
{
	// Enqueue dependencies
	if(target_aps_to_edges.count(source_ap) > 0)
	{
		set<Edge*>& dependencies=target_aps_to_edges[source_ap];
		if(DEBUG)
		{
			cout << "-----DEPENDENCIES-----" << endl;
			set<Edge*>::iterator it = dependencies.begin();
			for(; it!=dependencies.end(); it++)
			{
				cout << "\t " << (*it)->to_string() << endl;
			}
			cout << "-----END DEPENDENCIES-----" << endl;
		}

		ordered_edges.insert(dependencies.begin(), dependencies.end());

	}

	// Now update the target_aps_to_edges map
	AccessPath* stripped_target = new_target->strip_deref();
	set<AccessPath*> target_mem_aps;
	stripped_target->get_nested_memory_aps(target_mem_aps);
	set<AccessPath*>::iterator it = target_mem_aps.begin();
	for(; it!= target_mem_aps.end(); it++)
	{
		this->target_aps_to_edges[*it].insert(new_e);
	}

}

bool SummaryClosure::get_closed_targets(AccessPath* ap, Constraint c,
		set<pair<AccessPath*, Constraint> >& closed_targets)
{
	if(ap->get_ap_type() != AP_DEREF) return true;
	Deref* deref_ap = (Deref*) ap;
	ap = deref_ap->get_inner();
	set<pair<AccessPath*, Constraint> > inner_targets;
	bool res = get_closed_targets_rec(ap, c, inner_targets);
	if(!res) return false;
	set<pair<AccessPath*, Constraint> >::iterator it = inner_targets.begin();
	for(; it!= inner_targets.end(); it++)
	{
		AccessPath* inner_ap = it->first;
		Constraint c = it->second;
		AccessPath* res_ap = Deref::make(inner_ap, deref_ap->get_type());
		closed_targets.insert(pair<AccessPath*, Constraint>(res_ap, c));
	}
	return true;

}

bool SummaryClosure::get_closed_targets_rec(AccessPath* ap, Constraint c,
		set<pair<AccessPath*, Constraint> >& closed_targets)
{

	assert_context("Getting closed targets of " + AccessPath::safe_string(ap));
	switch(ap->get_ap_type())
	{
		case AP_ADDRESS:
		case AP_ALLOC:
		case AP_ARRAYREF:
		case AP_FIELD:
		case AP_VARIABLE:
		case AP_UNMODELED:
		case AP_DEREF:
		case AP_ADT:
		{
			MemoryLocation* loc = sg.mg->get_location(ap);
			int offset = loc->find_offset(ap);
			if(offset == -1) return true;
			//if(!loc->has_field_at_offset(offset)) return true;
			if(loc->get_successor_map().count(offset) == 0)
				return true;


			set<Edge*>* targets = loc->get_successors(offset);
			set<Edge*>::const_iterator it = targets->begin();
			for(; it!= targets->end(); it++)
			{
				Edge* e = *it;
				if(sg.edges.count(e) == 0) continue;
				AccessPath* target_ap = e->get_target_ap();
				AccessPath* target_val = target_ap->strip_deref();
				if(occurs_check(ap, target_val)) continue;
				Constraint overwrite_c = e->get_constraint();

				if(!target_overwritten_in_prev_it(ap, c, overwrite_c)){
					continue;
				}
				ConstraintGenerator::eliminate_free_vars(overwrite_c,
										all_sources);
				sg.mg->ivm.rename_index_vars(overwrite_c);

				Constraint sat_c = overwrite_c & c;
				if(sat_c.unsat_discard()) continue;


				closed_targets.insert(pair<AccessPath*, Constraint>(
						target_val, overwrite_c));
			}
			return true;

		}
		case AP_ARITHMETIC:
		{
			ArithmeticValue* av = (ArithmeticValue*) ap;
			const map<Term*, long int>& elems = av->get_elems();
			map<Term*, long int>::const_iterator it = elems.begin();
			vector<set<pair<AccessPath*, Constraint> > > ordered_value_sets;
			for(; it!= elems.end(); it++)
			{
				Term* elem = it->first;
				if(!elem->is_specialized()) continue;
				AccessPath* elem_ap = AccessPath::to_ap(elem);
				set<pair<AccessPath*, Constraint> > cur_vs;
				if(!get_closed_targets_rec(elem_ap, c, cur_vs)) return false;
				if(cur_vs.size() != 0) return false;
			}
			return true;
		}
		case AP_FUNCTION:
		{
			FunctionValue* fv = (FunctionValue*) ap;
			vector<AccessPath*>& args = fv->get_arguments();
			for(unsigned int i=0; i<args.size(); i++)
			{
				AccessPath* arg = args[i];
				set<pair<AccessPath*, Constraint> > cur_vs;
				if(!get_closed_targets_rec(arg, c, cur_vs)) return false;
			}
			return true;
		}
		case AP_CONSTANT:
		case AP_COUNTER:
		case AP_INDEX:
		case AP_NIL:
		case AP_STRING:
		case AP_TYPE_CONSTANT:
			return true;
		default:
			cout << "Unexpected: " << ap->to_string() << endl;
			c_assert(false);

	}
}

bool SummaryClosure::target_overwritten_in_prev_it(AccessPath* target,
		Constraint target_c, Constraint overwrite_c)
{

	IndexVariable* source_iv = target->find_outermost_index_var(true);

	if(source_iv == NULL) {

		return true;
	}

	IndexVariable* target_iv = IndexVariable::make_target(source_iv);

	// Renaming to avoid naming conflicts
	IndexVarManager::rename_source_to_fresh_index(target_c);
	IndexVarManager::rename_target_to_fresh_index(overwrite_c);


	overwrite_c = increment_iteration_counters(overwrite_c);
	Constraint eq_c = ConstraintGenerator::get_eq_constraint(target_iv, source_iv);


	Constraint query = overwrite_c & target_c & eq_c &
			get_counters_eq_constraint();
	return query.sat_discard();




}
Constraint SummaryClosure::get_counters_eq_constraint()
{
	if(!counter_ids_eq_constraint.is_true() )
		return counter_ids_eq_constraint;


	int last = sg.mg->ma.get_bec()->get_max_counter();
	IterationCounter* prev = IterationCounter::make_parametric(0);
	for(int i=1; i <= last; i++) {
		IterationCounter* cur = IterationCounter::make_parametric(i);
		Constraint c = ConstraintGenerator::get_eq_constraint(prev, cur);
		counter_ids_eq_constraint &= c;
		prev = cur;
	}
	return counter_ids_eq_constraint;
}

/*
 * Is ap1 contained in ap2?
 */
bool SummaryClosure::occurs_check(AccessPath* ap1, AccessPath* ap2)
{

	if(ap1==ap2) return true;
	switch(ap2->get_ap_type())
	{
	case AP_ARITHMETIC:
	{
		ArithmeticValue* at = (ArithmeticValue*) ap2;
		const map<Term*, long int> & terms = at->get_elems();
		map<Term*, long int>::const_iterator it = terms.begin();
		for(; it!= terms.end(); it++)
		{
			Term* t= it->first;
			if(!t->is_specialized()) continue;
			AccessPath* elem = AccessPath::to_ap(t);
			if(occurs_check(ap1, elem)) return true;
		}

		return false;
	}
	case AP_FUNCTION:
	{
		FunctionValue* fv = (FunctionValue*) ap2;
		vector<AccessPath*>& args = fv->get_arguments();
		for(unsigned int i=0; i< args.size(); i++){
			if(occurs_check(ap1, args[i])) return true;
		}
		return false;
	}

	default:
		return false;

	}
}







/*
 * Initializes the ivar  target_aps_to_edges.
 */
void SummaryClosure::prepare_graph()
{
	set<Edge*>::iterator it = sg.edges.begin();
	for(; it != sg.edges.end(); it++)
	{
		Edge* e = *it;
		AccessPath* target_ap = e->get_target_ap();
		AccessPath* stripped_ap = target_ap->strip_deref();
		set<AccessPath*> memory_aps;
		stripped_ap->get_nested_memory_aps(memory_aps);

		set<AccessPath*>::iterator it2 = memory_aps.begin();
		for(; it2!= memory_aps.end(); it2++)
		{
			target_aps_to_edges[*it2].insert(e);
		}


	}

}






void SummaryClosure::eliminate_non_linear_aps()
{

	assert_context("Eliminating non-linear access paths");

	set<Edge*>::iterator it = sg.edges.begin();

	for(; it != sg.edges.end(); it++)
	{
		Edge* e = *it;
		Constraint c = e->get_constraint();
		ConstraintGenerator::eliminate_free_vars(c, non_generalizable_aps);
		e->set_constraint(c);

	}

	map<Term*, Term*> delete_replacements;
	map<Term*, Term*> rev_delete_replacements;
	if(GlobalAnalysisState::track_delete())
	{
		set<AccessPath*>::iterator it = non_generalizable_aps.begin();
		for(; it != non_generalizable_aps.end(); it++)
		{
			AccessPath* ap = *it;
			ap = FieldSelection::make_deleted(ap);
			AccessPath* temp = Variable::make_temp(ap->get_type());
			delete_replacements[ap->to_term()] = temp->to_term();
			rev_delete_replacements[temp->to_term()] = ap->to_term();
		}
	}



	ConstraintGenerator::eliminate_free_vars(termination_cond,
			non_generalizable_aps);

	set<ErrorTrace*>::const_iterator it2 = sg.error_traces.begin();
	for(; it2 != sg.error_traces.end(); it2++)
	{
		ErrorTrace* et = *it2;
		Constraint c = et->get_failure_cond();
		c.replace_terms(delete_replacements);
		ConstraintGenerator::eliminate_free_vars(c, non_generalizable_aps);
		c.replace_terms(rev_delete_replacements);
		et->set_failure_cond(c);
	}
}


/*
 * Replaces values used in the constraint with their
 * generalizations.
 */
void SummaryClosure::generalize_constraints()
{
	assert_context("Generalizing constraints");
	set<Edge*>::iterator it = sg.edges.begin();
	for(; it!=sg.edges.end(); it++)
	{
		Edge* se = *it;

		if(generalized_aps.count(se->get_source_ap()) > 0){
			continue;
		}
		int counter_id = get_iteration_counter(se);
		Constraint c = se->get_constraint();
		MemoryLocation* source_loc = se->get_source_loc();
		AccessPath* source_ap = source_loc->get_access_path(
				se->get_source_offset());


		/*
		 * If the source variable is a designated exit variable,
		 * the generalization must refer to its value
		 * at termination.
		 */
		if(source_ap->get_ap_type() == AP_VARIABLE)
		{
			Variable* v = (Variable*) source_ap;
			if(v->is_loop_exit_var())
			{
				c = generalize_constraint(c, LAST, counter_id);
				se->set_constraint(c);
				continue;
			}

		}

		Constraint c2 = c;
		IndexVarManager::eliminate_source_vars(c2);
		IndexVarManager::eliminate_target_vars(c2);

		if(has_concrete_source(se) && sg.loop_continuation_cond.implies(c2)) {
			c = generalize_constraint(c, LAST, counter_id);

		}


		else {
			c = generalize_constraint(c, PARAMETRIC, counter_id);

		}
		se->set_constraint(c);

	}



	//generalize_error_traces();


}



void SummaryClosure::generalize_error_traces()
{

	set<ErrorTrace*> new_traces;

	set<ErrorTrace*>::iterator it = sg.error_traces.begin();
	for(; it!= sg.error_traces.end(); it++)
	{
		ErrorTrace* et = *it;
		assert_context("Generalizing error trace: " + et->to_string());
		Constraint error_c = et->get_failure_cond();

		int counter = et->get_counter_id();
		c_assert(counter != INVALID_COUNTER);
		error_c = generalize_constraint(error_c, PARAMETRIC, counter);
		IterationCounter* k = IterationCounter::make_parametric(counter);
		AccessPath* n =   IterationCounter::make_last(counter);
		if(error_c.contains_term(k->to_term()))
			error_c &= ConstraintGenerator::get_lt_constraint(k, n);
		string loop_id = "";
		if(sg.mg->su->is_superblock()){
			sail::SuperBlock* sb = (sail::SuperBlock*)sg.mg->su;
			loop_id = ":"+int_to_string(sb->get_block_id());
		}
		Variable* error_t = Variable::make_loop_error_temp(
				int_to_string(sg.mg->ma.get_cg_id())+loop_id);
		et->set_failure_cond(error_c);
		ErrorTrace* new_et = et->replace(k, error_t);
		new_traces.insert(new_et);


		//error_c.replace_term(k, error_t);


	//	et->set_failure_cond(error_c);


		/*vector<pair<callstack_t, error_cond> >& paths = et->get_error_paths();
		for(unsigned int i=0; i<paths.size(); i++)
		{
			Constraint& error_c = paths[i].second.error_c;
			error_c = generalize_constraint(error_c, PARAMETRIC, true,
					NULL, false);
		}*/

	}

	sg.error_traces = new_traces;


}
Constraint SummaryClosure::generalize_constraint(Constraint c, gen_kind_t kind,
		int counter_id)
{

	if(DEBUG) {
		cout << "CONSTRAINT TO GENERALIZE: " << c.to_string() << endl;
	}


	set<AccessPath*>::iterator it = generalized_aps.begin();
	for(; it!= generalized_aps.end(); it++)
	{
		AccessPath* cur = *it;
		c = generalize_ap_in_constraint(c, cur, kind, counter_id);

	}
	if(DEBUG) {
			cout << "GENERALIZATION BEFORE SIMPLIFY: " << c.to_string() << endl;
		}

	c.sat();
	c.valid();

	if(DEBUG) {
		cout << "GENERALIZATION: " << c.to_string() << endl;
	}

	return c;


}

bool SummaryClosure::target_overwritten_in_prev_it(Edge* e)
{
	if(!e->has_default_target()) {
		return false;
	}
	AccessPath* source = e->get_source_ap();
	IndexVariable* source_iv = source->find_outermost_index_var(true);
	if(source_iv == NULL) {
		return false;
	}
	IndexVariable* target_iv = IndexVariable::make_target(source_iv);

	Constraint edge_c1 = e->get_constraint();
	IndexVariable* f1 = IndexVariable::make_free();
	edge_c1.replace_term(target_iv->to_term(), f1->to_term());
	edge_c1 = increment_iteration_counters(edge_c1);

	Constraint edge_c2 = e->get_constraint();
	IndexVariable* f2 = IndexVariable::make_free();
	edge_c2.replace_term(source_iv->to_term(), f2->to_term());
	Constraint c = edge_c1 & ConstraintGenerator::get_eq_constraint(source_iv,
			target_iv) & edge_c2 & get_counters_eq_constraint();

	bool res = (c.sat_discard());
	return res;

}

bool SummaryClosure::source_overwritten_in_future_it(Edge* e)
{

	set<Edge*> edges;
	Constraint edge_c = e->get_constraint();
	sg.get_successors(e->get_source_loc(), e->get_source_offset(), edges);
	set<Edge*>::iterator it = edges.begin();
	for(; it!= edges.end(); it++)
	{
		Edge* cur_e = *it;
		if(cur_e == e) continue;


		/*
		 * We want to make sure cur_e cannot invalidate e.
		 */
		Constraint cur_c = cur_e->get_constraint();
		cur_c = decrement_iteration_counters(cur_c);
		Constraint counters_eq = get_counters_eq_constraint();
		Constraint c = edge_c & cur_c & counters_eq;
		if(c.sat_discard()){
			return true;
		}

	}

	return false;
}

Constraint SummaryClosure::increment_iteration_counters(Constraint c)
{
	set<Term*> terms;
	c.get_terms(terms, true);
	set<Term*>::iterator it = terms.begin();
	Variable* l = Variable::make_temp("l");
	map<Term*, Term*> replacements;
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		if(ap->get_ap_type() != AP_COUNTER) continue;
		IterationCounter* v = (IterationCounter*) ap;
		AccessPath* at = ArithmeticValue::make_minus(v, l);
		replacements[v->to_term()] = at->to_term();
	}

	c.replace_terms(replacements);
	c &= ConstraintGenerator::get_gtz_constraint(l);
	return c;
}

Constraint SummaryClosure::increment_iteration_counters(Constraint c,
		AccessPath* inc_amount)
{
	set<Term*> terms;
	c.get_terms(terms, true);
	set<Term*>::iterator it = terms.begin();
	map<Term*, Term*> replacements;
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		if(ap->get_ap_type() != AP_COUNTER) continue;
		IterationCounter* v = (IterationCounter*) ap;
		AccessPath* at = ArithmeticValue::make_minus(v, inc_amount);
		replacements[v->to_term()] = at->to_term();
	}

	c.replace_terms(replacements);
	return c;
}



Constraint SummaryClosure::decrement_iteration_counters(Constraint c)
{
	set<Term*> terms;
	c.get_terms(terms, true);
	set<Term*>::iterator it = terms.begin();
	Variable* l = Variable::make_temp("l");
	map<Term*, Term*> replacements;
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		if(ap->get_ap_type() != AP_COUNTER) continue;
		IterationCounter* v = (IterationCounter*) ap;
		AccessPath* at = ArithmeticValue::make_plus(v, l);
		replacements[v->to_term()] = at->to_term();
	}

	c.replace_terms(replacements);
	c &= ConstraintGenerator::get_gtz_constraint(l);
	return c;
}

Constraint SummaryClosure::generalize_ap_in_constraint(Constraint c,
		AccessPath* ap, gen_kind_t kind, int counter_id)
{

	assert_context("Generalizing access path " + AccessPath::safe_string(ap) +
			" in constraint " + c.to_string());
	if(!c.contains_term(ap->to_term())) return c;


	c_assert(generalization_map.count(ap) > 0);
	generalization_set& g = generalization_map[ap];
	c_assert(g.generalizations.size() > 0);
	Constraint or_c(false);
	set<generalization*>::iterator it2 = g.generalizations.begin();
	for(; it2!= g.generalizations.end(); it2++)
	{
		generalization* cur_gen = *it2;
		AccessPath* new_val = cur_gen->get_generalization(kind, counter_id);
		Constraint cur_c = c;
		cur_c.replace_term(cur_gen->base->to_term(), new_val->to_term());
		Constraint gen_c = cur_gen->constraint;
		Constraint and_c = cur_c & gen_c;
		or_c |= and_c;
	}


	/*
	 * This deals with k's inside of alloc() functions.
	 * Note that we have to rename them to N-1, since indices start
	 * counting at 0, not 1.
	 */
	if(kind == LAST){
		map<Term*, Term*> k_to_n;

		int end = sg.mg->ma.get_bec()->get_max_counter();
		for(int id = 0; id <= end; id++)
		{
			IterationCounter* k = IterationCounter::make_parametric(id);
			IterationCounter* n = IterationCounter::make_last(counter_id);
			AccessPath* n_minus_one = ArithmeticValue::make_minus(n,
					ConstantValue::make(1));
			k_to_n[k->to_term()] = n_minus_one->to_term();
		}
		or_c.replace_terms(k_to_n);
		return or_c;
	}
	return or_c;
}

/*
 * Generalizes the graph by updating the targets of
 * summary edges with their correct generalizations.
 */
void SummaryClosure::generalize_targets()
{
	assert_context("Generalizing targets");
	update_generalization_dependencies();


	map<Edge*, set<generalization*> >::iterator it = edges_to_update.begin();
	for(; it!= edges_to_update.end(); it++)
	{
		Edge* se = it->first;
		MemoryLocation* source_loc = se->get_source_loc();
		int source_offset = se->get_source_offset();
		set<generalization*>& gen_set = it->second;
		Constraint edge_c = se->get_constraint();
		/*
		 * Check if the source has an index variable.
		 * If this is the case, we can't assume that the generalization
		 * must refer to the last iteration; it could refer
		 * to any iteration.
		 */
		AccessPath* new_target_ap = NULL;
		bool is_concrete_source = has_concrete_source(se);


		set<generalization*>::iterator it2 = gen_set.begin();
		for(; it2!= gen_set.end(); it2++)
		{
			generalization* g = *it2;
			Constraint edge_and_gen = g->constraint & edge_c;

			if(edge_and_gen.unsat_discard()) continue;


			if(!is_concrete_source && sg.mg->cg.contains_access_path(edge_c,
					generalized_aps))
			{
				new_target_ap = g->parametric;
			}
			else {
				new_target_ap = g->last;
			}

			MemoryLocation* new_target_loc = sg.mg->get_location(new_target_ap);
			int new_target_offset = new_target_loc->find_offset(new_target_ap);
			Constraint new_edge_c = g->constraint;


			if(new_target_ap != g->last) {
				new_edge_c &= edge_c;
			}


			Edge* new_se = sg.add_edge(source_loc, source_offset,
					new_target_loc, new_target_offset, new_edge_c);
			new_se->get_block_ids().clear();
			new_se->get_block_ids().insert(se->get_block_ids().begin(),
					se->get_block_ids().end());


		}


		sg.remove_edge(se);


	}
}

/*
 * Is the source of this edge guaranteed to represent
 * a single concrete location?
 */
bool SummaryClosure::has_concrete_source(Edge* e)
{

	AccessPath* source = e->get_source_ap();

	/*
	 * If the base is an alloc, this cannot be a concrete location
	 * because there is a fresh allocation at every iteration
	 */
	AccessPath* base = source->get_base();
	if(base != NULL && base->get_ap_type() == AP_ALLOC) return false;

	/*
	 * If it's not an array, it's definitely a concrete location.
	 */
	IndexVariable* iv = source->find_outermost_index_var(false);
	if(iv == NULL) {
		return true;
	}

	/*
	 * We need to check if the source index must be the same
	 * across all iterations. If it is the same then C[x/i] &
	 * C[k'/k, y/i] & x!= y & k!=k' should be unsatisfiable.
	 *
	 *
	 */
	Constraint c = e->get_constraint();
	c = generalize_constraint(c, PARAMETRIC, this->get_iteration_counter(e));


	Constraint cur_c =c;
	Variable* x = Variable::make_temp("x");
	cur_c.replace_term(iv, x);

	IndexVarManager::rename_target_to_fresh_index(cur_c);

	Constraint other_c = c;
	set<IterationCounter*> counters;
	get_iteration_counters(c, counters);

	Constraint different_it_c;
	map<Term*, Term*> reps;
	int i=0;
	set<IterationCounter*>::iterator it = counters.begin();
	for(; it!= counters.end(); it++, i++) {
		IterationCounter* ic = *it;
		Variable* other_it = Variable::make_temp("kk" +
				int_to_string(ic->get_id()));
		reps[ic] = other_it;
		different_it_c &= ConstraintGenerator::get_neq_constraint(ic, other_it);
	}
	Variable* y = Variable::make_temp("y");
	reps[iv] = y;
	other_c.replace_terms(reps);

	Constraint different_index_c = ConstraintGenerator::get_neq_constraint(x,y);

	Constraint query_c1 = cur_c & other_c & different_it_c;
	if(query_c1.unsat_discard()) return false;

	Constraint query_c = query_c1 &  different_index_c;
	return (!query_c.sat_discard());

}



/*
 * If we were able to generalize an access path like "i",
 * we need to update any other access paths involving i, such
 * as i+1 etc.
 */
void SummaryClosure::update_generalization_dependencies()
{

	set<Edge*>::iterator it = sg.edges.begin();
	for(; it!= sg.edges.end(); it++)
	{
		Edge* se = *it;
		if(edges_to_update.count(se) > 0) continue;
		MemoryLocation* t_loc = se->get_target_loc();
		int t_offset = se->get_target_offset();
		AccessPath* orig_target = t_loc->get_access_path(t_offset);
		AccessPath* target_ap = orig_target->strip_deref();
		set<AccessPath*> mem_aps;
		target_ap->get_nested_memory_aps(mem_aps);
		set<AccessPath*> intersection;
		set_intersection(mem_aps.begin(), mem_aps.end(), generalized_aps.begin(),
				generalized_aps.end(), insert_iterator<set<AccessPath*> >(
						intersection, intersection.begin()));
		if(intersection.size() == 0) continue;


		set<AccessPath*>::iterator it2= intersection.begin();
		map<AccessPath*, generalization_set> map_set;

		for(; it2!= intersection.end(); it2++)
		{
			AccessPath* to_update = *it2;

			c_assert(generalization_map.count(to_update) > 0);
			generalization_set& gen_set = generalization_map[to_update];
			map_set[to_update] = gen_set;

		}



		set<map<AccessPath*, generalization*> > res;
		cross_product_generalizations(map_set, res);

		set<generalization*>& new_gens = edges_to_update[se];

		set<map<AccessPath*, generalization*> >::iterator it3 = res.begin();
		for(; it3!= res.end(); it3++)
		{
			map<AccessPath*, generalization*> cur = *it3;
			map<AccessPath*, AccessPath*> k_replacements; // i->i+k
			map<AccessPath*, AccessPath*> n_replacements; // i->i+N

			map<AccessPath*, generalization*>::iterator it4 = cur.begin();
			//Constraint inst_c = se->get_constraint();
			Constraint inst_c;
			if(this->generalized_aps.count(se->get_source_ap()) == 0){
				inst_c = se->get_constraint();
			}

			for(; it4!=cur.end(); it4++)
			{
				/*
				 * Generalization of ap is g.
				 */
				k_replacements[it4->first] = it4->second->parametric;
				n_replacements[it4->first] = it4->second->last;
				inst_c &= it4->second->constraint;
			}
			if(inst_c.unsat()) continue;
			AccessPath* param_gen = orig_target->replace(k_replacements);
			AccessPath* last_gen = orig_target->replace(n_replacements);
			generalization* g = new generalization();
			g->base = NULL;
			g->parametric = param_gen;
			g->last = last_gen;
			g->increment = 0;
			g->constraint = inst_c;
			g->counter = INVALID_COUNTER;
			new_gens.insert(g);
		}


	}
}
void SummaryClosure::cross_product_generalizations(
		map<AccessPath*, generalization_set>& map_set,
		set<map<AccessPath*, generalization*> > & set_map)
{
	map<AccessPath*, generalization*> empty;
	if(DEBUG)
	{
		cout << "=============CROSS PRODUCT=============" << endl;
		map<AccessPath*, generalization_set>::iterator it = map_set.begin();
		for(; it!= map_set.end(); it++) {
			AccessPath* ap = it->first;
			cout << "AP: " << ap->to_string() << " => " ;
			generalization_set& gs = it->second;
			cout << gs.to_string() << endl;
		}
	}


	cross_product_generalizations_rec(map_set, set_map, empty);
}

void SummaryClosure::cross_product_generalizations_rec(
		map<AccessPath*, generalization_set>& map_set,
		set<map<AccessPath*, generalization*> > & set_map,
		map<AccessPath*, generalization*>& cur_map)
{
	if(cur_map.size() == map_set.size()) {
		set_map.insert(cur_map);
		return;
	}

	map<AccessPath*, generalization_set>::iterator it = map_set.begin();
	for(; it!= map_set.end(); it++)
	{
		AccessPath* ap = it->first;
		generalization_set& gs = it->second;
		if(cur_map.count(ap) != 0) continue;
		set<generalization*>::iterator it2= gs.generalizations.begin();
		for(; it2!= gs.generalizations.end(); it2++)
		{
			generalization* cur = *it2;

			cur_map[ap] = cur;
			cross_product_generalizations_rec(map_set, set_map, cur_map);
			cur_map.erase(ap);
		}
	}
}


void SummaryClosure::get_iteration_counters(Constraint c,
		set<IterationCounter*>& counters)
{
	set<Term*> terms;
	c.get_terms(terms, true);
	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		if(ap->get_ap_type() != AP_COUNTER) continue;
		IterationCounter* v = (IterationCounter*) ap;
		if(v->is_parametric()) counters.insert(v);
	}
}

void SummaryClosure::get_termination_vars(Constraint c,
		set<IterationCounter*>& counters)
{
	set<Term*> terms;
	c.get_terms(terms, true);
	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		if(ap->get_ap_type() != AP_COUNTER) continue;
		IterationCounter* v = (IterationCounter*) ap;
		if(v->is_last()) counters.insert(v);
	}
}

bool SummaryClosure::is_parametric_ap(AccessPath* ap)
{
	set<IterationCounter*> counters;
	ap->get_counters(counters, true);
	return counters.size() > 0;
}

void SummaryClosure::eliminate_iteration_counters()
{

	assert_context("Eliminating iteration counters");
	map<Edge*, Constraint> new_edge_constraints;

	set<Edge*>::iterator it = sg.edges.begin();
	for(; it!= sg.edges.end(); it++)
	{
		Edge* e = *it;


		AccessPath* target_ap = e->get_target_ap();
		if(is_parametric_ap(target_ap)) continue;
		AccessPath* source_ap  = e->get_source_ap();
		if(source_ap->get_ap_type() == AP_VARIABLE) {
			Variable* v = (Variable*) source_ap;
			if(v->is_loop_exit_var())
				continue;
		}


		Constraint edge_c = e->get_constraint();
		bool target_overwritten = target_overwritten_in_prev_it(e);
		bool source_overwritten = source_overwritten_in_future_it(e);


		if(target_overwritten)
		{
			/*
			 * If target is overwritten, we don't know what the new target is.
			 */
			IndexVariable* source_iv= source_ap->find_outermost_index_var(true);
			IndexVariable* target_iv = IndexVariable::make_target(source_iv);
			ConstraintGenerator::eliminate_evar(edge_c, target_iv);

		}
		if(source_overwritten)
		{
			Constraint new_c(e->get_constraint(), Constraint(false));
			edge_c = new_c;
		}


		set<IterationCounter*> target_counters;
		target_ap->get_counters(target_counters, true);
		set<IterationCounter*> constraint_counters;
		Constraint c = edge_c;
		get_iteration_counters(c, constraint_counters);
		// to_eliminate = constraint counters - target_counters;
		set<AccessPath*> to_eliminate;
		to_eliminate.insert(constraint_counters.begin(), constraint_counters.end());

		{
			set<IterationCounter*>::iterator it = target_counters.begin();
			for(; it!=target_counters.end(); it++)
			{
				IterationCounter* v = *it;
				to_eliminate.erase(v);
			}
		}



		set<AccessPath*>::iterator it2  = to_eliminate.begin();
		for(; it2!= to_eliminate.end(); it2++)
		{
			AccessPath* ap = *it2;
			IterationCounter* v = (IterationCounter*) ap;
			AccessPath* term_var;

			term_var = IterationCounter::make_last(v);
			Constraint lt_c = ConstraintGenerator::get_lt_constraint(v, term_var);
			c &= lt_c;

		}

		cout << "C before elim: " << c << endl;
		ConstraintGenerator::eliminate_evars(c, to_eliminate);
		c.sat();
		c.valid();
		cout << "C after elim: " << c << endl;
		new_edge_constraints[e] =c;

	}

	map<Edge*, Constraint>::iterator it2= new_edge_constraints.begin();
	for(; it2!= new_edge_constraints.end(); it2++)
	{
		Edge* e = it2->first;
		Constraint c = it2->second;
		e->set_constraint(c);
	}
}

SummaryClosure::~SummaryClosure()
{

}


void SummaryClosure::print_base_to_index()
{
	cout << "-------BASE TO INDEX MAP----------" << endl;
	map<AccessPath*, AccessPath*>::iterator it = base_to_index.begin();
	for(; it!= base_to_index.end(); it++) {
		cout << "\t " << it->first->to_string() << " -> " <<
			it->second->to_string() << endl;
	}
	cout << "--------------------------------------" << endl;
}





/*
 * Debugging function
 */
void SummaryClosure::print_target_ap_to_edges()
{
	if(DEBUG) {
		cout << "---TARGET APS TO EDGES----" << endl;
		map<AccessPath*, set<Edge*> >::iterator it =
			target_aps_to_edges.begin();
		for(; it!= target_aps_to_edges.end(); it++)
		{
			AccessPath* ap = it->first;
			set<Edge*>& edges = it->second;
 			cout << "\t Access path: " <<  ap->to_string() << endl;
 			set<Edge*>::iterator it2 = edges.begin();
 			for(; it2!= edges.end(); it2++)
 			{
 				cout << "\t \t " << (*it2)->to_string() << endl;
 			}
		}
		cout << "---END TARGET APS TO EDGES----" << endl;
	}
}

void SummaryClosure::print_used_counters()
{
	cout << "--------USED COUNTERS----------" << endl;
	set<IterationCounter*>::iterator it = counters.begin();
	for(; it!= counters.end(); it++){
		IterationCounter* ic = *it;
		cout << "\t " << ic->to_string() << endl;
	}
	cout << "-------------------" << endl;
}
