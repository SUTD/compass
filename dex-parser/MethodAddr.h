#ifndef _METHOD_ADDR_H
#define _METHOD_ADDR_H

class MethodAddr
{
public:
	MethodAddr(unsigned int beginaddr, unsigned int endaddr);
	~MethodAddr();
	unsigned int getBeginAddr();
	unsigned int getEndAddr();

private:
	unsigned int _begin_addr;
	unsigned int _end_addr;
};


#endif
