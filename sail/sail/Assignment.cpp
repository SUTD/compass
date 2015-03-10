/*
 * Assignment.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "Assignment.h"
#include "Variable.h"
#include "Symbol.h"
#include "node.h"
#include <assert.h>

namespace sail {

Assignment::Assignment(Variable* v, Symbol* s, il::node* original, int line) {
	this->v = v;
	this->s = s;
	this->original = original;
	this->inst_id = ASSIGNMENT;
	this->line = line;
	assert(s != NULL);
}

//added by haiyan
Assignment::Assignment(Variable*v, Symbol*s, int line){
	this->v = v;
	this->s = s;
	this->original = NULL;
	this->inst_id = ASSIGNMENT;
	this->line = line;
}
//end of haiyan

string Assignment::to_string() const
{
	string res = v->to_string();
	res += (" = " + s->to_string());
	return res;
}

string Assignment::to_string(bool pretty_print) const
{
	string res = v->to_string(pretty_print);
	res += (" = " + s->to_string(pretty_print));
	//haiyan added to print line num.
	if(pretty_print)
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

	//added by haiyan
	if(1){
	if(this->is_return_inst())
		res += " RETURN INST! ";
	}

	return res;
}

Variable* Assignment::get_lhs()
{
	return v;
}

void Assignment::set_lhs(Variable* v)
{
	this->v = v;
}

void Assignment::set_rhs(Symbol* s)
{
	this->s = s;
}


Symbol* Assignment::get_rhs()
{
	return s;
}

bool Assignment::is_removable()
{
	return v->is_temp();
}


Assignment::~Assignment() {

}

}
