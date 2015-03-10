#ifndef _INS2DEBUG_H
#define _INS2DEBUG_H
#include "DexFileReader.h"
#include "DebugInfo.h"
#include "DexOpcode.h"
#include "OpcodeFormat.h"
#include "InstructionProcess.h"
#include <vector>
#include "Label.h" //hzhu add 5/4/2012
#include "TypetoClassDef.h"
#include "Graph.h"
#include "GraphUtil.h"
#include "InstructionMapping.h"
//#include "InstructionParser.h"
using namespace std;

struct AddrItem{
	unsigned int 	__address;
	DexOpcode	 	__opcode;
	short 			__firstReg;
};

struct InitSimpleTypeValue{
	double _digitalvalue;
	char _charvalue;
	string _stringvalue;
};
class Ins2Debug
{
public:
	Ins2Debug(DataIn* in);
	//Ins2Debug();
	~Ins2Debug();
	//hzhu 4/22/2012_ins_addr
	DataIn* getDataIn();
	//hzhu end 4/22/2012
	DexFileReader* getDexFileReader();
	void setInsAddrMap();
	void updateInsAddrMap();
	//this is the one that can be called in sail
	//map<ASMInstruction*,unsigned int>* getInsAddrMap();
	map<unsigned int, ASMInstruction*>* getInsAddrMap();
	//given an address, which yields its corresponding instruction; provide for sail
	ASMInstruction* getInsFromAddr(unsigned int);
	DebugInfo* retrieveDebug(unsigned int addr); //addr is the address of instruction in the dex file 
	
	//hzhu added 4/23/2012
//private:
//	vector<ClassDefInfo*>* localParaType(Method*);
	//hzhu end 4/23/2012
	void buildIns2Debug(); //set global and method's level
	map<ASMInstruction*, DebugInfo*>* getIns2Debug();
	vector<CodeHeader*>* getCodeHeaderList();
	void setupAll();
	//added 5/15
	map<string, ClassDefInfo*>* getTypeMap();
	void processSyncBlock(CodeHeader*,string, unsigned int, ASMInstruction*,vector<unsigned int>*, stack<pair<unsigned int, unsigned int>>*, vector<vector<pair<unsigned int, unsigned int>>*>*);
	void processSyncAttach(CodeHeader*, ASMInstruction*, stack<pair<unsigned int, unsigned int>>*, vector<pair<unsigned int, unsigned int>>*);
	void extendSyncStructures();
//	void extendSyncAddrs(CodeHeader*, unsigned int idx, unsigned int monitor_enter_addr, unsigned int monitor_exit_addr, vector<vector<pair<unsigned int, unsigned int>>*>* addrs);
	void extendSyncAddr(CodeHeader* ch);
	void extendTryBlock(CodeHeader* ch);
	void processTryBlock(CodeHeader* ch);

	//void processLibFunctioncall();
	//void addThrowExceptionIns();
	sail::Variable* lib_has_exception_var();
	sail::Variable* type_of_exception_var();
	sail::Variable* cur_exception_var();
	sail::Variable* throw_exception_var();

	//map<unsigned int>* getexceptionMidxlist();
	//haiyan added 8.20
	
	//haiyan comment out begin 8.29 this step is not necessary for static method check
	//set<unsigned int>* getStaticMIdx();
	//void printStaticM();
	//haiyan comment out end 8.29 this step is not necessary for static method check
	void fakeFieldsForLibClass(ASMInstruction* asm_ins);
	map<unsigned int, string>* getTypeIdx2Name(){return this->_type_idx_name;}

	//void collectGlobalVarIns(ASMInstruction* asm_ins);
	//used by test in main_test
	//map<string,ASMInstruction*>* getGlobalVarIns(){return this->_global_var_ins;}
	//build global variable name->var map
	void buildGlobalVars();
	void buildInitGlobalVarsIns();
	void buildInitGlobalVarsMethod();
	//////////////////////1/4
	//void buildInitGlobalVarsMethodFromClinit();
	//////////////////////1/4

	vector<sail::Instruction*>* buildClassDefInitFieldsIns(CodeHeader* ch);
	sail::Store* buildClassDefFieldsIput(sail::Variable* this_var, sail::Variable* temp_var, unsigned int offset, string name, il::type* t);
	
	//1.5 print clinit methods
	void printClinits(){
		map<int, CodeHeader*>::iterator it = _unsorted_clinit_cd.begin();
		for(; it != _unsorted_clinit_cd.end(); it++){
			cout << (*it).first << ": " << (*it).second->getMethod()->toString()<<endl;
		}

	}

	/**************************************************************************************
	 * the following 4 are not used currently, because they are not being called right now!!
	 * we don't need to make sure that every static fields that used are initialized before(e.g. static field from library)
	 **************************************************************************************/
	//void buildInitInsForGlobalVariableFromLib(string alias, string str_type, il::type*, sail::Variable* global_var);
	//void setInitStaticForGlobalVarFromLib();
	//void init_static_forlib();
	//void processFakedArraySize(); //set it to private
	/////////////////////////////////////////////////////////////

	vector<CodeHeader*> getSortedClinitsVector(){return _sorted_clinit_cd_vector;}
	map<string, sail::Variable*>* getGlobalVars(){return _global_variables;}
	//sail::Function* getInitstatic0(){return _init_static_0;}
	//1.5

	//1/10
	void makeGlobalVariablesandInstructions(string field_name, string field_type, string field_value);
	//void makeInitializationInstructions(sail::Variable* var, string field_value);
	//sail::Function* makeInitStatic(vector<sail::Instruction*>*);
	//1/10

	//1/12
	//void setSailInitFun();
	sail::Function* getSailInitFun();
	//1/12
private:
	void updateDeclaredVarMap(unsigned int addr, map<unsigned int, vector<RVT*>*>* rvt,
				    map<unsigned int, unsigned int>* linemap, map<unsigned int, unsigned int>* endmap, 
				    map<unsigned int, ClassDefInfo*>* declaredvar, map<unsigned int, ClassDefInfo*>* backup,
				    vector<struct AddrItem>& insAddr, int insAddr_idx);
	//map<ASMInstruction*, unsigned int>* _ins_addr;
	void collectGlobalVarIns(ASMInstruction* asm_ins);
	vector<ClassDefInfo*>* localParaType(Method*);

	//1/11
	void ProcessClinitOrder();
	sail::Variable* makeGlobalVariablesForStaticFieldHasValue(string field_name, string field_type);
	void makeInitializationInstructionsforStaiticFieldHasValue(sail::Variable* var, string field_name, string field_type);
	void setSailInitFun();
	void buildSailInitFun();
	void CollectAllTypes();
	//1/11
	//hzhu 4/22/2012
	DataIn* _datain;
	//hzhu end 4/22/2012
	DexFileReader* _dfr;
	vector <CodeHeader*>* cd;
	map<unsigned int, ASMInstruction*>* _ins_addr;
	map<ASMInstruction*, DebugInfo*>* _ins_debuginfo;
	//added 5/15
	map<string, ClassDefInfo*>* _classdef_map;
	sail::Variable* _lib_has_exception;
	sail::Variable* _type_of_exception;
	sail::Variable* _cur_exception;
	sail::Variable* _throw_exception;
	//added 8.20
	//map<unsigned int, Method*>* _static_method_calls;
	//haiyan comment out begin 8.29 this step is not necessary for static method check
//	set<unsigned int>* _static_midx;
	map<unsigned int, string>* _type_idx_name;

	//haiyan comment out end 8.29 this step is not necessary for static method check
	//map<string,ASMInstruction*>* _global_var_ins;

	//haiyan adde for field initilization for classdef;
//	map<ClassDefInfo*, vector<sail::Instruction*>*>* _class_field_init_ins;

	//1.5
	map<int, CodeHeader*> _unsorted_clinit_cd;
	map<int, CodeHeader*> _sorted_clinit_cd;
	vector<CodeHeader*> _sorted_clinit_cd_vector;
	//map<string, ASMInstruction*> _sget_from_libs;
	//vector<string> _sget_from_libs;
	map<string, sail::Variable*>* _global_variables;
	//vector<sail::Instruction*>* _lib_global_initialization;
	//vector<vector<sail::Instruction*>*> _clinits_initialization;
	//sail::Function* _init_static_0;
	//1/11
	vector<sail::Instruction*>* _fun_initi_static_fields;
	sail::Function* _init_statics;
	//1.5
};
#endif
