/*
 * block_expression.cpp
 *
 *  Created on: Jun 28, 2008
 *      Author: isil
 */

#include "block_expression.h"
#include "block.h"
#include "variable_declaration.h"

namespace il {

block_expression::block_expression()
{

}

block_expression::block_expression(block* b, variable_declaration* var_decl,
		type* t, location loc) {
	this->b = b;
	this->var_decl = var_decl;
	this->t =t;
	this->loc = loc;
	this->node_type_id = BLOCK_EXP;

}

string block_expression::to_string() const
{
	string res = "(";
	res += b->to_string();
	res += ")";
	return res;
}

block* block_expression::get_block()
{
	return b;
}
variable_declaration* block_expression::get_vardecl()
{
	return var_decl;
}

block_expression::~block_expression() {

}




}
