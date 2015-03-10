/*
 * vector_const_exp.cpp
 *
 *  Created on: Jun 27, 2008
 *      Author: isil
 */

#include "vector_const_exp.h"

namespace il {

vector_const_exp::vector_const_exp()
{

}

vector_const_exp::vector_const_exp(type* t, location loc) {
	this->t = t;
	this->loc = loc;
	this->node_type_id = VECTOR_CONST_EXP;

}

string vector_const_exp::to_string() const
{
	return "dummy vector const expr";
}

vector_const_exp::~vector_const_exp() {

}


}
