#include "case_label.h"
#include <sstream>
#include "util.h"

namespace il
{
case_label::case_label()
{

}

case_label::case_label(long low, long high,
			location loc)
{
	this->low = low;
	this->high = high;
	this->loc = loc;
	this->is_default = false;
	this->node_type_id = CASE_LABEL;
}

case_label::case_label(long num,
			location loc)
{
	this->low = num;
	this->high = num;
	this->loc = loc;
	this->is_default = false;
	this->node_type_id = CASE_LABEL;
}

case_label::case_label(location loc)
{
	this->low = -1;
	this->high = -1;
	this->loc = loc;
	this->is_default = true;
	this->node_type_id = CASE_LABEL;
}

bool case_label::is_default_label() const
{
	return is_default;
}

string case_label::to_string() const
{

	if(is_default) return "default: ";
	string res = "case ";
	stringstream s_low, s_high;
	string low_str, high_str;
	s_low << low;
	s_low >> low_str;
	res += low_str;

	if(has_range()){
		s_high << high;
		res+= " ... ";
		s_high >> high_str;
		res+=high_str;

	}



	res+=": ";
	return res;

}

long case_label::get_low()
{
	return low;
}

long case_label::get_high()
{
	return high;
}


bool case_label::has_range() const
{
	return (low != high);
}


case_label::~case_label()
{
}

}
