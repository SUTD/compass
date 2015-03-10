/*
 * InstanceOf.h
 *
 *  Created on: July 28, 2012
 *      Author: tdillig
 *
 */

#include "InstanceOf.h"
#include "Variable.h"
#include "Symbol.h"
#include "node.h"
#include <assert.h>

namespace sail {

InstanceOf::InstanceOf(Variable* v, Symbol* s, il::type* t,
		il::node* original, int line) {
	this->v = v;
	this->s = s;
	this->t = t;
	this->original = original;
	this->inst_id = INSTANCEOF;
	this->line = line;
	assert(v != NULL);
	assert(s != NULL);
	assert(t != NULL);

}

string InstanceOf::to_string() const
{
	return to_string(true);
}

string InstanceOf::to_string(bool pretty_print) const
{
	string res = v->to_string(pretty_print);
	res += (" = instanceof(" + s->to_string(pretty_print));
	res += ", " + t->to_string() + ")";
	return res;
}

Variable* InstanceOf::get_lhs()
{
	return v;
}

void InstanceOf::set_lhs(Variable* v)
{
	this->v = v;
}

Symbol* InstanceOf::get_rhs()
{
	return s;
}

void InstanceOf::set_rhs(Symbol* s)
{
	this->s = s;
}

il::type* InstanceOf::get_instance_type()
{
	return t;
}

bool InstanceOf::is_removable()
{
	return false;
}


InstanceOf::~InstanceOf() {

}

}
