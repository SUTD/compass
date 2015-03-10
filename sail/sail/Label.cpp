/*
 * Label.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "Label.h"
#include "node.h"
#include<assert.h>


namespace sail {


Label::Label(string label_name, il::node* original) {
	this->label_name = label_name;

	// NOTE: original can be NULL for artificially
	// introduced labels.
	this->original = original;
	this->inst_id = SAIL_LABEL;
}


string Label::to_string() const
{
	return label_name;
}

string Label::to_string(bool pretty_print) const
{
	return label_name;
}

string Label::get_label_name()
{
	return label_name;
}


bool Label::is_removable()
{
	return false;
}


Label::~Label() {

}

}
