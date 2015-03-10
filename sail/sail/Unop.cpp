/*
 * Unop.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "Unop.h"
#include "Variable.h"
#include "Symbol.h"
#include "node.h"
#include <assert.h>

namespace sail {

Unop::Unop(Variable* v, Symbol* s, il::unop_type unop,
		il::node* original, int line)
{
	this->v = v;
	this->s = s;
	this->unop = unop;
	this->original = original;
	this->inst_id = UNOP;
	this->line = line;
}

Unop::Unop(Variable* v, Symbol* s, il::unop_type unop, int line){
	this->v = v;
	this->s = s;
	this->unop = unop;
	this->original = NULL;
	this->inst_id = UNOP;
	this->line = line;
}

string Unop::to_string() const
{
	string res = v->to_string() +" = ";
	bool need_paren = false;
	res += il::unop_expression::unop_to_string(unop, &need_paren);
	res += s->to_string();
	res += "[" + s->get_type()->to_string() +"]";
	if(need_paren) res += ")";
	return res;
}

string Unop::to_string(bool pretty_print) const
{
	string res = v->to_string(pretty_print) + " = ";
	bool need_paren = false;
	res += il::unop_expression::unop_to_string(unop, &need_paren);
	res += s->to_string(pretty_print);
	res += "[" + s->get_type()->to_string() +"]";
	if(need_paren) res += ")";
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

Variable* Unop::get_lhs()
{
	return v;
}

void Unop::set_lhs(Variable* v)
{
	this->v = v;
}

void Unop::set_rhs(Symbol* s)
{
	this->s = s;
}

Symbol* Unop::get_operand()
{
	return s;
}
il::unop_type Unop::get_unop()
{
	return unop;
}

bool Unop::is_removable()
{
	return v->is_removable_temp();
}



Unop::~Unop() {

}

}
