#include "file.h"

#include "function_declaration.h"
#include "declaration.h"
#include "assembly.h"

namespace il
{

file::file()
{

}

file::file(string file_name):file_name(file_name)
{
	this->node_type_id = FILE_NODE;
	init_func = NULL;
	glob_asm = NULL;

}

file::~file()
{

}

void file::print()
{
	cout << "File with name: " << file_name << endl;
}

void file::add_declaration(declaration * decl)
{
	declarations.push_back(decl);
}

void file::set_initfunc(function_declaration* init_func)
{
	this->init_func = init_func;
}

void file::set_globasm(assembly* glob_asm)
{
	this->glob_asm = glob_asm;
}

vector<declaration*> & file::get_declarations()
{
	return declarations;
}

function_declaration * file::get_initfunc()
{
	return init_func;
}

assembly *file::get_globasm()
{
	return glob_asm;
}

string file::to_string() const
{
	string res = file_name + "\n";
	for(unsigned int i=0; i < declarations.size(); i++)
		res += declarations[i]->to_string() + "\n";


	return res;
}

string file::get_name()
{
	return file_name;
}




}
