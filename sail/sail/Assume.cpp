/*
 * Assume.cpp
 *
 *  Created on: Apr 10, 2009
 *      Author: tdillig
 */

#include "Assume.h"
#include "Symbol.h"
#include "node.h"
#include "type.h"
#include "expression.h"
namespace sail {

Assume::Assume()
{
	original = NULL;
	inst_id = ASSUME;
	line = -1;
}


Assume::Assume(Symbol*  arg, il::node * original, int line)
{
	this->original = original;
	inst_id = ASSUME;
	this->assume_predicate = arg;
	this->line = line;
}

Assume::~Assume() {

}

Symbol* Assume::get_predicate()
{
	return assume_predicate;
}


string Assume::to_string() const
{
	return "assume(" + assume_predicate->to_string() + ")";
}
string Assume::to_string(bool pretty_print) const
{
	return "assume(" + assume_predicate->to_string(pretty_print) + ")";
}
il::node* Assume::get_original_node()
{
	return original;
}

bool Assume::is_save_instruction()
{
	return false;
}


bool Assume::is_synthetic()
{
	return true;
}


bool Assume::is_removable()
{
	return false;
}



}
