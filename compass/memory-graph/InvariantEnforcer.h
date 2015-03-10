/*
 * InvariantEnforcer.h
 *
 *  Created on: Sep 20, 2009
 *      Author: tdillig
 */

#ifndef INVARIANTENFORCER_H_
#define INVARIANTENFORCER_H_

#include <set>
using namespace std;

#include "Constraint.h"

class AccessPath;
class MemoryLocation;
class Edge;
class MemoryGraph;
class Instantiator;



/*
 * The memory graph maintains the following invariant:
 * Given a source S, and a set of outgoing edges
 * from S:
 * 1) The necessary conditions of the edge constraints
 * should be mutually exclusive.
 * 2) The sufficient conditions should disjoint to true
 *
 * When summaries are applied, we might lose this invariant
 * due to imprecision, e.g., we are unable to reason about a loop
 * fully precisely, environment choice etc.
 *
 * If these invariants are lost, the InvariantEnforcer
 * recovers these invariants by artificially introducing
 * additional constraints.
 */
class InvariantEnforcer {

private:
	int counter;

public:
	InvariantEnforcer();
	/*
	 * Enforces the existence and uniqueness of outgoing edges from
	 * this location.
	 */
	void enforce_existence_and_uniqueness(MemoryLocation* source_loc,
			int source_offset, Constraint stmt_guard, MemoryGraph* mg);


	/*
	 * map<Constraint, Constraint> & uniqueness_to_summary
	 * only used for error diagnosis (if USER_INTERACTION_ENABLED is true.)
	 * It maps existence/uniqueness constraints of the form d=c to
	 * the original constraint in the summary.
	 *
	 */
	void enforce_existence_and_uniqueness(Instantiator& inst,
			AccessPath* summary_source, Constraint sum_update_c,
			set<Edge*>& sum_edges,
			map<Constraint, Constraint> & uniqueness_to_summary);




	virtual ~InvariantEnforcer();

	void clear();

private:

	Constraint get_disjointness_constraint(AccessPath* disjointness_term,
			int cur_edge_number, int num_edges);
	Constraint get_target_constraint(Constraint& edge_c, AccessPath* source_ap,
			AccessPath* target_ap, Constraint & target_eq_only);





};

#endif /* INVARIANTENFORCER_H_ */
