/*
 * Loop.h
 *
 *  Created on: Jul 11, 2008
 *      Author: isil
 */

#ifndef LOOP_H_
#define LOOP_H_

#include<set>
#include <vector>
#include <string>
#include <vector>
using namespace std;

namespace il
{
class node;
}

namespace sail{

class BasicBlock;
class CfgEdge;
class Loop;
class Instruction;

class Loop {

private:
	BasicBlock* header;
	BasicBlock* exit_block;
	set<CfgEdge*> backedges;
	set<BasicBlock*> body;
	int loop_id;
	set<Loop*> parent_loops;

	/*
	 * The blocks that are not part of the natural loop, i.e. that may reach
	 * a loop invocation instruction. The only blocks not part of the natural
	 * loop are either blocks that assign exit variables or the exit block
	 * of the loop.
	 */
	set<BasicBlock*> exit_blocks;


public:
	Loop(CfgEdge* backedge);
	void add_backedge(CfgEdge* backedge);
	BasicBlock* get_header() const;
	BasicBlock* get_exit_block() const;
	set<BasicBlock*>& get_body();
	set<CfgEdge*>& get_backedges();
	set<BasicBlock*>& get_exit_blocks();
	virtual ~Loop();
	string to_string() const;
	void get_edges(set<CfgEdge*> & edges);
	void make_unique_exit(set<BasicBlock*> & basic_blocks);
	void add_parent_loop(Loop *p);
	void insert_recursive_calls(set<BasicBlock*> & basic_blocks);



private:
	void construct_body(CfgEdge* backedge);
	void update_parents(BasicBlock *b, bool add_to_self);

};

void reset_loop_count();

class LoopCompare:public binary_function<Loop*, Loop*, bool> {

public:
	bool operator()(const Loop* b1, const Loop* b2) const;
};

}



#endif /* LOOP_H_ */
