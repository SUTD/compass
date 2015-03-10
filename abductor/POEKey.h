/*
 * POEkey.h
 *
 *  Created on: Jun 18, 2012
 *      Author: boyang
 */



#ifndef POEKEY_H_
#define POEKEY_H_


#include "SuperBlock.h"
#include "CallingContext.h"

using namespace std;
using namespace sail;
using namespace il;

class POEKey;

class ComparePOEKey:public binary_function<POEKey*, POEKey*, bool> {

public:
	bool operator()(const POEKey* k1, const POEKey* k2) const;

};


class POEKey {

private:

	Block *block;

	CallingContext *callingcontext;

public:

	POEKey();

	POEKey(Block *, CallingContext*);

	bool is_superblock();
	bool is_basicblock();

	set<CfgEdge*> get_successors();
	set<CfgEdge*> get_predecessors();

	~POEKey();

	Block * getBlock();

	CallingContext * getcallingcontext();

	int get_context_size() const;

	int get_block_id() const;

	bool operator==(const POEKey &  other) const;

	virtual string to_string() const;

};

#endif /* POEKEY_H_ */
