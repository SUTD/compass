/*
 * ReachingDefVarStat.cpp
 *
 *  Created on: Sep 22, 2012
 *      Author: boyang
 */

#include "ReachingDefVarStat.h"
#include "../Variable.h"

namespace sail {


ReachingDefVarStat::ReachingDefVarStat() {

}



ReachingDefVarStat::ReachingDefVarStat(Variable* var, Instruction* inst){
	this->var = var;
	set_reaching_inst.insert(inst);
}



ReachingDefVarStat::ReachingDefVarStat(const ReachingDefVarStat& other)
{
	this->var = other.var;
	this->set_reaching_inst = other.set_reaching_inst;
}



ReachingDefVarStat::~ReachingDefVarStat() {
}


void ReachingDefVarStat::merge(ReachingDefVarStat* other){
	assert(this->var->get_var_name() == (other->get_var()->get_var_name()));
	set<Instruction*> other_reaching_insts;
	other->get_set_reaching(other_reaching_insts);
	set<Instruction*>::iterator other_insts_iter = other_reaching_insts.begin();
	for(; other_insts_iter != other_reaching_insts.end(); other_insts_iter++)
	{
		bool hasFound = false;
		set<Instruction*>::iterator this_insts_iter = set_reaching_inst.begin();
		for(;this_insts_iter != set_reaching_inst.end(); this_insts_iter++)
		{
			if((*other_insts_iter) == (*this_insts_iter))
			{
				hasFound = true;
				break;
			}
		}
		//if not found then insert it.
		if(hasFound == false)
		{
			set_reaching_inst.insert(*other_insts_iter);
		}
	}
}




Variable* ReachingDefVarStat::get_var(){
	return this->var;
}



void ReachingDefVarStat::get_set_reaching(set<Instruction*>& reaching_inst){
	set<Instruction*>::iterator iter = set_reaching_inst.begin();
	for(; iter != set_reaching_inst.end(); iter++)
	{
		reaching_inst.insert(*iter);
	}
}


string ReachingDefVarStat::to_string() const
{
	string res;
	//string res =  "---------------\n";
	res += "var : " + this->var->to_string() + "\n";
	set<Instruction*>::iterator iter = set_reaching_inst.begin();
	for(; iter != set_reaching_inst.end(); iter++)
	{
		res += "\t";
		res += (*iter)->to_string() + "\n";
	}
	//res += "---------------" ;
	return res;
}


bool ReachingDefVarStat::find_inst(Instruction* inst){
	set<Instruction*>::iterator iter = set_reaching_inst.begin();
	for(; iter != set_reaching_inst.end(); iter++)
	{
		if(*iter == inst)
		{
			return true;
		}
	}
	return false;
}



bool ReachingDefVarStat::operator==(const ReachingDefVarStat &  other) const
{
	if(this->var->get_var_name() != other.var->get_var_name())
	{
		return false;
	}

	if(this->set_reaching_inst.size() != other.set_reaching_inst.size())
	{
		return false;
	}

	set<Instruction*>::const_iterator iter_other;
	set<Instruction*>::const_iterator iter_this;

	//other is subset of this
	iter_other = other.set_reaching_inst.begin();
	for(;iter_other != other.set_reaching_inst.end();iter_other++)
	{
		bool found = false;
		iter_this = set_reaching_inst.begin();
		for(; iter_this != set_reaching_inst.end(); iter_this++)
		{
			if((*iter_this) == (*iter_other))
			{
				found = true;
			}
		}
		if(found == false)
		{
			return false;
		}
	}

	//this is subset of other
	iter_this = set_reaching_inst.begin();
	for(;iter_this != set_reaching_inst.end(); iter_this++)
	{
		bool found = false;
		iter_other = other.set_reaching_inst.begin();
		for(;iter_other != other.set_reaching_inst.end();iter_other++)
		{
			if((*iter_this) == (*iter_other))
			{
				found = true;
			}
		}
		if(found == false)
		{
			return false;
		}
	}
	return true;
}

}
