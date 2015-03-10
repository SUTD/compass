/*
 * StaticAssert.cpp
 *
 *  Created on: Oct 10, 2008
 *      Author: tdillig
 */

#include "StaticAssert.h"
#include "Symbol.h"
#include "node.h"
#include "type.h"
#include "expression.h"
namespace sail{


StaticAssert::StaticAssert() {
	original = NULL;
	inst_id = STATIC_ASSERT;
	line = -1;

}

StaticAssert::StaticAssert(Symbol*  arg, il::node * original, int line)
{
	this->original = original;
	inst_id = STATIC_ASSERT;
	this->assert_predicate = arg;
	this->line = line;
}

StaticAssert::~StaticAssert() {

}

Symbol* StaticAssert::get_predicate()
{
	return assert_predicate;
}


void StaticAssert::set_predicate(Symbol* s)
{
	this->assert_predicate = s;
}



string StaticAssert::to_string() const
{
	return "static_assert(" + assert_predicate->to_string() + ")";
}
string StaticAssert::to_string(bool pretty_print) const
{
	return "static_assert(" + assert_predicate->to_string(pretty_print) + ")";
}
il::node* StaticAssert::get_original_node()
{
	return original;
}

bool StaticAssert::is_save_instruction()
{
	return false;
}

/*
 * is_synthetic() returns true if this instruction does not
 * have a valid mapping in the original AST.
 * E.g. is_synthetic is true for label statements artficially introduced
 * by SAIL.
 */
bool StaticAssert::is_synthetic()
{
	return true;
}


bool StaticAssert::is_removable()
{
	return false;
}




}
