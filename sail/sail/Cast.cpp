/*
 * Cast.cpp
 *
 *  Created on: Jun 29, 2008
 *      Author: isil
 */
#include "Cast.h"
#include "Variable.h"
#include "Symbol.h"

#include "type.h"
#include "node.h"

namespace sail {

Cast::Cast(Variable* v, Symbol* s, il::type* t, il::node* original, int line)
{
	this->v = v;
	this->s = s;
	this->t = t;
	assert(original != NULL);
	this->original = original;
	this->inst_id = CAST;
	this->line = line;
}

string Cast::to_string() const
{
	string res = v->to_string();
	res += (" = (" + t->to_string() + ") " + s->to_string());
	return res;
}

string Cast::to_string(bool pretty_print) const
{
	string res = v->to_string(pretty_print);
	res += (" = (" + t->to_string() + ") " + s->to_string(pretty_print));
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
	return res;
}

Variable* Cast::get_lhs()
{
	return v;
}
void Cast::set_lhs(Variable* v)
{
	this->v = v;
}

Symbol* Cast::get_rhs()
{
	return s;
}

void Cast::set_rhs(Symbol* sym)
{
	this->s = sym;
}


il::type* Cast::get_cast_type()
{
	return t;
}

bool Cast::is_removable()
{
	return v->is_removable_temp();
}



Cast::~Cast() {

}

}
