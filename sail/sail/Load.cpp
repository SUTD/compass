/*
 * Load.cpp
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 */

#include "Load.h"
#include "Variable.h"
#include "node.h"
#include <sstream>
#include <assert.h>
#include "util.h"

namespace sail {

Load::Load(Variable* v1, Symbol* v2, int offset, string name,
		il::node* original, int line) {
	this->v1 = v1;
	this->v2 = v2;
	assert(original != NULL);
	this->original = original;
	this->inst_id = LOAD;
	this->offset = offset;
	this->name = name;
	this->line = line;
}


Load::Load(Variable* v1, Symbol* v2, int offset, string name, int line){
		this->v1 = v1;
		this->v2 = v2;
		this->original = NULL;
		this->inst_id = LOAD;
		this->offset = offset;
		this->name = name;
		this->line = line;
}

string Load::to_string() const
{
	string res = v1->to_string();
	res += " = ";
	if(offset==0 && name == "") res += "*" + v2->to_string();
	else if(name != "") res += v2->to_string()+"->" +name;
	else res += "*(" + v2->to_string()+ "+" + int_to_string(offset)+")";
	return res;
}

string Load::to_string(bool pp) const
{
	string res = v1->to_string(pp);
	res += " = ";
	if(offset==0 && name == "") res += "*" + v2->to_string(pp);
	else if(name != "") res += v2->to_string(pp)+"->" +name;
	else res += "*(" + v2->to_string(pp)+ "+" + int_to_string(offset)+")";
	res += "(off: " + int_to_string(offset) + ")";
	res += "(type of LHS:  " + v1->get_type()->to_string() +")";

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

/*
//haiyan added for load's field in dex
string Load::to_string(bool pp) const
{
	string res = v1->to_string(pp);
	res += " = ";
	if(offset==0 && name == "") res += "*" + v2->to_string(pp);
	else if(name != "") res += v2->to_string(pp)+"->" +name;
	else res +=  v2->to_string(pp);
	return res;
}
//haiyan end for load's field in dex
*/

Variable* Load::get_lhs()
{
	return v1;
}

void Load::set_lhs(Variable* v)
{
	this->v1 = v;
}

void Load::set_rhs(Symbol* s)
{
	this->v2 = s;
}

Symbol* Load::get_rhs()
{
	return v2;
}

bool Load::is_removable()
{
	return v1->is_removable_temp();
}

int Load::get_offset()
{
	return offset;
}
string& Load::get_field_name()
{
	return name;
}




Load::~Load() {
}

}
