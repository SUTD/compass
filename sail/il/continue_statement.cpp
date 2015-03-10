#include "continue_statement.h"

namespace il
{

continue_statement::continue_statement()
{
	this->node_type_id = CONTINUE_STMT;
}

continue_statement::~continue_statement()
{
}

void continue_statement::print()
{
	cout << "Continue" << endl;
}

string continue_statement::to_string() const
{
	return "continue; ";
}

}
