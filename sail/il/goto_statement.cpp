#include "goto_statement.h"
#include <string>
#include "label.h"
namespace il
{

using namespace std;

goto_statement::goto_statement()
{

}
goto_statement::goto_statement(label *l, string label_name, location loc):
	label_name(label_name)
{
	this->loc = loc;
	this->l = l;
	this->node_type_id = GOTO_STMT;
}

string goto_statement::get_label_name()
{
	return label_name;
}

label* goto_statement::get_label()
{
	return l;
}
label** goto_statement::get_label_ref()
{
	return &l;
}

void goto_statement::print()
{
	cout << "Goto: " << label_name <<endl;
}
goto_statement::~goto_statement()
{
}

string goto_statement::to_string() const
{
	return "goto " + label_name +"; ";
}


}
