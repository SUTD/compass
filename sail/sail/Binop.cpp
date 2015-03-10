/*
 * Binop.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "Binop.h"
#include "Variable.h"
#include "Symbol.h"
#include "node.h"
#include <assert.h>

namespace sail {

Binop::Binop(Variable* v, Symbol* s1, Symbol* s2, il::binop_type binop,
		il::node* original, int line) {
	this->v = v;
	this->s1 = s1;
	this->s2 = s2;
	this->binop = binop;
	this->original = original;
	this->inst_id = BINOP;
	this->line = line;
}

Binop::Binop(Variable* v, Symbol* s1, Symbol* s2, il::binop_type binop, int line) {
	this->v = v;
	this->s1 = s1;
	this->s2 = s2;
	this->binop = binop;
	this->original = NULL;
	this->inst_id = BINOP;
	this->line = line;
}

string Binop::to_string() const
{
	string res = v->to_string () + " = ";
	res += s1->to_string() + " ";
	res += il::binop_expression::binop_to_string(binop);
	res += " " + s2->to_string();
	return res;
}

string Binop::to_string(bool pretty_print) const
{
	string res = v->to_string (pretty_print) + " = ";
	res += s1->to_string(pretty_print) + " ";
	res += il::binop_expression::binop_to_string(binop);
	res += " " + s2->to_string(pretty_print);
	//haiyan added to print line num.
	if(pretty_print)
	{
	    string linenum;
	    if((this->line != 0)&&(this->line != -1))
	    {
		stringstream ss;
		ss << this->line;
		linenum = ss.str();
		res +=  + "( line :" + linenum +" )";
	    }
	}
	return res;
}
Variable* Binop::get_lhs()
{
	return v;
}

void Binop::set_lhs(Variable* v)
{
	this->v = v;
}


void Binop::set_rhs1(Symbol* s1)
{
	this->s1 = s1;
}

void Binop::set_rhs2(Symbol* s2)
{
	this->s2 = s2;
}


Symbol* Binop::get_first_operand()
{
	return s1;
}
Symbol* Binop::get_second_operand()
{
	return s2;
}
il::binop_type Binop::get_binop()
{
	return binop;
}

bool Binop::is_removable()
{
	return v->is_removable_temp();
}

Binop::~Binop() {

}

}
