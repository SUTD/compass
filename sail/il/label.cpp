#include "label.h"

namespace il
{

label::label()
{

}

label::label(string label_name, location loc):label_name(label_name)
{
	this->loc = loc;
	this->node_type_id = LABEL;
}

string label::get_label()
{
	return label_name;
}

string label::to_string() const
{
	return label_name +":";

}

void label::print()
{
	cout << "Label: " << label_name << endl;
}



label::~label()
{
}

}
