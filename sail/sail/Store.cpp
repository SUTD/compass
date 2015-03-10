/*
 * Store.cpp
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 */

#include "Store.h"
#include "Variable.h"
#include "Symbol.h"
#include "node.h"
#include <assert.h>
#include "util.h"

namespace sail {

Store::Store(Symbol* v1, Symbol* v2, int offset, string name,
		il::node* original, int line) {
	this->v1 = v1;
	this->v2 = v2;
	this->offset = offset;
	this->name = name;
	assert(original != NULL);
	this->original = original;
	this->inst_id = STORE;
	this->line = line;
}

Store::Store(Symbol* v1, Symbol* v2, int offset, string name, int line) {
	this->v1 = v1;
	this->v2 = v2;
	this->offset = offset;
	this->name = name;
	this->original = NULL;
	this->inst_id = STORE;
	this->line = line;
}


string Store::to_string() const
{
	string res = "";
	if(offset==0 && name == "") res += "*" + v1->to_string();
	else if(name != "") res += v1->to_string()+"->" +name;
	else res += "*(" + v1->to_string()+ "+" + int_to_string(offset)+")";
	res += " = " + v2->to_string();
	return res;
}

string Store::to_string(bool pp) const
{
	string res = "";
	if(offset==0 && name == "") res += "*" + v1->to_string(pp);
	else if(name != "") res += v1->to_string(pp)+"->" +name;
	else res += "*(" + v1->to_string(pp)+ "+" + int_to_string(offset)+")";
	res += " = " + v2->to_string(pp);
	res += "( off: " + int_to_string(offset) + ")";
	//haiyan added to print line num.
	if(pp)
	{
	    string linenum;
	    if((this->line != -1)&&(this->line != 0))
	    {
		stringstream ss;
		ss << this->line;
	        linenum = ss.str();
		res +=  "( line :" + linenum + " )";
	    }
	}
	return res;
}

int Store::get_offset()
{
	return offset;
}
string & Store::get_field_name()
{
	return name;
}

Symbol* Store::get_lhs()
{
	return v1;
}

Symbol* Store::get_rhs()
{
	return v2;
}

void Store::set_lhs(Symbol* s1)
{
	this->v1 = s1;
}

void Store::set_rhs(Symbol* s2)
{
	this->v2 = s2;
}


bool Store::is_removable()
{
	return false;
}


Store::~Store() {
}

}
