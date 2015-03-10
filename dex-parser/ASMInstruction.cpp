#include "ASMInstruction.h"
#define PRINT_TRIES_INFO false

ASMInstruction::ASMInstruction(CodeHeader* codeheader, Format op_format, DexOpcode opcode, string ope,map<unsigned int, ClassDefInfo*>* vartype, vector<unsigned short>* regs,string operand1, string operand2, string operand3, Data* data, Switch* swit, unsigned int methodidx, Method* method, Tries* tries)
{
	_mtd_codeh = codeheader;
	_op_format = op_format;
	_opcode = opcode;
	_operator = ope;
	//hzhu begin 4/23/2012
	_var_type = vartype;
	//hzhu end 4/23/2012

	_regs = regs;
	_operand1 = operand1;
	_operand2 = operand2;
	_operand3 = operand3;
	_data = data;
	_switch = swit;
	//hzhu begin 4/22/2012
	_method_idx = methodidx;
	_method = method;
	//hzhu end 4/22/2012
	_tries = tries;
	_is_sd = false;
	_expt_t = NULL;
	_has_exception = false;
	//haiyan added 8.6
	_catch_exception_types = new vector<il::type*>();
	_catch_addrs = new vector<unsigned int>();
	//haiyan added 8.22
	_reg_in_use = NULL;
	//haiyan ended 8.22
	_need_cast = false;
	_has_register_reused = false;

	_reg_in_def = new vector<unsigned short>;

	_check_cast_classdef = NULL;
}

ASMInstruction::~ASMInstruction()
{
	delete _data;
	delete _switch;
	delete _catch_exception_types;
	delete _catch_addrs;
	delete _reg_in_def;
	delete _regs;
	//3/6/2013
	delete _var_type;
	delete _reg_in_use;
}

//hzhu add 4/23/2012
string ASMInstruction::getOperator()
{
	return _operator;
}

//hzhu end 4/23/2012
string ASMInstruction::getOperand1(){
	return _operand1;
}

string ASMInstruction::getOperand2(){
	return _operand2;
}

string ASMInstruction::getOperand3()
{
	return _operand3;
}


CodeHeader* ASMInstruction::getMthCodeH()
{
	return _mtd_codeh;
}

string ASMInstruction::toString()
{
	string ins;
	string ins_hasdata;
	string ins_hasswitch;
	string try_str;
	vector<Handler*>* handler = NULL;
	vector<Handler*>::iterator handler_it;
	//cout << "Good 1! " <<endl;
	if(hasTries())
	{
		handler = _tries->getHandlers();
		try_str =  "\n";
		for(handler_it = handler->begin(); handler_it != handler->end(); handler_it ++)
		{
			if((*handler_it)->getTypename() != "")
				try_str = try_str + ".catch " + (*handler_it)->getTypename() + " {:try_start_" + Int2Str(_tries->getStartAddr()) + " .. :try_end_" + Int2Str( _tries->getEndAddr())+ "} :catch_" + Int2Str((*handler_it)->getCatchAddr()) + "\n"; 
			else
				try_str = try_str + ".catchall " + " {:try_start_" + Int2Str(_tries->getStartAddr()) + " .. :try_end_" + Int2Str( _tries->getEndAddr())+ "} :catchall_" + Int2Str((*handler_it)->getCatchAddr()) + "\n";
		}
	}

	if (_operand2 == "")
	{
		ins += _operator + " " +  _operand1;
	}
	else if (_operand3 == "")
	{
		ins += _operator + " " +  _operand1+", " + _operand2;
	}
	else
	{
		ins += _operator + " " +  _operand1+ ", " + _operand2 + ", " + _operand3; 
	}
	
	
	//hzhu begin 4/22/2012
	if(_method_idx!= -1)// _method_idx != -1;
	{
		
		MethodAddr* methodaddr = _method->getMethodAddr();
		if(methodaddr!= NULL)
			ins += "[" + Int2Str(methodaddr->getBeginAddr()) + ", " + Int2Str(methodaddr->getEndAddr()) + "]";
	}
	//hzhu end 4/22/2012
	
	//hzhu begin 4/23/2012
	if(_var_type!= NULL)
	{
		ins += " ==>Types::::  ";
		map<unsigned int, ClassDefInfo*>::iterator it;
		for(it = _var_type->begin(); it != _var_type->end(); it++)
		{
			if(it->first!= 1000) //not constant
			{
				if(it->second != NULL)
				{
					ins += "v"+ Int2Str(it->first)+":" + it->second->getTypename() + "; ";
				}
			}
			else if(_operand2 == "") //constant
			{
				if(_var_type->count(1000)>0)
					ins += _operand1 + ":" + (*_var_type)[1000]->getTypename()+ "; ";
			}
			else
			{
				if((_operand3 != "")&(_var_type->count(1000)>0))
				{
					ins += _operand3 + ":" + (*_var_type)[1000]->getTypename()+ "; ";
				}
				else if(_var_type->count(1000)>0)
					ins += _operand2 + ":" + (*_var_type)[1000]->getTypename()+"; ";

			}
		}
	}
	//hzhu end 4/23/2012
	
	if(hasData())
	{
		int ele_width = _data->getEleWidth();
                int size = _data->getSize();
                vector<ArrayData>* realdata = _data->getData();
		short which = _data->getWhich(); // get which type
                vector<ArrayData>::iterator it;
                ins_hasdata = ins + "\n" + "element type: " + _data->getType()->getTypename()+ "\n"+ "element width: " + Int2Str(ele_width) + "\n" + "size: " + Int2Str(size) + "\n" + "data:" + "\n";
                for(it = realdata->begin(); it < realdata->end(); it++)
                {
			if (which == ARRAY8)
                        	ins_hasdata += (Int2Str((*it)._uint8_data))+"\n";
			else if (ARRAY16 == which)
                        	ins_hasdata += (Int2Str((*it)._uint16_data))+"\n";
			else if (ARRAY32 == which)
                        	ins_hasdata += (Int2Str((*it)._uint32_data))+"\n";
			else if (ARRAY64 == which)
                        	ins_hasdata += (Int2Str((*it)._uint64_data))+"\n";
                }
                return ins_hasdata + try_str;
	 }
	if(hasSwitch())
	{
		unsigned short size = _switch->getSize();
		vector<unsigned int>* target = _switch->getTarget();
		ins_hasswitch = ins + "\n" + "Branch Type: " + _switch->getType()->getTypename()+ "\n"+ "size: " + Int2Str(size) + "\n";
		// it is the packedSwitch derivative
		if(_switch->isPackedSwitch())
		{
			//int firstkey = _switch->getFirstKey();
			int firstkey = ((PackedSwitch*)_switch)->getFirstKey();
			ins_hasswitch = ins_hasswitch + "firstkey: " + Int2Str(firstkey) + "\n";
			
		}
		else // sparse-switch derivative
		{
			ins_hasswitch = ins_hasswitch + "keys:" + "\n";			
			vector<int>* keys =((SparseSwitch*)_switch)->getKeys();
			int temp;
			string tempstr;
			for(vector<int>::iterator iter = keys->begin(); iter < keys->end(); iter++)
			{
				temp = (*iter);
				char buffer[50];
				if(temp < 0)
				{
					sprintf(buffer,"%.8x",-temp);
					tempstr = buffer;
					tempstr = "-" + tempstr;
			        }
				else
				{
					sprintf(buffer, "%.8x",temp);
					tempstr = buffer;
				}
				ins_hasswitch += tempstr + "\n";
			}
		}
		ins_hasswitch = ins_hasswitch + "branch: " + "\n";
		for(vector<unsigned int>::iterator it = target->begin(); it < target->end(); it++)
		{
			unsigned int temp = (*it);
			char buffer[50];
			string tempstr;
			sprintf(buffer,"%d",temp);
                        string tempstr2 = buffer;
			ins_hasswitch += tempstr2 + "\n";
			
		}
		return ins_hasswitch + try_str;
		
	}
	return ins + try_str;
}

Data* ASMInstruction::getData()
{
	return _data;
}



bool ASMInstruction::hasData()
{
	return (!(_data == NULL));
}

bool ASMInstruction::hasSwitch()
{
	return(!(_switch == NULL));
}

Switch* ASMInstruction::getSwitch()
{
	return _switch;
}

vector<unsigned short>* ASMInstruction::getRegs()
{
	return _regs;	
}

//hzhu begin 4/22/2012
unsigned int ASMInstruction::getMethodidx()
{
	return _method_idx;
}

void ASMInstruction::setMethod(Method* method)
{
	_method = method;
}

Method* ASMInstruction::getMethod()
{
	return _method;
}
//hzhu end 4/22/2012


//hzhu begin 4/23/2012

map<unsigned int, ClassDefInfo*>* ASMInstruction::getTypeofVar()
{
	return _var_type;
}
//hzhu end 4/23/2012

void ASMInstruction::printVarsClassDef(){
	assert(_var_type != NULL);
	map<unsigned int, ClassDefInfo*>::iterator it = _var_type->begin();
	for(; it != _var_type->end(); it++){
		cout << "virtual register :: " << it->first << endl;
		if(it->second != NULL)
			cout << "virtual register classdef :: " << it->second->getTypename() << endl;
		else
			cout << "virtual register classdef is NULL "<< endl;
		cout << " = = = = == = = " <<endl;
	}
}

string ASMInstruction::getLabel()
{
	return "";
}

void ASMInstruction::set_label_name(string str)
{
	cout << "Do nothing!" << endl;
}


bool ASMInstruction::is_label()
{
    return false;
}
void ASMInstruction::setTries(Tries* tries)
{
	if(tries != NULL)
		_tries = tries;
}

Tries* ASMInstruction::getTries()
{
	if (_tries != NULL)
		return _tries;
	return NULL;
}

bool ASMInstruction::hasTries()
{
	return (_tries != NULL);
}

DexOpcode ASMInstruction::getOpcode()
{
	return _opcode;
}

Format ASMInstruction::getOpFormat()
{	
	return _op_format;
}

/*
void ASMInstruction::setFieldIdx(unsigned int idx)
{
	_field_idx = idx;
}

unsigned int ASMInstruction::getFieldIdx()
{
	return _field_idx;
}
*/

void ASMInstruction::setSelfDefinedM()
{
    _is_sd = true;
}


bool ASMInstruction::isSelfDefinedM()
{
    return _is_sd;
}

void ASMInstruction::setExceptionTypes( vector<string>* e)
{
    _expt_t = e;
    _has_exception = true;
}

vector<string>* ASMInstruction::getExceptionTypes()
{
    return _expt_t;
}

bool ASMInstruction::hasExceptions()
{
    return _has_exception;
}

bool ASMInstruction::isJump()
{
	switch(_opcode)
	{
	    case OP_GOTO:
	    case OP_GOTO_16:
	    case OP_GOTO_32:
	    case OP_IF_EQ:
	    case OP_IF_NE:
	    case OP_IF_LT:
	    case OP_IF_GE:
	    case OP_IF_GT:
	    case OP_IF_LE:
	    case OP_IF_EQZ:
	    case OP_IF_NEZ:
	    case OP_IF_LTZ:
	    case OP_IF_GEZ:
	    case OP_IF_GTZ:
	    case OP_IF_LEZ:
	    case OP_PACKED_SWITCH:
	    case OP_SPARSE_SWITCH:
	    {
		return true;
	    }
	    break;
	    default:
		return false;
	}
}


bool ASMInstruction::isCondition()
{
	switch(_opcode)
	{
	    case OP_IF_EQ:
	    case OP_IF_NE:
	    case OP_IF_LT:
	    case OP_IF_GE:
	    case OP_IF_GT:
	    case OP_IF_LE:
	    case OP_IF_EQZ:
	    case OP_IF_NEZ:
	    case OP_IF_LTZ:
	    case OP_IF_GEZ:
	    case OP_IF_GTZ:
	    case OP_IF_LEZ:
	    case OP_PACKED_SWITCH:
	    case OP_SPARSE_SWITCH:
	    {
		return true;
	    }
	    break;
	    default:
		return false;
	}
}

bool ASMInstruction::isReturn()
{
    switch(_opcode)
    {
	case OP_RETURN:
	case OP_RETURN_VOID:
	case OP_RETURN_WIDE:
	case OP_RETURN_OBJECT:
	case OP_THROW:
	    return true;
	default:
	    return false;
	
    }
}
bool ASMInstruction::onlyTrylabels()
{
	return false;
/*
    if(_label == "")
       return false;
    string l = this->_label;
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
*/
}

bool ASMInstruction::onlyTryorCatch()
{
    return false;
}
bool ASMInstruction::onlyCatches()
{
    return false;
}

bool ASMInstruction::realTryStart()
{
    if(!(_mtd_codeh->hasRealTries()))
	return false;
    vector<vector<pair<unsigned int, unsigned int>>*>* r_t_addr = _mtd_codeh->getRealTryAddrs();
    for(unsigned int i = 0; i< r_t_addr->size(); i++)
    {
	for(unsigned int j = 0; j < r_t_addr->at(i)->size();j++)
	{
	    unsigned int addr = r_t_addr->at(i)->at(j).first;
	    string add_str = Int2Str(addr);
	    if ((is_label())&&(getLabel().find(add_str) != string::npos))
		return true;
	}
    }
    return false;

       
}

bool ASMInstruction::realTryEnd()
{
    if(!(_mtd_codeh->hasRealTries()))
	return false;
    map<unsigned int, ASMInstruction*>* ins_map = _mtd_codeh->getAdd2InsMap();
    map<unsigned int, ASMInstruction*>::iterator it = ins_map->begin();
    map<unsigned int, ASMInstruction*>::iterator it_find;
    for(; it != ins_map->end(); it++)
    {
	if(it->second == this)
	{
	    it_find = it;
	}
    }
    vector<vector<pair<unsigned int, unsigned int>>*>* r_t_addr =_mtd_codeh->getRealTryAddrs();
    if(it_find != ins_map->end()) //the next ins is label and whose addr satisfy...
    {
	for(unsigned int i = 0; i< r_t_addr->size(); i++)
	{
	    for(unsigned int j = 0; j < r_t_addr->at(i)->size();j++)
	    {
		unsigned int end_addr = r_t_addr->at(i)->at(j).second;
		if ((it_find)->first == end_addr)
		    return true;
	    }
	}
    }
    return false;
}

//to judge if the funcall inside the try{} block, here try block means real try block
bool ASMInstruction::insideTryBlock(unsigned int addr)
{
	bool flag = false;
    vector<vector<pair<unsigned int, unsigned int>>*>* realtryaddr = _mtd_codeh->getRealTryAddrs();
    for(unsigned int i = 0; i< realtryaddr->size(); i++)
    {
		vector<pair<unsigned int, unsigned int>>* cur_try_addr =
				realtryaddr->at(i);
		for (unsigned int j = 0; j < cur_try_addr->size(); j++) {
			pair<unsigned int, unsigned int> p = cur_try_addr->at(j);
			unsigned int s_a = p.first;
			if (PRINT_TRIES_INFO)
				cout << " ********** try_begin addr " << s_a << "********** "
						<< endl;
			unsigned int e_a = p.second;
			if (PRINT_TRIES_INFO)
				cout << " ********** try_end addr " << e_a << "********** "
						<< endl;
			if ((s_a <= addr) && (e_a >= addr)) {
				if (PRINT_TRIES_INFO)
					cout << "Yes, functioncall inside the real try block "
							<< endl;
				//return true;
				flag = true;
			}
		}
	}
	if (PRINT_TRIES_INFO) {
		if (flag == true)
			cout << addr << " is inside tries " << endl;
		else
			cout << addr << " is not inside tries " << endl;
	}
    //return false;
    return flag;
}

int ASMInstruction::IdxinsideRealTry(unsigned int addr)
{
	vector<vector<pair<unsigned int, unsigned int>>*>* realtryaddr =
			_mtd_codeh->getRealTryAddrs();
	for (unsigned int i = 0; i < realtryaddr->size(); i++) {
		vector<pair<unsigned int, unsigned int>>* cur_try_addr =
				realtryaddr->at(i);
		for (unsigned int j = 0; j < cur_try_addr->size(); j++) {
			pair<unsigned int, unsigned int> p = cur_try_addr->at(j);
			unsigned int s_a = p.first;
			unsigned int e_a = p.second;
			if ((s_a <= addr) && (e_a >= addr)) {
				return i;
			}
		}
	}
	return -1;
}

void ASMInstruction::setOwnTry(unsigned int addr)
{
    int idx = IdxinsideRealTry(addr);
    assert(idx != -1);
    //return _mtd_codeh->getRealTries()->at(idx);
    //use this addr to find the Tries inside the vector<Tries*>
    _own_try = _mtd_codeh->getRealTries()->at(idx);
}


Tries* ASMInstruction::getOwnTry()
{
    return _own_try;
}
vector<il::type*>* ASMInstruction::getCatchExceptionType(unsigned int addr, DexFileReader* dfr, map<string,ClassDefInfo*>* type_map)
{
    if(insideTryBlock(addr)) //decide which try block
    {
	if(_own_try->hasHandlers())
	{
	    vector<Handler*>* hand = _own_try->getHandlers();
	    for(unsigned int i = 0; i < hand->size(); i++)
	    {
		ClassDefInfo* cur_type = hand->at(i)->getType();
		//il::type* t = TypeMapping(0,dfr,cur_type, type_map);
		il::type* t = NULL;
		if(cur_type->getTypename() != "")
			t = getType(0,dfr,cur_type, type_map);
		//add update type if it is a pointer that point to record_type 8.11
		//updateTypeMapping(dfr,t, cur_type,type_map);
		//haiyan ended 8.11
		_catch_exception_types->push_back(t);
	    }
	}
    }
    return _catch_exception_types;
}

vector<unsigned int>* ASMInstruction::getCatchAddrs(unsigned int addr) // if called after
		{
	if (insideTryBlock(addr)) {
		if (_own_try->hasHandlers()) {
			vector<Handler*>* hand = _own_try->getHandlers();
			for (unsigned int i = 0; i < hand->size(); i++) {
				unsigned int cur_catch_addr = hand->at(i)->getCatchAddr();
				_catch_addrs->push_back(cur_catch_addr);
			}
		}
	}
	return _catch_addrs;
}

void ASMInstruction::setInUseRegs(vector<unsigned short>* in_use)
{
   _reg_in_use =   in_use;
}

void ASMInstruction::setInDefRegs(){
	//first we copy all registers
	if(_regs == NULL)
		return;
	///copy all registers
	for(unsigned int i = 0; i < _regs->size();i++)
			{
				unsigned int reg = this->_regs->at(i);
				this->_reg_in_def->push_back(reg);
				//this->_reg_in_def = this->_regs;
			}
	if((_reg_in_use == NULL)||(_reg_in_use->size() ==0)){
		return;
	}
	else{ //there are _reg_in_use, check if all regs for the instruction belong to the in_use;
		if(_reg_in_use->size() == _regs->size()){
			_reg_in_def->clear();
			return;
		}

		vector<unsigned short>::iterator it;
		for(unsigned int i = 0; i < _reg_in_use->size(); i++){
			it = find(this->_reg_in_def->begin(), this->_reg_in_def->end(), _reg_in_use->at(i));
			if(it != this->_reg_in_use->end()) //didn't find it, so it belongs to _reg_in_def;
				_reg_in_def->erase(it);
		}
		assert(_reg_in_use->size()+_reg_in_def->size() == _regs->size());
	}
}

//to check if the is throw new Exception() functioncall, which directly go to the throw exception;
bool ASMInstruction::isNewException()
{
	bool flag = false;
	if((this->_operator.find("invoke") != string::npos)&&(this->_operand2.find("Exception") != string::npos)&&(this->_operand2.find("init") != string::npos))
	{
		flag = true;
	}
	cout << "is New Exception::::::::" << flag << endl;
	return flag;
		
}
