#include "expr_list_expression.h"
#include "type.h"

namespace il
{

expr_list_expression::expr_list_expression()
{

}

expr_list_expression::expr_list_expression(vector<expression*> & exprs, type* t,
		location loc)
{
	this->exprs = exprs;

	if(t == NULL) {
		t = exprs[exprs.size()-1]->get_type();
	}

	this->t = t;
	this->loc = loc;
	this->node_type_id = EXPR_LIST_EXP;
}

string expr_list_expression::to_string() const
{
	string res = "";
	for(int i=0; i<(int)exprs.size(); i++){
		res += exprs[i]->to_string();
		if(i!=(int)exprs.size()-1) res+= ", ";
	}
	return res;
}

vector<expression*> &expr_list_expression::get_inner_expressions()
{
	return exprs;
}


expr_list_expression::~expr_list_expression()
{
}

}
