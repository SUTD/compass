/*
 * LoopAbstractor.h
 *
 *  Created on: Apr 15, 2012
 *      Author: tdillig
 */

#ifndef LOOPABSTRACTOR_H_
#define LOOPABSTRACTOR_H_
#include <map>
#include "Constraint.h"
using namespace std;
class AccessPath;

class SummaryGraph;
class Term;
class AssertionTrace;

class LoopAbstractor {
private:
	map<AccessPath*, AccessPath*> ap_replacements;
	map<Term*, Term*> term_replacements;
private:
	SummaryGraph & sg;
	void assume_loop_continuation_cond();
	void add_after_loop_targets();
	void conjoin_term_cond();

public:
	LoopAbstractor(SummaryGraph& sg);
	~LoopAbstractor();
};

#endif /* LOOPABSTRACTOR_H_ */
