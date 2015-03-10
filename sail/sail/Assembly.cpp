/*
 * Assembly.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "Assembly.h"
#include "assembly.h"

namespace sail {

Assembly::Assembly(il::assembly* c_asm, int line) {
	this->original = c_asm;
	this->inst_id = SAIL_ASSEMBLY;
	this->line = line;

}

string Assembly::to_string() const
{
	return original->to_string();
}

string Assembly::to_string(bool pretty_print) const
{
	return original->to_string();
}

bool Assembly::is_removable()
{
	return false;
}

Assembly::~Assembly() {

}


}
