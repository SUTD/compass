#ifndef _LABEL_H
#define _LABLE_H
#include "ASMInstruction.h"
class Label: public ASMInstruction {

private:
        string _label_name;
public:
        Label(CodeHeader*, Format opformat, DexOpcode opcode, string ope, map<unsigned int, ClassDefInfo*>* vartype = NULL, vector<unsigned short>* regs = NULL, string operand1="",string operand2="", string operand3="", Data* data= NULL, Switch* swi = NULL, unsigned int methodidx = UINT_MAX,Method* method = NULL, Tries* tries = NULL);
        string to_string(); // including its ASMInstruction
	void set_label_name(string str);
        virtual string getLabel();
	virtual bool is_label();
	virtual bool onlyTrylabels();
	virtual bool onlyTryorCatch();
	virtual bool onlyCatches();
	virtual void setCheckCastClassDef(ClassDefInfo* classdef){this->_check_cast_classdef = classdef;}
	virtual ClassDefInfo* getCheckCastClassDef(){ return this->_check_cast_classdef;}
        ~Label();
};
#endif
