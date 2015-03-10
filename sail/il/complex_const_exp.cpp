/*
 * complex_const_exp.cpp
 *
 *  Created on: Jun 27, 2008
 *      Author: isil
 */

#include "complex_const_exp.h"

namespace il {


complex_const_exp::complex_const_exp()
{

}

complex_const_exp::complex_const_exp(type* t, location loc) {
	this->t = t;
	this->loc = loc;
	this->node_type_id = COMPLEX_CONST_EXP;

}

string complex_const_exp::to_string() const
{
	return "dummy complex const expr";
}




complex_const_exp::~complex_const_exp() {

}

}
