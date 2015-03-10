#ifndef _DATA_H
#define _DATA_H
#include <vector>
#include <stdint.h>
#include "ClassDefInfo.h" // hzhu added 4/29/2012
using namespace std;

#define ARRAY8 1
#define ARRAY16 2
#define ARRAY32 3
#define ARRAY64 4

typedef union ArrayData_t
{
	unsigned char 	_uint8_data;
	unsigned short 	_uint16_data;
	unsigned int 	_uint32_data;
	unsigned long _uint64_data;
} ArrayData;

class Data
{
	
public:
	Data(unsigned short elementwidth, unsigned int size, vector<ArrayData>* data, short which=0, ClassDefInfo* type = NULL);
	~Data();
	unsigned short getEleWidth();
	unsigned int getSize();
	vector<ArrayData>* getData();
	short getWhich();
	ClassDefInfo* getType(); //hzhu added 4/29/2012
private:
	unsigned short _ele_width; // how many bytes per element in the struct
	unsigned int _size;
	vector<ArrayData>* _data;
	short	_which;
	ClassDefInfo* _classdef;//hzhu added 4/29/2012
};
#endif
