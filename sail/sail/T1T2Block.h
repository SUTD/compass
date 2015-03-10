/*
 * T1T2Block.h
 *
 *  Created on: Jul 8, 2008
 *      Author: isil
 *  Equivalence class representative for blocks for doing T1 T2 transformations.
 */

#ifndef T1T2BLOCK_H_
#define T1T2BLOCK_H_
#include <set>
#include <queue>
#include <vector>
#include <map>
using namespace std;

namespace sail {

class T1T2Block;
class BasicBlock;
class CfgEdge;
class Symbol;
class Compare:public binary_function<T1T2Block*, T1T2Block*, bool> {

public:
	bool operator()(const T1T2Block* b1, const T1T2Block* b2) const;
};

T1T2Block* make_T1T2Block(BasicBlock* b);
void clear_map_t1t2();

class T1T2Block {

public:
	int pred_count;
	set<T1T2Block*> succs;
	set<T1T2Block*> preds;
	set<BasicBlock*> representees;
	BasicBlock* representative_block;
	//string to_string() const;
	T1T2Block(BasicBlock *b);
	T1T2Block(const T1T2Block& other, T1T2Block* pred,
			BasicBlock* rep_block, set<BasicBlock*>& representatives);
	virtual ~T1T2Block();

	void split_node(set<T1T2Block*, Compare>& heap,
					set<BasicBlock*>& blocks, BasicBlock* & old_exit,
					BasicBlock* & old_exception);

	/**
	 * Makes a deep copy of all memebers of eq class and reajusts pointer.
	 * Does not adjust rep.preds.
	 */
	BasicBlock* copy_equivalence_class(BasicBlock* rep,
			set<BasicBlock*> & rep_class, set<BasicBlock*>& blocks,
			set<BasicBlock*> & exit_blocks, set<BasicBlock*> & exception_blocks,
			BasicBlock* & old_exit, BasicBlock* & old_exception);

	set<CfgEdge*>* get_edges(set<BasicBlock*> & rep_class);
	BasicBlock* get_block(BasicBlock*old, set<BasicBlock*> & rep_class,
			set<BasicBlock*>& blocks, map<BasicBlock*, BasicBlock*> &old_to_new,
			set<BasicBlock*> & exit_blocks, set<BasicBlock*> & exception_blocks,
			BasicBlock* & old_exit, BasicBlock* & old_exception);

	void merge(T1T2Block *other,
			set<T1T2Block*, Compare> & heap);

	void clear_original_class(set<BasicBlock*>& blocks);
	void connect_blocks(BasicBlock* b1, BasicBlock*b2, Symbol* cond);
};


}


#endif /* T1T2BLOCK_H_ */
