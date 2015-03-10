#include "do_while_loop.h"

namespace il
{

do_while_loop::do_while_loop()
{

}

do_while_loop::do_while_loop(expression* continue_cond, statement* body)
{
	this->continue_cond = continue_cond;
	this->body = body;
	this->node_type_id = DO_WHILE_LOOP;
}

expression* do_while_loop::get_continuation_cond()
{
	return continue_cond;
}
statement* do_while_loop::get_body()
{
	return body;
}

void do_while_loop::print()
{
	cout << "do while loop " << endl;
}

do_while_loop::~do_while_loop()
{
}

}
