#ifndef _ASMINSTRUCTION_H
#define _ASMINSTRUCTION_H
#include <string>
#include "Data.h"
#include "Int2Str.h"
#include "Switch.h"
#include <stdio.h>
#include <iostream>
#include "CodeHeader.h"
#include <vector>
//hzhu begin 4/22/2012
#include "Method.h"
#include <map>
#include "DexOpcode.h"
#include "OpcodeFormat.h" // which is Formati
#include "Format.h"
#include "DexFileReader.h"
#include "sail.h"
//hzhu end 4/22/2012

class CodeHeader;
class DexFileReader;
//enum Format;
using namespace std;
/*
enum Format
{
     // 1 short
                F10t = 1,
                F10x,
                F11n,
                F11x,
                F12x, // 5
                // 2 shorts
                //F20bc, //6
                F20t,
                F21c, // 7
                F21h,
                F21s, //9
                F21t,
                F22b,
                F22c,
        //      F22cs,
                F22s,
                F22t,
                F22x,
                F23x,//17
                // 3 shorts
                F30t, //18
                F31c,
                F31i,
                F31t,
          //    F32s,
                F32x,
       //       F33x,
                F35c,
               //F35mi,
                F3rc,//26
        //      F3rmi,
        //      F3rms,
                // 4 shorts
                F40sc,//24
                //5 short
                F51l //25       
        };
*/

class Tries;
class ASMInstruction
{
public:
	//hzhu add 4/22/2012
	ASMInstruction(CodeHeader*, Format op_format, DexOpcode opcode, string ope, map<unsigned int, ClassDefInfo*>* vartype = NULL, vector<unsigned short>* regs = NULL, string operand1="",string operand2="", string operand3="", Data* data= NULL, Switch* swi = NULL, unsigned int methodidx = UINT_MAX,Method* method = NULL, Tries* tries = NULL);
	//hzhu 4/22/2012
	virtual	~ASMInstruction();
	//hzhu 4/23/2012
	string getOperator();
	//hzhu 4/23/2012
	string getOperand1();
	string getOperand2();
	string getOperand3();	
	string toString();
	bool hasData();
	Data* getData();
	bool hasSwitch();
	Switch* getSwitch();
	CodeHeader* getMthCodeH();
	vector<unsigned short>* getRegs(); 
	//hzhu begin 4/22/2012
	unsigned int getMethodidx();
	void setMethod(Method*);
	Method* getMethod();
	//hzhu end 4/22/2012
	void set_label_name(string str);
	virtual string getLabel();
	virtual bool is_label();
	void setTries(Tries* tries);
	Tries* getTries();
	bool hasTries();
	DexOpcode getOpcode();
	Format getOpFormat();

	//hzhu begin 4/23/2012 for each register and for each operand, the order is operand based. for example for each registers inside the first operand, then second operand
	map<unsigned int, ClassDefInfo*>* getTypeofVar();
	
	
	//void setFieldIdx(unsigned int);
	//unsigned int getFieldIdx();
	//hzhu end 4/23/2012
	//haiyan added 7.15
	void setSelfDefinedM();
	bool isSelfDefinedM();
	bool hasExceptions();
	void setExceptionTypes(vector<string>*);
	vector<string>* getExceptionTypes();
	
	//haiyan end 7.15
	bool isJump();
	bool isCondition();
	bool isReturn();
	virtual bool onlyTrylabels();
	virtual bool onlyTryorCatch();
	virtual bool onlyCatches();
	bool realTryStart();
	bool realTryEnd();
	bool insideTryBlock(unsigned int addr);
	int IdxinsideRealTry(unsigned int addr);
	void setOwnTry(unsigned int addr);
	Tries* getOwnTry();
	vector<il::type*>* getCatchExceptionType(unsigned int addr, DexFileReader*, map<string,ClassDefInfo*>*);
	vector<unsigned int>* getCatchAddrs(unsigned int);
	//haiyan added 8.22
	void setInUseRegs(vector<unsigned short>*);
	vector<unsigned short>* getInUseRegs() {return _reg_in_use;}
	//bool getNeedCast(){return this->_need_cast;}
	//void setNeedCast(){this->_need_cast = true;}
	//haiyan ended 8.22
	void setRegisterReused() {this->_has_register_reused = true;}
	bool hasRegisterReused(){return this->_has_register_reused;}
	unsigned int getReusedRegIdx(){return this->_reused_reg_idx;}
	void setReusedRegIdx(unsigned short index){this->_reused_reg_idx = index;}
	bool isNewException();
	
	vector<unsigned short>* getInDefRegs() {setInDefRegs(); return this->_reg_in_def;}
	void setInDefRegs();

	//haiyan added 10.4
	virtual void setCheckCastClassDef(ClassDefInfo* classdef){this->_check_cast_classdef = classdef;}
	virtual ClassDefInfo* getCheckCastClassDef(){ return this->_check_cast_classdef;}
	//haiyan ended 10.4

	//12/16
	void printVarsClassDef();
	//12/16
private:
	CodeHeader* _mtd_codeh; // point to the method code header
	Format _op_format;
	DexOpcode _opcode;
	string _operator;
	vector<unsigned short> * _regs;
	string _operand1;
	string _operand2;
	string _operand3;
	Data* _data;
	Switch* _switch;
	//hzhu begin 4/22/2012
	unsigned int _method_idx;
	Method* _method;
	//hzhu end 4/22/2012
	
	//hzhu begin 4/22/2012
	map<unsigned int,ClassDefInfo*>* _var_type;
	//hzhu end 4/22/2012
	//string _label;
	Tries* _tries;
	//unsigned int _field_idx;
	//haiyan added 7.15
	bool _is_fcall;
	bool _is_sd;
	vector<string>* _expt_t;
	bool _has_exception;
	//haiyan end 7.15
	vector<il::type*>* _catch_exception_types;
	vector<unsigned int>* _catch_addrs;
	Tries* _own_try;
	//haiyan added 8.22
	vector<unsigned short>* _reg_in_use;
	bool _need_cast;
	//haiyan ended 8.22
	bool _has_register_reused;
	unsigned short _reused_reg_idx;
	vector<unsigned short>* _reg_in_def;
protected:
	ClassDefInfo* _check_cast_classdef;
};
#endif 
