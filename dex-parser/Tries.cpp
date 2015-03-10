#include "Tries.h"

Tries::Tries(unsigned int beginaddr, unsigned int endaddr, vector<Handler*>* handlerlist, bool has_finally)
{
	_begin_addr = beginaddr;
	_end_addr = endaddr;
	_handler_list = handlerlist;
	_has_finally = has_finally;
	_has_excep_processed = false;
}

Tries::~Tries()
{
	//3/6/2013
	for(unsigned int i = 0; i < _handler_list->size(); i++){
		delete _handler_list->at(i);
	}
	delete _handler_list;
}

unsigned int Tries::getStartAddr()
{
	return _begin_addr;
}

unsigned int Tries::getEndAddr()
{
	return _end_addr;
}

vector<Handler*>* Tries::getHandlers()
{	
	return _handler_list;
}

bool Tries::hasHandlers()
{
	return (_handler_list != NULL);
}

bool Tries::hasFinally()
{
    return _has_finally;
}

void Tries::setExceptionProcessed()
{
    _has_excep_processed = true;
}

bool Tries::getExceptionProcessed()
{
    return _has_excep_processed;
}

void Tries::setETypeCheckLabel(sail::Label* l)
{
    _e_t_check_label = l;
}
sail::Label* Tries::getETypeCheckLabel()
{
    return _e_t_check_label;
}
