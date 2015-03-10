#include "while_loop.h"
#include <assert.h>

namespace il
{

while_loop::while_loop()
{

}

while_loop::while_loop(expression* continue_cond, statement* body)
{
	this->continue_cond = continue_cond;
	this->body = body;
	this->node_type_id = WHILE_LOOP;
}

while_loop::~while_loop()
{
}
expression* while_loop::get_continuation_cond(){return continue_cond;}
statement* while_loop::get_body(){return body;}
void while_loop::print()
{
	cout << "while loop " << endl;
}

}
