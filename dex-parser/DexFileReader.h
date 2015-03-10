#ifndef _DEXFILEREADER_H
#define _DEXFILEREADER_H
#include <stdlib.h>
#include <string>
#include <iostream>
#include "DataIn.h"
#include "Field.h"
#include "Method.h"
#include "ClassDefInfo.h"
#include <map>
#include <iterator>
#include "CodeHeader.h"
#include "DebugInfoReader.h"
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "MethodAddr.h" //hzhu add 4/19/2012
#include <cmath>
#include "TypetoClassDef.h" //hzhu add 5/2/2012
#include "TypeMapping.h"
#include <fstream>
#include <stack>
#include "Int2Str.h"

#define EXCEPTION_CHECK false
#define NO_INDEX 0xffffffff
#define GLOBAL_MAP true //make sure never turn it off, because ClassDefInfo's typeidx is affected!
#define ADDING_ANNOATION true
#define NEED_ILTYPE_MAP true
#define USING_CLINIT true //1/5
#define USING_STATIC_VALUE true //1/9
//
using namespace std;
class CodeHeader;
class DebugInfoReader;
class DexFileReader
{	

public:	
	DexFileReader( DataIn* in);
       //	DexFileReader();
	~DexFileReader();
	bool getLEndianF();
	DataIn* getDataIn();
	unsigned int getFileSize();
	unsigned int getStringIdsSize();
	int getStringIdsOff();
	unsigned int getTypeIdsSize();
	int getTypeIdsOff();
	unsigned int getProtoIdsSize();
	int getProtoIdsOff();
	unsigned int getFieldIdsSize();
	int getFieldIdsOff();
	unsigned int getMethodIdsSize();
	int getMethodIdsOff();
	unsigned int getClassDefSize();
	int getClassDefOff();
	unsigned int getDataSize();
	int getDataOff();
	string getString(unsigned int id);
	string getType(unsigned int id);
	bool isPrimitiveType(unsigned int id);
	Field* getField(unsigned int id);
       // Proto* getProto(int id);
	Method* getMethod(unsigned int method_id);
	///////////////////////////////////////////////
	map<int, int>* getClassToTypeidx(); // key(typeid)->value(classdef id) //this method make sure only doing one if map allready filled.
	string getAccessFlag(unsigned int value); //from the value to the "ACC_";

	//all codeHeader list in this dex file;
	static vector<CodeHeader*>* getCodeHeaderList(DexFileReader* dfr);
	 // one code head corresponding to one DebugInfoReader(contain map<line, offset> and vector<RVT*>*)
	DebugInfoReader* getDebugInfoReader(CodeHeader* codeheader);
	//hzhu add 4/19/2012
	void setMethodidx2AddrsM();
	map<unsigned int, MethodAddr*>* getMethodidx2AddrsM();
	//hzhu end 4/19/2012
	
	//hzhu added 5.24
	map<unsigned int, Method*>* getMIdx2Method();
	//12/16////////////////////////////////////////////
	map<unsigned int, Method*>* getSelfDefinedMethodMap() {return this->_selfd_method_map;}
	//12/16////////////////////////////////////////////
	//void addMethodThrowsMap();
	void readMethodThrows(unsigned int ann_off);
	map<unsigned int, vector<string> *>* getMethodThrows();
	//set<unsigned int>* getSelfDefinedMidSet();
	void printClassDefInfos();
	ofstream& getFunctionCallfstream();
	void addGlobalVars(string str, sail::Variable* var){(*_global_variables)[str] = var;}
	//map<string, sail::Variable*>* getGlobalVars(){return _global_variables;}
	
	int getGlobalVariableReg() {return _global_variables_starting_number ++;} // increase by 1 for each call
	void printClassDefFields();
	map<string, ClassDefInfo*>* getStr2ClassDef(){return _str_classdef;}
//private:
	bool insideClassDef(string str) {return (_str_classdef->count(str) >0);}
	ClassDefInfo* getClassDef(string str) {return (*_str_classdef)[str];} //only for those class defined by Users,use asfter calling insideClassDef()
	//12/12
//public:
	ClassDefInfo* getClassDefByName(string str){
		ClassDefInfo* cdf = NULL;
		if(insideClassDef(str)){
			cdf = getClassDef(str);
		}
		if(cdf == NULL){
			cdf = new ClassDefInfo(str);
			//12/20
			unsigned int typeidx = this->getTypeId(str);
			cdf->setTypeIdx(typeidx);
			//12/20
			(*_str_classdef)[str] = cdf;
		}
		return cdf;
	}
	//12/12
	//map<il::type*, ClassDefInfo*>* getilt2ClassDef(){return _ilt_2_classdef;}
	//void ilt2ClassDefAdd(il::type* ilt, ClassDefInfo* classdef);
	//void printilt2ClassDef();
	map<int,vector<il::type**>*>* getUnresolvedTypes() { return this->_unresolved_types;}
	map<int, il::type*>* getResolvedTypes() {return this->_resolved_types;}

	//bool isInheritedFrom(il::type* base, il::type* herited);
	map<unsigned int,ClassDefInfo*>* getClassDefIdx2Ele() {return this->_classdef_idx_ele;}
	bool selfDefinedClass(unsigned int t_idx);
	vector<unsigned int>* getUnresolvedLibBoundTypeIdx() {return this->_unresolved_lib_boundary_type_idx;} //the innerest base class for self defind Class reach for A->L1->L2 (A is self defined class, and L1, L2 is lib class, and L1 should be the Bound Class)
	vector<pair<unsigned int, unsigned int>>*initialBaseClassFiledsToDerivative(); //provide top_two_levels and build _lib_super_selfdefined_derivative
	void addBaseClassFieldsToDerivative(vector<pair<unsigned int, unsigned int>>* top_two_level );
	void updateDerivativeFields(ClassDefInfo* deriv_cdf, ClassDefInfo* base_cdf);
	void printClassDef();
	//bool foundvalue(vector<unsigned int>* vec, unsigned int value);
	ofstream& getFileStream(){return this->_file;}
	void PrintDerivativeSelfDefinedBaseLib();
	//this one is purposely for faked fileds lib class
	void setfakeLibClassDefWithFields();
	map<unsigned int, ClassDefInfo*>* getfakedLibClassDefWithFields(){return this->_faked_lib_classdef_with_fields;}
	bool insideFakedLibClassDefwithFields(string str);
	ClassDefInfo* getFakedLibClassDef(string str);
	//void buildTypeMapping();
	map<unsigned int, string>* getTypeMapping(){return  this->_type_map;}
	string getTypename(unsigned int typeidx){assert(this->_type_map->count(typeidx) > 0); return (*_type_map)[typeidx];}
	unsigned int getTypeIdx(string str);
	vector<pair<unsigned int, unsigned int>>*getLibsuperSelfDefinedDerivative(){return this-> _lib_super_selfdefined_derivative;}
	bool insideTopSelfDefinedClass(string str);
	map<string, sail::Variable*>* getGlobalVars(){return this->_global_vars;}
	vector<sail::Instruction*>* getGlobalVarIns(){return this->_initial_global_vars_ins;}

	//void setsailFunctionforGlobalVarsInit(sail::Function* f){this->_global_init_f = f; }
	//sail::Function* getGlobalVarsInitFunction(){return this->_global_init_f;}


//////////////////////////////////////////////////////////////////////////////////
//private:     
//	ClassDefInfo* getClassDefInfo(unsigned int classdefidx);

	//12/15
	void buildGlobalStr2ClassDefMap();
	//12/15

	//12/15
	void buildTypeMapping();

	/////////////////// 12/16
	void buildMethodMap();
	void PrintMethod();

	///////////////////12/16
	//12/20
	unsigned int getTypeId(string str){
		unsigned int typeidx = -1;
		if(_name_typeid->count(str) > 0)
			typeidx = (*_name_typeid)[str];
		return typeidx;
	}
	bool isSelfDefinedType(unsigned int type_idx);
	//12/20
	//1.3
	bool insideBuiltilType(ClassDefInfo* cdi){return _classdef_iltype->count(cdi) > 0;}
	il::type* getilType(ClassDefInfo* cdi) {return (*_classdef_iltype)[cdi];}
	void setilTypeBuilt(ClassDefInfo* cdi, il::type* il_type){(*_classdef_iltype)[cdi] = il_type;}
	//1.3

	//1/10
	map<string, string>* getStaticFieldTypes(){ return _all_static_fields_type;}
	map<string, string>* getStaticFieldValues() {return _all_static_fields_value;}
	string getStaticFieldValue(string str) {assert(_all_static_fields_value->count(str) > 0);return (*_all_static_fields_value)[str];}
	//1/10

	//1/20
	vector<sail::Annotation*>* getMethodFlowAnnotation(unsigned int methodid){
		if(_method_flow_annotations->count(methodid) > 0)
			return (*_method_flow_annotations)[methodid];
		else
			return NULL;
	}
	//1/20
private:
	ClassDefInfo* getClassDefInfo(unsigned int classdefidx);
	CodeHeader* getCodeHeader(ClassDefInfo* classdefinfo, unsigned int methodid);
	int readBytes(unsigned int size);
	//12/15
	//1/8/2013
	string readEncoded_value(unsigned int value_type, unsigned int value_arg,vector<vector<string>*>*, unsigned int length);
	//1/8/2013


private:
	static const char DEX_FILE_MAGIC[3];// = {0x64, 0x65,0x78} ;
	static const char VERSION_35[3];// = {0x30,0x33, 0x35};
        static const char VERSION_36[3];// = {0x30,0x33, 0x36};
	static const int ENDIAN_CONSTANT  = 0X12345678; //little endian;
	static const int REVERSE_ENDIAN_CONSTANT = 0x78563412;//big endian;
	//add for debug information
	static const int DBG_END_SEQUENCE=0;
	static const int DBG_ADVANCE_PC = 1;
   	static const int DBG_ADVANCE_LINE = 2;
    	static const int DBG_START_LOCAL = 3;
    	static const int DBG_START_LOCAL_EXTENDED = 4;
    	static const int DBG_END_LOCAL = 5;
   	static const int DBG_RESTART_LOCAL = 6;
    	static const int DBG_SET_PROLOGUE_END = 7;
    	static const int DBG_SET_EPILOGUE_BEGIN = 8;
    	static const int DBG_SET_FILE = 9;
    	static const int DBG_FIRST_SPECIAL = 10;
   	static const int DBG_LINE_BASE = -4;
    	static const int DBG_LINE_RANGE = 15;

    static const unsigned int VISIBILITY_RUNTIME = 1;
	static const unsigned int VISIBILITY_SYSTEM = 2;
	//// Value Formats
	static const unsigned int VALUE_BYTE = 0x00;
	static const unsigned int VALUE_SHORT = 0x02;
	static const unsigned int VALUE_CHAR = 0x03;
	static const unsigned int VALUE_INT = 0x04;
	static const unsigned int VALUE_LONG = 0x06;
	static const unsigned int VALUE_FLOAT = 0x10;
	static const unsigned int VALUE_DOUBLE = 0x11;
	static const unsigned int VALUE_STRING = 0x17;
	static const unsigned int VALUE_TYPE = 0x18;
	static const unsigned int VALUE_FIELD = 0x19;
	static const unsigned int VALUE_METHOD = 0x1a;
	static const unsigned int VALUE_ENUM = 0x1b;
	static const unsigned int VALUE_ARRAY = 0x1c;
	static const unsigned int VALUE_ANNOTATION = 0x1d;
	static const unsigned int VALUE_NULL = 0x1e;
	static const unsigned int VALUE_BOOLEAN = 0x1f;
	////
	static vector<CodeHeader*>* _codeheader_list;
	//hzhu add 4/19/2012
	map<unsigned int, MethodAddr*>* _midx_addrs_m;
	//hzhu end 4/19/2012
	
        //end for debug info
	
	DataIn* _datain;
	bool _little_endian_flag;
	unsigned int _file_size;
	
	unsigned int _string_ids_size;
	int _string_ids_off;
	
	unsigned int _type_ids_size;
	int _type_ids_off;
	
	unsigned int _proto_ids_size;
	int _proto_ids_off;
	
	unsigned int _field_ids_size;
	int _field_ids_off;

	
	unsigned int _method_ids_size;
	int _method_ids_off;
	
	unsigned int _class_def_size;
	int _class_def_off; 

	unsigned int _data_size;
	int _data_off;
	
	map<int, int>* _typeidx_classdefidx;
	map<int, int>* _classdefidx_typeidx;

	//hzhu added 5.24
	map<unsigned int, Method*>* _mid_method;
	//hzhu end 5.24
	//haiyan added for Throw mapping
	map<unsigned int, vector< string> * >* _m_throws;
	//set<unsigned int>* _selfd_mid;
	ofstream _fun_call;
	
	map<string,sail::Variable*>* _global_variables;
	int _global_variables_starting_number;
	map<string,ClassDefInfo*>* _str_classdef;
	map<il::type*, ClassDefInfo*>* _ilt_2_classdef;
	map<int, vector<il::type**>*>* _unresolved_types;
	map<int, il::type*>* _resolved_types;
	map<unsigned int, ClassDefInfo*>* _classdef_idx_ele;
	////those self defined class whose fields need to be update ?
	map<unsigned int, ClassDefInfo*>* _base_field_unresolved_selfclassdef;
	vector<unsigned int>* _unresolved_lib_boundary_type_idx;
	// pair<super_type_id, derivative_type_id>
	vector<pair<unsigned int, unsigned int>>* _lib_super_selfdefined_derivative; // this one is used by the collected lib class's filed information(whose offset is faked)
	vector<string>* _top_selfdefined_class;
	ofstream _file;
	map<unsigned int, ClassDefInfo*>* _faked_lib_classdef_with_fields; //not including primitive
	map<unsigned int, string>* _type_map; //type id -> str

	map<string, sail::Variable*>* _global_vars;
	vector<sail::Instruction*>* _initial_global_vars_ins;
	sail::Function* _global_init_f;

	map<CodeHeader*, DebugInfoReader*>* _codeh_debuginfo_map;
	map<unsigned int, Field*>* _f_map;
	map<unsigned int, Method*>* _selfd_method_map;
	//12/19
	map<unsigned int, CodeHeader*>* _mid_codeh; //one to one map
	//12/19
	map<string, unsigned int>* _name_typeid;
	//1.3
	/******
	 * the mapping from ClassDefInfo->il::type( which means that only primitive or pointer type has been kept, no record type individually used!)
	 */
	map<ClassDefInfo*, il::type*>* _classdef_iltype;
	//1.3

	//1.5
	map<string, sail::Variable*>* _global_variable;
	//1.5

	//1/9 for those static fields having value stored in dalvik;
	map<string, string>* _all_static_fields_type; //key field_name, value field type
	map<string, string>* _all_static_fields_value; // key field_name, value : value stored in dalvik
	//1/9

	map<unsigned int, vector<sail::Annotation*>*>* _method_flow_annotations;
};	

vector<sail::Annotation*>* makeAnnotation(vector<vector<string>*>*);
#endif
