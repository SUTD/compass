/*
 * fixed_const_exp.cpp
 *
 *  Created on: Jun 27, 2008
 *      Author: isil
 */

#include "fixed_const_exp.h"
#include "type.h"

namespace il {

fixed_const_exp::fixed_const_exp()
{

}

fixed_const_exp::fixed_const_exp(type* t, location loc) {
	this->t = t;
	this->loc= loc;
	this->node_type_id = FIXED_CONST_EXP;

}

string fixed_const_exp::to_string() const
{
	return "<dummy fixed const expr>";
}

fixed_const_exp::~fixed_const_exp() {

}




}
