#ifndef _TRIES_H
#define _TRIES_H
#include "Handler.h"
#include <vector>
#include "sail.h"
using namespace std;
class Handler;
class Tries
{
public:
	Tries(unsigned int, unsigned int, vector<Handler*>* handlerlist, bool has_finally = false);
	~Tries();
	unsigned int getStartAddr();
	unsigned int getEndAddr();
	vector<Handler*>* getHandlers();
	bool hasHandlers();
	//haiyan added
	bool hasFinally();
	void setExceptionProcessed();
	bool getExceptionProcessed();
	///////////////////
	void setETypeCheckLabel(sail::Label*);
	sail::Label* getETypeCheckLabel();
	/////////////////
	//end

private:
	unsigned int _begin_addr;
	unsigned int _end_addr;
	vector<Handler*>* _handler_list;
	//haiyan added
	bool _has_finally;
	bool _has_excep_processed;
	sail::Label* _e_t_check_label; //if _has_excep_processed, we can just directly jump to one of them depends on if it is a lib function call or not
	//haiyan end
};
#endif
