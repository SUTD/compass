/*
 * BasicBlock.h
 *
 *  Created on: Jul 6, 2008
 *      Author: isil
 */

#ifndef BASICBLOCK_H_
#define BASICBLOCK_H_

#define INVALID_BLOCK_ID -1

#include <vector>
#include <string>
#include <set>
#include "Block.h"
using namespace std;

namespace sail {

class Instruction;
class CfgEdge;
class Symbol;

/**
 * \brief Represents a basic block in the CFG.
 */
class BasicBlock: public Block {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Block>(*this);
        ar & statements;
        ar & dominators;
        ar & post_dominators;
    }

private:
	vector<Instruction*> statements;
	set<BasicBlock*> dominators;
	set<BasicBlock*> post_dominators;


public:
	BasicBlock();
	BasicBlock(const BasicBlock& other);
	void add_statement(Instruction* inst);

	//add an inst right before the inst_pos - Boyang 12/01/12
	void add_statement_before(Instruction* inst, Instruction* inst_pos);
	void add_statements(vector<Instruction*>& stmts);
	void set_dominators(set<BasicBlock*>& doms);
	void set_post_dominators(set<BasicBlock*>& pdoms);
	set<BasicBlock*>& get_dominators();
	set<BasicBlock*>& get_post_dominators();
	bool is_empty_block();

	/**
	 * @return the sequence of low-level instructions used in this
	 * basic block.
	 */
	vector<Instruction*>& get_statements();

	string to_string() const;
	string to_string(bool pretty_print, bool print_block_id = true) const;
	virtual string to_dotty(string prelude, bool pretty_print = true);
	virtual bool is_basicblock();
	virtual ~BasicBlock();

	//added by haiyan 8/5/2013
	void set_statements_unreachable();
};

}

#endif /* BASICBLOCK_H_ */
