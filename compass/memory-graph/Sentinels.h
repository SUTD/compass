/*
 * Sentinels.h
 *
 *  Created on: Oct 26, 2009
 *      Author: tdillig
 */

#ifndef SENTINELS_H_
#define SENTINELS_H_

class AccessPath;
class MemoryGraph;
#include "Constraint.h"

class Sentinels {
public:
	Sentinels();
	bool has_sentinel(AccessPath* ap);
	long int get_sentinel(AccessPath* ap);
	Constraint get_eq_sentinel_constraint(AccessPath* ap, bool is_loop);

	/*
	 * Adds the assumption that array[length] = sentinel &
	 * length(array) < array.size
	 */
	void add_sentinel_assumption(AccessPath* ap, MemoryGraph* mg);

	~Sentinels();
};

#endif /* SENTINELS_H_ */
