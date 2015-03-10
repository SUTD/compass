#include "try_catch_statement.h"
#include "expression.h"

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

namespace il
{

try_catch_statement::try_catch_statement() {}

try_catch_statement::try_catch_statement(statement *try_stmt, location loc)
	: try_statement(try_stmt)
{
	this->loc = loc;
	this->node_type_id = TRY_CATCH_STMT;
}

try_catch_statement::try_catch_statement(statement *try_stmt, location loc,
		vector<type *> catch_types, vector<statement *> catch_bodies):
		try_statement(try_stmt), catch_types(catch_types),
		catch_bodies(catch_bodies)
{
	this->loc = loc;
	this->node_type_id = TRY_CATCH_STMT;
}

try_catch_statement::~try_catch_statement()
{
}


void try_catch_statement::print()
{
	cout << to_string();
}

statement *try_catch_statement::get_try_statement()
{
	return try_statement;
}

vector<type *> &try_catch_statement::get_catch_types()
{
	return catch_types; 
}

vector<statement *> &try_catch_statement::get_catch_bodies()
{
	return catch_bodies;
}

void try_catch_statement::add_handler(type *catch_type, statement *catch_body)
{
	catch_types.push_back(catch_type);
	catch_bodies.push_back(catch_body);
}

string try_catch_statement::to_string() const
{
	stringstream sstr;
	sstr << "try {" << endl;
	sstr << try_statement->to_string() << endl;
	sstr << "}";
	
	for (unsigned int i = 0; i < catch_types.size(); ++i) {
		type *t = catch_types[i];
		sstr << " catch (" << (t ? t->to_string() : "...") << ") {" << endl;
		sstr << catch_bodies[i]->to_string() << endl;
		sstr << "}";
	}
	sstr << endl;
	
	return sstr.str();
}

ostream& operator <<(ostream &os, const try_catch_statement &obj)
{
	os << obj.to_string();
	return os;
}

}
