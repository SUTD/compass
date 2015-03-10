#ifndef _CLASSDEF_H
#define _CLASSDEF_H
#include "Field.h"
#include "Method.h"
#include <vector>
#include <map>
using namespace std;



class Method;

class ClassDefInfo
{
public:
	ClassDefInfo(string name, string accflag = "", string super = "", string filename = "", vector<Field*>* field = NULL, vector<Method*>* method = NULL, 
	vector<string>* interface = NULL ,map<int,int>* midx_codeoff = NULL); //if the type is primitive, it should be the default
	~ClassDefInfo();
	string getTypename();//class Type
	string getClassFlag();//public/private/qualifier_type
	string getSuperClass();
	string getFilename();
	bool isPrimitive();
	bool hasMethod();
        vector<Field*> * getField(); //provide all field information for this class
	//this method vector can save map info
	vector<Method*> * getMethod();//provide all method information for this class
	/*inferface list added for DebugInfo*/
	vector<string> * getInterface(); //provide all interface for this class
	//this is designed for other purpose, you do not need to use it when this is treated as type(class);
	map<int,int>* getMethod2Codeoff(); //methodid and the codeoffset mapping in this classDefInfo
	void setSuperClassIdx(int s_id){this->_super_class_idx = s_id;}
	int getSuperClassIdx(){return this->_super_class_idx;}
	void setSuperClassf(bool);
	void setInterfacef(bool);
	bool hasSuperClass();
	bool hasInterface();
	void setTypeIdx(unsigned int typeidx){this->_typeidx = typeidx;}
	int getTypeIdx(){return this->_typeidx;}
	void setTotalOffset();
	unsigned int getTotalOffset(){setTotalOffset(); return this->_total_offset;}
	void printFields();
	
	//used by the faked field of base/super class in Lib
	void setLatestOff(unsigned int latestoff) {this->_latest_off = latestoff;}
	unsigned int getLatestOff() {return this->_latest_off;}
	void setFields(vector<Field*>* fields){this->_field = fields;}
	bool insideOriginalFields(string str);
	bool isDigits();
//	void buildInitFieldsIns();
//	vector<sail::Instruction*>* getInitFieldsIns(){return this->_init_fields_ins;}
	//1.8
	void setStaticFields(vector<Field*>* static_fields){this->_static_field = static_fields;}
	vector<Field*>* getStaticFields(){return this->_static_field;}
	//1.8
	
	//3/7/2013
	void setLibTypeFieldsUpdated(){this->_lib_type_fields_updated = true;}
	bool getLibTypeFieldsUpdateFlag() {return this->_lib_type_fields_updated;}
	//3/7/2013
private:
	string _type_name;
	string _class_acc; //derive from the value(0x1~0x20000) {uint type}
	string _super_class;
	bool _super_f;
	bool _inte_f;
	string _file_name;
	vector <Field*>* _field;
	vector <Method*>* _method;
	vector <string> *_interface;
        //this is designed for other purpose, you do not need to use it!
	map <int,int>* _method_codeoff;
	int  _typeidx;
	int _super_class_idx;
	unsigned int _total_offset;
	//used only of faked filds of lib classes
	unsigned int _latest_off;
//	vector<sail::Instruction*>* _init_fields_ins;
	//1/8
	vector<Field*>* _static_field;
	//1/8

	//3/7/2013
	bool _lib_type_fields_updated;
	//3/7/2013
};

#endif
