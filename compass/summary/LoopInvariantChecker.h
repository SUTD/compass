/*
 * LoopInvariantChecker.h
 *
 *  Created on: May 6, 2012
 *      Author: tdillig
 */

#ifndef LOOPINVARIANTCHECKER_H_
#define LOOPINVARIANTCHECKER_H_

class SummaryGraph;
#include "Constraint.h"
class AssertionTrace;
class MemoryGraph;

class LoopInvariantChecker {
private:
	SummaryGraph& sg;
	MemoryGraph* mg;
public:
	LoopInvariantChecker(SummaryGraph& sg, MemoryGraph* mg);
	~LoopInvariantChecker();
private:
	Constraint replace_vars_with_values(Constraint c);
	void discharge_assertions();
	void discharge_assertion(AssertionTrace* at);
	bool prove_invariant(Constraint to_show, Constraint bg,
			set<Constraint> consistency, Constraint loop_cont_c);
	bool query_precondition(Constraint precond);
	bool query_client(Constraint cond, Constraint assumption);

};

#endif /* LOOPINVARIANTCHECKER_H_ */
