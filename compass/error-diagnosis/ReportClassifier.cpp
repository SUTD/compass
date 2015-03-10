/*
 * ReportClassifier.cpp
 *
 *  Created on: Sep 15, 2011
 *      Author: isil
 */

#include "ReportClassifier.h"
#include "access-path.h"
#include "ErrorTrace.h"
#include "util.h"
#include "cnode.h"
#include "SummaryGraph.h"
#include "IndexVarManager.h"
#include "DisjointVarManager.h"
#include "AssertionTrace.h"
#include "ConstraintGenerator.h"

#define HIGH_COST 100
#define INFINITE_COST 10000


/*
 * As a heuristic, we want to favor queries that contain fewer disjuncts.
 */
bool CompareQuery::operator()(const CNode* c1, const CNode* c2) const
{
   int num_ops1 = 1;
   int num_ops2 = 1;
   if(c1->get_type() == OR) {
	   Connective* cc1 = (Connective*) static_cast<const Connective*>(c1);
	   num_ops1 = cc1->get_operands().size();
   }

   if(c2->get_type() == OR) {
	   Connective* cc2 = (Connective*) static_cast<const Connective*>(c2);
	   num_ops2 = cc2->get_operands().size();
   }

   if(num_ops1 < num_ops2) return true;
   else if(num_ops1 > num_ops2) return false;
   return c1 < c2;
}

/*
 * As a heuristic, among queries that correspond to leaves, we want to
 * favor those that contain the fewest variables.
 *
 */
bool CompareLeafQuery::operator()(const CNode* c1, const CNode* c2) const
{
	set<Term*> terms1, terms2;
	((CNode*)c1)->get_vars(terms1);
	((CNode*)c2)->get_vars(terms2);
	if(terms1.size() < terms2.size()) return true;
	else if(terms1.size() > terms2.size()) return false;
	return c1 < c2;
}



ReportClassifier::ReportClassifier(ErrorTrace* report, Constraint inv,
		SummaryGraph* sg)
{
	this->sg = sg;
	this->report = report;
	this->invariants = inv;
	this->failure_cond = report->get_failure_cond();
	this->success_cond = !this->failure_cond;

	cout << "**failure cond: " << failure_cond << endl;
	cout << "invariants: " << invariants << endl;

	assert(failure_cond.is_precise());


	if(report->get_trace_type() == ASSERTION_TRACE){
		AssertionTrace* at = static_cast<AssertionTrace*>(report);
		stmt_guard = at->get_stmt_guard();
	}


	this->failure_cond =
		DisjointVarManager::replace_disjointness_constraints(
				this->failure_cond, sg);

	this->success_cond =
			DisjointVarManager::replace_disjointness_constraints(
							this->success_cond, sg);

	stmt_guard = DisjointVarManager::replace_disjointness_constraints(stmt_guard, sg);





	if(!failure_cond.is_precise()){
		cout << "++++ Failure cond after eliminating disjointness conditions: " <<
				failure_cond << endl;
		assert(false);
	}




	init_var_mappings();
	init_costs();


	failure_cond.replace_terms(fun_terms_to_vars);
	success_cond.replace_terms(fun_terms_to_vars);
	invariants.replace_terms(fun_terms_to_vars);
	stmt_guard.replace_terms(fun_terms_to_vars);


	/*
	 * Simplify invariants so they only contain variables mentioned
	 * in error condition
	 */
	set<Term*> success_vars;
	success_cond.get_free_variables(success_vars);
	set<Term*> inv_vars;
	invariants.get_free_variables(inv_vars);

	set<Term*>::iterator it = inv_vars.begin();
	for(; it != inv_vars.end(); it++) {
		Term* t = *it;
		if(success_vars.count(t) >0) continue;
		VariableTerm* vt = static_cast<VariableTerm*>(t);
		invariants.eliminate_evar(vt);
	}




	cout << "FAIL C: " << failure_cond << endl;
	cout << "SUCCESS C: " << success_cond << endl;
	cout << "INVARIANTS: " << invariants << endl;
	cout << "STATEMENT GUARD: " << stmt_guard << endl;


}


/*
 * Sets the cost of each variable
 */
void ReportClassifier::init_costs()
{
	cout << "INIT COSTS!!!" << endl;
	set<Term*> terms;
	success_cond.get_terms(terms, false);
	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		cout << "Term: " << t->to_string() << endl;
		AccessPath* ap = AccessPath::to_ap(t);
		if(!ap->is_unobservable(false)) {
			cout << "Observable: " << ap->to_string() << endl;
			if(fun_terms_to_vars.count(ap->to_term()) > 0)
				t = fun_terms_to_vars[ap->to_term()];
			cout << "Replaced " << t->to_string() << endl;
			if(t->get_term_type() != VARIABLE_TERM) continue;
			var_costs[static_cast<VariableTerm*>(t)] = HIGH_COST;
			cout << "High cost: " << t->to_string() << endl;

		}
	}

}

/*
 * Builds a mapping from function terms to variables used internally
 * and the reverse mapping
 */
void ReportClassifier::init_var_mappings()
{
	set<Term*> terms;
	success_cond.get_terms(terms, false);
	invariants.get_terms(terms, false);



	set<Term*>::iterator it = terms.begin();
	for(; it!= terms.end(); it++)
	{
		Term* t = *it;
		AccessPath* ap = AccessPath::to_ap(t);


		if(replace_with_var(ap)) {

			int count = fun_terms_to_vars.size();
			string name = "v" + int_to_string(count) + "{" +
					ap->to_string() + "}";
			Variable* temp = Variable::make_temp(name);
			fun_terms_to_vars[t] = temp->to_term();
			vars_to_fun_terms[temp->to_term()] = t;

		}

	}

}

/*
 * Error is discharged if invariants imply success condition.
 */
bool ReportClassifier::error_discharged()
{
	if(invariants.implies(success_cond))
		return true;
	return false;
}

/*
 * Error is validated if either:
 * 1) invariants imply failure condition or
 * 2) (invariants & success condition) implies non-invariant.
 * Rational: If success condition implies phi, phi is a necessary condition
 * for the error to be discharged. If phi is known not to hold (i.e., there is
 * a witness that violates this necessary condition), then the program must have
 * a real error.
 * This is also equivalent to checking if invariants & !(non-invariant) =>
 * failure condition.
 */
bool ReportClassifier::error_validated()
{


	if(invariants.implies(failure_cond)) {
		return true;
	}

	set<Constraint>::iterator it = non_invariants.begin();
	for(; it!= non_invariants.end(); it++)
	{
		Constraint cur = *it;
		Constraint inv_and_success = invariants & success_cond;
		if(inv_and_success.implies(cur)) {
			return true;
		}
	}
	return false;
}


/*
 * There are two ways in which we know we have exhausted all the queries
 * we can ask without resolving the report:
 * 1) A fact is necessary to discharge the error, but user cannot show it,
 *    i.e., (invariants & success condition) imply A, but user does not
 *    know if A holds.
 *  2) A fact is necessary to validate the error, but user cannot show it,
 *    i,e.,  (invariants & failure condition) imply A, so there must be some
 *    execution where A holds <=> !A is a non-invariant, but user cannot
 *    say !A is a non-invariant.
 */
bool ReportClassifier::queries_exhausted()
{
	/*
	 * Case 1: We have exhausted all queries to discharge the error
	 */

	bool discharge_queries_left = true;

	set<Constraint>::iterator it = unknown_facts.begin();
	for(; it!= unknown_facts.end(); it++)
	{
		Constraint c = *it;
		Constraint neg_c = !c;
		Constraint to_check = invariants & success_cond & neg_c;
		if(to_check.unsat()) {
			discharge_queries_left = false;
			break;
		}
	}

	/*
	 * If discharge_queries_left is true, we still have queries we can
	 * ask to discharge the error
	 */
	if(discharge_queries_left) return false;


	/*
	 * Case 2: We have exhausted all queries to validate the error
	 */
	for(it = unknown_facts.begin(); it != unknown_facts.end(); it++)
	{
		Constraint unknown = *it;
		Constraint neg_unknown = !unknown;
		Constraint inv_and_fail = invariants & failure_cond;
		if(inv_and_fail.implies(neg_unknown)) return true;
	}

	return false;
}

/*
 * Computes the set of assumptions the MSA must obey
 * so that we don't repeat the same queries over and over again
 */
void ReportClassifier::compute_msa_assumptions(set<Constraint>& background)
{


	background.insert(invariants);

	set<Constraint>::iterator it = non_invariants.begin();
	for(; it!= non_invariants.end(); it++) {
		Constraint non_inv = *it;
		Constraint new_bg = invariants & !non_inv;
		background.insert(new_bg);

	}

	for(it = unknown_facts.begin(); it != unknown_facts.end(); it++)
	{
		Constraint cur = *it;
		Constraint cur_and_inv = invariants & (!cur);
		background.insert(cur_and_inv);

	}


}

struct resolve_query
{
	int cost;
	set<VariableTerm*> *msa_vars;
	bool validate_chosen;
	Constraint query_c;
	int number;

	resolve_query(int cost, set<VariableTerm*> *msa_vars, bool validate_chosen,
			Constraint query_c, int number)
	{
		this->cost = cost;
		this->msa_vars = msa_vars;
		this->validate_chosen = validate_chosen;
		this->query_c = query_c;
		this->number = number;
	}

	bool operator<(const resolve_query & other) const
	{
		if(cost < other.cost) return true;
		if(cost > other.cost) return false;

		Constraint my_c =
				ReportClassifier::get_min_var_sufficient_cond(query_c, *msa_vars);
		Constraint other_c =
				ReportClassifier::get_min_var_sufficient_cond(other.query_c,
				*other.msa_vars);

		CNode* c1 = my_c.get_cnodes().first;
		CNode* c2 = other_c.get_cnodes().first;

		c1 = c1->to_cnf();
		c2 = c2->to_cnf();

		int num1 = c1->num_disjuncts();
		int num2 = c2->num_disjuncts();
		if(num1 < num2) return true;
		if(num1 > num2) return false;

		return number < other.number;
	}


};

/*
 * There are four ways to conclusively resolve the report
 * 1) If we show the assertion condition, error is discharged
 * (for this, we can assume the stmt guard because assertion needs to reachable)
 * 2) If we show assertion is unreachable, error is discharged
 * (i.e., show the negation of statement guard)
 * 3) If we show !stmt_guard | assertion condition, error is discharged
 * (this supercedes 1&2, only when there is something disjunctive)
 * 4) If we show stmt_guard & !assertion_condition, error is validated
 *
 * We compute the cost of each, and pick the cheapest one as our
 * current strategy.
 */
UserQuery ReportClassifier::get_query()
{

	cout << "Getting next proof obligation... " << endl;

	/*
	 * If we already have pending queries, do those first!
	 */
	if(query_queue.size() > 0) {
		CNode* next_q = *query_queue.begin();
		query_queue.erase(next_q);
		return atom_to_query(next_q, cur_qt);
	}

	current_clause_assumptions.clear();

	set<Constraint> background;
	compute_msa_assumptions(background);

	/*
	 * Case 1: Prove assertion condition
	 */
	Constraint invariants1 = invariants & stmt_guard;
	Constraint assertion_c = success_cond;
	assertion_c.assume(stmt_guard);
	Constraint c1 =  ((!invariants1) | assertion_c);
	set<Constraint> background1;
	for(set<Constraint>::iterator it = background.begin();
			it != background.end(); it++)
	{
		Constraint c = *it;
		background1.insert(c & stmt_guard);
	}
	set<VariableTerm*> msa_vars1;
	int cost1 = c1.msa(msa_vars1, background1, var_costs);

	/*
	 * Case 2: Prove negation of stmt guard
	 */
	Constraint c2= ((!invariants) | (!stmt_guard));
	set<VariableTerm*> msa_vars2;
	int cost2 = c2.msa(msa_vars2, background, var_costs);


	/*
	 * Case 3: Prove success condition (either stmt guard not reached or
	 * assertion cond holds)
	 */
	Constraint c3 = ((!invariants) | success_cond); // I=> success
	set<VariableTerm*> msa_vars3;
	int cost3 = c3.msa(msa_vars3, background, var_costs);

	/*
	 * Case 4: Find witness to failure condition
	 */
	Constraint c4 = ((!invariants) | failure_cond); // I=> fail
	cout << "c4: " << c4 << endl;
	set<VariableTerm*> msa_vars4;
	int cost4 = c4.msa(msa_vars4, background, var_costs);
	cout << "cost: " << cost4 << endl;



	Constraint query_c;
	set<VariableTerm*> *msa_vars;
	bool validate_chosen = false;
	cur_qt = INVARIANT_QUERY;



	//-------------------------
	resolve_query q1(cost1, &msa_vars1, false, c1, 1);
	resolve_query q2(cost2, &msa_vars2, false, c2, 2);
	resolve_query q3(cost3, &msa_vars3, false, c3, 3);
	resolve_query q4(cost4, &msa_vars4, true, c4, 4);

	set<resolve_query> query_set;
	//query_set.insert(q1);
	//query_set.insert(q2);
	//query_set.insert(q3);
	query_set.insert(q4);

	resolve_query query_to_prove = *query_set.begin();
	query_c = query_to_prove.query_c;
	msa_vars = query_to_prove.msa_vars;
	validate_chosen = query_to_prove.validate_chosen;
	if(validate_chosen) cur_qt = WITNESS_QUERY;

	//-------------------------



	/*
	set<int> costs;
	costs.insert(cost1);
	costs.insert(cost2);
	costs.insert(cost3);
	costs.insert(cost4);




	int min_cost = *costs.begin();
	if(min_cost == cost4) {
		query_c = c4;
		msa_vars = &msa_vars4;
		validate_chosen = true;
		cur_qt = WITNESS_QUERY;
	}

	else if(min_cost == cost1) {
		query_c = c1;
		msa_vars = &msa_vars1;
	}
	else if(min_cost == cost2) {
		query_c = c2;
		msa_vars = &msa_vars2;
	}
	else if(min_cost == cost3) {
		query_c = c3;
		msa_vars = &msa_vars3;
	}
	else {
		assert(false);
	}
*/




	cout << "Query c: " << query_c << endl;





	if(msa_vars->size() == 0) cout << "MSA EMPTY!!! " << endl;
	else {
		cout << "+++ Vars in MSA ++++" << endl;
		set<VariableTerm*>::iterator it = msa_vars->begin();
		for(; it!= msa_vars->end(); it++){
			VariableTerm* t = *it;
			cout << "\t" << t->to_string() << endl;
		}
		cout << " +++++++++++++++++++" << endl;
	}


	query_c = get_min_var_sufficient_cond(query_c, *msa_vars);



	if(validate_chosen) {
		query_c = !query_c;
	}



	query_c.assume(invariants);
	if(query_to_prove.number == 1) {
		query_c.assume(stmt_guard);
	}

	cout << "Proof obligation after assuming: " << query_c << endl;

	//query_c.replace_terms(vars_to_fun_terms);


	CNode* query_node = query_c.get_cnodes().first;


	UserQuery q =  decompose_proof(query_node, cur_qt);
	cout << "New query: " << q.to_string() << endl;

	return q;

	//return proof_obligation;




}

/*
 * Computes a non-trivial (i.e., satisfiable) sufficient condition for
 * c containing the minimum number of variables. This is done
 * by universally quantifying variables that are not
 * part of the MSA of the constraint c.
 */
Constraint ReportClassifier::get_min_var_sufficient_cond(Constraint c,
		set<VariableTerm*>& msa_vars)
{
	set<Term*> vars;
	c.get_free_variables(vars);
	set<VariableTerm*> to_eliminate;
	set<Term*>::iterator it = vars.begin();
	for(; it!= vars.end(); it++)
	{
		Term* t = *it;
		if(t->get_term_type() != VARIABLE_TERM) continue;
		VariableTerm* vt = static_cast<VariableTerm*>(t);
		if(msa_vars.count(vt) == 0) {
			to_eliminate.insert(vt);
		}

	}

	c.eliminate_uvars(to_eliminate);
	return c;


}
/*
 * Returns the conjunction of current clause assumptions
 */
Constraint ReportClassifier::get_current_clause_assumptions()
{
	Constraint res(true);
	set<CNode*, CompareLeafQuery>::iterator it =
			current_clause_assumptions.begin();
	for(; it!= current_clause_assumptions.end(); it++) {
		CNode* cur = *it;
		Constraint cur_c(cur);
		res &= cur_c;
	}

	return res;
}


UserQuery ReportClassifier::decompose_proof(CNode* proof_c, query_type qt)
{
	cout << "Decomposing proof: " << proof_c->to_string() << endl;
	CNode* proof_cnf = proof_c->to_cnf();
	cout << "IN CNF: " << proof_cnf->to_string() << endl;


	if(proof_cnf->get_type() != AND)
	{
		return clause_to_query(proof_cnf, qt);
	}

	else {

		/*
		 * Sort queries so we first query the clause with minimum
		 * number of OR's. Also, clause may contain redundant entries after
		 * converting so CNF (e.g., already implied by invariant), so
		 * simplify the clause.
		 */
		set<CNode*, CompareQuery> sorted_queries;

		Connective* and_c = static_cast<Connective*>(proof_cnf);
		const set<CNode*> & ops = and_c->get_operands();
		set<CNode*>::const_iterator it = ops.begin();
		for(; it!= ops.end(); it++) {
			CNode* cur = *it;
			Constraint cur_c(cur);

			/*
			 * Assume the invariant so that we don't ask anything redundant
			 */

			cur_c.assume(invariants);
			if(cur_c.valid()) continue;

			/*
			 * Don't ask the user the same query that she/he has already
			 * said does not hold.
			 */
			set<Constraint> asked_queries;
			asked_queries.insert(non_invariants.begin(), non_invariants.end());
			asked_queries.insert(unknown_facts.begin(), unknown_facts.end());

			bool skip = false;
			set<Constraint>::iterator it = asked_queries.begin();
			for(; it!= asked_queries.end(); it++)
			{
				Constraint non_inv = *it;
				if(cur_c.implies(non_inv)) {
					skip = true;
					break;
				}
			}

			if(skip) continue;


			CNode* simplified_c = cur_c.get_cnodes().first;
			sorted_queries.insert(simplified_c);
		}

		// Return the first query
		CNode* first_q = *sorted_queries.begin();
		return clause_to_query(first_q, qt);

	}



}



void ReportClassifier::check_query(Constraint query)
{
	if(query.valid() || query.unsat() || queries_exhausted()) return;
	/*
	 * query should not be implied by invariants
	 */
	if(invariants.implies(query)){
		cout << "Invariants already imply query!" <<  endl;
		cout << "Invariants: " << invariants << endl;
		cout << "Query: " << query << endl;
		assert(false);
	}

	if(current_clause_assumptions.size()>0) return;

	/*
	 * query should not imply anything the user told us
	 * is *not* an invariant
	 */
	set<Constraint>::iterator it = non_invariants.begin();
	for(; it!= non_invariants.end(); it++)
	{
		Constraint not_inv = *it;
		if(query.implies(not_inv)){
			cout << "Query implies non-invariant!" << endl;
			cout << "Query: " << query << endl;
			cout << "Non-invariant: " << not_inv << endl;
			assert(false);
		}
	}
}

bool ReportClassifier::query_implied_by_non_invariant(CNode* n)
{

	set<Constraint> to_check;
	to_check.insert(non_invariants.begin(), non_invariants.end());

	Constraint c(n);
	c.replace_terms(fun_terms_to_vars);
	set<Constraint>::iterator it = to_check.begin();
	for(; it!= to_check.end(); it++ ) {
		Constraint cur = *it;
		if(cur.implies(c)) return true;
	}

	return false;
}

bool ReportClassifier::query_implied_by_unknown(CNode* n)
{

	set<Constraint> to_check;
	to_check.insert(unknown_facts.begin(), unknown_facts.end());

	Constraint c(n);
	c.replace_terms(fun_terms_to_vars);
	set<Constraint>::iterator it = to_check.begin();
	for(; it!= to_check.end(); it++ ) {
		Constraint cur = *it;
		if(cur.implies(c)) return true;
	}

	return false;
}


/*
 * Simplifies the query to be presented to the user with respect
 * to the assumptions made for the current clause
 */
CNode* ReportClassifier::simplify_query_wrt_clause_assumptions(CNode* c)
{
	if(current_clause_assumptions.size() == 0) return c;

	/*
	 * Simplify the current query by using the assumptions for the current
	 * clause (if current_clause_assumptions is non-empty)
	 */
	Constraint query_c(c);

	Constraint known = invariants;
/*	set<Constraint> background;

	for(set<CNode*, CompareLeafQuery>::iterator it =
			current_clause_assumptions.begin();
			it!=current_clause_assumptions.end(); it++)
	{

		CNode* cur = *it;
		Constraint cur_c(cur);
		known &= cur_c;
	}
	background.insert(known);


	Constraint msa_c = ((!known) | query_c);
	set<VariableTerm*> msa_vars;
	msa_c.msa(msa_vars, background, var_costs);

	{
		cout << "++++++++++MSA VARS+++++++++++" << endl;
		set<VariableTerm*>::iterator it = msa_vars.begin();
		for(; it!= msa_vars.end(); it++) {
			VariableTerm* vt = *it;
			cout << "\t " << vt->to_string() << endl;
		}
		cout << "+++++++++++++++++++++++++++++++++++" << endl;
	}

	Constraint new_query = get_min_var_sufficient_cond(msa_c, msa_vars); */
	Constraint new_query(c);
	new_query.assume(known);

	cout << "********** OLD C: " << c->to_string() << " *** NEW C: " <<
			new_query << endl;

	CNode* res = new_query.get_cnodes().first;
	return res;
}

UserQuery ReportClassifier::atom_to_query(CNode* c, query_type qt)
{
	/*
	 * sanity check
	 */
	if(!c->is_literal()){
		cerr << "Atom_to_query called with non literal: " << c->to_string() << endl;
		assert(false);
	}
	/*
	 * save current query so that if the user can prove it, we can
	 * add it as an invariant
	 */
	current_query = c;
	cur_qt = qt;

	c = simplify_query_wrt_clause_assumptions(c);


	Constraint query_c(c);
	check_query(query_c);

	cout << "&&&&&&&&&& QUERY ATOM " << c->to_string() << endl;
	cout << "&&&&&&&&& SIZE OF CLAUSE QUEUE " << current_clause_assumptions.size()
			<< endl;







	if(qt == WITNESS_QUERY)
			c = Connective::make_not(c);


	/*
	 * Convert variables used internally to their original meaning
	 */
	c = c->substitute(vars_to_fun_terms);

	set<Term*> terms;

	/*
	 * If we make assumptions about the execution, list those
	 */
	string prefix;
	if(current_clause_assumptions.size() > 0) {
		prefix = "In the same execution (where ";
		assert(qt == WITNESS_QUERY);

		set<CNode*>::iterator it = current_clause_assumptions.begin();
		for(int i=0;it != current_clause_assumptions.end(); it++, i++)
		{
			CNode* cur = *it;
			cur = cur->substitute(vars_to_fun_terms);
			cur->get_nested_terms(terms, true);
			prefix += cur->to_string();
			if(i!=current_clause_assumptions.size()-1) prefix += ", ";

		}
		prefix += ")\n";


	}



	/*
	 * Ask the actual query
	 */

	string query = prefix + c->to_string() + "\n";


	/*
	 * Print what unknowns correspond to
	 */
	set<pair<string, int> > source_info;
	c->get_nested_terms(terms, true);
	set<Term*>::iterator it = terms.begin();
	bool first = true;
	for(int i=0; it!= terms.end(); it++, i++) {
		Term* t = *it;
		AccessPath* ap = AccessPath::to_ap(t);
		if(ap->get_ap_type() == AP_UNMODELED) {

			UnmodeledValue* um = static_cast<UnmodeledValue*>(ap);
			const DisplayTag & dt = um->get_display_tag();
			string display_str = dt.get_description();
			if(first) {
				query += "where: ";
			}
			first = false;
			query += "\t " + ap->to_string() + " represents " +
					display_str + "\n";
			source_info.insert(make_pair(dt.get_file(), dt.get_line()));
		}
	}

	UserQuery q(query, source_info, qt);

	return q;
}

/*
 * CNode* c is in terms of the internal representation!
 */
UserQuery ReportClassifier::clause_to_query(CNode* c, query_type qt)
{


	if(c->is_literal()) {
		return atom_to_query(c, qt);
	}

	assert(c->get_type() == OR);

	qt = WITNESS_QUERY;


	Connective* or_c = static_cast<Connective*>(c);
	const set<CNode*>& ops = or_c->get_operands();
	set<CNode*>::const_iterator it = ops.begin();


	for(; it!= ops.end(); it++)
	{
		CNode* cur = *it;
		if(query_implied_by_unknown(cur)){
			current_clause_assumptions.insert(Connective::make_not(cur));
			continue;
		}
		else if(query_implied_by_non_invariant(cur)) {
			current_clause_assumptions.insert(Connective::make_not(cur));
			continue;
		}



		query_queue.insert(cur);
	}

	{
		cout << "======BUILT QUERY QUEUE======" << endl;
		set<CNode*>::iterator it = query_queue.begin();
		for(; it!= query_queue.end(); it++) {
			cout << "\t " << (*it)->to_string() << endl;
		}
		cout << "============================" << endl;
	}




	CNode* first = NULL;
	if(query_queue.size() == 0)
	{
		assert(current_clause_assumptions.size() > 0);
		CNode* cur = *current_clause_assumptions.rbegin();
		current_clause_assumptions.erase(cur);
		first = Connective::make_not(cur);

	}
	else
	{
		assert(query_queue.size()>0);
		first = *query_queue.begin();
		query_queue.erase(first);
	}
	UserQuery q = atom_to_query(first, qt);
	return q;


}



void ReportClassifier::set_query_result(QueryResult qr)
{

	Constraint inv_q = current_query;

	/*
	 * If this was a witness query, normalize it
	 * to an invariant query
	 */
	if(cur_qt == WITNESS_QUERY)
	{

		if(qr == YES_RESULT) {
			qr = NO_RESULT;
		}

		else if(qr == NO_RESULT)
		{
			qr = YES_RESULT;
		}

	}

	Constraint clause_assumptions = get_current_clause_assumptions();

	if(qr == NO_RESULT) {
		Constraint non_inv(inv_q);

		non_inv = ((!clause_assumptions) | non_inv);

		if(query_queue.size() > 0) {
			Constraint not_non_inv = !Constraint(inv_q);
			current_clause_assumptions.insert(not_non_inv.get_cnodes().first);
		}

		non_invariants.insert(non_inv);
		return;
	}

	if(qr == UNRESOLVED)
	{
		Constraint unknown(inv_q);

		unknown = ((!clause_assumptions) | unknown);

		if(query_queue.size() > 0) {
			Constraint not_unknown = !Constraint(inv_q);
			current_clause_assumptions.insert(not_unknown.get_cnodes().first);
		}

		unknown_facts.insert(unknown);
		return;
	}

	Constraint new_inv = Constraint(inv_q);
	new_inv = ((!clause_assumptions) | new_inv);

	learned_invariants &= new_inv;
	invariants &= new_inv;


	query_queue.clear();
	current_clause_assumptions.clear();


}



/*
 * Any function term that has a variable inside it should be treated
 * as a variable because, e.g., we want assignment to drf(x), not to x!!
 * On the other hand, we don't want to ask the user to show something
 * about drf(0)...
 * Also, to make this complete, we need to add Ackermanization axioms
 * as part of the invariants.
 */
bool ReportClassifier::replace_with_var(AccessPath* ap)
{
	switch(ap->get_ap_type())
	{
		case AP_ARITHMETIC:
		case AP_CONSTANT:
		case AP_VARIABLE:
		case AP_INDEX:
		case AP_COUNTER:
		case AP_STRING:
		case AP_NIL:
		case AP_PROGRAM_FUNCTION:
		case AP_TYPE_CONSTANT:
			return false;
		case AP_ARRAYREF:
		case AP_ALLOC:
		case AP_ADDRESS:
		case AP_DEREF:
		case AP_FIELD:
		case AP_FUNCTION:
		case AP_UNMODELED:
		case AP_ADT:
			return true;
		default:
			assert(false);
	}
}

ReportClassifier::~ReportClassifier()
{

}
