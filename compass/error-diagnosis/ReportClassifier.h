/*
 * ReportClassifier.h
 *
 *  Created on: Sep 15, 2011
 *      Author: isil
 */

#ifndef REPORTCLASSIFIER_H_
#define REPORTCLASSIFIER_H_
#include "Constraint.h"
#include "UserQuery.h"
#include <map>
using namespace std;

class ErrorTrace;
class VariableTerm;
class SummaryGraph;

enum QueryResult
{
	YES_RESULT,
	NO_RESULT,
	UNRESOLVED
};

class CompareQuery:public binary_function<CNode*, CNode*, bool> {

public:
	/*
	 * As a heuristic, we want to favor queries that contain fewer disjuncts.
	 */
   bool operator()(const CNode* c1, const CNode* c2) const;
};

class CompareLeafQuery:public binary_function<CNode*, CNode*, bool> {

public:
	/*
	 * As a heuristic, among queries that correspond to leaves, we want to
	 * favor those that contain the fewest variables.
	 *
	 */
   bool operator()(const CNode* c1, const CNode* c2) const;
};



class ReportClassifier {
	friend class resolve_query;
private:
	SummaryGraph* sg;
	ErrorTrace* report;
	Constraint invariants;

	/*
	 * Succes_cond, failure_cond, stmt_guard, non_invariants, unknown_facts
	 * all in internal representation (with only variable terms)
	 */
	Constraint success_cond;
	Constraint failure_cond;
	Constraint stmt_guard;
	map<VariableTerm*, int> var_costs;

	/*
	 * "Invariants" user told us do not hold.
	 * Example: If we ask user to prove x>0 and he/she says "no",
	 * non-invariants would contain x>0
	 */
	set<Constraint> non_invariants;

	/*
	 * Queries the user cannot decide; they may or may not hold.
	 */
	set<Constraint> unknown_facts;


	/*
	 * Invariants we learned from the user
	 */
	Constraint learned_invariants;

	/*
	 * For the purposes of finding MSA's, we want to treat function terms
	 * such as drf(x) as a variable d_x because we are interested in an
	 * assignment to drf(x), not to x!
	 */
	map<Term*, Term*> fun_terms_to_vars;
	map<Term*, Term*> vars_to_fun_terms;


	CNode* current_query;

	/*
	 * Query queue represents the disjunction of facts
	 * that need to be proven for the current query to be shown.
	 */
	set<CNode*, CompareLeafQuery> query_queue;

	/*
	 * We can assume elements of this set when we process the
	 * next query
	 */
	set<CNode*, CompareLeafQuery> current_clause_assumptions;


	/*
	 * Type of the query: invariant query or validate query
	 */
	query_type cur_qt;








public:
	ReportClassifier(ErrorTrace* report, Constraint inv, SummaryGraph* sg);
	~ReportClassifier();
	UserQuery get_query();
	void set_query_result(QueryResult qr);
	bool error_discharged();
	bool error_validated();
	bool queries_exhausted();


private:
	void init_var_mappings();
	void init_costs();
	bool replace_with_var(AccessPath* ap);
	UserQuery decompose_proof(CNode* proof_c, query_type qt);

	/*
	 * Generates a query from a leaf
	 */
	UserQuery atom_to_query(CNode* c, query_type qt);

	/*
	 * Generates the first query for a disjunct and populates
	 * query queue
	 */
	UserQuery clause_to_query(CNode* c, query_type qt);



	/*
	 * Performs a sanity check on the query to make sure we don't ask
	 * things we have already asked or that is inconsistent.
	 * Used for debugging
	 */
	void check_query(Constraint query_c);

	/*
	 * Computes the set of assumptions the MSA must obey
	 * so that we don't repeat the same queries over and over again
	 */
	void compute_msa_assumptions(set<Constraint>& background);

	bool query_implied_by_non_invariant(CNode* c);
	bool query_implied_by_unknown(CNode* c);

	/*
	 * Returns the conjunction of current clause assumptions
	 */
	Constraint get_current_clause_assumptions();



	/*
	 * Computes a non-trivial (i.e., satisfiable) sufficient condition for
	 * c containing the minimum number of variables. This is done
	 * by universally quantifying variables that are not
	 * part of the MSA of the constraint c.
	 */
	static Constraint get_min_var_sufficient_cond(Constraint c,
			set<VariableTerm*>& msa);

	/*
	 * Simplifies the query to be presented to the user with respect
	 * to the assumptions made for the current clause
	 */
	CNode* simplify_query_wrt_clause_assumptions(CNode* c);












};

#endif /* REPORTCLASSIFIER_H_ */
