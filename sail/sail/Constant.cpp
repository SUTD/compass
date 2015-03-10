/*
 * Constant.cpp
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 */

#include "Constant.h"
#include "const_expression.h"
#include "integer_const_exp.h"
#include "string_const_exp.h"
#include <assert.h>
#include <sstream>
#include "node.h"
#include "util.h"

namespace sail {

Constant::Constant(il::const_expression* constant_exp)
{
	this->constant_exp = constant_exp;
	this->constant = 0;
	this->_is_signed = false;
	this->size = 0;
}

Constant::Constant(long constant, bool _is_signed, short size)
{
	this->constant_exp = NULL;
	this->constant = constant;
	this->_is_signed = _is_signed;
	this->size = size;

}

Constant::Constant()
{
	this->constant_exp = NULL;
	this->constant = 0;
	this->_is_signed = false;
	this->size = 0;

}

bool Constant::is_integer()
{
	if(constant_exp == NULL) return true;
	if(constant_exp->node_type_id == INTEGER_CONST_EXP)
		return true;
	return false;
}

bool Constant::is_string_constant()
{
	if(constant_exp == NULL) return false;
	return (constant_exp->node_type_id == STRING_CONST_EXP);
}

long int Constant::get_integer()
{
	if(constant_exp == NULL) return this->constant;
	assert(constant_exp->node_type_id == INTEGER_CONST_EXP);
	il::integer_const_exp* ice = (il::integer_const_exp*) constant_exp;
	return ice->get_integer();
}

string Constant::get_string_constant()
{

	assert(constant_exp!= NULL);
	assert(constant_exp->node_type_id == STRING_CONST_EXP);
	il::string_const_exp* se = (il::string_const_exp*) constant_exp;
	return se->get_string();
}

bool Constant::is_signed()
{
	if(constant_exp == NULL) return this->_is_signed;
	assert(constant_exp->node_type_id == INTEGER_CONST_EXP);
	il::integer_const_exp* ice = (il::integer_const_exp*) constant_exp;
	return ice->isSigned();
}

int Constant::get_size()
{
	if(constant_exp == NULL) return this->size;
	return constant_exp->get_type()->get_size();
}

string Constant::to_string() const
{
	if(constant_exp == NULL) {
		return int_to_string(constant);
	}
	return constant_exp->to_string();
}

string Constant::to_string(bool printOriginal) const
{
	if(constant_exp == NULL) return int_to_string(constant);
	return constant_exp->to_string();
}

/*
 * This can return NULL!
 */
il::node* Constant::get_original_node()
{
	return constant_exp;
}
bool Constant::is_constant()
{
	return true;
}
bool Constant::is_variable()
{
	return false;
}

il::type* Constant::get_type()
{
	return il::get_integer_type();
}


Constant::~Constant() {

}

}
