/*
 * Jump.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "Jump.h"
#include "Label.h"
#include "node.h"
#include <assert.h>

namespace sail {

Jump::Jump(Label* l, il::node* original) {
	this->l = l;
	/*This can be null */
	this->original = original;
	this->inst_id = JUMP;
}

string Jump::to_string() const
{
	return "goto " + l->to_string();
}

string Jump::to_string(bool pretty_print) const
{
	return "goto " + l->to_string(pretty_print);
}


bool Jump::is_removable()
{
	return false;
}

void Jump::set_label(Label* l)
{
	this->l=l;
}

Label* Jump::get_label()
{
	return this->l;
}


Jump::~Jump() {

}

}
