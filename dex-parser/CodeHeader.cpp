#include "CodeHeader.h"
#define REG_CHECK false
#define CHECK_EXISTING_VARIABLES false
#define CHECK_MAKING_VAR false
#define CHECKME  true
#define GLOBAL_START_REG_NUM 30000

//hzhu 4/22/2012
CodeHeader::CodeHeader(unsigned int mid,DexFileReader* dfr, ClassDefInfo* classdef, DataIn* in,int offset, int total_reg, int triessize, int debug_off, int ins_size, int instructionbegin, Method* method, vector<Tries*>* tries, vector<string>* throwexceptions, bool ef, bool hastries)
{

	_midx = mid;
	_dfr = dfr;
	_classdef = classdef;
	_in = in;
	_offset = offset;
	_total_registers_size = total_reg;
	//cout << "total_register_size inside" <<  _total_registers_size <<" inside " << method->toString() <<endl;
	_tries_size = triessize; //hzhu add 5/1/2012
	_debug_off = debug_off;
	_instruction_size = ins_size;
	_instruction_begin_off = instructionbegin;
	//hzhu 4/22/2012
	_method = method;
	_tries = tries;
	_label_list = new set<string>(); //hzhu added
	_local_var_type = NULL;
	//hzhu 5.24 added
	//_reg_var = new map<unsigned int, sail::Variable*>();
	//haiyan changed 8.19
	_reg_var = new map<pair<unsigned int, il::type*>,sail::Variable*>();
	//hzhu end 5.24
	_reg_const = new map<string,long>();
	_arr_size = new map<string,long>();
	//haiyan added 5.30
	_signature = NULL;
	//haiyan end 5.30
	_throw_exceptions = throwexceptions;

	_e_f = ef;
	_has_tries = hastries;
	_args = new map<unsigned int, unsigned int>();
	_argsReg = new vector<unsigned int>();
	//haiyan added 7.18
	_sync_obj = new vector<unsigned int>();
	_sync_stack = new stack<pair<unsigned int, unsigned int>>();
	//_sync_addr = new vector<pair<unsigned int, unsigned int>*>();
	_sync_addr = new vector<vector<pair<unsigned int, unsigned int>>*>();
	_sync_catches = new vector<pair<unsigned int, unsigned int>>();
	_sync_idx = 0;
	//haiyan ended 7.18
	_real_tries = new vector<Tries*>();
	//haiyan added 8.3
	_real_try_addr = new vector<vector<pair<unsigned int, unsigned int>>*>();
	///for simplify haiyan
	_pair_label = new pair<sail::Label*, sail::Label*>();
	this->_this_reg = 5000;
	this->_this_cdi = NULL;
	_const_reg_insaddr = new map<unsigned int, unsigned int>();
	_latest_line = -1;
//	_unique_variable = new map<pair<string, il::type*>, sail::Variable*>(); //for example v13= foo(v13), <<v13, typea>, variable 1> and <<v14, typeb>, varaiable 2>
	//_map_keys = new vector<pair<unsigned int, il::type*>>();
	_reg_kill_at_addr = new map<unsigned int, unsigned int>();
	_declared_var = new map<unsigned int, ClassDefInfo*>();

	//buildThisPointerVar(); //which is used to build this pointer // _this_var = ?
	_has_field_init = false;

	_init_fields_existing = new vector<string>();
	
	_try_finally = false;

	//1/5
	_static_fields_in_clinit = new vector<string>();
	_is_clinit = false;
	//1/5
	//3/6/2013
	_t_map = NULL;

	zero_variable = NULL;
}

CodeHeader::~CodeHeader()
{
	delete _label_list;
	delete _local_var_type;
	delete _reg_var;
	delete _reg_const;
	delete _arr_size;
	delete _args;
	delete _argsReg;
	//haiyan 7.18 add
	delete _sync_obj;
	delete _sync_stack;
	//3/6/2013
	delete_sync_addr_ele();
	//3/6/2013
	delete _sync_addr;
	delete _sync_catches;
	delete _pair_label;
	delete _const_reg_insaddr;
	//delete _map_keys;
	delete _reg_kill_at_addr;
	delete _declared_var;
	//9.13
	delete _init_fields_existing;

	//3/6/2013
	delete_tries();
	delete _tries;
	delete _real_tries;

	//delete _t_map; //shared with Ins2Debug
	delete _reg_addrs;
	delete _static_fields_in_clinit;

	//delete _this_var; //used later

	delete_asmins(); //delete the ASMinstruction;
	delete _add_ins;
	delete_debuginf();
	delete _ins_debug;
}

ClassDefInfo* CodeHeader::getClassDefInfo()
{
	return _classdef;
}
int CodeHeader::getInstructionSize()
{
	return _instruction_size;
}
	
int CodeHeader::getTotalRegisterSize()
{
	return _total_registers_size;
}
	
int CodeHeader::getDebugInfoOff()
{
	return _debug_off;
}
	
int CodeHeader::getInstructionBeginOff()
{
	return _instruction_begin_off;
}

DataIn* CodeHeader::getDataIn()
{
	return _in;
}

int CodeHeader::getOffset()
{
	return _offset;
}

//hzhu 4/22/2012
Method* CodeHeader::getMethod()
{
	return _method;
}

int CodeHeader:: getTriessize()
{
	return _tries_size;
}

vector<Tries*>* CodeHeader::getTries()
{
	return _tries;
}


void CodeHeader::setLabelList(set<string>* labellist)
{
	_label_list = labellist;
}

//vector<string>* CodeHeader::getLabelList()
set<string>* CodeHeader::getLabelList()
{
	return _label_list;
}

//hzhu added 5/4
map<unsigned int, ASMInstruction*>* CodeHeader::getAdd2InsMap()
{
	return _add_ins;
}

void CodeHeader::setAdd2InsMap(map<unsigned int, ASMInstruction*>* addr2ins)
{
	_add_ins = addr2ins;
}




void CodeHeader::msetIns2Debug(map<ASMInstruction*, DebugInfo*>* tmp_ins_debug)
{
		_ins_debug = tmp_ins_debug;
}
 
map<ASMInstruction*, DebugInfo*>* CodeHeader::mgetIns2Debug()
{
	return _ins_debug;
}

//haiyan add 5.24
void CodeHeader::setLocalVarType(map<unsigned int, ClassDefInfo*>* localvartype)
{
    _local_var_type = localvartype;
}

//map<unsigned int, ClassDefInfo*>* CodeHeader::getLocalVarType()
//{
//    return _local_var_type;
//}



/**************************************************************
 * factory function of variable,
 * when using type-inference, don't do rename at all,
 * because renaming can't propagate,
 * would effect the reaching points;
 *****************************************************************/

sail::Variable* CodeHeader::getRegVarEle(unsigned int reg_idx, il::type* vartype,
		map<string, ClassDefInfo*>* typemap, bool in_use, bool check_cast){

	//assert(_t_map != NULL);

	if (CHECK_MAKING_VAR)
		cout << "In making variable for REG " << reg_idx << "  whose type is "
				<< vartype->to_string() << endl;
	assert(_reg_var != NULL);
	assert(_local_var_type != NULL);
	pair<unsigned int, il::type*> p(reg_idx, vartype);
	map<pair<unsigned int, il::type*>, sail::Variable*>::iterator it =
			_reg_var->begin();
	pair<unsigned int, il::type*> found_p;

	string reg_name = "v" + Int2Str(reg_idx);

	/////////////////////////////////////////////////
	//if it is a invalid type, we need do is nothing,
	//but making a same variable name, same type variable;
	

	//found
	if (_reg_var->count(p) > 0){
		if (CHECK_MAKING_VAR)
			cout << "Found inside the variable map " << endl;
		sail::Variable* var = (*_reg_var)[p];
		/*
		if (!in_use) {
			_map_keys->push_back(p);
		}*/
		if (0) {
			if (var->is_argument()) {
				cout << "yes, it is a parameter!! " << endl;
				cout << "arg_num : " << var->get_arg_number() << endl;
				cout << endl;
			}
		}
		assert(var != NULL);
		return var;
	} else{ // not found,cannot use the build variable(register number and type should be same in if, but here there is possible that register is the same, but type is not)

		//whether this register is used before?
		//cout << "didn't found ! so check if same register used before" << endl;
		//if use in_valid type (which means that USING_TYPE_INFER flag is true)
		bool foundreg = false;
		for (; it != _reg_var->end(); it++) {
			if (reg_idx == it->first.first) {
				foundreg = true;
				//cout << "reg number found " << endl;
				break;
			}
		}

		sail::Variable* newvar = NULL;

		// local register
		if (reg_idx < GLOBAL_START_REG_NUM){  //has register number,may be not temp variable
			//whether this register is a parameter
			if (isArg(reg_idx)){ //making an argument Variable;
				bool is_arg = true;

				unsigned int arg_num = (*_args)[reg_idx];

				if (0) {
					cout << endl;
					cout << "REG :: " << reg_idx << endl;
					cout << "yes, it is a parameter! NEED MORE WORK HERE!!"
							<< endl;
					cout << "arg_num : " << arg_num << endl;
				}
				newvar = new sail::Variable(reg_name, vartype, false, is_arg,
						arg_num, false);

				//if it is invalid_type, directly return without doing anything(e.g. save to map!)
				//is it possible that parameter is a invalid type??, anyway, let this statement stay
				if((USING_TYPE_INFER)&&(vartype->is_invalid_type())){
					return newvar;
				}

				//when USING_TYPE_INFER is true, I never rename variable, let boyang's infer do it
				if(!USING_TYPE_INFER){ //if didn't use TYPE-INFERENCE, DO THE RENAMING;

					assert(!vartype->is_invalid_type());
					//				if(foundreg && (!vartype->is_invalid_type()))
					//					newvar->set_var_name_as_temp_name();

					if(foundreg)
						newvar->set_var_name_as_temp_name();
				}

			} else //not a parameter and not found, making a local variable(not an argument)
			{
				//cout << "not a parameter, but not found! " << endl;
				//first give a warning, this may have issues, but it doesn't gurantee, because of branch!! code in dex is not totally sequential.
				if((in_use)&&(!vartype->is_invalid_type())){
					//cout << "in_use :: " << in_use << endl;
					//cout << "vartype ::" << vartype->to_string()<< endl;
					//cerr<< "CHECKME!!!!!!!!!!, variable " << reg_name << " in_use but not a invalid type!, in_use must being defined" << endl;
					//cout << "CHECKME!!!!!!!!!!, variable " << reg_name << " in_use but not a invalid type!, in_use must being defined" << endl;
					//assert(false);
				}


				newvar = new sail::Variable(reg_name, vartype, false, false, -1,
						false);
				//whenever USING_TYPE_INFER is false, do renaming, otherwise do nothing, let type-inference phase to do it;
				if(!USING_TYPE_INFER){

					assert(!vartype->is_invalid_type());

					if(foundreg)
						newvar->set_var_name_as_temp_name();

				}


				//bool generate_temp = false;
//
//				if(foundreg && (!(vartype->is_invalid_type()))){
//					newvar->set_var_name_as_temp_name();
//					generate_temp = true;
//				}
//
//
//
//				if ((foundreg)&&(!USING_TYPE_INFER)){
//					newvar->set_var_name_as_temp_name();
//					generate_temp = true;
//				}

				//Only check when USING_TYPE_INFER is false;
				if((in_use)&&(!USING_TYPE_INFER))
				{
					if(CHECKME)
						cout << "CHECK ME!!" << endl;
					cout << reg_idx <<" is not a parameter, but in_use first time, failed!!" << endl;
					assert(false);
				}

			}
			//newvar->set_name(reg_name);
		} else { //global
			newvar = new sail::Variable("", vartype);
			assert(reg_idx >= GLOBAL_START_REG_NUM);

			reg_name = "global_" + Int2Str(reg_idx);
			newvar = new sail::Variable(reg_name, vartype, false, false, -1,
						true);

		}

		if (CHECK_MAKING_VAR) {
			cout << "adding reg " << p.first << endl;
			cout << "adding type " << p.second->to_string() << endl;
			cout << "adding variable " << newvar->to_string() << endl;
		}


		(*_reg_var)[p] = newvar;

		assert(newvar != NULL);
		return newvar;
	}
}

////map<unsigned int, sail::Variable*>* CodeHeader::getRegVar()
//map<pair<unsigned int,il::type*>, sail::Variable*>* CodeHeader::getRegVar()
//{
//    return _reg_var;
//}

void CodeHeader::setTypeMap(map<string, ClassDefInfo*>* typemap)
{
    _t_map = typemap;
}
//haiyan added 5.30
map<string,long>* CodeHeader::getRegConst()
{
    return _reg_const;
}

void CodeHeader::updateConst(string arr_size_reg_n,long constant)
{
    (*_reg_const)[arr_size_reg_n] = constant;
}
void CodeHeader::updateArrSize(string arr,long arr_size)
{
    (*_arr_size)[arr] = arr_size;
}
map<string,long>* CodeHeader::getArrSize()
{
    return _arr_size;
}
//haiyan added 5.24

//haiyan zhu added 5.31
void CodeHeader::setSignature(il::function_type* signature)
{
    this->_signature = signature;
}
il::function_type* CodeHeader::getSignature()
{
    return this->_signature;
}
//haiyan zhu end 5.31
vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* CodeHeader::getRegAddrs()
{
	DebugInfoReader* dr = _dfr->getDebugInfoReader(this);
	if(dr == NULL)
	{
		this->_reg_addrs = NULL;
	}
	else
	{
		this->_reg_addrs = dr->getRegBeginEndAddr();
		if((this->_reg_addrs != NULL)&&(this->_reg_addrs->size() > 0))
		{
			//cout << "here is the thing !" << endl;
			//update the content inside the vector
			for(unsigned int i = 0; i < this->_reg_addrs->size(); i++)
			{
				//cout << "reg " << this->_reg_addrs->at(i).first << endl;
				this->_reg_addrs->at(i).second.first =
				_instruction_begin_off + this->_reg_addrs->at(i).second.first*2;
				//cout << "begin addr " << this->_reg_addrs->at(i).second.first << endl;
				if(this->_reg_addrs->at(i).second.second == UINT_MAX)
				this->_reg_addrs->at(i).second.second =
				_instruction_begin_off + _instruction_size*2;
				else
				this->_reg_addrs->at(i).second.second =
				_instruction_begin_off + this->_reg_addrs->at(i).second.second*2;

			}
		}
	}
	return this->_reg_addrs;
}

vector<string>* CodeHeader::getThrowExceptions()
{
    return  _throw_exceptions;
}

bool CodeHeader::isThrowExceptions()
{
    return _e_f;
}
bool CodeHeader::hasTries()
{
    return _has_tries;
}
map<unsigned int, unsigned int>* CodeHeader::getArgs()
{
    return _args;
}

bool CodeHeader::isArg(unsigned int reg)
{
    if(_args->count(reg)> 0)
	return true;
    return false;
}

vector<unsigned int>* CodeHeader::getArgsReg()
{
    return _argsReg;
}
unsigned int CodeHeader::getMidx()
{
    return _midx;
}

DexFileReader* CodeHeader::getdfr()
{
    return  _dfr;
}

vector<unsigned int>* CodeHeader::getSyncObj()
{
    return _sync_obj;
}
stack<pair<unsigned int, unsigned int>>* CodeHeader::getSyncStack()
{
    return _sync_stack;
}

vector<vector<pair<unsigned int, unsigned int>>*>* CodeHeader::getSyncAddr()
{
    return _sync_addr;
}
vector<pair<unsigned int, unsigned int>>* CodeHeader::getSyncCatches()
{
    return _sync_catches;
}
int CodeHeader::getSyncIdx()
{
    return _sync_idx;
}
void CodeHeader::incrSyncIdx()
{
    _sync_idx ++;
}

void CodeHeader::resetSyncIdx()
{
    _sync_idx = 0;
}

void CodeHeader::printSyncAddr()
{
    unsigned int size = _sync_addr->size();
    for(unsigned int i = 0; i < size ; i++)
    {
		vector<pair<unsigned int, unsigned int>>* inner_vector
		= _sync_addr->at(i);
		unsigned int inner_size = inner_vector->size();
		for (unsigned int j = 0; j < inner_size; j++) {
			cout << "[ " << inner_vector->at(j).first << ",";
			cout << inner_vector->at(j).second << "] ";
		}
	}
}

void CodeHeader::printSyncCatches()
{
    unsigned int size =_sync_catches->size();
    for(unsigned int i = 0; i< size; i++){
	cout << " ++++++++++++== monitor v" << _sync_obj->at(i) << endl;
	cout << " monitor enter catchall ==" << _sync_catches->at(i).first << endl;
	cout << " monitor exit catchall == " << _sync_catches->at(i).second << endl;
    }
}

void CodeHeader::printTryAddr()
{
	unsigned int size = _real_try_addr->size();
	cout << "size is " << size << endl;
	for (unsigned int i = 0; i < size; i++) {
		vector<pair<unsigned int, unsigned int>>* inner_vector =
				_real_try_addr->at(i);
		unsigned int inner_size = inner_vector->size();
		cout << "inner size " << inner_size << endl;
		for (unsigned int j = 0; j < inner_size; j++) {
			cout << "[ " << inner_vector->at(j).first << ",";
			cout << inner_vector->at(j).second << "] " << endl;
		}
	}
}

void CodeHeader::setRealTries()
{
	unsigned int size = _tries->size();
	vector<Handler*>* h = NULL;
	bool flag = false;
	//one case : only has try-finally then this is also a real try

	if (size == 1) {
		h = _tries->at(0)->getHandlers();
		if ((h->size() == 1) && (h->at(0)->getTypename() == "")) {
			_real_tries->push_back(_tries->at(0));
			flag = true;
			_try_finally = true;
		}
	}

	vector<unsigned int> lastCatchAddr; //= getCatchAddr
	bool init = false;
	bool equal = false;
	for (unsigned int i = 0; i < size; i++) {
		h = _tries->at(i)->getHandlers();
		if (!((h->size() == 1) && (h->at(0)->getTypename() == ""))) {
			//_real_tries->push_back(_tries->at(i));
			//cout << "real tries size " << _real_tries->size() << endl;
			//this one just used to try to fix one bug, may be not generalize
			if(!init){ //first time
				_real_tries->push_back(_tries->at(i));
				//cout << "real tries size " << _real_tries->size() << endl;
				flag = true;
				for(unsigned int j = 0; j < h->size(); j ++)
					lastCatchAddr.push_back(h->at(j)->getCatchAddr());
				init = true;
			}else{
				if (lastCatchAddr.size() == h->size()) { //the number of handlers equals to the number of handler addrs inside the handle
					for (unsigned int k = 0; k < h->size(); k++) {
						if (lastCatchAddr.at(k) == h->at(k)->getCatchAddr()) {
							equal = true;
							continue;
						} else {
							equal = false;
							break;
						}
					}
					if (!equal) {
						_real_tries->push_back(_tries->at(i));
						//cout << "real tries size " << _real_tries->size()
						//		<< endl;
					}
				}else{ //numbers are not the same indicate that not the same handles
					_real_tries->push_back(_tries->at(i));
					//cout << "real tries size " << _real_tries->size() << endl;
				}

			}
		}
	}
	_has_real_tries = flag;
}

bool CodeHeader::hasRealTries()
{
    return _has_real_tries;
}

vector<Tries*>* CodeHeader::getRealTries()
{
    return _real_tries;
}

void CodeHeader::printRealTries()
{
	cout << "Inside print RealTries === real try size " << _real_tries->size()
			<< endl;
	for (unsigned int i = 0; i < _real_tries->size(); i++) {
		cout << "******************************************" << endl;
		cout << "begin addr:  " << _real_tries->at(i)->getStartAddr() << endl;
		cout << "end addr:  " << _real_tries->at(i)->getEndAddr() << endl;
		for (unsigned int j = 0; j < _real_tries->at(i)->getHandlers()->size();
				j++) {
			cout << "handler type: "
					<< _real_tries->at(i)->getHandlers()->at(j)->getTypename()
					<< endl;
			cout << "handler addr: "
					<< _real_tries->at(i)->getHandlers()->at(j)->getCatchAddr()
					<< endl;
		}
	}
}


void CodeHeader::setRealTryAddrs()
{
    if(hasRealTries())
    {
		map<unsigned int, ASMInstruction*>::iterator it;
		//cout << "real_try_size " << _real_tries->size()<< endl;
		for (unsigned int i = 0; i < _real_tries->size(); i++) {
			unsigned int s_a = _real_tries->at(i)->getStartAddr();
			unsigned int e_a = _real_tries->at(i)->getEndAddr();
			it = _add_ins->find(e_a);
			e_a = (--it)->first;
			pair<unsigned int, unsigned int> p(s_a, e_a);
			vector<pair<unsigned int, unsigned int>>* cur_vector = new vector<
					pair<unsigned int, unsigned int>>();
			cur_vector->push_back(p);
			_real_try_addr->push_back(cur_vector);
			//_real_try_addr->at(i)->push_back(p);
		}
	}
}

vector<vector<pair<unsigned int, unsigned int>>*>* CodeHeader::getRealTryAddrs()
{
    return _real_try_addr;
}

void CodeHeader::printRealTryAddrs()
{
	cout << "size of real tries " << _real_try_addr->size() << endl;
	for (unsigned int i = 0; i < _real_try_addr->size(); i++) {
		vector<pair<unsigned int, unsigned int>>* cur_vec = _real_try_addr->at(
				i);
		for (unsigned int j = 0; j < cur_vec->size(); j++) {
			cout << "real_try_begin :::" << cur_vec->at(j).first << endl;
			cout << "real_try_end :::" << cur_vec->at(j).second << endl;
		}

	}
}

void CodeHeader::setPairLabelFirst(sail::Label* l)
{
    _pair_label->first = l;
}
void CodeHeader::setPairLabelSecond(sail::Label* l)
{
    _pair_label->second = l;
}
sail::Label* CodeHeader::getPairLabelFirst()
{
    return _pair_label->first;
}
sail::Label* CodeHeader::getPairLabelSecond()
{
    return _pair_label->second;
}

void CodeHeader::printArgs()
{
    map<unsigned int, unsigned int>::iterator it = _args->begin();
    for(; it != _args->end(); it++)
    {
	cout << "arg : " << it->first << " at pos " << it->second << endl;
    }
}



void CodeHeader::buildThisPointerVar()
{
	string temp_var_name = "v" + Int2Str(this->_this_reg);
	ClassDefInfo* this_cdf = _classdef;
	il::type* this_t = NULL;
	if(this_cdf != NULL)
	{
		this_t = getType(0,_dfr, this_cdf, str2Classdef(_dfr));
		//factoring variable
		_this_var = this->getRegVarEle(this->_this_reg, this_t, this->_t_map,false, false);

	}
}



