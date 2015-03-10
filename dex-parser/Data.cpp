#include "Data.h"
#include <string>
using namespace std;
Data::Data(unsigned short elementwidth, unsigned int size, vector<ArrayData>* data, short which, ClassDefInfo* type): _ele_width(elementwidth), _size(size), _data(data), _which(which), _classdef(type)
{
}

Data::~Data()
{
	delete _data;
}

unsigned short Data::getEleWidth()
{
	return _ele_width;
}

unsigned int Data::getSize()
{
	return _size;
}

vector<ArrayData>* Data::getData()
{
	return _data;
}

short Data::getWhich()
{
	return _which;
}

//hzhu added 4/29/2012
ClassDefInfo* Data::getType()
{
	return _classdef;
}
//hzhu end 4/29/2012
