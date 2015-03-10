/*
 * function_address_expression.cpp
 *
 *  Created on: Jun 28, 2008
 *      Author: isil
 */

#include "function_address_expression.h"
#include "function_declaration.h"

namespace il {

function_address_expression::function_address_expression()
{

}

function_address_expression::function_address_expression(
		string fn_name, type* fn_signature, type* t, location loc) {
	this->fn_name = fn_name;
	this->fn_signature = fn_signature;
	this->t = t;
	this->loc = loc;
	this->node_type_id = FUNCTION_ADDRESS_EXP;

}

string function_address_expression::to_string() const
{
	return "&" + fn_name;
}

string function_address_expression::get_function_name()
{
	return fn_name;
}
type* function_address_expression::get_function_signature()
{
	return fn_signature;
}



function_address_expression::~function_address_expression() {

}

}
