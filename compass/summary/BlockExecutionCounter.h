/*
 * BlockExecutionCounter.h
 *
 *  Created on: Sep 26, 2009
 *      Author: tdillig
 */

#ifndef BLOCKEXECUTIONCOUNTER_H_
#define BLOCKEXECUTIONCOUNTER_H_

#include <map>
using namespace std;

class Variable;

#include "Constraint.h"

namespace sail {
	class SuperBlock;
	class Block;
};

/*
 * This class decorates every block in loops with variables indicating
 * how many times this block will execute. For example, in the code snippet,
 *
 * Block 1
 * if(x) {
 * 	break;
 * }
 * Block 2
 *
 *
 * Block 1 and Block 2 may execute different numbers of times, therefore
 * they need to be annotated with different iteration counters.
 *
 */
class BlockExecutionCounter {
private:
	map<int, int> block_to_counter;
	int counter;
	Constraint counter_relations;
	sail::SuperBlock* su;



public:
	BlockExecutionCounter(sail::SuperBlock* su);
	virtual ~BlockExecutionCounter();
	int get_iteration_counter(int block_id);
	Constraint get_counter_relations();
	int get_max_counter();

private:
	bool has_exit_children(sail::Block* b);
	bool is_exit_block(sail::Block* b);

};

#endif /* BLOCKEXECUTIONCOUNTER_H_ */
