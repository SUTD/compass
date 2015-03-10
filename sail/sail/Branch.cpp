/*
 * Branch.cpp
 *
 *  Created on: Jun 30, 2008
 *      Author: isil
 */

#include "Branch.h"
#include "Variable.h"
#include "Label.h"
#include "Symbol.h"
#include "node.h"
#include <assert.h>

namespace sail {

Branch::Branch(Symbol* if_var, Symbol* else_var, Label* then_label, Label* else_label,
		il::node* original) {
	this->targets = new vector<pair<Symbol*, Label*> >();
	targets->push_back(pair<Symbol*, Label*>(if_var, then_label));
	targets->push_back(pair<Symbol*, Label*>(else_var, else_label));
	this->original = original;
	this->is_if = true;
	this->inst_id = BRANCH;

}


Branch::Branch(vector<pair<Symbol*, Label*> > *targets,
				il::node* original)
{
	this->targets = targets;
	this->original = original;
	this->is_if = false;
	this->inst_id = BRANCH;
}

string Branch::to_string() const
{
	if(is_if)
	{
		string res = "if(" + (*targets)[0].first->to_string() + ")";
		res += " then goto " + (*targets)[0].second->to_string();
		res += " else goto " + (*targets)[1].second->to_string();
		res +=  "( else cond: " + (*targets)[1].first->to_string() + ")";
		return res;
	}
	string res = "switch (";
	for(int i =0; i < (int)targets->size(); i++)
	{
		res += "\n\t<" + (*targets)[i].first->to_string() + " => " +
		(*targets)[i].second->to_string() + "> ";
	}
	res += "\n)";
	return res;

}

string Branch::to_string(bool pretty_print) const
{
	if(is_if)
	{
		string res = "if(" + (*targets)[0].first->to_string(pretty_print) + ")";
		res += " then goto " + (*targets)[0].second->to_string(pretty_print);
		res += " else goto " + (*targets)[1].second->to_string(pretty_print);
		res +=  "( else cond: " + (*targets)[1].first->to_string() + ")";
		return res;
	}
	string res = "switch (";
	for(int i =0; i < (int)targets->size(); i++)
	{
		res += "\n\t<" + (*targets)[i].first->to_string(pretty_print) + " => " +
		(*targets)[i].second->to_string(pretty_print) + "> ";
	}
	res += "\n)";
	return res;

}

bool Branch::is_if_statement()
{
	return is_if;
}


vector<pair<Symbol*, Label*> > * Branch::get_targets()
{
	return targets;
}


Label* Branch::get_then_label()
{
	assert(this->is_if == true);
	return (*targets)[0].second;

}
Label* Branch::get_else_label()
{
	assert(this->is_if == true);
	return (*targets)[1].second;
}
Symbol* Branch::get_if_condition()
{
	assert(this->is_if == true);
	return (*targets)[0].first;
}
Symbol* Branch::get_else_condition()
{
	assert(this->is_if == true);
	return (*targets)[1].first;
}

bool Branch::is_removable()
{
	return false;
}

Branch::~Branch() {
	delete targets;
}

}
