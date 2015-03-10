#ifndef _DEBUGINFO_H
#define _DEBUGINFO_H
#include <vector>
#include "RVT.h"
using namespace std;
class DebugInfo
{
public:
	DebugInfo(int line, string filename, vector<RVT*>* rvt);
	~DebugInfo();
	int getLine();
	string getFileName();
	vector<RVT*>* getRVTList();
	vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* getRegBeginEndAddr();
	void setRegBeginEndAddr();

private:
	int _line;
	string _filename;
	vector<RVT*>* _R_V_T;
	//vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* _reg_begin_end;
	

};
#endif
