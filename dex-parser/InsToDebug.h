#ifndef _INSTODEBUG_H
#define _INSTODEBUG_H
#include<map>
#include<iterator>
#include "DexFileReader.h"
#include "ASMInstruction.h"
#include "DebugInfo.h"

using namespace std;
class
{
	public:

		InsToDebug();
		~InsToDebug();
		map<int,ASMInstruction>* buildInstrAddrMap();
		DebugInfo* retrieveDebugInfo(int address); //given the address of the instruction, return its debug information
	private:
		DexFileReader* _dfr;
		static map<int,ASMInstruction>* _ins_offset_map;		
};
#endif
