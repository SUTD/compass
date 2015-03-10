/*
 * PreciseTypeVarStat.cpp
 *
 *  Created on: Sep 27, 2012
 *      Author: boyang
 */

#include "PreciseTypeVarStat.h"
#include "TypeInference.h"
#include "../Variable.h"


using namespace std;
using namespace sail;
using namespace il;


#define DEBUG true


namespace sail {

PreciseTypeVarStat::PreciseTypeVarStat() {

}



PreciseTypeVarStat::~PreciseTypeVarStat() {
}




PreciseTypeVarStat::PreciseTypeVarStat(Variable* var, type* type){
	this->var = var;
	this->cur_type = type;
}




PreciseTypeVarStat::PreciseTypeVarStat(const PreciseTypeVarStat& other)
{
	this->var = other.var;
	this->cur_type = other.cur_type;
}



Variable* PreciseTypeVarStat::get_var(){
	return this->var;
}


type* PreciseTypeVarStat::get_type(){
	return this->cur_type;
}



void PreciseTypeVarStat::merge(PreciseTypeVarStat* other){
	assert(this->var->get_var_name() == (other->get_var()->get_var_name()));
	type* other_type = other->get_type();
	//type* lub_type = sail::compute_lub(this->cur_type, other_type);
	int indicate = 0;
	type* lub_type = sail::lub_compute(this->cur_type, other_type, indicate);
	if(lub_type != NULL)
		this->cur_type = lub_type;

}


record_type* PreciseTypeVarStat::get_base(record_type* rt_cur)
{
	map<int, record_type*> map_rt_cur_bases = rt_cur->get_bases();
	return (map_rt_cur_bases.begin())->second;
}



bool PreciseTypeVarStat::operator==(const PreciseTypeVarStat &  other) const{
	if(this->var->get_var_name() != other.var->get_var_name())
	{
		return false;
	}

	if(cur_type != other.cur_type)
	{
		return false;
	}
	return true;
}




string PreciseTypeVarStat::to_string() const
{
	string res =  "---------------\n";
	res += "var : " + this->var->to_string() + "\n";
	res += "type : " + cur_type->to_string() + "\n";
	res += "---------------" ;
	return res;
}


}
