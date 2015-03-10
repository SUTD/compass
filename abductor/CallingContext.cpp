/*
 * CallingContext.cpp
 *
 *  Created on: Jun 18, 2012
 *      Author: boyang
 */

#include "CallingContext.h"
#include "function_declaration.h"
#include "Function.h"
#include "Instruction.h"
#include "FunctionCall.h"
#include "variable_declaration.h"
#include "VariableTerm.h"

CallingContext::CallingContext() {
	// TODO Auto-generated constructor stub

}

CallingContext::~CallingContext() {
	// TODO Auto-generated destructor stub
}


CallingContext::CallingContext(const CallingContext& other)
{
	//this->context = other.context;
	vector <pair<Function *, int> >::const_iterator iter = other.context.begin();
	for(; iter != other.context.end(); iter++)
	{
		this->context.push_back(pair<Function *, int>((*iter).first, (*iter).second));
	}
}


void CallingContext::add_context(Function * func, int line)
{
	this->context.push_back(pair<Function *, int>(func, line));
}


bool CallingContext::operator==(const CallingContext &  other) const
				{
	if(this->context.size() != other.context.size())
	{
		return false;
	}
	vector <pair<Function *, int> >::const_iterator iter_other = other.context.begin();
	vector <pair<Function *, int> >::const_iterator iter_this = context.begin();
	for(; iter_other != other.context.end();)
	{
		//		cout << "iter_other first : " << iter_other->first;
		//		cout << "iter_this first : " << iter_this->first;
		function_declaration* declare_other = iter_other->first->get_original_declaration();
		string name_other = declare_other->get_name();
		function_declaration* declare_this = iter_this->first->get_original_declaration();
		string name_this = declare_this->get_name();
		if(strcmp(name_other.c_str(), name_this.c_str()) != 0)
		{
			return false;
		}
		if(iter_other->second != iter_this->second)
		{
			return false;
		}
		iter_other++;
		iter_this++;
	}
	return true;

	//	//correct?
	//	if(this->context == other.context)
	//	{
	//		return true;
	//	}
	//	else
	//	{
	//		return false;
	//	}
				}


int CallingContext::get_context_size()
{
	return this->context.size();
}


string CallingContext::get_caller_name()
{
	string res = " ";

	if(!context.empty())
	{

		//vector <pair<Function *, int> >::const_iterator iter = context.begin();
		//for(; iter != context.end(); iter++)
		//{
		//	function_declaration* declare = (*iter).first->get_original_declaration();
		//	res += 	declare->get_name()+"::";
		//}
		//only get the function name of caller
		function_declaration* declare = context.back().first->get_original_declaration();
		res += 	declare->get_name();
	}
	return res;
}




string CallingContext::to_string() const
{
	string res = "calling context:  \n";

	if(context.empty())
	{
		res += "main function \n";
	}
	else
	{
		vector <pair<Function *, int> >::const_iterator iter = context.begin();
		for(; iter != context.end(); iter++)
		{
			//res += "function:  " + (*iter).first->get_file();
			function_declaration* declare = (*iter).first->get_original_declaration();
			res += "function:  " + declare->get_name();
			res += "\t line:   " + std::to_string((*iter).second) + "\n";
		}
	}

	return res;
}

void CallingContext::pop_back_context()
{
	context.pop_back();
}

int CallingContext::get_back_line_num()
{
	return this->context.back().second;
}

string CallingContext::get_curfunc_name()
{
	string str_return = " ";
	if(this->context.size() == 0)
	{
		return str_return;
	}
	else
	{
		Function * f = this->context.back().first;
		int line = this->context.back().second;
		vector<Instruction*> *instructions = f->get_body();
		vector<Instruction*> ::iterator iter;
		for (iter = (*instructions).begin(); iter != (*instructions).end(); ++iter) {
			if((*iter)->get_instruction_id() == FUNCTION_CALL)
			{
				FunctionCall* functionCall = static_cast<sail::FunctionCall*> (*iter);
				if(functionCall->line == line)
				{
					string name = functionCall->get_function_name();
					str_return = name;
					return str_return;
				}
			}
		}

	}

	//should get return value
	assert(false);
}



void CallingContext::get_curfunc_args(vector<Term*>& v_return)
{
	//	if(this->context.size() == 0)
	//	{
	//		return v_return;
	//	}
	//	else
	//	{

	cout << "into get_curfunc_args   " << endl;
	cout << this->to_string() << endl;
	cout << "get_curfunc_name    "  << this->get_curfunc_name() << endl;

	Function * f = this->context.back().first;
	int line = this->context.back().second;
	vector<Instruction*> *instructions = f->get_body();
	vector<Instruction*> ::iterator iter;
	for (iter = (*instructions).begin(); iter != (*instructions).end(); ++iter) {
		if((*iter)->get_instruction_id() == FUNCTION_CALL)
		{
			FunctionCall* functionCall = static_cast<sail::FunctionCall*> (*iter);
			if(functionCall->line == line)
			{
				function_declaration* declare = f->get_original_declaration();
				string name_declare =  declare->get_name();

				vector<variable_declaration*> args_declare = declare->get_arguments();
				for(unsigned int i = 0; i < args_declare.size(); i++)
				{
					cout << "called!!!!!   "  << endl;
					Term *term = VariableTerm::make(args_declare[i]->get_variable()->get_name());
					v_return.push_back(term);
					//Term *term = VariableTerm::make(var_prefix +name_declare+"::"+ args_declare[i]->get_variable()->get_name());
					//Term *term = VariableTerm::make(name_declare+"::"+ args_declare[i]->get_variable()->get_name());
					//phi_temp.replace_term(args[i], term);
				}
				//return v_return;
			}
		}
	}
	//
	//	}
	//
	//	//should get return value
	//	assert(false);
}
