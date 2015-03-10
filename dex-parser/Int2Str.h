#ifndef _INT2STRING_H
#define _INT2STRING_H

#include <sstream>
#include <assert.h>
using namespace std;

string Int2Str(int);
string Float2Str(float);
string Double2Str(double);
string Long2Str(long);

int Str2Int(string);
double Str2Double(string);

unsigned int zeroExtendedto4Bytes(unsigned int value, unsigned int readbytes);

string splitFieldStr_getFieldClass(string str);
string splitFieldStr_getFieldName(string str);
string splitFieldStr_getFieldType(string str);
#endif
