#include"RVT.h"
//RVT::RVT(string reg, string var,string sig, ClassDefInfo* type): _reg_name(reg), _var_name(var), _signature(sig), _type(type)
RVT::RVT(unsigned int reg, string var,string sig, ClassDefInfo* type): _reg(reg), _var_name(var), _signature(sig), _type(type)
{
	this->_restart_flag = false;
}

RVT::~RVT()
{
}

/*
bool RVT::isPrimitiveType()
{
	return(_type->getTypename.length()==1);
}

bool RVT::isArrayType()
{
	return(_type->getTypename.begin()=="[");
}
*/

unsigned int RVT::getReg()
{
	return _reg;
}
/*
bool RVT::hasField()
{
	return((*_type->getField()).size()> 0);
}*/


/*
bool RVT::hasMethod()
{
	return((*_type->getMethod()).size()> 0);
}*/


string RVT::getRegName()
{
	
	return "v"+ Int2Str(_reg);
}

string RVT::getVarName()
{
	return _var_name;
}

string RVT::getSignature()
{
	return _signature;
}

ClassDefInfo* RVT::getClassType()
{
	return _type;
}
