/*
 * DisjointVarManager.h
 *
 *  Created on: Mar 21, 2009
 *      Author: tdillig
 */

#ifndef DISJOINTVARMANAGER_H_
#define DISJOINTVARMANAGER_H_
#include <vector>
#include "Constraint.h"
using namespace std;
class Variable;
class MemoryGraph;
class SummaryGraph;

class DisjointVarManager {
private:
	int count;
	string id;
public:
	DisjointVarManager();
	void set_id(int id);
	Variable* get_disjointness_constraints(int num, vector<Constraint> & res);
	static void eliminate_disjoint_vars(Constraint & c);
	bool contains_disjointness_vars(Constraint c);
	void clear();


	/*
	 * Replaces disjointness constraints with what they actually mean, as
	 * given by the map in the SummaryGraph. This function is currently
	 * only used for the purposes of explaining/diagnosing errors.
	 */
	static Constraint replace_disjointness_constraints(Constraint c,
			SummaryGraph* sg);

private:
	static bool is_disjoint_var(Term* t);

};

#endif /* DISJOINTVARMANAGER_H_ */

