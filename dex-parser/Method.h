#ifndef _METHOD_H
#define _METHOD_H

#include <string>
#include <iostream>
//hzhu add 4/20/2012
#include "MethodAddr.h"
#include "Int2Str.h"
#include "DataIn.h"
#include <vector> //hzhu add 4/23/2012
#include "sail.h"
#include "type.h"
#include "ClassDefInfo.h"
//hzhu add 4/20/2012
//#include "TypeMapping.h"

//#include "TypetoClassDef.h"
//#include "DexFileReader.h"
using namespace std;

class DexFileReader;
class ClassDefInfo;

class Method
{
public:
	//hzhu add 4/20/2012
	Method(DataIn* in, unsigned int methodid, const string name, const string classowner, unsigned int classowneridx, const string returnval, string* para, const int parasize,vector<unsigned int>*paratypeids, unsigned int returnidx);
	//hzhu add 4/20/2012
	~Method();
	string getName();
	string getClassOwner();
	//hzhu added 4/27/2012
	unsigned int getClassOwneridx();
	//hzhu end 4/27/2012
	string getReturnType();
	int getParaSize();
	string* getParas();
	string getDesc(); //used to get the signature of the method
	string toString();
	void setFlag(string flag); //added for DebugInfo
	string getFlag();
	//hzhu add 4/20/2012
	unsigned int getMethodIdx();
	void setMethodAddr(MethodAddr*);
	MethodAddr* getMethodAddr();
	//hzhu end
	//hzhu begin 4/22/2012
	DataIn* getDataIn();
	//hzhu end 4/22/2012
	
	//hzhu begin 4/23/2012
	vector<unsigned int>* getParaTypeIds();
	//hzhu end 4/23/2012

	//hzhu begin 4/23/2012
        unsigned int getReturnidx();
        //hzhu end 4/23/2012

	//haiyan added 8.10	
	void setVirtual();
	bool isVirtual();
	bool isStatic();
	bool isAbstract();
	//il::type* getClassOwnerType
	//    (DexFileReader*, map<string, ClassDefInfo*>*);
	il::type* getClassOwnerType(DexFileReader* dfr,map<string,ClassDefInfo*>* typemap);

private:
	void setAccFlag();
	//haiyan ended 8.10
	
	
	


private:
	//hzhu begin 4/22/2012
	DataIn* _datain;
	//hzhu end 4/22/2012
	//hzhu add 4/20/2012
	unsigned int _midx;
	//hzhu add 4/20/2012
	/*descriptor of the method, this will build after {@link #getDesc()}*/
	string _desc;

	/*name of the method*/
	string _name;
	
	/*class owner of the method*/
	string _class_owner;
	
	//hzhu added 4/27/2012
	unsigned int _class_owner_idx;
	//hzhu end 4/27/2012

	/*return type of the method*/
	string _return_type;

	
	/*parameters array*/
	string* _parameter;
	
	/*size of the parameters*/
	int _para_size;
	
	/*flag info needed for DebugInfo*/
	string _flag;
	
	//hzhu add 4/20/2012
        MethodAddr* _maddr;
        //hzhu add 4/20/2012
	
	//hzhu 4/23/2012
	vector <unsigned int> * _para_type_ids;
	//hzhu 4/23/2012
	
	//hzhu 4/23/2012
	unsigned int _return_idx;
	//hzhu 4/23/2012
	//haiyan added 8.10
	bool _is_virtual;
	bool _is_static;
	bool _is_abstract;
	//haiyan ended 8.10
};
#endif
