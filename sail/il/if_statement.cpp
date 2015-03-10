#include "if_statement.h"
#include "expression.h"

namespace il
{

if_statement::if_statement()
{

}

if_statement::if_statement(expression* test, statement* if_branch,
		statement* else_branch, location loc)
{
	this->test = test;
	this->if_branch = if_branch;
	this->else_branch = else_branch;
	this->loc = loc;
	this->node_type_id = IF_STMT;
}
void if_statement::print()
{
	cout << "if(";
	test->print();
	cout << ") ";
	if_branch->print();
	if(else_branch != NULL)
	{
		cout << " else ";
		else_branch->print();
	}
	cout << endl;
}
expression* if_statement::get_test()
{
	return test;
}
statement* if_statement::get_if_statement(){return if_branch;}
statement* if_statement::get_else_statement(){return else_branch;}

if_statement::~if_statement()
{
}

string if_statement::to_string() const
{
	string res = "if(";
	res += test->to_string();
	res+= ") \n";
	res += if_branch->to_string();
	if(else_branch != NULL)
	{
		res += "\n else ";
		res += else_branch->to_string();
	}
	res+="; ";

	return res;
}

ostream& operator <<(ostream &os, const if_statement &obj)
{
      os  << obj.to_string();
      return os;
}

}
