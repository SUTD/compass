/*
 * LoopTree.h
 *
 *  Created on: Jul 25, 2012
 *      Author: boyang
 */

#ifndef LOOPTREE_H_
#define LOOPTREE_H_

#include <map>
#include <set>
#include <iostream>
#include <list>

#include "POEKey.h"
using namespace std;


class LoopTree {

private:


	map<POEKey*,set<POEKey*> >  tm;


public:

	LoopTree();

	~LoopTree();

	void insert(POEKey*, POEKey*);

	/*
	 * get inner loop list of the loop
	 */
	void getInnerLoopList(POEKey* outer, list<POEKey*>& list_inner);


	void getAllLoopList(list<POEKey*>& list_int);

	string to_string() const;

};

#endif /* LOOPTREE_H_ */
