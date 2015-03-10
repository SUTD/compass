#include "try_finally_statement.h"
#include "expression.h"

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

namespace il
{

try_finally_statement::try_finally_statement() {}

try_finally_statement::try_finally_statement(statement *try_stmt,
                                             statement *exit_stmt,
                                             location loc)
	: try_statement(try_stmt),
	  exit_statement(exit_stmt)
{
	this->loc = loc;
	this->node_type_id = TRY_FINALLY_STMT;
}

try_finally_statement::~try_finally_statement()
{
}

void try_finally_statement::print()
{
	cout << to_string();
}

statement *try_finally_statement::get_try_statement()
{
	return try_statement;
}

statement *try_finally_statement::get_exit_statement()
{
	return exit_statement;
}

string try_finally_statement::to_string() const
{
	stringstream sstr;
	sstr << "try {" << endl;
	sstr << try_statement->to_string() << endl;
	sstr << "} finally {" << endl;
	sstr << exit_statement->to_string() << endl;
	sstr << "}" << endl;
	
	return sstr.str();
}

ostream &operator <<(ostream &os, const try_finally_statement &obj)
{
	os << obj.to_string();
	return os;
}


}
