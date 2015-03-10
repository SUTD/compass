#include "break_statement.h"

namespace il
{

break_statement::break_statement()
{
	this->node_type_id = BREAK_STMT;
}

break_statement::~break_statement()
{
}

void break_statement::print()
{
	cout << "break; " << endl;
}

string break_statement::to_string() const
{
	return "break; ";
}



}
