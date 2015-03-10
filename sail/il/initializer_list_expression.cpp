/*
 * initializer_list_expression.cpp
 *
 *  Created on: Jun 27, 2008
 *      Author: isil
 */

#include "initializer_list_expression.h"

namespace il {

initializer_list_expression::initializer_list_expression()
{

}

initializer_list_expression::initializer_list_expression(
		vector<expression*>& init_list, type* t, location loc) {
	this->init_list = init_list;
	this->t=t;
	this->loc= loc;
	this->node_type_id = INITIALIZER_LIST_EXP;

}

vector<expression*> & initializer_list_expression::get_init_list()
{
	return init_list;
}
string initializer_list_expression::to_string() const
{
	string res = "{";
	for(int i=0; i<(int)init_list.size(); i++){
		res += init_list[i]->to_string();
		if(i!= (int)init_list.size()-1) res+= ", ";
	}
	res += "} ";
	return res;
}



initializer_list_expression::~initializer_list_expression() {

}

}
