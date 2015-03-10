/*
 * InvariantChecker.h
 *
 *  Created on: Mar 11, 2010
 *      Author: isil
 */

#ifndef INVARIANTCHECKER_H_
#define INVARIANTCHECKER_H_


#define CHECK_INVARIANTS 1

#ifdef CHECK_INVARIANTS
#define check_invariants(loc, offset, mg) (InvariantChecker::_check_invariants(loc, offset, mg));
#define check_summary_invariants(sg) (InvariantChecker::_check_summary_invariants(sg));
#endif

#ifndef CHECK_INVARIANTS
#define check_invariants(loc, offset, mg)
#define check_summary_invariants(sg)
#endif


class AccessPath;
class MemoryGraph;
class MemoryLocation;
class SummaryGraph;
class Edge;

#include "Constraint.h"

#include <set>
using namespace std;

class InvariantChecker {
public:
	InvariantChecker();
	static void _check_invariants(MemoryLocation* loc, int offset,
			MemoryGraph* mg);
	static void _check_summary_invariants(SummaryGraph* sg);
	virtual ~InvariantChecker();
private:
	static void check_soundness_invariants(MemoryLocation* loc, int offset,
			MemoryGraph* mg);
	static bool check_sc_mutually_exclusive(AccessPath* ap, set<Edge*>* edges,
			Constraint bg);
	static bool check_target_restrictions(Edge* e);
	static void compute_transitive_edge_constraints(
			SummaryGraph* sg, map<AccessPath*, Constraint>& pred_constraints);
	static bool is_redundant_rtti_field(MemoryLocation* loc, int offset);

};






#endif /* INVARIANTCHECKER_H_ */
