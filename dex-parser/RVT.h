#ifndef _RVT_H
#define _RVT_H
#include "ClassDefInfo.h"
#include "Int2Str.h"
//this class used as 
class RVT
{
public:
	//RVT(string reg, string var,string sig, ClassDefInfo* type);
	RVT(unsigned int reg, string var="",string sig="", ClassDefInfo* type= NULL);
	~RVT();
	
	//bool isArrayType();
	//bool hasField();
	string getRegName();
	unsigned int getReg(); //added 
	string getVarName();
	string getSignature();
	//bool hasMethod();
	ClassDefInfo* getClassType(); // by return the pointer of this object, we can call its getMethod() and getField();
	void setRestartFlag(){this->_restart_flag = true;}
	bool getRestartFlag() {return this->_restart_flag;}
private:
	unsigned int _reg;
	//string _reg_name;//from register number
	string _var_name;
	string _signature;
	ClassDefInfo* _type;
	bool _restart_flag;
	
	
};
	
#endif
