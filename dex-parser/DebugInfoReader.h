#ifndef _DEBUGINFOREADER_H
#define _DEBUGINFOREADER_H
#include "DataIn.h"
#include "RVT.h"
#include "CodeHeader.h"
#include <map>
#include <vector>
#include <string>
#include <limits.h>


class CodeHeader;
class DebugInfoReader
{
public:
    //haiyan changed rvtlist to map 8.28
	//DebugInfoReader(CodeHeader* codeheader,string filename, map<unsigned int,unsigned int>* addressline, vector<RVT*>* rvt, vector<string>* paras);//this constructor should fill two maps
	DebugInfoReader(CodeHeader* codeheader,string filename, map<unsigned int,unsigned int>* addressline, map<unsigned int,vector<RVT*>*>* rvt, vector<string>* paras);//this constructor should fill two maps
	~DebugInfoReader();
	map<unsigned int, unsigned int>* getAddressLine();
	string getFileName();
	//8.28 changed rvtlist to rvt map
	//vector<RVT*>* getRVTList();
	map<unsigned int, vector<RVT*>*>* getRVTList();
	//map<int,String>* getOff_Ins();
	void setRegBeginEndAddr(vector<pair<unsigned int, pair<unsigned int, unsigned int>>>*);
	vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* getRegBeginEndAddr();
	vector<string>* getParasList();

private:
	CodeHeader* _codeheader; // include everything we need if we parse the _codeheader
	string _filename;
	map<unsigned int, unsigned int>* _address_line; 
	//map<int,string>* _instruoff_instruction;
	//haiyan 8.28 change rvt vector to map
	//vector<RVT*> * _R_V_T;
	map<unsigned int,vector<RVT*>*> * _R_V_T;
	vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* _reg_begin_end_addr;
	vector<string>* _paras;
}; 
#endif
