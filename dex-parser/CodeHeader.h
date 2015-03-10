#ifndef _CODE_HEADER_H
#define _CODE_HEADER_H
#include "DataIn.h"
#include "ClassDefInfo.h"
//hzhu 4/22/2012
#include "Method.h"

#include "Tries.h"
#include <set>
#include "ASMInstruction.h"
#include "DebugInfo.h"
#include "sail.h"
#include "TypeMapping.h"
#include <utility>
#include "string_const_exp.h"

#define MONITOR true
#define USING_TYPE_INFER true

#define CHECK_CLINIT_FIELD false

class Tries;
class ASMInstruction;
class DebugInfo;
class ASMInstruction;
class DexFileReader;
class CodeHeader
{
public:
	//hzhu 4/22/2012
	//CodeHeader(ClassDefInfo* classdef,DataIn* in,int offset, int total_reg, int debug_off, int ins_size, int instructionbegin);
	//CodeHeader(ClassDefInfo* classdef,DataIn* in,int offset, int total_reg, int debug_off, int ins_size, int instructionbegin, Method* method);
	CodeHeader(unsigned int mid,DexFileReader* dfr, ClassDefInfo* classdef,DataIn* in,int offset, int total_reg,int triessize, int debug_off, int ins_size, int instructionbegin, Method* method, vector<Tries*>* tries, vector<string>*, bool, bool);
	~CodeHeader();
	ClassDefInfo* getClassDefInfo();
	int getInstructionSize();
	int getTotalRegisterSize();
	int getDebugInfoOff();
	int getInstructionBeginOff();
	DataIn* getDataIn();
	int getOffset();
	//hzhu 4/22/2012
	unsigned int getMidx();
	Method* getMethod();
	//hzhu added 5/1/2012
	int getTriessize();
	vector<Tries*>* getTries();
	
	//hzhu added 5/3/2012
	//void setLabelList(vector<string>* labellist);
	void setLabelList(set<string>* labellist);
	//vector<string>* getLabelList();
	set<string>* getLabelList();
	void setAdd2InsMap(map<unsigned int, ASMInstruction*>*);
	map<unsigned int, ASMInstruction*>* getAdd2InsMap(); //hzhu added 5/4/2012
	
	//added by haiyan 5/8
	void msetIns2Debug(map<ASMInstruction*, DebugInfo*>*);
	map<ASMInstruction*, DebugInfo*>* mgetIns2Debug();
	//haiyan added 5/23


	/*****************************
	 * used to factory Variable;
	 ********************************/
	sail::Variable* getRegVarEle(unsigned int, il::type*,
			map<string, ClassDefInfo*>*, bool in_use,
			bool check_cast); //if exists, return old variable, otherwise new one
	

	void setLocalVarType(map<unsigned int, ClassDefInfo*>*);

	//map<unsigned int, ClassDefInfo*>* getLocalVarType();

	void setTypeMap(map<string,ClassDefInfo*>*);
	//haiyan ##


//	map<pair<unsigned int,il::type*>, sail::Variable*>* getRegVar();
	//haiyan ##
	//end by haiyan 5/8
	map<string,long>* getRegConst();
	void updateConst(string,long);
	void updateArrSize(string,long); //array register and its size
	map<string,long>* getArrSize();	
	//haiyan added 5.31
	void setSignature(il::function_type* signature);
	il::function_type* getSignature();
	vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* getRegAddrs();
	DexFileReader* getdfr();
	//haiyan ended 5.31
	//
	//haiyan added for throw exception declarations
	vector<string>* getThrowExceptions();
	bool isThrowExceptions();
	bool hasTries();
	map<unsigned int, unsigned int>* getArgs();
	vector<unsigned int>* getArgsReg();
	void printArgs();
	bool isArg(unsigned int);
	//haiyan add 7.18
	//map<unsigned int, pair<string, string>>* getMonitorcatches();
	vector<unsigned int>* getSyncObj();
	stack<pair<unsigned int, unsigned int>>* getSyncStack(); //index and register num
	//vector<pair<unsigned int, unsigned int>*>* getSyncAddr(); // same index hold the information of begin and end of the monitored block
	vector<vector<pair<unsigned int, unsigned int>>*>* getSyncAddr(); // same index hold the information of begin and end of the monitored block
	vector<pair<unsigned int, unsigned int>>* getSyncCatches(); //monitor enter/exit vx, x is the number of the register hold reference of object, string for example catchall_xxand empty string represent that only moniter-exit has corresponding catchall_xx
	void printSyncAddr();
	void printTryAddr();
	void printSyncCatches();
	void incrSyncIdx();
	int getSyncIdx();
	void resetSyncIdx();
	void setRealTries();
	bool hasRealTries();
	vector<Tries*>* getRealTries();
	void printRealTries();
	//haiyan end 7.18
	
	//haiyan start 8.3 for real try block
	void setRealTryAddrs();
	vector<vector<pair<unsigned int,unsigned int>>*>* getRealTryAddrs();
	void printRealTryAddrs();


	////////////haiyan for simplify
	void setPairLabelFirst(sail::Label*);
	void setPairLabelSecond(sail::Label*);
	sail::Label* getPairLabelFirst();
	sail::Label* getPairLabelSecond();
	unsigned int getThisReg(){return this->_this_reg;}
	ClassDefInfo* getThisType(){return this->_this_cdi;}
	void setThisReg(unsigned int i) { this->_this_reg = i;}
	void setThisType(ClassDefInfo* t){this->_this_cdi = t;}
	void setFirstArgu(unsigned int firstargu){this->_first_argu = firstargu;}
	unsigned int getFirstArgu(){return this->_first_argu;}
	//map<pair<string,il::type*>, sail::Variable*>* getUniqueVariables();_const_reg_insaddr
	map<unsigned int, unsigned int>* getConstRegToInsAddr(){ return _const_reg_insaddr;}
	void setSailIns(vector<sail::Instruction*>* sail_ins){this->_sail_ins = sail_ins;}
	int getLatestLine(){return this->_latest_line;}
	void setLatestLine(int line){this->_latest_line = line;}
	map<unsigned int, unsigned int>* getRegKillMap(){return this->_reg_kill_at_addr;}

	//haiyan added 8.29
	void setRealArgsSize(unsigned int real_args_size ) {_args_size = real_args_size;}
	unsigned int getRealArgsSize() {return this->_args_size;}
	//haiyan ended 8.29
	map<unsigned int, ClassDefInfo*>* getDeclaredVar() {return this->_declared_var;}
	sail::Variable* getThisPointerVar(){return this->_this_var;}
	void buildThisPointerVar();

	void setFieldInitIns(vector<sail::Instruction*>* field_init_ins){this->_field_init_ins = field_init_ins;}
	vector<sail::Instruction*>* getFieldInitIns(){return this->_field_init_ins;}
	void setFieldInitFlag(){this->_has_field_init = true;}
	bool getFieldInitFlag(){return this->_has_field_init;}
	vector<string>* getExistingFields() {return this->_init_fields_existing;}
	//bool hasImplicitThrows();
	
	bool isTryFinally() {return this->_try_finally;}

	//1/3 used to determine the order of clinits
	void setStaticFieldsInClinit(string str){ _static_fields_in_clinit->push_back(str);} //collect sget instrucitons in clinit

	bool shouldMovetosorted(map<int,CodeHeader*>& _unsorted, map<int,CodeHeader*>& _sorted) {
		bool ready = true;
		map<string,CodeHeader*> sorted_classnames;
		if(_sorted.size() != 0){
			for(map<int,CodeHeader*>::iterator it = _sorted.begin(); it != _sorted.end(); it++){
				string classname_sget = (*it).second->getClassDefInfo()->getTypename();
				sorted_classnames[classname_sget] = (*it).second;
				if(CHECK_CLINIT_FIELD)
					cout << "HZHU , sorted name is == " << classname_sget << endl;
			}
		}

		map<string, CodeHeader*> unsorted_classnames;
		if(_unsorted.size() != 0){
			for(map<int,CodeHeader*>::iterator it = _unsorted.begin(); it != _unsorted.end(); it++) {
				string classname_sget = (*it).second->getClassDefInfo()->getTypename();
				unsorted_classnames[classname_sget] = (*it).second;
				if(CHECK_CLINIT_FIELD)
				cout << "HZHU , unsorted name is == " << classname_sget << endl;
			}
		}

		string classname = this->_classdef->getTypename();
		if(CHECK_CLINIT_FIELD) {
			cout << " --" <<endl;
			cout << "HZHU :: classname is == " << classname << endl;
		}
		//to each sget instruction, check if it ready to move to sorted vector
		for(unsigned int i = 0; i < _static_fields_in_clinit->size(); i++) {
			string check_sget_class = _static_fields_in_clinit->at(i);
			if(CHECK_CLINIT_FIELD)
				cout << "check static fields contain class: " << check_sget_class << endl;
			if(check_sget_class != classname){ //not self
				if((!(sorted_classnames.count(check_sget_class) > 0)) && (unsorted_classnames.count(check_sget_class) > 0)){//not depends on the class on sorted list or the method itself doesn't have a clinit method;
						ready = false;
				}
			}
		}
		if(CHECK_CLINIT_FIELD) {
			cout << "could remove " <<classname << " ? " << ready << endl;
			cout << "size of unsorted " << _unsorted.size() << endl;
			cout << "size of sorted " << _sorted.size() << endl;
		}
		return ready;
	}

	void set_clinit_flag() {_is_clinit = true;}
	bool is_clinit() {return _is_clinit;};

	//1/3


	sail::Variable* get_zero_variable(){
		string temp_variable_name = "t0";
		zero_variable = new sail::Variable(temp_variable_name, il::get_integer_type(),
		true, false, -1, false);


		return zero_variable;
	}


private:
	int idexInsideRealTry(unsigned int addr);
	Tries* getTry(unsigned int addr);
	
	//haiyan end 8.3

	//3/6/2013 delete ASMInstruction from this codeheader
	void delete_asmins() {
			map <unsigned int, ASMInstruction*>::iterator it = _add_ins->begin();
			while(it != _add_ins->end()){
				delete it->second;
				it ++;
			}

		}
	void delete_tries(){
		for(unsigned int i = 0; i < _tries->size(); i++){
			delete _tries->at(i);
		}
	}
	void delete_debuginf() {
		map <ASMInstruction*, DebugInfo*>::iterator it = _ins_debug->begin();
		while(it != _ins_debug->end()) {
			delete it->second;
			it ++;
		}
	}

	void delete_sync_addr_ele () {
		if(_sync_addr != NULL) {
			for(unsigned int i = 0; i < _sync_addr->size(); i++) {
				delete _sync_addr->at(i);
			}
		}
	}
private:
	//haiyan added 5.30
	map<string,long>* _reg_const; //first is the register hold the constant 
	map<string,long>* _arr_size; //array register and the size of the register  
	//haiyan ended 5.30
	DexFileReader* _dfr;
	ClassDefInfo* _classdef;
	DataIn* _in;
	int _offset;
	int _total_registers_size;

	//haiyan added 8.29
	int _args_size;
	//hayan ended 8.29
	
	//int _in_registers_size;
       	int _tries_size; //hzhu added 5/1/2012
	int _debug_off;
	int _instruction_size;
	int _instruction_begin_off;
	//hzhu 4/2/2012
	unsigned int _midx;
	Method* _method;
	vector<Tries*>* _tries;
	vector<Tries*>* _real_tries;
	//vector<string>* _label_list; //added by hzhu 5/3/2012
	set<string>* _label_list; //added by hzhu 5/3/2012
	map<unsigned int, ASMInstruction*>* _add_ins; //hzhu added 5/4/2012
	//hzhu added 5/23
	map<ASMInstruction*, DebugInfo*>* _ins_debug; //haiyan added 5/8

	map<unsigned int, ClassDefInfo*>* _local_var_type;
	//map<unsigned int, sail::Variable*>* _reg_var;
	//haiyan changed it 8.19 
	map<pair<unsigned int, il::type*>, sail::Variable*>* _reg_var;
	//map<unsigned int, pair<bool,sail::Variable*>>* _reg_var;
	map<string,ClassDefInfo*>* _t_map;
	il::function_type* _signature;
	vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* _reg_addrs;
	//haiyan added for throw exception declarations
	vector<string>* _throw_exceptions;
	bool _e_f;
	bool _has_tries;
	//haiyan ended
	map<unsigned int, unsigned int>* _args; 
	vector<unsigned int>* _argsReg;

	//haiyan added 7.18
	int _sync_idx;
	vector<unsigned int>* _sync_obj;
	stack<pair<unsigned int, unsigned int>>* _sync_stack; //monitor register state;
	//vector<pair<unsigned int, unsigned int>*>* _sync_addr; // same index hold the information of begin and end of the monitored block
	vector<vector<pair<unsigned int, unsigned int>>*>* _sync_addr; // same index hold the information of begin and end of the monitored block
	vector<pair<unsigned int, unsigned int>>* _sync_catches; //monitor enter/exit vx, x is the number of the register hold reference of object, string for example catchall_xxand empty string represent that only moniter-exit has corresponding catchall_xx
	//haiyan end 7.18
	
	//haiyan start 8.3
	bool _has_real_tries;
	vector<vector<pair<unsigned int, unsigned int>>*>* _real_try_addr;

	///haiyan added for method level one time LIB_HAS_EXCEPTION AND CUR_EXCEPTION ins, want to simplify
	pair<sail::Label*, sail::Label*>* _pair_label;
	//map<pair<unsigned, il::type*>, sail::Variable*> * _unique_variable;
	unsigned int _this_reg;
	sail::Variable* _this_var;
	ClassDefInfo* _this_cdi;
	unsigned int _first_argu;
	//haiyan added 8.22
	map<unsigned int, unsigned int>* _const_reg_insaddr;
	vector<sail::Instruction*>* _sail_ins;
	int _latest_line;
	//vector<pair<unsigned int, il::type*>>* _map_keys;
	//haiyan ended 8.22
	map<unsigned int, unsigned int>* _reg_kill_at_addr; //key is instruction addr, and value is the reg that at that address killed
	map<unsigned int, ClassDefInfo*>* _declared_var;
	vector<sail::Instruction*>* _field_init_ins;
	bool _has_field_init;
	
	vector<string>* _init_fields_existing;
	bool _try_finally;

	//1/5
	vector<string>* _static_fields_in_clinit;
	bool _is_clinit;
	//1/5

	sail::Variable* zero_variable;
};  
#endif
