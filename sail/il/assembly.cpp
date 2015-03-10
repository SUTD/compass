#include "assembly.h"
#include "expression.h"
#include <vector>

namespace il
{

assembly::assembly()
{

}

assembly::assembly(string asm_instruction,
		vector<expression*> & inputs, vector<expression*> &outputs,
		vector<string> & clobber_registers, location loc)
{
	this->asm_instruction = asm_instruction;
	this->inputs = inputs;
	this->outputs = outputs;
	this->clobber_registers = clobber_registers;
	this->loc = loc;
	this->node_type_id = ASSEMBLY;
}

assembly::~assembly()
{
}
void assembly::print()
{

}

string assembly::to_string() const
{
	return "asm(\"" + asm_instruction + "\"); ";
}

string assembly::get_instruction()
{
	return this->asm_instruction;
}
vector<expression*>& assembly::get_input_registers()
{
	return this->inputs;
}
vector<expression*>& assembly::get_output_registers()
{
	return this->outputs;
}
vector<string>& assembly::get_clobber_registers()
{
	return this->clobber_registers;
}

ostream& operator <<(ostream &os, const assembly &obj)
{
      os  << obj.to_string();
      return os;
}



}
