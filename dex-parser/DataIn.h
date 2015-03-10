#ifndef  _DATAIN_H
#define  _DATAIN_H
#include<stack>
#include<string>
#include<iostream>
#include<fstream>
#include<sys/types.h>
#include<sys/stat.h>
#include<iomanip>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

using namespace std;


class DataIn {
public:
	DataIn();
	~DataIn();
	int readDexFile(string filename);
	int getCurrentPosition();
	void move(int absoffset);
	void pop();
	void push();
	void pushMove(int absoffset);
	char readByte();
	int readIntx();
	unsigned int readUIntx();
	int readShortx();
	unsigned int readUShortx();
	long readLongx();
	float readFloatx();
	double readDoublex();
	int bigEndian_readIntx();
	int bigEndian_readShortx();
	long bigEndian_readLongx();
	int readLeb128();
	unsigned int readULeb128();
	void skip(int bytes);
	char* getByteArray();
	int getStackSize();
	////////////////////////////////
	float readVariantFloatx(unsigned int bytes);
	double readVariantDoublex(unsigned int bytes);
	///////////////////////////////
private:
	//1/8
	void feedArray(char* a,unsigned int bytes, unsigned int total_size);
	//1/8
private:
	char* _barray;
	stack<int> _stack;
	int _pos;
};

#endif
