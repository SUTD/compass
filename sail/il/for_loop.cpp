#include "for_loop.h"
#include "expression.h"
#include "statement.h"

namespace il
{

for_loop::for_loop()
{

}

for_loop::for_loop(expression* init, expression* continue_cond,
		expression* update, statement* body, location loc)
{
	this->init = init;
	this->continue_cond = continue_cond;
	this->update = update;
	this->body = body;
	this->loc = loc;
	this->node_type_id = FOR_LOOP;

}

expression* for_loop::get_continuation_cond(){return continue_cond;}
expression* for_loop::get_update_expression(){return update;}
statement* for_loop::get_body(){return body;}
void for_loop::print()
{
	cout << "For loop: " << endl;
}

for_loop::~for_loop()
{
}

string for_loop::to_string() const
{
	string res = "for(";
	if(init!=NULL) res += init->to_string();
	res+= ";";
	if(continue_cond!=NULL) res+= continue_cond->to_string();
	res+=";";
	if(update!=NULL) res+= update->to_string();
	res+=") \n";
	if(body!=NULL) res+= "\t" + body->to_string();
	return res;

}

}
