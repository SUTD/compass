#ifndef _SWITCH_H
#define _SWITCH_H
#include <vector>
#include "ClassDefInfo.h" //hzhu added for this
using namespace std;
class Switch
{
public:
	Switch(unsigned short ident, int size, vector<unsigned int>* target);
	virtual ~Switch();
	unsigned short getSize();
	vector<unsigned int>* getTarget();
	bool isPackedSwitch();
	ClassDefInfo* getType(); //hzhu added for it 4/29/2012 

private:
	unsigned short _size;
	vector<unsigned int>* _target;
	bool _packedswitch_flag;
	ClassDefInfo* _classdef; //hzhu added for it 4/29/2012
	
};

class PackedSwitch: public Switch
{
public:
	PackedSwitch(unsigned short ident, int size, vector<unsigned int>* target, int firstkey);
	~PackedSwitch();
	int getFirstKey();
	

private:
	int _fir_key;

};

class SparseSwitch: public Switch
{
public:
	SparseSwitch(unsigned short ident, int size, vector<unsigned int>* target, vector<int>* keys);
	~SparseSwitch();
	vector<int>* getKeys();

private:
	vector<int>* _keys;
};
#endif
