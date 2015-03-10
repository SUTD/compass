#include "DebugInfoReader.h"
DebugInfoReader::DebugInfoReader(CodeHeader* codeheader, string filename, map<unsigned int,unsigned int>* addressline, map<unsigned int,vector<RVT*>*>*rvt, vector<string>* paras):
						 _codeheader(codeheader),
						_filename(filename), 
						_address_line(addressline),
						_R_V_T(rvt),
						_paras(paras)

{}

DebugInfoReader::~DebugInfoReader()
{
	delete _codeheader;
	delete _address_line;
	delete _R_V_T;
	delete _paras;
}

string DebugInfoReader::getFileName()
{
	return _filename;
}

map<unsigned int,unsigned int>* DebugInfoReader:: getAddressLine()
{
	return _address_line;
}

//haiyan 8.28 changed rvt list to rvt map
map<unsigned int,vector< RVT*>*>* DebugInfoReader:: getRVTList()
{
	return _R_V_T;
}

//haiyan added 6.12
void DebugInfoReader::setRegBeginEndAddr(vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* reg_begin_end)
{
    _reg_begin_end_addr = reg_begin_end;
}

vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* DebugInfoReader::getRegBeginEndAddr()
{
    assert(_reg_begin_end_addr != NULL);
    return _reg_begin_end_addr;
}
//haiyan end 6.12
vector<string>* DebugInfoReader::getParasList()
{
    return _paras;
}
