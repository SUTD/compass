#include "StringSplit.h"
void StringSplit::getSeperatedStrs()
{
    string cur_str = this->_s;
    size_t found = cur_str.find(this->_spc);
    while(found != string::npos)
    {
	string sub_str = cur_str.substr(0,found);
	unsigned int size = cur_str.length();
	_sp_strs->push_back(sub_str);
	cur_str = cur_str.substr(found+1,size-1);
	found = cur_str.find(this->_spc);
    }
    if(found == string::npos)
    {
	if(cur_str != "")
	    _sp_strs->push_back(cur_str);
	//return _sp_strs;
    }
}

void StringSplit::printSubStrs()
{
    for(unsigned int i = 0; i < _sp_strs->size(); i++)
    {
	cout << i << " ::substr " << _sp_strs->at(i) << endl;
    }
}

vector<string>* StringSplit::getSubStrs()
{
	getSeperatedStrs();
	return _sp_strs;
}
il::namespace_context StringSplit::makeNameSpace()
{
    getSeperatedStrs(); //set the string vector
    il::namespace_context* ns = new il::namespace_context();
    unsigned int k = 0;
    while(k < _sp_strs->size())
    {
	string inner_class_str = _sp_strs->at(k);
	ns = new il::namespace_context(inner_class_str,*ns);
	k++;
    }
    return *ns;
}
