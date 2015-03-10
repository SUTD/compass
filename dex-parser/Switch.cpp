#include "Switch.h"

Switch::Switch(unsigned short ident, int size, vector<unsigned int>*target): _size(size),_target(target)
{
	if(ident == 0x0100)
	{
		_packedswitch_flag = true;
	}
	else
		_packedswitch_flag = false;
	_classdef = new ClassDefInfo("Lable");
}

Switch::~Switch()
{
	delete _target;
}

unsigned short Switch::getSize()
{
	return _size;
}

vector<unsigned int>* Switch::getTarget()
{
	return _target;
}

bool Switch::isPackedSwitch()
{
	return _packedswitch_flag;
}

ClassDefInfo* Switch::getType()
{
	return _classdef;
}

PackedSwitch::PackedSwitch(unsigned short ident,int size,vector<unsigned int>* target, int firstkey):
								Switch(ident,size,target),
								_fir_key(firstkey)
{}

PackedSwitch::~PackedSwitch()
{
}


int PackedSwitch::getFirstKey()
{
	return _fir_key;
}

SparseSwitch::SparseSwitch(unsigned short ident, int size, vector<unsigned int>* target, vector<int>* key): Switch(ident,size,target), _keys(key)
{
}

SparseSwitch::~SparseSwitch()
{
	delete _keys;
}


vector<int>* SparseSwitch::getKeys()
{
	return _keys;
}
