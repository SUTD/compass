#ifndef _STRING_SPLIT
#define _STRING_SPLIT
#include <string>
#include <vector>
#include <iostream>
#include "sail.h"

#define NAME_SPACE_PRINT false
using namespace std;

class StringSplit{
    public:
	StringSplit(string s, string spc)  {this->_s = s; this->_spc = spc; this->_sp_strs = new vector<string>();}
	~StringSplit() {delete _sp_strs;}
	il::namespace_context makeNameSpace();
	void printSubStrs();
	vector<string>* getSubStrs();

    private:
	string _s;
	string  _spc;
	vector<string>* _sp_strs;
	void getSeperatedStrs();
};

#endif
