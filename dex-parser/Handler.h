#ifndef _HANDLER_H
#define _HANDLER_H
#include "ClassDefInfo.h"
#include "CodeHeader.h"

class Handler
{
public:
	Handler(ClassDefInfo* type, unsigned int addr);
	~Handler();
	ClassDefInfo* getType();
	string getTypename();
	unsigned int getCatchAddr();
private:
	ClassDefInfo* _type;
	string _type_name;
	unsigned int _addr;
	
};
#endif
