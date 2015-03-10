#include "integer_const_exp.h"
#include <assert.h>
#include <sstream>

namespace il
{

integer_const_exp::integer_const_exp()
{

}

integer_const_exp::integer_const_exp(long int int_const,
			type* t, location loc)
{
	if(t->is_base_type())
	{
		base_type* bt = (base_type*) t;
		is_signed = bt->is_signed_type();
		is_char = (bt->get_name().find("char", 0)!=string::npos);
	}
	else
	{
		is_signed = is_char = false;
	}
	this->t = t;
	this->loc = loc;
	this->int_const = int_const;
	this->node_type_id = INTEGER_CONST_EXP;
}

long int integer_const_exp::get_integer()
{
	return int_const;
}
bool integer_const_exp::isSigned()
{
	return is_signed;
}
bool integer_const_exp::isChar()
{
	return is_char;
}



string integer_const_exp::to_string() const
{
	string res;

	if(is_char && is_signed){
		signed char sc = (signed char) int_const;
		stringstream ss;
		ss << sc;
		ss >> res;
	}

	else if(is_char && !is_signed){
		unsigned char usc = (unsigned char) int_const;
		stringstream ss;
		ss << usc;
		ss >> res;
	}

	//else if(!is_char && is_signed){
	else{
		long int i = (long int) int_const;
		stringstream ss;
		ss << i;
		ss >> res;
	}
	//}
		/*
	else if(!is_char && !is_signed){
		unsigned long int i = (unsigned long int) int_const;
		stringstream ss;
		ss << i;
		ss >> res;
	}
	*/
	return res;
}


integer_const_exp::~integer_const_exp()
{
}

}
