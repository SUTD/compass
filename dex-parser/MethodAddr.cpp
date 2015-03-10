#include "MethodAddr.h"

MethodAddr::MethodAddr(unsigned int beginaddr, unsigned int endaddr)
{
	_begin_addr = beginaddr;
	_end_addr = endaddr;
}

MethodAddr::~MethodAddr()
{
}

unsigned int MethodAddr::getBeginAddr()
{
	return _begin_addr;
}

unsigned int MethodAddr::getEndAddr()
{
	return _end_addr;
}




