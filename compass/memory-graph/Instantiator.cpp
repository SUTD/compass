/*
 * Instantiator.cpp
 *
 *  Created on: Oct 21, 2008
 *      Author: isil
 */

#include "Instantiator.h"
#include "MemoryLocation.h"
#include "access-path.h"
#include "MemoryGraph.h"
#include "SummaryGraph.h"
#include "IndexVarManager.h"
#include "Edge.h"
#include "sail/Variable.h"
#include "sail/Symbol.h"
#include "sail/Constant.h"
#include "sail/SummaryUnit.h"
#include "DisjointVarManager.h"
#include "ErrorTrace.h"
#include "util.h"
#include "ConstraintSolver.h"
#include "MemoryAnalysis.h"
#include "il/variable_expression.h"
#include "GlobalAnalysisState.h"
#include "il/type.h"
#include "compass_assert.h"

#define DEBUG false


/*
 * Constructor for instantiating function summaries.
 * mg: Memory Graph for the current function
 * sg: Summary Graph of the callee
 * stmt_guard: statement guard under which function call happens
 * args: arguments to the called function. For instance,
 * 		 for the call foo(x,y) args are x and y
 * retvar: variable capturing the return value of callee, NULL if none.
 */
Instantiator::Instantiator(MemoryGraph* mg, SummaryGraph* sg,
		Constraint& stmt_guard, vector<sail::Symbol*>* args,
		sail::Variable* retvar) : stmt_guard(stmt_guard)
{


	assert_context("Instantiating summary of: " + sg->get_sum_id());
	mg->ma.push_dotty_level("Instantiating function: " + sg->get_sum_id());
	GlobalAnalysisState::disable_checks();
	this->report_leaked_stack_vars = true;
	this->mg = mg;
	this->sg = sg;
	this->args = args;
	this->retvar = retvar;
	this->is_loop = false;
	this->inst_guard = Constraint(true);

	instantiate_access_paths();
	apply_summary();
	instantiate_error_traces();
	instantiate_loop_termination_constraints();

	Constraint::set_background_knowledge(loop_termc_inst);
	assert_context("Instantiating return condition: " + sg->return_cond.to_string());
	Constraint inst_return_c = instantiate_constraint(sg->return_cond);



	mg->ma.update_statement_guard(inst_return_c);
	remove_temps();
	GlobalAnalysisState::enable_checks();

	mg->ma.pop_dotty_level();

}

bool Instantiator::is_new_calling_context()
{
	return (mg->ma.get_su()->is_function() && this->sg->is_function_summary());
}

int Instantiator::get_counter_id()
{
	return mg->ma.get_cur_iteration_counter();
}

Constraint Instantiator::get_stmt_guard()
{
	return stmt_guard;
}

callstack_entry Instantiator::get_calling_context()
{
	sail::SummaryUnit* su = mg->ma.get_su();
	callstack_entry ce(su->get_identifier().to_string(),
			su->get_identifier().get_file_id().to_string(), mg->line);
	return ce;
}

/*
 * Constructor for instantiating loop summaries.
 */
Instantiator::Instantiator(MemoryGraph* mg, SummaryGraph* sg,
		Constraint& stmt_guard): stmt_guard(stmt_guard)
{

	assert_context("Instantiating summary of: " + sg->get_sum_id());
	mg->ma.push_dotty_level("Instantiating loop: " + sg->get_sum_id());
	GlobalAnalysisState::disable_checks();
	this->report_leaked_stack_vars = true;
	this->mg = mg;
	this->sg = sg;
	this->args = NULL;
	this->retvar = retvar;
	this->is_loop = true;
	this->inst_guard = Constraint(true);

	instantiate_access_paths();
	apply_summary();

	Constraint inst_return_c = instantiate_constraint(sg->return_cond);


	cout << "------INSTANTIATED RET COND: " << sg->return_cond << " --> " <<
				inst_return_c << endl;

	instantiate_error_traces();

	mg->ma.update_statement_guard(inst_return_c);

	instantiate_loop_termination_constraints();
	Constraint::set_background_knowledge(loop_termc_inst);
	remove_temps();
	GlobalAnalysisState::enable_checks();

	mg->ma.pop_dotty_level();
}

void Instantiator::remove_temps()
{
	set<Variable*>::iterator it = all_temps.begin();
	for(; it!= all_temps.end(); it++)
	{
		mg->remove_root(*it);
	}
}

Constraint Instantiator::simplify_pos(Constraint c)
{

	set<Term*> terms;
	c.get_terms(terms, false);
	set<Term*>::iterator it = terms.begin();
	set<FunctionValue*> pos;
	for(; it != terms.end(); it++)
	{
		Term* t = *it;
		if(!t->is_specialized()) continue;
		AccessPath* ap = AccessPath::to_ap(t);
		if(ap->get_ap_type() != AP_FUNCTION) continue;
		FunctionValue* fv = (FunctionValue*)ap;
		if(!fv->is_adt_pos_selector()) continue;
		pos.insert(fv);
	}

	if(pos.size() == 0) return c;

	Constraint to_assume;

	set<FunctionValue*>::iterator it2 = pos.begin();
	for(; it2 != pos.end(); it2++)
	{
		//Constraint to_assume;
		FunctionValue* fv = *it2;
		Constraint gtz = ConstraintGenerator::get_geqz_constraint(fv);
		to_assume &= gtz;
		AccessPath* adt_ptr= fv->get_arguments()[0];
		AccessPath* adt = adt_ptr->add_deref();
		MemoryLocation* adt_loc = mg->get_location(adt);
		mg->add_default_edges(adt_loc);
		adt_loc = (*adt_loc->get_successors(0)->begin())->get_target_loc();
		mg->add_default_edges(adt_loc);
		AccessPath* size = adt_loc->get_access_path(SIZE_OFFSET);
		set<pair<AccessPath*, Constraint> > size_vals;
		mg->get_value_set(size, size_vals);
		Constraint or_c(false);
		set<pair<AccessPath*, Constraint> >::iterator it = size_vals.begin();
		for(; it!= size_vals.end(); it++) {
			AccessPath* cur_size = it->first;
			Constraint cur_c = it->second;
			if(cur_size->get_ap_type() == AP_CONSTANT) {
				ConstantValue* cv = (ConstantValue*) cur_size;
				if(cv->get_constant() == 0) continue;
			}

			Constraint lt = ConstraintGenerator::get_lt_constraint(fv, cur_size);
			Constraint cur = cur_c & lt;
			or_c |= cur;

		}
		if(or_c.unsat_discard()) continue;


		to_assume &= or_c;


	}
	c.assume(to_assume);


	return c;
}

void Instantiator::apply_summary()
{

	enforce_existence_and_uniqueness();

	set<AccessPath*> updated_sources;

	set<Edge*, CompareTimestamp>::iterator it = sg->edges.begin();
	for(; it!= sg->edges.end(); it++)
	{
		Edge* e = *it;
		assert_context("Applying summary edge: " + e->to_string());



		AccessPath* source_ap = e->get_source_ap()->strip_deref();
		AccessPath* target_ap = e->get_target_ap()->strip_deref();

		bool simp_pos = !source_ap->is_size_field_ap() &&
				!target_ap->is_size_field_ap() &&
				(target_ap->get_ap_type() != AP_ARITHMETIC ||
						(target_ap->get_inner() == NULL ||
								!target_ap->get_inner()->is_size_field_ap()));
	//	simp_pos = false;





		Constraint edge_c = e->get_constraint();
		// This can happen, for example, for ignored return values.
		if(instantiations.count(source_ap) == 0) {
			continue;
		}






		if(instantiations.count(target_ap) == 0){
			assert_context("Target ap instantiation not found: " + target_ap->to_string());
			c_assert(false);

		}
		set<pair<AccessPath*, Constraint> >& inst_source =
				instantiations[source_ap];
		set<pair<AccessPath*, Constraint> >& inst_target =
				instantiations[target_ap];

		c_assert(this->edge_to_inst_constraints.count(e) > 0);
		Constraint inst_edge_c = edge_to_inst_constraints[e] & stmt_guard;


		Constraint inst_update_c(false);
		if(this->inst_update_c.count(e->get_source_ap()) > 0)
			this->inst_update_c[e->get_source_ap()] & stmt_guard;






		if(!inst_edge_c.sat()) {
			continue;
		}




		set<pair<AccessPath*, Constraint> >::iterator it1 =
				inst_source.begin();

		for(; it1!= inst_source.end(); it1++)
		{
			AccessPath* inst_source_ap = it1->first;




			if(inst_source_ap->get_base() != NULL &&
					inst_source_ap->get_base()->get_ap_type() == AP_VARIABLE) {
				Variable* v = (Variable* ) inst_source_ap->get_base();
				if(v->is_summary_temp()) {
					assert_context("Problem instantiating: " +
							AccessPath::safe_string(source_ap) +
							 " instantiates to " + v->to_string());
					c_warn(false);
				}
			}


			MemoryLocation* loc = mg->get_location(inst_source_ap);
			if(!sg->is_init_fn_summary())
			{
				bool add_default = true;
				if(inst_source_ap->get_ap_type() == AP_VARIABLE)
				{
					Variable* v = (Variable*) inst_source_ap;
					if(v->is_return_var())
						add_default = false;
				}
				AccessPath* base = source_ap->get_base();
				if(base == NULL)
				{
					add_default = false;
				}

				if(add_default){
					int offset = loc->find_offset(inst_source_ap);
					mg->add_default_edges(loc, offset);
				}
			}

			bool is_size = inst_source_ap->is_size_field_ap();
			Constraint source_inst_c = it1->second;




			Constraint initial_c = inst_edge_c & source_inst_c;
			if(initial_c.unsat()) continue;

			/*
			 * If we are processing the side effects of a delete statement,
			 * do the necessary checks.
			 */
			if(GlobalAnalysisState::check_double_deletes() &&
					inst_source_ap->is_deleted_field() && target_ap ==
					ConstantValue::make(1)) {
				AccessPath* to_check = inst_source_ap->get_inner();
				mg->check_access_to_deleted_mem(to_check, initial_c,
						inst_source_ap->to_string(), true);
			}



			if(updated_sources.count(inst_source_ap) == 0) {
				updated_sources.insert(inst_source_ap);
				Constraint cur_update_c = source_inst_c & inst_update_c;
				IndexVarManager::eliminate_sum_index_vars(cur_update_c);
				if(simp_pos) cur_update_c = this->simplify_pos(cur_update_c);
				mg->update_outgoing_edges(inst_source_ap, cur_update_c, true);

			}





			set<pair<AccessPath*, Constraint> >::iterator it2 =
							inst_target.begin();


			Constraint update_c = initial_c;


			IndexVarManager::eliminate_sum_index_vars(update_c);

			if(simp_pos) update_c = this->simplify_pos(update_c);
			mg->update_outgoing_edges(inst_source_ap, update_c, true);



			mg->ma.add_dotty("Updating outgoing edges: " +
					inst_source_ap->to_string(), inst_source_ap);

			for(; it2!= inst_target.end(); it2++)
			{
				AccessPath* inst_target_ap = it2->first;


				Constraint target_inst_c = it2->second;



				if(inst_target_ap->get_base()!= NULL &&
						inst_target_ap->get_base()->get_ap_type() == AP_VARIABLE) {
					Variable* v = (Variable* ) inst_target_ap->get_base();
					if(v->is_summary_temp()){
						assert_context("Problem instantiating: " +
								AccessPath::safe_string(target_ap) +
								 " instantiates to " + v->to_string());
						c_warn(false);
					}

				}


				// If the source is a size field, the target may contain
				// index variables in terms of summary indices.
				if(is_size) {
					set<IndexVariable*> sum_indices;
					IndexVarManager::get_sum_index_vars(inst_target_ap, sum_indices);
					c_assert(sum_indices.size()  <= 1);
					if(sum_indices.size() == 1) {
						IndexVariable* sum_iv = *sum_indices.begin();
						AccessPath* rep;
						IndexVariable* source_var =
								inst_source_ap->find_outermost_index_var(true);
						if(source_var == NULL)
							rep = ConstantValue::make(0);
						else {
							rep = source_var;
							Constraint c = ConstraintGenerator::get_eq_constraint(
									source_var, IndexVariable::make_target(source_var));
							initial_c &= c;
						}
						inst_target_ap = inst_target_ap->replace(sum_iv, rep);
					}
				}



				/*
				 * If the inst target still contains a summary index, this must
				 * be due to "fake" arrays, e.g., 0[s_i1] that got introduced
				 * because when we upgrade a constant location to an array,
				 * all instances of it get upgraded.
				 */

				inst_target_ap = IndexVarManager::set_summary_indices_to_zero(
						inst_target_ap);


				IndexVarManager::rename_inst_source_to_inst_target(target_inst_c);

				IndexVarManager::rename_source_to_target_index(target_inst_c);



				Constraint inst_edge_c = initial_c & target_inst_c;
				if(inst_edge_c.unsat()) continue;



				/*
				 * Eliminate any index variables introduced for instantiation
				 */
				IndexVarManager::eliminate_sum_index_vars(inst_edge_c);


				if(simp_pos) inst_edge_c = this->simplify_pos(inst_edge_c);


				Edge* res_e = mg->put_edge(inst_source_ap, inst_target_ap,
						inst_edge_c, false, true);




				string edge_str = "Adding edge: " + inst_source_ap->to_string() +
						" to " + inst_target_ap->to_string();
					mg->ma.add_dotty(edge_str, inst_source_ap, inst_target_ap);



				inst_sources.insert(inst_source_ap);

			}

		}


	}
}

void Instantiator::instantiate_access_paths()
{
	set<AccessPath*>::iterator it = sg->aps_to_instantiate.begin();
	for(; it!=sg->aps_to_instantiate.end(); it++)
	{
		AccessPath* ap = *it;
		instantiate_ap(ap);

	}

	this->report_leaked_stack_vars = false;
	set<AccessPath*> trace_aps;
	{

		set<ErrorTrace*>::iterator it = sg->error_traces.begin();
		for(; it!= sg->error_traces.end(); it++)
		{
			ErrorTrace* et = *it;
			et->get_access_paths_to_instantiate(trace_aps);
		}
	}
	it=trace_aps.begin();
	for(; it!= trace_aps.end(); it++) {
		instantiate_ap(*it);
	}
	this->report_leaked_stack_vars = true;

}


Constraint Instantiator::get_instantiation(Constraint c)
{
	Constraint inst_c = instantiate_constraint(c, true);
	return inst_c;
}
void Instantiator::get_instantiation(AccessPath* sum_ap,
		set<pair<AccessPath*, Constraint> >& instantiation, bool get_value)
{
	assert_context("Getting instantiation of: " + AccessPath::safe_string(sum_ap));
	c_assert(instantiations.count(sum_ap) > 0);
	set<pair<AccessPath*, Constraint> >& _instantiation =
			instantiations[sum_ap];

	if(!get_value) {
		instantiation = instantiations[sum_ap];
		return;
	}
	set<pair<AccessPath*, Constraint> >::iterator it = _instantiation.begin();
	for(; it!= _instantiation.end(); it++)
	{
		AccessPath* ap = it->first;
		instantiation.insert(make_pair(ap->strip_deref(), it->second));
	}
}


void Instantiator::instantiate_loop_termination_constraints()
{
	set<Constraint>::iterator it = sg->loop_termination_constraints.begin();
	for(; it != sg->loop_termination_constraints.end(); it++)
	{
		Constraint ltc = *it;
		assert_context("Instantiating loop termination constraint: " +
				ltc.to_string());
		Constraint inst_ltc = instantiate_constraint(ltc);
		inst_ltc.assume(stmt_guard);
		if(inst_ltc.valid()) continue;
		inst_ltc.sat();
		if(DEBUG) {
			cout << "Instantiated loop termination condition: " <<
			inst_ltc << endl;
		}
		mg->loop_termination_constraints.insert(inst_ltc);
		if(inst_ltc.sat()) Constraint::set_background_knowledge(inst_ltc);
		else loop_termc_inst = inst_ltc;


	}
}

void Instantiator::get_aps_inside_function(FunctionValue* fv,
		set<AccessPath*>& aps)
{
	const vector<AccessPath*>& args = fv->get_arguments();
	vector<AccessPath*>::const_iterator it = args.begin();
	for(; it!= args.end(); it++) {
		AccessPath* ap = *it;
		if(ap->get_ap_type() == AP_FUNCTION) {
			get_aps_inside_function((FunctionValue*) ap, aps);
		}
		else aps.insert(ap);
	}
}

Constraint Instantiator::instantiate_constraint(Constraint c,
		bool eliminate_counters)
{
	Constraint nc = instantiate_constraint_helper(c.nc(), eliminate_counters);
	Constraint sc = instantiate_constraint_helper(c.sc(), eliminate_counters);
	Constraint res= Constraint(nc, sc);
	return res;
}

Constraint Instantiator::instantiate_constraint_helper(Constraint c,
		bool eliminate_counters)
{
	assert_context("Instantiating constraint: " + c.to_string());


	set<Term*> all_terms;
	c.get_terms(all_terms, false);

	Constraint orig_c = c;





	Constraint sum_c = c;


	map<Term*, Term* > subs;
	map<Term*, Term*> reverse_subs; // just for debugging
	set<VariableTerm*> temps;
	set<Term*>::iterator it = all_terms.begin();
	for(; it!= all_terms.end(); it++)
	{
		Term* cur = *it;
		if(!cur->is_specialized())
		{
			if(cur->get_term_type() != ARITHMETIC_TERM){
				cout << "Unexpected non-specialized term: " <<
					cur->to_string() << endl;
				c_assert(false);
			}
			else continue;
		}

		/*
		 * For function values, we need to open them up because
		 * get_terms(all_terms, false) doesn't include nested terms.
		 */
		AccessPath* cur_ap = AccessPath::to_ap(cur);



		/*
		 * This can happen for constants.
		 */
		if(temporaries.count(cur_ap) == 0) continue;

		AccessPath* inst_ap = temporaries[cur_ap];



		Term* inst_term = inst_ap->to_term();
		c_assert(inst_term->get_term_type() == VARIABLE_TERM);
		subs[cur] = inst_term;



 		temps.insert((VariableTerm*)inst_term);
 		reverse_subs[inst_term] = cur;


	}
	c.replace_terms(subs);
	Constraint inst_c = c;

	set<AccessPath*> instantiated_aps;



	map<Term*, Term*>::iterator it2 = subs.begin();
	for(; it2!= subs.end(); it2++)
	{
		Term* inst_temp = it2->second;
		c_assert(inst_temp->is_specialized());
		AccessPath* inst_ap = AccessPath::to_ap(inst_temp);
		set< pair<AccessPath*, Constraint> > value_set;
		mg->get_value_set(inst_ap, value_set, stmt_guard);

		set< pair<AccessPath*, Constraint> >::iterator vs_it = value_set.begin();
		for(; vs_it != value_set.end(); vs_it++) {
			instantiated_aps.insert(vs_it->first);
		}


		if(value_set.size() == 0) {
			assert_context("Value set size of " + AccessPath::safe_string(inst_ap) +
					" is 0! Trying to instantiate: " +
					(reverse_subs[inst_temp] ? reverse_subs[inst_temp]->to_string() :
							"null") +
					" in constraint: " + orig_c.to_string() +
					" in summary " + sg->get_sum_short_id() +
					" in calling context: " +
					mg->su->get_identifier().to_string() + " statement guard: " +
					stmt_guard.to_string());
			c_warn(false);
			return Constraint();
		}

		Constraint cur_c = ConstraintGenerator::get_eq_to_value_set_constraint(
				inst_ap, value_set, eliminate_counters);


		inst_c &= cur_c;




	}



	/*
	 * Eliminate the temporaries introduced for instantiation
	 */
	inst_c.sat();



	inst_c.eliminate_evars(temps);
	inst_c.sat();
	inst_c.valid();

	inst_c = enforce_aliasing_assumptions(inst_c, instantiated_aps);


	return inst_c;

}

Constraint Instantiator::enforce_aliasing_assumptions(Constraint inst_c,
		set<AccessPath*>& instantiations)
{




	//cout << "Working on: " << inst_c << endl;
	set<Term*> vars;
	set<Term*> allocs;
	set<Term*> addresses;

	set<AccessPath*>::iterator it = instantiations.begin();
	for(; it!= instantiations.end(); it++)
	{
		AccessPath* ap = *it;

		if(!ap->get_type()->is_pointer_type()) continue;

		Term* t = ap->to_term();
		if(ap->get_ap_type() == AP_ADDRESS) {
			addresses.insert(t);
			continue;
		}
		AccessPath* base = ap->get_base();
		if(base == NULL) continue;
		if(base->get_ap_type() == AP_VARIABLE) {
			Variable* v = (Variable*) base;
			if(v->is_argument()) vars.insert(t);
		}
		else if(base->get_ap_type() == AP_ALLOC)
		{
			allocs.insert(t);
		}
		else if(base->get_ap_type() == AP_ADDRESS) {
			addresses.insert(t);
		}


		MemoryLocation* loc = mg->get_location(ap->add_deref());
		map<int, AccessPath*>& offset_map = loc->get_access_path_map();
		map<int, AccessPath*>::iterator off_it = offset_map.begin();
		for(; off_it != offset_map.end(); off_it++) {
			int cur = off_it->first;
			AccessPath* cur_ap= ArithmeticValue::make_plus(ap,
					ConstantValue::make(cur));
			Term* cur_t = cur_ap->to_term();
			if(base->get_ap_type() == AP_VARIABLE) {
				Variable* v = (Variable*) base;
				if(v->is_argument()) vars.insert(cur_t);
			}
			else if(base->get_ap_type() == AP_ALLOC)
			{
				allocs.insert(cur_t);
			}
			else if(base->get_ap_type() == AP_ADDRESS) {
				addresses.insert(cur_t);
			}

		}

	}

	Constraint neq(true);
	neq &= get_disequality_constraints(vars, allocs);
	neq &= get_disequality_constraints(vars, addresses);
	neq &= get_disequality_constraints(allocs, addresses);
	if(mg->is_entry_function()) neq &= get_disequality_constraints(vars, vars);

	Constraint orig = inst_c;

	inst_c.assume(neq);

	//if(!neq.valid()){
		/*cout << "Enforcing aliasing assumptions on: " << orig << endl;
		cout << "Assuming: " << neq.to_string() << endl;
		cout << "Result: " << inst_c << endl;*/
	//}


	return inst_c;

}

Constraint Instantiator::get_disequality_constraints(
		set<Term*>& ts1, set<Term*>& ts2)
{
	Constraint res(true);

	set<Term*>::iterator it1 = ts1.begin();
	for(; it1 != ts1.end(); it1++){
		Term* t1 = *it1;
		set<Term*>::iterator it2 = ts2.begin();
		for(; it2!= ts2.end(); it2++) {
			Term* t2 = *it2;
			if(t1 == t2) continue;
			AccessPath* ap1 = AccessPath::to_ap(t1);
			AccessPath* ap2 = AccessPath::to_ap(t2);
			if(ap1->get_base() == ap2->get_base()) continue;
			Constraint cur = Constraint(t1, t2, ATOM_NEQ);
			res &= cur;
		}
	}

	return res;
}

Variable* Instantiator::instantiate_arithmetic_value(ArithmeticValue* av)
{
	if(temporaries.count(av) > 0)
		return temporaries[av];
	Variable* res = NULL;

	assert_context("Instantiating arithmetic value: " + av->to_string());


	long int c = av->get_constant();
	const map<Term*, long int>& elems = av->get_elems();
	if(c!=0)
	{
		AccessPath* elems_ap = ArithmeticValue::make(elems, 0);
		AccessPath* inst_elems = instantiate_ap(elems_ap);
		ConstantValue* cv = ConstantValue::make(c);
		Variable* temp = Variable::make_sum_temp(av->get_type());
		all_temps.insert(temp);
		string inst = temp->to_string() + "=";
		mg->process_plus(temp, inst_elems, cv, inst_guard);
		inst += inst_elems->to_string() + "+" + int_to_string(c);
		mg->ma.add_dotty(inst, temp, inst_elems, cv);
		mg->remove_root(temp);
		res = temp;

	}

	else {
		map<Term*, long int>::const_iterator it = elems.begin();
		Term* t = it->first;
		long int coef = it->second;
		c_assert(t->is_specialized());
		AccessPath* t_ap = AccessPath::to_ap(t);

		assert_context("Instantiating term: " + t_ap->to_string() + " with type: " +
				t_ap->get_type()->to_string());
		AccessPath* inst_ap = instantiate_ap(t_ap);
		assert_context("Result of instantiating term: " + t_ap->to_string()
				+ " with type: " + inst_ap->get_type()->to_string());



		//Variable* cur_temp = Variable::make_sum_temp(t_ap->get_type());
		Variable* cur_temp = Variable::make_sum_temp(inst_ap->get_type());


		all_temps.insert(cur_temp);
		ConstantValue* cv = ConstantValue::make(coef);
		string pp_exp  = "Instantiation of " + av->to_string();
		mg->process_binop(cur_temp, inst_ap, cv, inst_guard, il::_MULTIPLY,
				pp_exp);
		string cur_inst = cur_temp->to_string() + "= " +
				int_to_string(coef) + "*" + inst_ap->to_string();
		mg->ma.add_dotty(cur_inst, cur_temp, inst_ap, cv);


		it++;
		map<Term*, long int> new_elems;
		new_elems.insert(it, elems.end());
		AccessPath* rest_ap = ArithmeticValue::make(new_elems, 0);

		assert_context("Instantiating rest: " + rest_ap->to_string() +
				" with type: " + rest_ap->get_type()->to_string());


		AccessPath* rest_ap_inst = instantiate_ap(rest_ap);

		assert_context("Result of instantiating rest: " + rest_ap_inst->to_string() +
				" with type: " + rest_ap_inst->get_type()->to_string());



		//Variable* temp = Variable::make_sum_temp(av->get_type());
		Variable* temp;
		if(rest_ap_inst->get_type()->is_pointer_type()) {
			temp = Variable::make_sum_temp(rest_ap_inst->get_type());
		}
		else {
			temp = Variable::make_sum_temp(cur_temp->get_type());
		}

		all_temps.insert(temp);
		string inst = temp->to_string() + "=";

		mg->process_plus(temp, cur_temp, rest_ap_inst, inst_guard);
		inst += cur_temp->to_string() + "+" + rest_ap->to_string();
		mg->ma.add_dotty(inst, temp, cur_temp, rest_ap_inst);
		res = temp;


	}

	add_instantiation(av, res);
	return res;
}


Variable* Instantiator::instantiate_function_value(FunctionValue* av)
{

	assert_context("Instantiating function value: " + AccessPath::safe_string(av));
	bool is_length= av->is_length();
	bool is_shared_base = av->is_shared_base_predicate();

	const vector<AccessPath*>& args = av->get_arguments();
	vector<AccessPath*>::const_iterator it = args.begin();
	vector<set<pair<AccessPath*, Constraint> > > new_args;
	for(; it!= args.end(); it++)
	{
		AccessPath* ap = *it;
		cout << "Function arg: " << ap->to_string() << endl;
		if(is_length) ap= ap->strip_deref();
		AccessPath* inst_ap = instantiate_ap(ap);
		cout << "Inst ap: " << inst_ap->to_string() << endl;
		set<pair<AccessPath*, Constraint> > vs;
		mg->get_value_set(inst_ap, vs, stmt_guard);





		cout << "Value set: " << this->value_set_to_string(vs) << endl;

		if(is_length)
		{
			set<pair<AccessPath*, Constraint> > new_vs;
			set<pair<AccessPath*, Constraint> >::iterator it = vs.begin();
			for(; it!= vs.end(); it++) {
				AccessPath* ap = it->first;
				ap = ap->add_deref();
				new_vs.insert(make_pair(ap, it->second));
			}
			new_args.push_back(new_vs);
		}
		else if(is_shared_base) {
			set<pair<AccessPath*, Constraint> > new_vs;
			set<pair<AccessPath*, Constraint> >::iterator it = vs.begin();
			for(; it!= vs.end(); it++) {
				Constraint c = it->second;
				IndexVarManager::eliminate_source_vars(c);
				new_vs.insert(make_pair(it->first, c));
			}
			new_args.push_back(new_vs);
		}

		else new_args.push_back(vs);

	}

	 set<pair<vector<AccessPath*> , Constraint>  > all_args;
	AccessPath::n_ary_cross_product(new_args, all_args);

	Variable* temp = Variable::make_sum_temp(av->get_type());

	 set<pair<vector<AccessPath*> , Constraint>  >::iterator it2 =
			 all_args.begin();
	for(; it2!= all_args.end(); it2++)
	{
		const vector<AccessPath*>& cur_args = it2->first;
		Constraint c = inst_guard & it2->second;
		if(c.unsat()) continue;
		AccessPath* cur_fv = FunctionValue::make(av->get_fun_type(), cur_args,
				av->get_attribute());


		if(is_length) {
			AccessPath* fn = cur_fv;
			if(fn->get_ap_type() == AP_ARITHMETIC) {
				ArithmeticValue* av = (ArithmeticValue*) fn;
				const map<Term*, long int> elems = av->get_elems();
				map<Term*, long int>::const_iterator it = elems.begin();
				for(; it!= elems.end(); it++) {
					Term* t = it->first;
					if(!t->is_specialized()) continue;
					AccessPath* ap = AccessPath::to_ap(t);
					if(ap->is_length_function()) fn = ap;
				}
			}

			c_assert(fn->is_length_function());
			mg->sentinels.add_sentinel_assumption(fn, mg);
		}

		AccessPath* target = cur_fv->add_deref();
		IndexVarManager::rename_source_to_target_index(c);
		mg->put_edge(temp, target, c );

	}

	add_instantiation(av, temp);




	string inst = temp->to_string() + "=" + av->to_string();
	mg->ma.add_dotty(inst, temp, av);

	return temp;

}

void Instantiator::add_instantiation(AccessPath* sum_ap, Variable* inst_temp)
{

	temporaries[sum_ap] = inst_temp;
	set< pair<AccessPath*, Constraint> > & pts_to_set = instantiations[sum_ap];
	mg->get_points_to_set(inst_temp, pts_to_set, stmt_guard);

/*
	cout << "INST for " << sum_ap->to_string() << endl;
	set<pair<AccessPath*, Constraint> >::iterator it = pts_to_set.begin();
	for(; it != pts_to_set.end(); it++)
	{
		cout << it->first->to_string() << " under " << it->second << endl;
	}
*/
}


AccessPath* Instantiator::instantiate_ap(AccessPath* ap)
{

	assert_context("Instantiating access path: " + AccessPath::safe_string(ap));
	if(temporaries.count(ap) > 0) return temporaries[ap];

	// If this access path only contains constants,
	// its instantiation is itself.
	if(ap->is_derived_from_constant()) {
		add_instantiation(ap, ap);
		return ap;
	}

	if(ap->get_ap_type() == AP_STRING)
	{
		StringLiteral* sum_ap = (StringLiteral*) ap;
		StringLiteral* new_ap = StringLiteral::make(sum_ap->get_string_constant());
		add_instantiation(sum_ap, new_ap);
		return new_ap;
	}

	if(ap->get_ap_type() == AP_PROGRAM_FUNCTION)
	{
		ProgramFunction* sum_ap = (ProgramFunction*) ap;
		ProgramFunction* new_ap =
				ProgramFunction::make(sum_ap->get_function_name(),
						sum_ap->get_signature());
		add_instantiation(sum_ap, new_ap);
		return new_ap;
	}

	if(ap->get_ap_type() == AP_TYPE_CONSTANT)
	{
		TypeConstant* tc = (TypeConstant*) ap;
		TypeConstant* new_tc = TypeConstant::make(tc->get_type_constant());
		add_instantiation(tc, new_tc);
		return new_tc;

	}




	if(ap->get_ap_type() == AP_ARITHMETIC &&
			(ap->get_base() == NULL || ap->get_base()->get_ap_type() == AP_INDEX))
	{

		ArithmeticValue* av = (ArithmeticValue*) ap;
		AccessPath* res= instantiate_arithmetic_value(av);
		return res;
	}

	if(ap->get_ap_type() == AP_FUNCTION)
	{
		FunctionValue* fv = (FunctionValue*) ap;
		AccessPath* res= instantiate_function_value(fv);
		return res;
	}


	if(ap->get_ap_type() == AP_DEREF){
		AccessPath* inner = ap->get_inner();
		if(inner->get_ap_type() == AP_FUNCTION ||
				inner->get_ap_type() == AP_ARITHMETIC)
		{
			AccessPath* inner_inst = instantiate_ap(inner);
			AccessPath* res= Deref::make(inner_inst, ap->get_type());
			return res;
		}
	}

	AccessPath* sum_base = ap->get_base();
	Constraint inst_c(true);
	AccessPath* inst_base = get_base_instantiation(sum_base, inst_c);
	if(inst_base == NULL) return NULL;

	/*
	 * If inst_base and sum_base have different types,
	 * we need to process a cast.
	 */

	if(!(*inst_base->get_type() == *sum_base->get_type()))
	{
		//cout << *inst_base->get_type() << " ---> " << *sum_base->get_type() << endl;
		Variable* new_inst_base = Variable::make_sum_temp(sum_base->get_type());
		all_temps.insert(new_inst_base);
		mg->process_cast(new_inst_base, inst_base, inst_guard & inst_c);

		string inst = new_inst_base->to_string() + "= ";
		inst += "(" + new_inst_base->get_type()->to_string() + ")";
		inst += inst_base->to_string();
		inst_base = new_inst_base;
		mg->ma.add_dotty(inst, new_inst_base, inst_base);
	}

	/* to_generate is an access path whose base is a valid access path
	 * in the calling context; but we need to figure out what the actual
	 * target is. For instance, if to_generate is (*(*x.f)).g,
	 * we achieve this by generating instructions of the
	 * form t1 = *x.f, t2 = *t1.g, and the result of instantiate_internal
	 * is t2.
	 */

	AccessPath* to_generate = ap->set_base(inst_base);
	AccessPath* repl = ap->replace(sum_base, inst_base);
	if(to_generate != repl) {
		//cout << "INSTANIATING AP: " << ap->to_string() << endl;
		//cout << "Sum base: " << sum_base->to_string() << endl;
		//cout << "Inst base: " << inst_base->to_string() << endl;
		//cout << "Result set_base: " << to_generate->to_string() << endl;
		//cout << "Result replace: " << repl->to_string() << endl;
		//c_assert(false);
	}
	Variable* v = instantiate_ap_internal(to_generate, inst_c);
	add_instantiation(ap, v);
	return v;


}

AccessPath* Instantiator::instantiate_index(AccessPath* index)
{
	if(index->get_ap_type() == AP_INDEX)
	{
		IndexVariable* iv = (IndexVariable*) index;
		return iv;
	}

	AccessPath* inst_index = instantiate_ap(index);
	return inst_index;

}

Variable* Instantiator::instantiate_unit_ap(AccessPath* ap, Constraint inst_c)
{



	il::type* ap_type = ap->get_type();

	assert_context("Instantiating unit ap: " + ap->to_string() +
			" constraint: " + inst_c.to_string() + " type of temp: "
			+ ap_type->to_string());

	int offset = 0;
	string field_name;
	bool first = true;
	while(ap->get_ap_type() == AP_FIELD) {
		FieldSelection* fs = (FieldSelection*) ap;
		if(first) {
			offset = fs->get_field_offset();
			first = false;
		}

		field_name =  fs->get_field_name() + "." + field_name;
		ap = ap->get_inner();
	}


	// Field read or assignment case: temp = x or temp = x.f.g...
	if(ap->is_base())
	{

		assert_context("Instantiating unit using process_assignment: " +
				AccessPath::safe_string(ap));
		Variable* inst_temp = Variable::make_sum_temp(ap_type);
		all_temps.insert(inst_temp);
		string inst = inst_temp->to_string() + " = ";
		Variable* v = (Variable*) ap;
		mg->process_assignment(inst_temp, v, offset, inst_guard & inst_c);
		inst +=  ap->to_string() + field_name;
		mg->ma.add_dotty(inst, inst_temp, v);
		return inst_temp;
	}

	// Load case: temp = *(x+offset)
	else if(ap->get_ap_type() == AP_DEREF)
	{

		assert_context("Instantiating unit using process_load: " +
				AccessPath::safe_string(ap));
		AccessPath* inner =ap->get_inner();



		if(ap_type->is_abstract_data_type()) {
			il::type* value_t = ap_type->get_adt_value_type();
			if(!value_t->is_abstract_data_type() && value_t->is_record_type()) {
				il::record_type* rt = (il::record_type*) value_t;
				if(rt->get_field_from_offset(offset) != NULL) {
					value_t = rt->get_field_from_offset(offset)->t;
				}
			}
			ap_type = il::pointer_type::make(value_t,"");
		}
		Variable* inst_temp = Variable::make_sum_temp(ap_type);
		all_temps.insert(inst_temp);
		string inst = inst_temp->to_string() + " = ";
		c_assert(inner != NULL);
		c_assert(inner->get_ap_type() == AP_VARIABLE);
		Variable* inner_var = (Variable*) inner;
		Constraint load_c = inst_guard & inst_c;
		mg->process_load(inst_temp, inner_var, offset, load_c);

		if(offset == 0) inst += "*" + inner_var->to_string();
		else inst +=  inner_var->to_string() + "->"
					+ field_name;

		mg->ma.add_dotty(inst, inst_temp, inner_var);
		return inst_temp;

	}

	// Static array read case: x[c].f.g...
	else if(ap->get_ap_type() == AP_ARRAYREF && ap->get_inner()->get_ap_type()
			== AP_VARIABLE)
	{
		ArrayRef* ar = (ArrayRef*) ap;
		Variable* res = instantiate_unit_static_array_read(ar,
				offset, field_name, ap_type, inst_c);
		return res;


	}

	// Pointer array read: (*x)[c].f.g..
	else if(ap->get_ap_type() == AP_ARRAYREF && ap->get_inner()->get_ap_type()
			== AP_DEREF)
	{
		ArrayRef* ar = (ArrayRef*) ap;
		Variable* res = instantiate_unit_pointer_array_read(ar, offset,
				field_name, ap_type, inst_c);
		return res;

	}

	// Pointer arithmetic case: p+index+offset
	else if(ap->get_ap_type() == AP_ARITHMETIC)
	{
		ArithmeticValue* av = (ArithmeticValue*) ap;
		Variable* res = instantiate_unit_pointer_arithmetic(av, ap_type, inst_c);
		return res;
	}

	// Read from ADT: (*x)<i>
	else if(ap->get_ap_type() == AP_ADT)
	{
		AbstractDataStructure* abs = (AbstractDataStructure*) ap;
		il::type* abs_type = abs->get_type();
		//il::type* abs_ptr_type = il::pointer_type::make(abs_type, "");
		il::type* res_type = il::pointer_type::make(
				abs_type->get_adt_value_type(), "");


		Variable* res = instantiate_unit_adt_read(abs, offset, field_name,
					res_type, inst_c);

		set<pair<AccessPath*, Constraint> > vv;
 		this->mg->get_value_set(res, vv);
		return res;
	}

	else if(ap->get_ap_type() == AP_ADDRESS)
	{
		assert_context("Instantiating unit using process_address: " +
				AccessPath::safe_string(ap));
		c_assert(offset == 0);
		AccessPath* inner = ap->get_inner();
		Variable* inst_temp = Variable::make_sum_temp(ap_type);
		all_temps.insert(inst_temp);
		string inst = inst_temp->to_string() + " = &" +  inner->to_string();
		mg->process_address(inst_temp, inner, inst_guard);
		mg->ma.add_dotty(inst, inst_temp, inner);
		return inst_temp;


	}
	else c_assert(false);



}

/*
 * Access path of the form x[c].f.g...
 */
Variable* Instantiator::instantiate_unit_static_array_read(ArrayRef* ar,
		int offset, const string& field_name, il::type* result_type,
		Constraint inst_c)
{

	assert_context("Instantiating unit static array read: " +
			AccessPath::safe_string(ar) + " offset: " + int_to_string(offset) +
			" field name: "+ field_name + " result type: " +
			(result_type ? result_type->to_string() : "null") +
			" constraint: "+ inst_c.to_string());
	/*
	 * We want to get the element type of this array.
	 */
	while(result_type->is_array_type())
		result_type= result_type->get_inner_type();

	Variable* inst_temp = Variable::make_sum_temp(result_type);
	all_temps.insert(inst_temp);
	AccessPath* inner = ar->get_inner();
	Variable* inner_var = (Variable*) inner;
	AccessPath* index = ar->get_index_expression();
	AccessPath* inst_index= instantiate_index(index);
	Constraint c = inst_c & inst_guard;
	mg->process_array_ref_read(inst_temp, inner_var,
			inst_index, offset, c);
	string inst = inst_temp->to_string() + "=" +
			inner->to_string() + "[" + inst_index->to_string() +
			"]" + field_name;
	mg->ma.add_dotty(inst, inst_temp, inner_var, inst_index);
	return inst_temp;
}


Variable* Instantiator::instantiate_unit_adt_read(AbstractDataStructure* abs,
		int offset, const string& field_name, il::type* result_type,
		Constraint inst_c)
{

	assert_context("Instantiating unit adt read: "+ AccessPath::safe_string(abs) +
			" offset: " + int_to_string(offset) + " field name: " +
			field_name + " result type: " +
			(result_type ? result_type->to_string() : " null") + " constraint: "+
			inst_c.to_string());



	AccessPath* index= abs->get_index_expression();
	AccessPath* inst_index = instantiate_index(index);



	Variable* inst_temp;

	Constraint c = inst_c & inst_guard;

	AccessPath* inner = abs->get_inner();
	//c_assert(inner->get_ap_type() == AP_DEREF);
	AccessPath* adt_ptr = inner->strip_deref();

	string inst;
	if(offset != SIZE_OFFSET) {
		inst_temp = Variable::make_sum_temp(result_type);
		mg->process_adt_read(inst_temp, adt_ptr, inst_index, c, false, offset);
		inst  = inst_temp->to_string() + "=" +
				inner->to_string() + "<" + inst_index->to_string() + ">";
		mg->ma.add_dotty(inst, inst_temp, adt_ptr, inst_index);

	}
	else {
		c_assert(offset == SIZE_OFFSET);
		inst_temp = Variable::make_sum_temp(il::get_integer_type());

		mg->process_adt_get_size(inst_temp, adt_ptr, c);
		inst = inst_temp->to_string() + "=" +
				inner->to_string() + "<" + inst_index->to_string() + ">.size";
		mg->ma.add_dotty(inst, inst_temp, adt_ptr);


	}
	all_temps.insert(inst_temp);

	return inst_temp;



}

Variable* Instantiator::instantiate_unit_pointer_arithmetic(ArithmeticValue* av,
		il::type* result_type, Constraint base_c)
{
	assert_context("Instantiating unit pointer arithmetic: " +
			AccessPath::safe_string(av) + " result type: " +
			(result_type ? result_type->to_string() : "null") +
			" constraint: " + base_c.to_string());


	if(av->is_structured_ptr_arithmetic()) {
		return instantiate_structured_pointer_arithmetic(av, result_type, base_c);
	}
	else{
		return instantiate_unstructured_pointer_arithmetic(av, result_type,
				base_c);
	}
}

Variable* Instantiator::instantiate_structured_pointer_arithmetic(
		ArithmeticValue* av, il::type* result_type, Constraint base_c)
{
	assert_context("Instantiating structured pointer arithmetic: " +
			AccessPath::safe_string(av) + " result type: " +
			(result_type ? result_type->to_string() : "null") +
			" constraint: " + base_c.to_string());

	AccessPath* base_ptr = av->get_base_ptr();
	const vector<offset>& offsets = av->get_offsets();
	c_assert(offsets.size() > 0);

	AccessPath* cur_temp = base_ptr;
	il::type* cur_type = base_ptr->get_type();
	for(unsigned int i=0; i<offsets.size(); i++)
	{
		Variable* new_temp;
		string inst;
		offset o = offsets[i];
		if(o.is_field_offset())
		{
			cur_type =  il::pointer_type::make(o.field_type, "");
			new_temp = Variable::make_sum_temp(cur_type);
			all_temps.insert(new_temp);
			inst = new_temp->to_string() + " = " + cur_temp->to_string();


			mg->process_field_offset(new_temp, cur_temp, o.field_offset,
					inst_guard & base_c);
			inst += "+" + int_to_string(o.field_offset);
			mg->ma.add_dotty(inst, new_temp, cur_temp);
		}
		else {
			new_temp = Variable::make_sum_temp(cur_type);
			all_temps.insert(new_temp);
			inst = new_temp->to_string() + " = " + cur_temp->to_string();
			AccessPath* index = o.index;
			AccessPath* inst_index = instantiate_ap(index);
			Variable* offset_v = Variable::make_sum_temp(il::get_integer_type());
			all_temps.insert(offset_v);
			Constraint true_c;
			string display_str = "Instantiation of " + av->to_string();
			mg->process_binop(offset_v, inst_index, ConstantValue::make(o.elem_size),
					true_c, il::_MULTIPLY, display_str);
			mg->process_pointer_plus(new_temp, cur_temp, offset_v,
					inst_guard & base_c, o.elem_size);


			inst += "+" + int_to_string(o.elem_size) + "*" + inst_index->to_string();
			mg->ma.add_dotty(inst, new_temp, cur_temp, offset_v);


		}



		cur_temp = new_temp;
	}
	c_assert(cur_temp->get_ap_type() == AP_VARIABLE);
	return (Variable*)cur_temp;
}

/*
 * Instantiates an access path corresponding to pointer arithmetic:
 * p+index+offset
 */
Variable* Instantiator::instantiate_unstructured_pointer_arithmetic(
		ArithmeticValue* av, il::type* result_type, Constraint base_c)
{

	int offset =av->get_constant();
	AccessPath* pointer_ap = av->get_inner();
	c_assert(pointer_ap != NULL);

	const map<Term*, long int>& elems = av->get_elems();
	c_assert(elems.size() <=2);
	if(elems.size() == 1)
	{
		Variable* inst_temp = Variable::make_sum_temp(result_type);
		all_temps.insert(inst_temp);
		string inst = inst_temp->to_string() + " = ";
		ConstantValue* offset_ap = ConstantValue::make(offset);
		mg->process_plus(inst_temp, pointer_ap, offset_ap, inst_guard & base_c);
		inst += pointer_ap->to_string() + "+" + int_to_string(offset);
		mg->ma.add_dotty(inst, inst_temp, pointer_ap, offset_ap);
		return inst_temp;
	}

	else {
		map<Term*, long int>::const_iterator it = elems.begin();
		Term* t1 = it->first;
		it++;
		Term* t2= it->first;

		Term* index_t = (t1 == pointer_ap->to_term() ? t2 : t1);
		c_assert(index_t->is_specialized());
		AccessPath* index_ap = AccessPath::to_ap(index_t);
		long int index_factor = av->get_coefficient(index_ap);

		AccessPath* index_av = ArithmeticValue::make_times(index_ap,
				ConstantValue::make(index_factor));

		AccessPath* inst_index = instantiate_ap(index_av);

		// First generate, p+inst_index
		Variable* temp1 = Variable::make_sum_temp(pointer_ap->get_type());
		all_temps.insert(temp1);
		mg->process_plus(temp1, pointer_ap, inst_index, inst_guard & base_c);

		string inst =
				temp1->to_string() + "=" + pointer_ap->to_string() + "+"
				+ inst_index->to_string();
		mg->ma.add_dotty(inst, temp1, pointer_ap, inst_index);

		// If offset is not zero, also do index plus
		if(offset == 0) return temp1;
		ConstantValue* offset_ap = ConstantValue::make(offset);
		string prev_inst = temp1->to_string() + "= " +
				pointer_ap->to_string() + inst_index->to_string();

		Variable* inst_temp = Variable::make_sum_temp(result_type);
		all_temps.insert(inst_temp);
		mg->process_plus(inst_temp, temp1, offset_ap, inst_guard & base_c);
		string inst2 = inst_temp->to_string() + "=" +
				temp1->to_string() + "+" + int_to_string(offset);
		 mg->ma.add_dotty(inst2, inst_temp, temp1, offset_ap);
		return inst_temp;

	}
}

Variable* Instantiator::instantiate_unit_pointer_array_read(ArrayRef* ar,
		int offset, const string& field_name, il::type* result_type,
		Constraint inst_c)
{

	assert_context("Instantiating unit pointer array read: "+
			AccessPath::safe_string(ar) + " offset: " + int_to_string(offset) +
			" field name: " + field_name + " result type: " +
			(result_type? result_type->to_string() : " null") +
			" constraint: "+ inst_c.to_string());

	AccessPath* index= ar->get_index_expression();
	AccessPath* inst_index = instantiate_index(index);
	Deref* inner = (Deref*) ar->get_inner();
	int elem_s= ar->get_elem_size();
	ConstantValue* elem_size = ConstantValue::make(elem_s);

	Variable* temp1 = Variable::make_sum_temp(il::get_integer_type());
	all_temps.insert(temp1);
	string  inst1 = temp1->to_string() + "=" + int_to_string(elem_s) + "*" +
			inst_index->to_string();
	string display_str = "Instantiation of: " +  ar->to_string();
	mg->process_binop(temp1, inst_index, elem_size, inst_guard, il::_MULTIPLY,
			display_str);

	mg->ma.add_dotty(inst1, temp1, inst_index, elem_size);

	AccessPath* deref_inner = inner->get_inner();
	il::type* deref_inner_type = deref_inner->get_type();
	Variable* temp2 = Variable::make_sum_temp(deref_inner_type);
	all_temps.insert(temp2);
	mg->process_pointer_plus(temp2, deref_inner, temp1,
			inst_guard & inst_c, elem_s);
	string  inst2 = temp2->to_string() + "=" + deref_inner->to_string() + "+" +
			temp1->to_string();
	mg->ma.add_dotty(inst2, temp2, deref_inner, temp1);

	Variable* inst_temp = Variable::make_sum_temp(result_type);
	all_temps.insert(inst_temp);
	string inst = inst_temp->to_string() + " = ";


	mg->process_load(inst_temp, temp2, offset, inst_guard);
	if(offset == 0) {
			inst += "*" + temp2->to_string();
	}
	else {
		inst +=  temp2->to_string() + "->"
				+ field_name;
	}

	mg->ma.add_dotty(inst, inst_temp, temp2);
	return inst_temp;
}




Variable* Instantiator::instantiate_ap_internal(AccessPath* ap, Constraint inst_c)
{


	// Base case: We found one unit we can instantiate
	// with calling memory graph primitives
	if(is_directly_instantiable(ap))
	{
		Variable* res = instantiate_unit_ap(ap, inst_c);
		return res;
	}


	AccessPath* next = get_next_unit(ap);
	c_assert(next != NULL);
	AccessPath* res = instantiate_ap_internal(next, inst_c);
	AccessPath* replaced = ap->replace(next, res);
	c_assert(is_directly_instantiable(replaced));
	Variable* res_final = instantiate_unit_ap(replaced, inst_c);
	return res_final;



}

AccessPath* Instantiator::get_base_instantiation(AccessPath* ap, Constraint & c)
{
	if(ap->get_ap_type() == AP_INDEX) return ap;
	if(ap->get_ap_type() == AP_CONSTANT) return ap;
	if(ap->get_ap_type() == AP_STRING)
		return StringLiteral::make(((StringLiteral*)ap)->get_string_constant());
	if(ap->get_ap_type() == AP_PROGRAM_FUNCTION) {
		ProgramFunction* pf = (ProgramFunction*)ap;
		return ProgramFunction::make(pf->get_function_name(),
				pf->get_signature());
	}
	if(ap->get_ap_type() == AP_COUNTER) return get_unique_ap(ap, c);
	if(ap->get_ap_type() == AP_VARIABLE)
	{
		Variable* v = (Variable*) ap;
		if(sg->is_function_summary())
		{

			if(v->is_anonymous())
			{

				return get_unique_ap(v, c);
			}

			if(v->is_argument())
			{
				int arg_num = v->get_arg_number();
				assert_context(" Var is: " + v->to_string());
				assert_context(" Arg num is " + int_to_string(arg_num));
				c_assert(arg_num>=0 && arg_num< args->size());
				sail::Symbol* s = (*args)[arg_num];
				return mg->get_access_path_from_symbol(s);
			}

			if(v->is_global_var())
			{


				bool add_default = true;
				if(sg->is_function_summary())
				{

					if(sg->is_init_fn_summary()) add_default  = false;
				}
				if(add_default)
				{
					MemoryLocation* loc = mg->get_location(ap);
					mg->add_default_edges(loc);
				}
				return ap;
			}

			if(v->is_return_var())
			{
				if(retvar == NULL) return NULL;
				return mg->get_access_path_from_symbol(retvar);
			}

			if(v->is_string_const())
			{
				return v;
			}

			if(v->is_local_var())
			{

				sail::Variable* sail_v = v->get_original_var();
				c_assert(sail_v != NULL);
				if(report_leaked_stack_vars && !sail_v->is_static())
				{


					mg->report_error(ERROR_STACK_RETURN, "Callee " +
							sg->get_sum_short_id() +
							" leaks stack allocated variable " +
							v->to_string() + " Access path: " + ap->to_string() );
					string display_str = "Callee-leaked stack variable "
								+ v->to_string();
					DisplayTag dt(display_str, mg->line, mg->file);
					return UnmodeledValue::make_invalid(v->get_type(), dt);
				}
				return v;
			}
			return get_unique_ap(v, c);
		}
		else
		{
			//In loop
			return get_unique_ap(v, c);
		}
	}

	if(ap->get_ap_type() == AP_ALLOC || AP_UNMODELED)
	{
		AccessPath* inst_ap = get_unique_ap(ap, c);
		return inst_ap;
	}



	assert_context("Unexpected base ap in get_base_instantiation: " +
			ap->to_string());
	c_assert(false);
}

AccessPath* Instantiator::get_unique_ap(AccessPath* ap, Constraint & c)
{


	AccessPath* res = NULL;
	string context = int_to_string(mg->instruction_number) + "$" +
			sg->get_sum_short_id();
	switch(ap->get_ap_type())
	{
	case AP_UNMODELED:
	{
		/*
		 * We want to map one unmodeled ap in the summary
		 * always to the same unmodeled ap in the instantiation
		 */
		if(unmodeled_inst_map.count(ap) > 0) return unmodeled_inst_map[ap];
		UnmodeledValue* uv = (UnmodeledValue*) ap;
		IndexVariable* index_var = NULL;
		AccessPath* new_unmodeled;
		if(uv->get_index_expression()!= NULL) {
			IndexVariable* dummy_index = mg->ivm.get_dummy_index();
			UnmodeledValue* dummy_unmodeled = UnmodeledValue::make(
					uv->get_type(), dummy_index, uv->get_kind(),
					uv->get_display_tag());
			AccessPath* sum_index = uv->get_index_expression();
			AccessPath* inst_index_exp = instantiate_ap(sum_index);
			index_var = mg->ivm.get_new_source_index(dummy_unmodeled, false);
			set<pair<AccessPath*, Constraint> > values;

			mg->get_value_set(inst_index_exp, values, stmt_guard);
			set<pair<AccessPath*, Constraint> >::iterator it = values.begin();
			c = Constraint(false);
			for(; it!= values.end(); it++)
			{
				AccessPath* val = it->first;
				Constraint val_c = it->second;
				c |= val_c &
						ConstraintGenerator::get_eq_constraint(index_var, val);
			}
			new_unmodeled = dummy_unmodeled->replace(dummy_index, index_var);

		}

		else if(mg->ma.get_su()->is_superblock())
		{
			IndexVariable* dummy_index = mg->ivm.get_dummy_index();
			UnmodeledValue* dummy_uv = UnmodeledValue::make(uv->get_type(),
					dummy_index, uv->get_kind(), uv->get_display_tag());
			index_var = mg->ivm.get_new_source_index(dummy_uv, false);
			new_unmodeled = dummy_uv->replace(dummy_index, index_var);
			IndexVariable* target_index_var = IndexVariable::make_target(
					index_var);

			IterationCounter* iteration_counter = IterationCounter::make_parametric(
					mg->ma.get_cur_iteration_counter());
			Constraint target_index_c = ConstraintGenerator::get_eq_constraint(
					target_index_var, iteration_counter);
			c = target_index_c;
		}
		else{
			new_unmodeled = UnmodeledValue::make(uv->get_type(), uv->get_kind(),
					uv->get_display_tag());
		}
		unmodeled_inst_map[ap] = new_unmodeled;
		res = new_unmodeled;
		break;


	}
	case AP_ALLOC:
	{
		Alloc* a = (Alloc*) ap;
		alloc_info new_id(a->get_alloc_id());
		new_id.push_caller(mg->line, mg->instruction_number,
				mg->su->get_identifier());
		IndexVariable* index_var = NULL;
		if(a->get_index_expression() != NULL) {
			IndexVariable* dummy_index = mg->ivm.get_dummy_index();
			Alloc* dummy_alloc = Alloc::make(new_id, a->get_type(),
				dummy_index, a->is_nonnull());
			AccessPath* sum_index = a->get_index_expression();
			AccessPath* inst_index_exp = instantiate_ap(sum_index);
			index_var = mg->ivm.get_new_source_index(dummy_alloc, false);
			set<pair<AccessPath*, Constraint> > values;

			mg->get_value_set(inst_index_exp, values, stmt_guard);
			set<pair<AccessPath*, Constraint> >::iterator it = values.begin();
			c = Constraint(false);
			for(; it!= values.end(); it++)
			{
				AccessPath* val = it->first;
				Constraint val_c = it->second;
				c |= val_c &
						ConstraintGenerator::get_eq_constraint(index_var, val);
			}
		}

		else if(mg->ma.get_su()->is_superblock())
		{
			IndexVariable* dummy_index = mg->ivm.get_dummy_index();
			Alloc* dummy_alloc =
					Alloc::make(new_id, a->get_type(), dummy_index, a->is_nonnull());
			index_var = mg->ivm.get_new_source_index(dummy_alloc, false);
			IndexVariable* target_index_var = IndexVariable::make_target(index_var);

			IterationCounter* iteration_counter = IterationCounter::make_parametric(
					mg->ma.get_cur_iteration_counter());
			Constraint target_index_c = ConstraintGenerator::get_eq_constraint(
					target_index_var, iteration_counter);
			c = target_index_c;

		}

		Alloc* new_ap = Alloc::make(new_id, a->get_type(),
				index_var, a->is_nonnull());
		res= new_ap;
		break;
	}
	case AP_COUNTER:
	{

		IterationCounter* ic = (IterationCounter*) ap;
		return IterationCounter::add_context(ic,mg->instruction_number,
					sg->get_sum_short_id(), !is_loop);

	}
	case AP_VARIABLE:
	{
		Variable* v= (Variable*) ap;

		if(sg->is_loop_summary()) return ap;

		if(v->get_original_var() == NULL) return ap;
		sail::Variable* sail_var = (sail::Variable*) v->get_original_var();
		if(!sail_var->is_anonymous()) return ap;
		string name = sail_var->get_var_name();
		name += "_" + sg->get_sum_short_id() + int_to_string(mg->instruction_number);
		sail::Variable* sail_var2 = new sail::Variable(name,
				sail_var->get_type(), true);
		res= Variable::make_program_var(sail_var2);
		break;


	}
	default:
		return ap;
	}

	Term* t = ap->to_term();
	term_attribute_type att = t->get_attribute();
	res->to_term()->set_attribute(att);
	return res;


}

/*
 * Can the instantiation of this access path be reduced
 * to processing a single instruction by calling
 * memory graph primitives?
 */
bool Instantiator::is_directly_instantiable(AccessPath* ap)
{
	return is_directly_instantiable(ap, true);
}

bool Instantiator::is_directly_instantiable(AccessPath* ap, bool ok)
{
	switch(ap->get_ap_type())
	{
	case AP_VARIABLE:
	case AP_COUNTER:
	case AP_CONSTANT:
	case AP_INDEX:
		return true;
	case AP_ALLOC:
	case AP_UNMODELED:
		return ok;
	case AP_DEREF:
		if(!ok) return false;
		else
			return is_directly_instantiable(ap->get_inner(), false);

	case AP_FIELD:
	{
		if(!ok) return false;
		return is_directly_instantiable(ap->get_inner(), ok);
	}
	case AP_ARRAYREF:
	case AP_ADT:
	{
		if(!ok) return false;
		AccessPath* inner = ap->get_inner();
		if(inner->get_ap_type() == AP_DEREF)
		{
			AccessPath* next = inner->get_inner();
			return is_directly_instantiable(next, false);
		}
		// static array case
		else {
			return is_directly_instantiable(inner, false);
		}

	}
	case AP_ARITHMETIC:
	{
		AccessPath* inner = ap->get_inner();
		c_assert(inner != NULL); // otherwise this can't be a memory access path
		return inner->is_base();
	}
	case AP_ADDRESS:
	{
		AccessPath* inner = ap->get_inner();

		// Invariant check: Inside an address access path,
		// we should either only only see a variable or
		// pointer arithmetic
		bool ok = false;
		if(inner->get_ap_type() == AP_VARIABLE) ok = true;
		if(inner->get_ap_type() == AP_STRING) ok = true;
		if(inner->get_ap_type() == AP_PROGRAM_FUNCTION) ok =true;
		if(inner->get_ap_type() == AP_UNMODELED) ok=true;
		if(inner->get_ap_type() == AP_ARITHMETIC) {
			ArithmeticValue* av = (ArithmeticValue*) inner;
			if(av->get_inner() != NULL) ok =true;
		}
		c_assert(ok);
		return true;
	}

	default:
		assert_context("Unexpected ap in is_directly_instantiable: " +
				ap->to_string());
		c_assert(false);


	}
}

AccessPath* Instantiator::get_next_unit(AccessPath*& orig_ap)
{

	assert_context("Getting next unit ap: " + AccessPath::safe_string(orig_ap));

	AccessPath* ap = orig_ap;
	vector<offset> offsets;

	while(ap->get_ap_type() == AP_FIELD) {
		FieldSelection* fs = (FieldSelection*) ap;
		offset o(fs->get_field_name(), fs->get_field_offset(), fs->get_type());
		offsets.push_back(o);

		if(offsets.size() > 0) {
			for(int i=offsets.size()-2; i>=0; i-- ) {
				offset& cur = offsets[i];
				if(cur.is_field_offset()) {
					cur.field_offset -= o.field_offset;
					break;
				}
			}
		}

		ap = ap->get_inner();

	}


	if(ap->get_ap_type() == AP_ARRAYREF || ap->get_ap_type() == AP_ADT)
	{
		AccessPath* inner = ap->get_inner();
		if(inner->get_ap_type() == AP_DEREF){
			return inner->get_inner();
		}

		if(inner->get_ap_type() == AP_FIELD) {

			AccessPath* index_exp = ap->get_index_expression();
			c_assert(index_exp != NULL);
			int elem_size = 1;
			if(ap->get_ap_type() == AP_ARRAYREF)
			{
				ArrayRef* ar = (ArrayRef*) ap;
				elem_size = ar->get_elem_size();
			}
			offset o(elem_size, index_exp);
			offsets.push_back(o);
			while(inner->get_ap_type() == AP_FIELD ||
					inner->get_ap_type() == AP_ARRAYREF ||
					inner->get_ap_type() == AP_ADT) {
				offset o;
				if(inner->get_ap_type() == AP_FIELD) {
					FieldSelection* fs = (FieldSelection*) inner;
					o = offset(fs->get_field_name(),
							fs->get_field_offset(), fs->get_type());

					if(offsets.size() > 0) {
						for(int i=offsets.size()-2; i>=0; i-- ) {
							offset& cur = offsets[i];
							if(cur.is_field_offset()) {
								cur.field_offset -= o.field_offset;
								break;
							}
						}
					}

				}
				else if(inner->get_ap_type() == AP_ADT)
				{
					o = offset(1, ap->get_index_expression());
				}
				else {
					ArrayRef* ar = (ArrayRef*) inner;
					o = offset(ar->get_elem_size(), ar->get_index_expression());
				}
				offsets.push_back(o);
				inner=inner->get_inner();
			}

			vector<offset> reversed_offsets;
			for(int i=offsets.size()-1; i>=0; i--) {
				reversed_offsets.push_back(offsets[i]);
			}


			AccessPath* new_inner = inner->strip_deref();
			new_inner = ArithmeticValue::make_structured_pointer_arithmetic(
					new_inner, reversed_offsets);


			orig_ap = Deref::make(new_inner);
			return new_inner;

		}

	}

	/*if(ap->get_ap_type() == AP_ADT) {
		AccessPath* inner = ap->get_inner();
		//c_assert(inner->get_ap_type() == AP_DEREF);
		AccessPath* res = inner->get_inner();
		return inner->get_inner();
	}*/

	return ap->get_inner();


}


void Instantiator::add_instantiation(AccessPath* sum_ap, AccessPath* inst_ap)
{
	pair<AccessPath*, Constraint> val(inst_ap->add_deref(), Constraint());
	instantiations[sum_ap].insert(val);
}

MemoryGraph* Instantiator::get_memory_graph()
{
	return this->mg;
}

sail::SummaryUnit* Instantiator::get_current_unit()
{
	return this->mg->ma.get_su();
}



string Instantiator::value_set_to_string(set<pair<AccessPath*, Constraint> >& vs)
{
	string res = "[";
	set<pair<AccessPath*, Constraint> >::iterator it = vs.begin();
	for(int i=0; it!= vs.end(); it++, i++)
	{
		AccessPath* ap = it->first;
		Constraint c = it->second;
		res += "(" + ap->to_string() + ", " + c.to_string() + ")";
		if(i!= (int)vs.size() -1) res += ", ";

	}
	res += "]";
	return res;
}


/*
 * Instantiates the trigger conditions for errors and reports any
 * errors if (i) they are implied by the statement guard or (ii) if
 * the current function has no callers. Otherwise, if they cannot be
 * discharged propagates error traces up the call chain.
 */
void Instantiator::instantiate_error_traces()
{

	set<ErrorTrace*>::iterator it = sg->get_error_traces().begin();
	for(; it!= sg->get_error_traces().end(); it++)
	{
		ErrorTrace* et = *it;
		assert_context("Instantiating error trace: " + et->to_string());
		set<ErrorTrace*> new_traces;
		et->instantiate_trace(*this, new_traces, mg->report_all_errors);
		set<ErrorTrace*>::iterator it = new_traces.begin();
		for(; it!= new_traces.end(); it++)
		{
			ErrorTrace* cur = *it;
			Constraint new_cond = (this->simplify_pos(cur->get_failure_cond()));
			if(new_cond.unsat()) continue;
			cur->set_failure_cond(new_cond);
			mg->error_traces.insert(cur);



		}

	}
}

void Instantiator::enforce_existence_and_uniqueness()
{

	assert_context("Enforcing existence and uniqueness after instantiation of " +
			sg->get_sum_short_id());

	map<AccessPath*, set<Edge*> > outgoing_edges;
	set<Edge*>::iterator it = sg->edges.begin();
	for( ; it!= sg->edges.end(); it++)
	{
		Edge* e = *it;
		Constraint sum_c = e->get_constraint();



		Constraint inst_c = instantiate_constraint(sum_c);

		if(inst_c.unsat()) {
			this->edge_to_inst_constraints[e] = Constraint(false);

			continue;
		}
		outgoing_edges[e->get_source_ap()].insert(e);


	}



	map<AccessPath*, set<Edge*> >::iterator it2 = outgoing_edges.begin();
	for(; it2 != outgoing_edges.end(); it2++)
	{

		AccessPath* source_ap = it2->first;
		set<Edge*>& edges = it2->second;
		Constraint update_c = sg->get_update_condition(source_ap);


		mg->ie.enforce_existence_and_uniqueness(*this,source_ap, update_c,edges,
				mg->uniqueness_to_sum_c);



	}

	cout << "************** D Mapping ************" << endl;
	map<Constraint, Constraint>::iterator d_it = mg->uniqueness_to_sum_c.begin();
	for(; d_it != mg->uniqueness_to_sum_c.end(); d_it++)
	{
		cout << d_it->first << " -----> " << d_it->second << endl;
	}
	cout << "************** D Mapping END ********" << endl;


}

Instantiator::~Instantiator()
{

}
