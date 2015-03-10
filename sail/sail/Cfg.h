/*
 * Cfg.h
 *
 *  Created on: Jul 6, 2008
 *      Author: isil
 */

#ifndef CFG_H_
#define CFG_H_

#include <string>
#include <vector>
#include <map>
#include <set>
#include <deque>
#include "Loop.h"
#include "Identifier.h"
using namespace std;

namespace sail {
class BasicBlock;
class Instruction;
class Label;
class CfgEdge;
class Symbol;
class Loop;
class Block;
class SuperBlock;
class Function;



/*
 * Force deterministic ordering of CfgEdge sets in reverse postorder
 * numbering so that block id's are assigned deterministically for regressions.
 */
class CompareEdge:public binary_function<CfgEdge*, CfgEdge*, bool> {

public:
	bool operator()(const CfgEdge* e1, const CfgEdge* e2) const;
};

class CompareBlock:public binary_function<BasicBlock*, BasicBlock*, bool> {

public:
	bool operator()(const Block* b1, const Block* b2) const;
};

/**
 * \brief Representation of a control flow graph.
 */
/**
 * Each CFG maintains the invariants of having a single entry and
 * single exit block. In addition, there is an additional
 * exception block that is target of any exit function (function that aborts
 * execution) or exception.
 */
class Cfg {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & entry_block;
        ar & exit_block;
        ar & exception_block;
        ar & basic_blocks;
        ar & blocks;
        ar & super_blocks_ordered;
        ar & id_to_superblock;
    }

private:
	BasicBlock* entry_block;
	BasicBlock* exit_block;
	//exceptional exit block due to exit, exceptions, fatal etc.
	BasicBlock* exception_block;
	set<BasicBlock*> basic_blocks;
	set<Block*> blocks;

	// basic blocks embedded with in a loop/superblock
	set<BasicBlock*> embedded_basic_blocks;
	set<CfgEdge*> edges;
	set<Loop*, LoopCompare> loops;
	map<BasicBlock*, Loop*> loopheaders;
	map<BasicBlock*, SuperBlock*> headers_to_superblocks;
	map<Label*, BasicBlock*> resolved_blocks;
	map<Label*, set<CfgEdge*> > unresolved_blocks;

	/*
	 * A mapping from the superblock id (su->get_id()) to its
	 * SuperBlock.
	 */
	map<string, SuperBlock*> id_to_superblock;

	/*
	 * If loop X is nested inside loop Y, X appears before Y
	 * in this vector.
	 */
	vector<SuperBlock*> super_blocks_ordered;

	Function* f;

	bool exit_fn_as_exception;


public:
	/**
	 * @param f The function to generate the CFG from.
	 * @param exit_fn_modifies_control If this is set to true,
	 * functions that abort execution modify control flow. e.g.,
	 * if there is a call to exit(), then the successor of this
	 * basic block is the exception block.
	 */
	Cfg(Function* f, bool exit_fn_modifies_control = false);
	Cfg();
	virtual ~Cfg();

	/**
	 * For drawing a dotty representation of the CFG.
	 * @param pretty_print If pretty_print is false, then the low level
	 * instructions are printed, otherwise the higher level representation
	 * is used.
	 * @return a string that can be fed to the dot program.
	 */
	string to_dotty(bool pretty_print = true);

	/**
	 * @return all blocks in the cfg (both basic and superblocks)
	 */
	set<Block*>& get_blocks();

	/**
	 *
	 * @return set of basic blocks in the CFG.
	 */
	set<BasicBlock*>& get_basic_blocks();

	/**
	 * @return Entry block of the CFG.
	 */
	BasicBlock* get_entry_block();

	/**
	 *
	 * @return Exit block of the CFG.
	 */
	BasicBlock* get_exit_block();

	/**
	 *
	 * @return Exception block of the CFG.
	 */
	BasicBlock* get_exception_block();

	/**
	 *
	 * @param id of a SuperBlock
	 * @return SuperBlock with this id, NULL if it does not exist.
	 */
	SuperBlock* get_superblock(const Identifier & id);

	/**
	 * If loop X is nested inside loop Y, X appears before Y
	 * in the returned vector.
	 */
	const vector<SuperBlock*>& get_ordered_superblocks();

	bool is_header_of_superblock(BasicBlock* b);
	SuperBlock* get_superblock_of_entry_block(BasicBlock* b);


private:
	void build_cfg(vector<Instruction*>* body);
	int fill_basic_block(BasicBlock* cur, vector<Instruction*>* body,
				int start_index);
	bool is_control_instruction(Instruction* inst);
	BasicBlock* get_new_block(Instruction* inst);
	CfgEdge* connect_blocks(BasicBlock* pred, BasicBlock* succ, Symbol* cond);
	void add_unresolved_edge(Label* l, CfgEdge* e);
	void process_next_block(BasicBlock*& cur_block, int& cur_index,
			vector<Instruction*>* function_body);
	void process_jump_label(BasicBlock* cur_block, Label* l, Symbol* cond);
	void assign_block_ids();
	void print_cfg();

	string blocks_to_dotty();
	bool check_cycle(bool edge);
	void optimize_cfg();
	void remove_unreachable_blocks();
	void remove_empty_blocks();
	void merge_redundant_blocks();
	void merge_blocks(BasicBlock* b1, BasicBlock* b2, CfgEdge* e);
	void make_reducible();
	void traverse_postorder(BasicBlock* cur, int& counter,
				set<BasicBlock*>& visited);
	void compute_dominators();
	void compute_post_dominators();
	void check_cfg();
	void identify_loops();
	void find_backedges();
	void get_edges(set<CfgEdge*>& edges);
	void finalize_cfg();
	void make_superblocks();

	void type_inference();

};

}

#endif /* CFG_H_ */
