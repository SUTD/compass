#ifndef _INSTRUCTIONPROCESS_H
#define _INSTRUCTIONPROCESS_H
#include "OpcodeFormat.h"
#include "ASMInstruction.h"
#include "DexOpcode.h"
#include "CodeHeader.h"
#include "DataIn.h"
#include <map>
#include <sstream>
#include "DexFileReader.h"
#include <stdlib.h>
#include <stdio.h>
#include "Int2Str.h"
#include "Switch.h"
#include <cmath>
#include <iostream>
#include "TypetoClassDef.h" //hzhu add 5/2
#include <assert.h>

using namespace std;
class InstructionProcess
{
public:	
	//InstructionProcess(CodeHeader* codeheader, Format format, DexOpcode opcode, unsigned int address_ins);
	InstructionProcess(DexFileReader*, CodeHeader*, Format, DexOpcode, unsigned int , map<unsigned int, ClassDefInfo*>*, map<unsigned int, ClassDefInfo*>*);
	void Init();
	~InstructionProcess();
	ASMInstruction* processAll();
	//10t format done

	ASMInstruction* processF10t(DataIn* in, DexOpcode opcode, unsigned int address);
	//10x format done
	ASMInstruction* processF10x(DataIn*in, DexOpcode opcode, unsigned int address);
	//11n format done
	ASMInstruction* processF11n(DataIn* in,DexOpcode opcode, unsigned int address);
	//for 11x format done
	string F11xoperand1(unsigned short);
	ASMInstruction* fillOperandandins_F11x(DexOpcode,string,unsigned short,vector<unsigned short>*);
	ASMInstruction* processF11x(DataIn* in,DexOpcode opcode, unsigned int address);
	//for 12x Format done
	string F12xoperand1(unsigned short,vector<unsigned short>*);
	string F12xoperand2(unsigned short,vector<unsigned short>*);
	//ASMInstruction* fillOperandandins_F12x(string ,unsigned short,vector<unsigned short>*);
	ASMInstruction* fillOperandandins_F12x(DexOpcode opcode,string ,unsigned short,vector<unsigned short>*);
	ASMInstruction* processF12x(DataIn* in,DexOpcode opcode, unsigned int address);
	//for 20t format
	ASMInstruction* processF20t(DataIn* in,DexOpcode opcode, unsigned int address);
	//for 21c format
	string F21cOperand1(DataIn* in,vector<unsigned short>*);
	string F21cOperand2_string(DataIn* in);
	string F21cOperand2_type(unsigned int);
	//hzhu added 4/24/2012
	unsigned int F21cOperand2_typeidx(DataIn*);
	//hzhu end 4/24/2012
	//hzhu added 4/29/2012
	unsigned int F21cOperand2_fieldidx(DataIn*);
	string F21cOperand2_field(unsigned int);
	void F21cGetOperandTypes(DataIn*, vector<unsigned short>*, map<unsigned int, ClassDefInfo*>*, unsigned int, string);
	//hzhu end 4/29/2012
	ASMInstruction* processF21c(DataIn* in,DexOpcode opcode, unsigned int address);
	
	//for 21h format
	string F21hoperand1(DataIn*,vector<unsigned short>*);
        string F21hoperand2_const(DataIn*);
	string F21hoperand2_const_wide(DataIn*);
	ASMInstruction* processF21h(DataIn* in,DexOpcode opcode, unsigned int address);
	//for 21s format
	string F21soperand1(DataIn*,vector<unsigned short>*);
	string F21soperand2(DataIn*);
	ASMInstruction* fillOperandandins_F21s(DexOpcode,string, DataIn*,vector<unsigned short>*);
	ASMInstruction* processF21s(DataIn* ,DexOpcode , unsigned int);
	//for 21t format
	string F21toperand1(unsigned short ,vector<unsigned short>*);
	string F21toperand2(DataIn* , unsigned int );
	ASMInstruction* fillOperandandins_F21t(DexOpcode ,string ,unsigned short , DataIn* , unsigned int ,vector<unsigned short>*);
	ASMInstruction* processF21t(DataIn* ,DexOpcode , unsigned int );
	//for 22b format
	string F22boperand1(DataIn*,vector<unsigned short>*);
	string F22boperand2(DataIn*,vector<unsigned short>*);
	string F22boperand3(DataIn*);
	ASMInstruction* fillOperandandins_F22b(DexOpcode,string, DataIn*,vector<unsigned short>*);
	ASMInstruction* processF22b(DataIn* in,DexOpcode opcode, unsigned int address);
	//for 22c format
	string F22coperand1(unsigned short,vector<unsigned short>*);
	string F22coperand2(unsigned short,vector<unsigned short>*);

	//hzhu add 4/25/2012
	unsigned int F22coperand3_typeidx(DataIn*);
	string F22coperand3_type(unsigned int);
	//hzhu end 4/25/2012
	
	//hzhu add 4/25/2012
	unsigned int F22coperand3_fieldidx(DataIn*);
	string F22coperand3_field(unsigned int);
	//hzhu end 4/25/2012

	ASMInstruction* processF22c(DataIn* in,DexOpcode opcode,unsigned int address);
//	ASMInstruction* processF22cs(DataIn* in,DexOpcode opcode,unsigned int address);
	//for 22s format 
	string F22soperand1(DataIn*,vector<unsigned short>*);
	string F22soperand2(DataIn*,unsigned int,vector<unsigned short>*);
	string F22soperand3(DataIn*);
	ASMInstruction* fillOperandandins_F22s(DexOpcode,string,vector<unsigned short>*, string, string, string);
	ASMInstruction* processF22s(DataIn* in,DexOpcode opcode,unsigned int address);
	//for 22t format
	string F22toperand1(DataIn*, unsigned short ,vector<unsigned short>*);
	string F22toperand2(DataIn*, unsigned short ,vector<unsigned short>*);
	string F22toperand3(DataIn*, unsigned int );
	ASMInstruction* fillOperandandins_F22t(DexOpcode,string , DataIn*, unsigned int ,unsigned short,vector<unsigned short>* );
	ASMInstruction* processF22t(DataIn* in,DexOpcode opcode,unsigned int address);

        //for 22x format
	string F22xoperand1(DataIn*,vector<unsigned short>*);
        string F22xoperand2(DataIn*,vector<unsigned short>*);
	ASMInstruction* processF22x(DataIn* in,DexOpcode opcode,unsigned int address);

	//for 23x format 	
	string F23xoperand1(DataIn*,vector<unsigned short>*);
	string F23xoperand2(DataIn*,vector<unsigned short>*);
	string F23xoperand3(DataIn*,vector<unsigned short>*);
	ASMInstruction* fillOperandandins_F23x(DexOpcode,string, DataIn*);
	ASMInstruction* processF23x(DataIn* in,DexOpcode opcode,unsigned int address);

	//for 30t format
	ASMInstruction* processF30t(DataIn* in,DexOpcode opcode,unsigned int address);
	//for 31c format
	ASMInstruction* processF31c(DataIn* in,DexOpcode opcode,unsigned int address);
	//for 31i format
	string F31iOperand1(unsigned short);
	string F31iOperand2(DataIn*, ClassDefInfo*);
	ASMInstruction* fillOperandandins_F31i(DexOpcode,string, unsigned short, DataIn*);
	ASMInstruction* processF31i(DataIn* in,DexOpcode opcode,unsigned int address);
	
	//for 31t format
	ASMInstruction* fillInstructiondata_F31t(DexOpcode,DataIn*, string, string,string,int, int,vector<unsigned short>*);
	ASMInstruction* processF31t(DataIn* in,DexOpcode opcode,unsigned int address);

	//ASMInstruction* processF32s(DataIn* in,DexOpcode opcode,unsigned int address);
	//for 32x format
	ASMInstruction* processF32x(DataIn* in,DexOpcode opcode,unsigned int address);
	
	//for 33x format
	ASMInstruction* processF33x(DataIn* in,DexOpcode opcode,unsigned int address);
	//for 35c format
	string F35cOperand1(DataIn* in,vector<unsigned short>*);
	unsigned int F35cOperand2_typeidx(DataIn*, int);
	string F35cOperand2_type( unsigned int);
	unsigned int F35cOperand2_methodidx(DataIn* in, int);
	string F35cOperand2_method(unsigned int);
	//hzhu begin 4/22/2012
	unsigned int F35c_method(DataIn* in, int pos);
	//hzhu end 4/22/2012

	//hzhu begin 4/24/2012
	void F35c_processTypes(DataIn*,map<unsigned int, ClassDefInfo*>*, vector<unsigned short>*, Method* ,DexOpcode,vector<unsigned short>*);
	//hzhu end 4/24/2012

	//hzhu begin 4/24/2012
	void F35c_SetReturn(Method*, ClassDefInfo*);
	//hzhu end 4/24/2012
	
	
	ASMInstruction* processF35c(DataIn* in,DexOpcode opcode,unsigned int address);
	//for 3rc format
	string F3rcOperand1(DataIn*, unsigned short,vector<unsigned short>*);
	unsigned int F3rcOperand2_typeidx(DataIn* , unsigned int);
	string F3rcOperand2_type(unsigned int);
	unsigned int F3rcOperand2_methodidx(DataIn*, unsigned int);
	string F3rcOperand2_method(unsigned int); //added by hzhu 4/30/2012
	void F3rc_processTypes(DataIn* in, map<unsigned int, ClassDefInfo*>* ins_type, vector<unsigned short>*allregs, Method* method, DexOpcode opcode,vector<unsigned short>* in_use); //hzhu added 4/30/2012
	ASMInstruction* processF3rc(DataIn* in,DexOpcode opcode,unsigned int address);
	void F3rc_SetReturn(Method*, ClassDefInfo*);
//	ASMInstruction* processF3rmi(DataIn* in,DexOpcode opcode,unsigned int address);
//	ASMInstruction* processF3rms(DataIn* in,DexOpcode opcode,unsigned int address);

	//for 40sc format
//	ASMInstruction* processF40sc(DataIn* in,DexOpcode opcode,unsigned int address);
	//for 51l format
	ASMInstruction* processF51l(DataIn* in,DexOpcode opcode,unsigned int address);
//	map<unsigned int, ASMInstruction*>* getAddressToInstructionMap();
	CodeHeader* getCodeH(); // added by hzhu 5.13
	
private:
	DexFileReader* _dfr;
	//DexFileReader* _dfr;
	CodeHeader* _codeheader;
	Format _format;
	DexOpcode _opcode;
	unsigned int _address_ins;
//	static map<unsigned int, ASMInstruction*>* _address_asminstruction;
//	static void buildAddressToInstructionMap(); // construct the map<add_ins, instruction>
	//hzhu added 4/23/2012
	map<unsigned int, ClassDefInfo*>* _var_type;
	map<unsigned int, ClassDefInfo*>* _declared_map;
	map<string, ClassDefInfo*>* _str2type;
	//hzhu end 4/23/2012
	//haiyan added 7.17
	ClassDefInfo* _invalid_type;
	bool in_use_flag;
};
#endif
