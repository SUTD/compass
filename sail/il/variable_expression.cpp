#include "variable_expression.h"
#include "assert.h"

#include "variable.h"
#include "block.h"

namespace il
{
variable_expression::variable_expression()
{

}

variable_expression::variable_expression(variable *v, location loc)
{
	assert(v!=NULL);
	this->loc = loc;
	this->var = v;
	this->t = v->get_type();
	this->node_type_id = VARIABLE_EXP;
}

variable_expression::~variable_expression()
{
}

ostream& operator <<(ostream &os, const variable_expression &obj)
{
      os << obj.to_string();
      return os;
}

string variable_expression::to_string() const
{
	return var->to_string();
}

variable *variable_expression::get_variable()
{
	return this->var;
}




}
