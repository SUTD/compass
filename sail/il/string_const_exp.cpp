#include "string_const_exp.h"

namespace il
{

string_const_exp::string_const_exp()
{

}

string_const_exp::string_const_exp(string str_const, type* t, location loc)
{
	this->str_const = str_const;
	this->t = t;
	this->loc = loc;
	this->node_type_id = STRING_CONST_EXP;
}

string string_const_exp::get_string()
{
	return str_const;
}

string_const_exp::~string_const_exp()
{
}

string string_const_exp::to_string() const
{
	return ("\"" + str_const + "\"");
}
}
