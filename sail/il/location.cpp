#include "location.h"


#include <sstream>

namespace il
{

location::location(uint16 start_byte, uint32 start_line, uint16 end_byte,
		uint32 end_line)
{
	this->byte_start = start_byte;
	this->byte_end = end_byte;
	this->line_start = start_line;
	this->line_end = end_line;
}

location::location()
{
	this->byte_start = (uint16) INVALID;
	this->byte_end = (uint16) INVALID;
	this->line_start = INVALID;
	this->line_end = INVALID;
}




location::~location()
{
}

bool location::is_valid()
{
	return (line_start != INVALID);
}

void location::print()
{
	cout << "loc:<" << line_start << ":" << byte_start << ","<< line_end << ":"
	<< byte_end << ">";
}

string location::to_string() const
{

	stringstream out;
	out << line_start;
	string ls = out.str();

	stringstream out2;
	out2 << byte_start;
	string bs = out2.str();

	string res = "";
	res += "(" + ls + ":" + bs + ")";
	return res;
}

string location::to_string(bool pp)
{
	if(!pp) return to_string();
	stringstream out;
	out << line_start;
	string ls = out.str();
	return ls;
}

ostream& operator <<(ostream &os, const location &obj)
{
      os << obj.to_string();
      return os;
}

}
