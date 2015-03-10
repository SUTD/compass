#include "Handler.h"

Handler::Handler(ClassDefInfo* type, unsigned int addr)
{
	_type = type;
	_type_name = _type->getTypename();
	_addr = addr;
}

Handler::~Handler()
{
}

ClassDefInfo* Handler::getType()
{
	return _type;
}

string Handler::getTypename()
{
	return _type_name;
}

unsigned int Handler::getCatchAddr()
{
	return _addr;
}

