#include "block.h"

#include "variable_declaration.h"
#include "statement.h"

namespace il
{

block::block()
{

}

block::block(vector<variable_declaration*> & var_decls,
			vector<statement*> & statements)
{
	this->var_decls = var_decls;
	this->statements = statements;
	this->node_type_id = BLOCK;
}
vector<variable_declaration*> & block::get_var_declarations()
{
	return var_decls;
}
vector<statement*> & block::get_statements()
{
	return statements;
}

void block::print()
{
	cout << "Block: " << endl;
}

block::~block()
{
	cout << "Deleting block : " << to_string() << endl;
}

string block::to_string() const
{
	string res = "{\n";
	for(unsigned int i=0; i < var_decls.size(); i++)
	{
		res+="\t" + var_decls[i]->to_string();
		res += "; \n";
	}
	res += "/* Decl end */\n";

	for(unsigned int i=0; i<statements.size(); i++)
	{
		il::statement* s = statements[i];
		res += "\t " +statements[i]->to_string();
		res += "\n";
	}

	res+="}";
	return res;
}

ostream& operator <<(ostream &os, const block &obj)
{
      os << obj.to_string();
      return os;
}




}
