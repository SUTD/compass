/*
 * LoopInvocationInstruction.h
 *
 *  Created on: Jul 12, 2008
 *      Author: isil
 *  Instantiation point for the current loop summary. This corresponds to a
 *  backedge in the control flow graph.
 */

#ifndef LOOPINVOCATIONINSTRUCTION_H_
#define LOOPINVOCATIONINSTRUCTION_H_

#include "Instruction.h"


namespace sail {
class BasicBlock;

/**
 * \brief In SAIL loops can be treated as tail-recursive functions; this
 * instruction represents a tail-recursive invocation of the loop body
 * and corresponds to a back-edge.
 */
class LoopInvocationInstruction: public sail::Instruction {
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    	ar & boost::serialization::base_object<sail::Instruction>(*this);
        ar & header;
    }
private:
	BasicBlock* header;
public:
	LoopInvocationInstruction(BasicBlock* header);
	LoopInvocationInstruction(){};
	virtual ~LoopInvocationInstruction();
	virtual string to_string() const;
	virtual string to_string(bool pretty_print) const;
	virtual bool is_removable();
	BasicBlock* get_header();
};

}

#endif /* LOOPINVOCATIONINSTRUCTION_H_ */
