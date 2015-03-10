#include "noop_instruction.h"

namespace il
{

noop_instruction::noop_instruction()
{
	this->node_type_id = NOOP_INSTRUCTION;
}



noop_instruction::~noop_instruction()
{
}

}
