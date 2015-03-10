#include"DebugInfo.h"

DebugInfo::DebugInfo(int line, string filename, vector<RVT*> * rvt): _line(line),_filename(filename),_R_V_T(rvt)
{
}

DebugInfo::~DebugInfo()
{
	//delete _reg_begin_end;
	delete _R_V_T;
}

int DebugInfo::getLine()
{
	return _line;
}

string DebugInfo::getFileName()
{
	return _filename;
}

vector<RVT*>* DebugInfo::getRVTList()
{
	return _R_V_T;
}


