/*
 * LoopInvocationInstruction.cpp
 *
 *  Created on: Jul 12, 2008
 *      Author: isil
 */

#include "LoopInvocationInstruction.h"
#include "util.h"
#include "node.h"
#include "BasicBlock.h"
namespace sail {

LoopInvocationInstruction::LoopInvocationInstruction(BasicBlock *header) {
	this->inst_id = LOOP_INVOCATION;
	this->original = NULL;
	this->header = header;

}
string LoopInvocationInstruction::to_string() const
{
	return "<LoopInvocation>" + int_to_string(header->get_block_id());
}

string LoopInvocationInstruction::to_string(bool pretty_print) const
{
	return to_string();
}

bool LoopInvocationInstruction::is_removable()
{
	return false;
}



LoopInvocationInstruction::~LoopInvocationInstruction() {

}

BasicBlock* LoopInvocationInstruction::get_header(){
	return this->header;
}

}
