/*
 * SuperBlock.h
 *
 *  Created on: Jul 11, 2008
 *      Author: isil
 */

#ifndef SUPERBLOCK_H_
#define SUPERBLOCK_H_
#include "Block.h"
#include "SummaryUnit.h"
#include "Instruction.h"
#include "Identifier.h"

namespace sail {

class Loop;
class BasicBlock;
class Function;

/**
 * \brief This is the structured representation of a loop with a single exit point.
 */

/**
 * Back edges are also replaced by LoopInvocation instructions.
 */
class SuperBlock:public Block, public SummaryUnit {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Block>(*this);
    	ar & boost::serialization::base_object<sail::SummaryUnit>(*this);
        ar & entry_block;
        ar & exit_block;
        ar & body;
        ar & exit_blocks;
        ar & parent_fn;
        ar & id;
    }
private:
	BasicBlock* entry_block;
	BasicBlock* exit_block;
	set<Block*> body;
	/*
	 * The blocks that are not part of the natural loop, i.e. that may not reach
	 * a loop invocation instruction. The only blocks not part of the natural
	 * loop are either blocks that assign exit variables or the exit block
	 * of the loop.
	 */
	set<BasicBlock*> exit_blocks;
	// The function this super block appears in.
	Function* parent_fn;


	Identifier id;



public:
	SuperBlock(Loop* l, Function* parent_fn);
	SuperBlock(){};
	virtual ~SuperBlock();
	virtual string to_string() const;
	virtual string to_dotty(string prelude, bool pretty_print = true);

	/**
	 *
	 * @return the basic block that serves as the entry block of this super block
	 */
	virtual BasicBlock* get_entry_block();

	/**
	 *
	 * @return the basic block that serves as the exit block of this super block
	 */
	virtual BasicBlock* get_exit_block();

	/**
	 * @return the basic block that serves as the exception block of this super block
	 */
	virtual BasicBlock* get_exception_block();
	set<Block*> & get_body();

	/**
	 * Is this block post-dominated by the exit block of the loop?
	 */
	bool is_exit_block(BasicBlock* b);

	/**
	 * Gives blocks that are sources of backedges (i.e., end in
	 * loop invocation instructions)
	 */
	void get_loop_invocation_blocks(set<Block*>& blocks);

	/**
	 *
	 * @return The function in which this super block appears.
	 */
	sail::Function* get_parent_function();


	/*
	 * Summary-unit related functions.
	 */
	virtual bool is_function();
	virtual bool is_superblock();

	virtual Identifier get_identifier();


	Instruction* get_first_instruction();
	Instruction* get_last_instruction();

private:
	void make_exitpoint(Loop *l);
};

}

#endif /* SUPERBLOCK_H_ */
