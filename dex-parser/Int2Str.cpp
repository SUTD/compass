#include "Int2Str.h"
string Int2Str(int value)
{
	stringstream ss;
	ss << value;
	return ss.str();
}
string Long2Str(long value){
	stringstream ss;
	ss << value;
	return ss.str();
}
string Float2Str(float value)
{
	stringstream ss;
	ss << value;
	return ss.str();
}

string Double2Str(double value)
{
	stringstream ss;
	ss << value;
	return ss.str();
}

int Str2Int(string str)
{   
    return atoi(str.c_str());
}

unsigned int zeroExtendedto4Bytes(unsigned int value, unsigned int readbytes)
{
	unsigned int retvalue;
	if(readbytes == 1){
				retvalue = value & 0x000000ff;
			}else if(readbytes == 2){
				retvalue = value & 0x0000ffff;
			}else if(readbytes == 3){
				retvalue = value & 0x00ffffff;
			}else{
				assert(readbytes == 4);
				retvalue = value && 0xffffffff;
			}
	return retvalue;
}

double Str2Double(string what){
	istringstream instr(what);
	double val;
	instr >> val;
	return val;
}


string splitFieldStr_getFieldClass(string str){
	int pos1 = str.find("->");
	return str.substr(0, pos1);
}
string splitFieldStr_getFieldName(string str){
	int pos1 = str.find("->");
	int pos2 = pos1 + 2;
	int pos3 = str.find(":");
	return  str.substr(pos2, pos3 - pos1 - 2);
}
string splitFieldStr_getFieldType(string str){
	int pos4 = str.find(" ");
	return str.substr(pos4 + 1, str.length() - pos4 - 1);
}
