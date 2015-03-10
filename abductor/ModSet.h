/*
 * ModSet.h
 *
 *  Created on: Nov 26, 2012
 *      Author: tdillig
 */

#ifndef MODSET_H_
#define MODSET_H_

#include <map>
#include <set>

class VariableTerm;

namespace sail{
	class BasicBlock;
	class Block;
	class Variable;

}

using namespace std;

class ModSet {

private:
	map<sail::Block*, set<VariableTerm*> > mod_sets;


public:
	ModSet();
	/*
	 * Yields the set of variables modified in block b
	 */
	const set<VariableTerm*>& get_modset(sail::Block* b);

	/*
	 * Prints the set of variables modified in block b
	 */
	void print_modset(sail::Block* b);

	~ModSet();

private:
	void compute_modset(sail::Block* b, set<VariableTerm*>& mod_set);
	void compute_modset_basic_block(sail::BasicBlock* b,
			set<VariableTerm*>& mod_set);

};

#endif /* MODSET_H_ */
