#include "real_const_exp.h"

namespace il
{

real_const_exp::real_const_exp(type* t, location loc)
{
	this->t = t;
	this->loc = loc;
	this->node_type_id = REAL_CONST_EXP;
}



real_const_exp::~real_const_exp()
{
}

string real_const_exp::to_string() const
{
	return "dummy real_const_exp";
}

}
