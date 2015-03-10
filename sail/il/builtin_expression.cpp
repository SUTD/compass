/*
 * builtin_expression.cpp
 *
 *  Created on: Jun 28, 2008
 *      Author: isil
 */

#include "builtin_expression.h"


namespace il {


builtin_expression::builtin_expression()
{

}

builtin_expression::builtin_expression(compass_builtin_type bt,
		vector<expression*> &args, type* t, location loc) {
	this->bt = bt;
	this->args = args;
	this->t = t;
	this->loc = loc;
	this->node_type_id = BUILTIN_EXP;
}

string builtin_expression::to_string() const
{
	string res = builtin_type_to_string(bt) + "(";
	for(int i= 0; i<(int)args.size(); i++){
		res += args[i]->to_string();
		if(i!=(int)args.size()-1) res+= ", ";
	}
	res+= ")";
	return res;
}

compass_builtin_type builtin_expression::get_builtin_type()
{
	return bt;
}
vector<expression*> & builtin_expression::get_arguments()
{
	return args;
}

builtin_expression::~builtin_expression() {
	// TODO Auto-generated destructor stub
}

string builtin_expression::builtin_type_to_string(compass_builtin_type b)
{
	switch(b)
	{
	case _VA_ARG:
		return "va_arg";
	default:
		return "unknown builtin expression";
	}
}


}
