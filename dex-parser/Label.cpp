#include "Label.h"
Label::Label(CodeHeader* codeh, Format op_format, DexOpcode opcode,string ope, map<unsigned int, ClassDefInfo*>* vartype, vector<unsigned short>* regs, string operand1, string operand2, string operand3, Data* data, Switch* swi, unsigned int methodidx, Method* method, Tries* tries): ASMInstruction(codeh, op_format, opcode, ope,vartype, regs, operand1, operand2, operand3, data, swi, methodidx,method, tries)
{
	_label_name = "";
}

string Label::to_string()
{
	return _label_name;
}
        
void Label::set_label_name(string str)
{
	_label_name = str ;
}

string Label::getLabel()
{
	return _label_name;
}
       
bool Label::is_label()
{
    return true;
}

Label::~Label()
{
}

bool Label::onlyTrylabels()
{
    if(_label_name == "")
       return false;
    string l = this->_label_name;
    size_t  found;
    string sub_l;
    while(1)
    {
	found = l.find("&");
	if(found == string::npos) //only one label
	{
	    if(l.find("try") != string::npos)
		return true;
	    else
		return false;
	}
	else{
		if(l.substr(0,int(found)).find("try") == string::npos)
		    return false;
		else{
		    sub_l = l.substr(found+1,l.size()-int(found)-1);
		    l = sub_l;
		}
	    }
    }
}

bool Label::onlyTryorCatch()
{
    if(_label_name == "")
       return false;
    string l = this->_label_name;
    size_t  found;
    string sub_l;
    while(1)
    {
	found = l.find("&");
	if(found == string::npos) //only one label
	{
	    if((l.find("try") != string::npos)||(l.find("catch") != string::npos))
		return true;
	    else
		return false;
	}
	else{
		if((l.substr(0,int(found)).find("try") == string::npos)&&(l.substr(0,int(found)).find("catch") == string::npos))
		    return false;
		else{
		    sub_l = l.substr(found+1,l.size()-int(found)-1);
		    l = sub_l;
		}
	    }
    }
}
bool Label::onlyCatches()
{
    if(_label_name == "")
       return false;
    string l = this->_label_name;
    size_t  found;
    string sub_l;
    while(1)
    {
	found = l.find("&");
	if(found == string::npos) //only one label
	{
	    if(l.find("catch") != string::npos)
		return true;
	    else
		return false;
	}
	else{
		if(l.substr(0,int(found)).find("catch") == string::npos)
		    return false;
		else{
		    sub_l = l.substr(found+1,l.size()-int(found)-1);
		    l = sub_l;
		}
	    }
    }
}

