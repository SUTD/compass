#include "return_statement.h"
#include "expression.h"

namespace il
{

return_statement::return_statement(expression* e, location loc)
{
	this->exp =e;
	this->loc = loc;
	this->node_type_id = RETURN_STMT;
}

return_statement::~return_statement()
{
}

void return_statement::print()
{
	cout << "Return: ";
	exp->print();
	cout << endl;
}

string return_statement::to_string() const
{
	string res = "return ";
	if(exp!= NULL) res+= exp->to_string();
	res += "; ";
	return res;
}

expression* return_statement::get_ret_expression()
{
	return exp;
}


bool return_statement::has_return_value()
{
	return (exp != NULL);
}

}
