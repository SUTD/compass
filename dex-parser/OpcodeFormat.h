#ifndef _OPCODE_FORMAT_H
#define _OPCODE_FORMAT_H
#include <iostream>
#include <stdlib.h>
#include "DexOpcode.h"
#include "ASMInstruction.h"
#include "CodeHeader.h"
#include "Format.h"
using namespace std;
class CodeHeader;


class OpcodeFormat
{

public:
	OpcodeFormat(CodeHeader* _codeheader, DexOpcode opcode);
	~OpcodeFormat();
	Format getFormat();
	void printFormat(Format f);
	int getSize(Format format);
	DexOpcode getOpcode();
private:
	CodeHeader* _codeheader;  // use its DataIn to read and jump
	DexOpcode _opcode;
	int _size;
};
#endif
