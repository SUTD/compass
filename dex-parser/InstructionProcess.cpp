#include "InstructionProcess.h"
//per instruction per object
//static field
#define PRINT_INFO false
#define DUPLICATE_REGISTER 2000
#define RETURNREG 10000
//#define USING_TYPE_INFER true
InstructionProcess::InstructionProcess(DexFileReader* dfr, CodeHeader* codeheader, 
					Format format, DexOpcode opcode, unsigned int address_ins, 
					map<unsigned int,ClassDefInfo*>* vartype,
					map<unsigned int, ClassDefInfo*>* declared)
{
	_dfr = dfr;
	_codeheader = codeheader;
	_format = format;
	_opcode = opcode;
	_address_ins = address_ins;
	_var_type = vartype;
	this->_declared_map = declared;
	//haiyan added 7.15
	//_str2type = str2Classdef(_dfr);
	if(!GLOBAL_MAP)
		this->_invalid_type = new ClassDefInfo("INVALID");
	else
		this->_invalid_type = _dfr->getClassDefByName("INVALID");
	if(USING_TYPE_INFER)
		in_use_flag = false;
	else
		in_use_flag = true;
}

void InstructionProcess::Init()
{
    _str2type = str2Classdef(_dfr);
}
InstructionProcess::~InstructionProcess()
{
	delete _invalid_type;
}

//one instruction per process, so before use the InstructionProcess, please first create an InstructionProcess Object
ASMInstruction* InstructionProcess::processAll()
{
	DataIn* in = _codeheader->getDataIn();//prepare for the in parameter
	switch(_format)
	{
		case F10t: return processF10t(in, _opcode, _address_ins);
		case F10x: return processF10x(in, _opcode, _address_ins);
		case F11n: return processF11n(in, _opcode, _address_ins);
		case F11x: return processF11x(in, _opcode, _address_ins);
		case F12x: return processF12x(in, _opcode, _address_ins);
		//case F20bc:return processF20bc(in, _opcode, _address_ins);
		case F20t: return processF20t(in, _opcode, _address_ins);
		case F21c: return processF21c(in, _opcode, _address_ins);
		case F21h: return processF21h(in, _opcode, _address_ins);
		case F21s: return processF21s(in, _opcode, _address_ins);
		case F21t: return processF21t(in, _opcode, _address_ins);
		case F22b: return processF22b(in, _opcode, _address_ins);
		case F22c: return processF22c(in, _opcode, _address_ins);
		case F22s: return processF22s(in, _opcode, _address_ins);
		case F22t: return processF22t(in, _opcode, _address_ins);
		case F22x: return processF22x(in, _opcode, _address_ins);
		case F23x: return processF23x(in, _opcode, _address_ins);
		case F30t: return processF30t(in, _opcode, _address_ins);
		case F31c: return processF31c(in, _opcode, _address_ins);
		case F31i: return processF31i(in, _opcode, _address_ins);
		case F31t: return processF31t(in, _opcode, _address_ins);
		//case F32s: return processF32s(in, _opcode, _address_ins);
		case F32x: return processF32x(in, _opcode, _address_ins);
		//case F33x: return processF33x(in, _opcode, _address_ins);
		case F35c: return processF35c(in, _opcode, _address_ins);
		case F3rc: return processF3rc(in, _opcode, _address_ins);
		//case F40sc: return processF40sc(in, _opcode, _address_ins);
		case F51l: return processF51l(in, _opcode, _address_ins);
		default: return NULL;
	}
}

//format 10t goto +aa; //done
ASMInstruction* InstructionProcess::processF10t(DataIn* in, DexOpcode opcode, unsigned int address)
{//liveness done
	in->move(address);
	in->skip(1);
	switch(opcode)
	{	
		case OP_GOTO:
		{
			string ope = "goto";
			int goto_addr = in->readByte();
			unsigned int addr = address + goto_addr*2;
			char buffer[50];
			sprintf(buffer,"%u",addr);
			string tempstr = buffer;
			tempstr = "+" + tempstr;
			string operand1 = tempstr;
			/*ClassDefInfo* vartype = new ClassDefInfo("Label");
			map<unsigned int, ClassDefInfo*>* type = new map<unsigned int, ClassDefInfo*>();
			(*type)[1000] = vartype;
			return new ASMInstruction(_codeheader, _format, opcode, ope, type, NULL, operand1);*/
			return new ASMInstruction(_codeheader, _format, opcode, ope, NULL, NULL, operand1);
		}
		default:
			return NULL;
	}
}

//format 10x // done
ASMInstruction* InstructionProcess::processF10x(DataIn*in, DexOpcode opcode, unsigned int address)
{//liveness done     
	in->move(address);
        in->skip(1); // this byte for opcode

	switch(opcode)
	{
		case OP_NOP:
		{
			string ope = "nop";
			return new ASMInstruction(_codeheader,_format,opcode,ope);
		}
        	case OP_RETURN_VOID:
		{
			string ope = "return-void";
			return new ASMInstruction(_codeheader,_format,opcode, ope);
		}
		default:
			return NULL;
	}
}

//format 11n //done
ASMInstruction* InstructionProcess::processF11n(DataIn* in,DexOpcode opcode, unsigned int address)
{
	in->move(address);
	in->skip(1); //skip the opcode 8 bits
	string ope = "const/4";
	//unsigned int temp= in->readByte();
	int temp =(int)in->readByte();
	//process operand1
	unsigned short reg = temp & 0x0f; //for example 1d => d
	string operand1 = "v" + Int2Str(reg);
	
	//process oprand2
	signed int instance = temp >> 4; //for example 1d => 1
	string operand2 = "#+"+ Int2Str(instance);
	vector<unsigned short>* regs = new vector<unsigned short>();
	regs->push_back(reg);

	//hzhu begin 4/23/2012
	vector<unsigned short>::iterator it;
	ClassDefInfo* vartype = NULL;
	if(!GLOBAL_MAP)
		vartype = new ClassDefInfo("I"); //"I" means int
	else
		vartype = _dfr->getClassDefByName("I");
	//12/20
	unsigned int typeidx = _dfr->getTypeId("I");
	vartype->setTypeIdx(typeidx);
	//12/20
    map<unsigned int, ClassDefInfo*>* type = new map<unsigned int, ClassDefInfo*>();

	//cout << "reg " << reg << endl;
	//assert(_declared_map->count(reg));
	if((*_declared_map)[reg] != NULL){
	    (*type)[reg] = (*_declared_map)[reg];
	    if(reg < _codeheader->getFirstArgu())
		(*_var_type)[reg] =  (*_declared_map)[reg];
	}
	else if(USING_TYPE_INFER)
	{
	    (*type)[reg] = this->_invalid_type;
	}
	else{
	    (*type)[reg] = vartype; //by infer
	    //if(reg < _codeheader->getFirstArgu()) //PAY ATTENTION WE MAY WANT TO CHANGE ALL OF THOSE THINGS, 9.20
	    (*_var_type)[reg] =  vartype;
	}
	/*
        //(*type)[1000] = vartype;
	for(it = regs->begin(); it!=regs->end();it++)
	{
		(*type)[*it] =  vartype;
		if(*it < _codeheader->getFirstArgu())
		    (*_var_type)[*it] = vartype;//update the method's local variable types	
	}
	(*type)[1000] = vartype;
	*/
	 //hzhu end 4/23/2012
	return new ASMInstruction(_codeheader, _format,opcode, ope,type,regs,operand1, operand2);
}

//for format 11x //done
string InstructionProcess::F11xoperand1(unsigned short regs)
{
        string oprand1= "v"+Int2Str(regs);
        return oprand1;
}

ASMInstruction* InstructionProcess::fillOperandandins_F11x(DexOpcode opcode,string op,unsigned short regs,vector<unsigned short>* all_regs)
{
        string operand1 = F11xoperand1(regs);
	//hzhu add 4/23/2012
	vector<unsigned short>::iterator it;
	map<unsigned int, ClassDefInfo*>* vartype = new map<unsigned int, ClassDefInfo*>();
	it = all_regs->begin();
	ClassDefInfo* classdef;
	if((op == "move-result") ||(op == "move-result-wide") || (op == "move-result-object"))
	{//liveness done
		assert(_declared_map->count(*it));
		if ((*_declared_map)[*it]!=NULL){
		    (*vartype)[*it] = (*_declared_map)[*it];
		    if(*it < _codeheader->getFirstArgu())
			(*_var_type)[*it] = (*_declared_map)[*it];
		}
		else if(USING_TYPE_INFER)
		{
			(*vartype)[*it] = this->_invalid_type;
		}	
		else
		{   
		    assert( _var_type->count(RETURNREG)> 0 );// have to be in RETURNREG register
		    (*vartype)[*it] = (*_var_type)[RETURNREG]; //use infered type
		    //because parameter register could be reused as different type
		    //if(*it < _codeheader->getFirstArgu())
		    //{
			//cout << "update REG " << *it << "  type is " << (*_var_type)[RETURNREG] << endl;
			(*_var_type)[*it] =(*_var_type)[RETURNREG]; //update _var_type
              //      }
		    _var_type->erase(RETURNREG); //erase RETURNREG register
		}
	}
	//if((op == "throw") || ( op == "move-exception")) //in_def
	if( op == "move-exception") //in_def
	{
		if(!GLOBAL_MAP)
			classdef = new ClassDefInfo("Ljava/lang/Exception;");
		else
			classdef = _dfr->getClassDefByName("Ljava/lang/Exception;");
		assert(_declared_map->count(*it) > 0); 
		if((*_declared_map)[*it] != NULL){
			classdef = (*_declared_map)[*it];
		}
		(*vartype)[*it] = classdef;
		if(*it < _codeheader->getFirstArgu())
		    (*_var_type)[*it] = classdef;
	}
        //in_use register
	if(op == "throw"){
		if(!GLOBAL_MAP)
			classdef = new ClassDefInfo("Ljava/lang/Exception;");
		else
			classdef = _dfr->getClassDefByName("Ljava/lang/Exception;");
		assert(_declared_map->count(*it) >0);
		if((*_declared_map)[*it] != NULL)
			classdef = (*_declared_map)[*it];
		else if((*_var_type)[*it]!= NULL)
			classdef = (*_var_type)[*it];
		(*vartype)[*it] = classdef;
	}	
	if((op=="return")||(op == "return-wide")||(op == "return-object")||(op == "monitor-enter")||(op =="monitor-exit")) //return from the register
	{
		/*if(op == "return-object")
		{
		    string return_t_str = _codeheader->getMethod()->getReturnType();
		    map<string,ClassDefInfo*>* str_classdef = _dfr->getStr2ClassDef();
		    ClassDefInfo* r_classdef = NULL;
		    if(str_classdef->count(return_t_str) > 0)
		    {
			//cout << "sainity check  found " <<  return_t_str << endl;
			r_classdef =(*str_classdef)[return_t_str];
		    }
		    else
		    {
			cout << "can't found " << return_t_str << endl;
			r_classdef = new ClassDefInfo(return_t_str);
		    }
		    (*vartype)[*it] = r_classdef;
		    if(*it < _codeheader->getFirstArgu())
			(*_var_type)[*it] = r_classdef;
		}
		else if((*_var_type)[*it] != NULL)
		{
			(*vartype)[*it] = (*_var_type)[*it];
		}
		else
		    cout << "bail out! NO return value!!" << endl;*/
		if((op == "monitor-enter")||(op == "monitor-exit"))
		{
			assert(_declared_map->count(*it) > 0);
			if((*_declared_map)[*it] != NULL){
				(*vartype)[*it] = (*_declared_map)[*it];
			}else if(USING_TYPE_INFER){
				(*vartype)[*it] = this->_invalid_type;
			}else{
				assert(_var_type->count(*it) > 0);
				assert((*_var_type)[*it] != NULL);
				(*vartype)[*it] = (*_var_type)[*it];
			}
		}
	    if((op=="return")||(op == "return-wide")||(op == "return-object")){
		assert(_declared_map->count(*it) > 0);
		if ((*_declared_map)[*it]!=NULL){
		    (*vartype)[*it] = (*_declared_map)[*it];
		}
		else if(USING_TYPE_INFER)
		{
			 (*vartype)[*it] = this->_invalid_type;
		}
		else
		{   
		    assert( _var_type->count(*it)> 0 );
		    assert((*_var_type)[(*it)] != NULL);
		    (*vartype)[*it] = (*_var_type)[*it];
		    if(*it < _codeheader->getFirstArgu())
			(*_var_type)[*it] =(*_var_type)[*it]; //update
		}
	    }
	}
        ASMInstruction* ins = new ASMInstruction(_codeheader, _format,opcode,op,vartype, all_regs,operand1);
	//hzhu end 4/24/2012
        return ins;
}

ASMInstruction* InstructionProcess::processF11x(DataIn* in,DexOpcode opcode, unsigned int address)
{
	in->move(address);
	in->skip(1);
	unsigned short return_reg = in->readByte()&0x00ff;
	vector<unsigned short>* allregs = new vector<unsigned short>();
	allregs->push_back(return_reg);
	switch(opcode)
	{
		case OP_MOVE_RESULT:
		{
			string ope = "move-result"; //done
			//ASMInstruction* asm_ins = fillOperandandins_F11x(ope,return_reg,allregs);
			ASMInstruction* asm_ins = fillOperandandins_F11x(opcode,ope,return_reg,allregs);
                        return asm_ins;
		}
        	case OP_MOVE_RESULT_WIDE:
		{
			string ope = "move-result-wide"; //done
                        //ASMInstruction* asm_ins = fillOperandandins_F11x(ope,return_reg,allregs);
			ASMInstruction* asm_ins = fillOperandandins_F11x(opcode,ope,return_reg,allregs);
                        return asm_ins;
		}

        	case OP_MOVE_RESULT_OBJECT:
		{
			string ope = "move-result-object"; //done
			//ASMInstruction* asm_ins = fillOperandandins_F11x(ope,return_reg,allregs);
			ASMInstruction* asm_ins = fillOperandandins_F11x(opcode,ope,return_reg,allregs);
			return asm_ins;
		}
        	case OP_MOVE_EXCEPTION:
		{
			string ope = "move-exception"; //done
                        //ASMInstruction* asm_ins = fillOperandandins_F11x(ope,return_reg,allregs);
			ASMInstruction* asm_ins = fillOperandandins_F11x(opcode,ope,return_reg,allregs);
                        return asm_ins;
		}
        	case OP_RETURN:
		{
			string ope = "return";//done
                        //ASMInstruction* asm_ins = fillOperandandins_F11x(ope,return_reg, allregs);
			ASMInstruction* asm_ins = fillOperandandins_F11x(opcode,ope,return_reg,allregs);
                        return asm_ins;
		}
        	case OP_RETURN_WIDE:
		{
			string ope = "return-wide";//done
                        //ASMInstruction* asm_ins = fillOperandandins_F11x(ope,return_reg,allregs);
			ASMInstruction* asm_ins = fillOperandandins_F11x(opcode,ope,return_reg,allregs);
                        return asm_ins;
		}
        	case OP_RETURN_OBJECT:
		{
			string ope = "return-object";//done
                        //ASMInstruction* asm_ins = fillOperandandins_F11x(ope,return_reg,allregs);
			ASMInstruction* asm_ins = fillOperandandins_F11x(opcode,ope,return_reg,allregs);
                        return asm_ins;
		}
        	case OP_MONITOR_ENTER:
		{
			string ope = "monitor-enter"; //done
                        //ASMInstruction* asm_ins = fillOperandandins_F11x(ope,return_reg,allregs);
			ASMInstruction* asm_ins = fillOperandandins_F11x(opcode,ope,return_reg,allregs);
                        return asm_ins;
		}
        	case OP_MONITOR_EXIT:
		{
			string ope = "monitor-exit";//done
                        //ASMInstruction* asm_ins = fillOperandandins_F11x(ope,return_reg,allregs);
			ASMInstruction* asm_ins = fillOperandandins_F11x(opcode,ope,return_reg,allregs);
                        return asm_ins;
		}
        	case OP_THROW:
		{
			string ope = "throw"; //done
                        //ASMInstruction* asm_ins = fillOperandandins_F11x(ope,return_reg,allregs);
			ASMInstruction* asm_ins = fillOperandandins_F11x(opcode,ope,return_reg,allregs);
                        return asm_ins;
		}
		default:
			return NULL;

	}
}

//for F12x format

string InstructionProcess::F12xoperand1(unsigned short regs, vector<unsigned short>* all_regs)
{
	unsigned short dest_reg = regs & 0x0f;
	if (all_regs != NULL)
		all_regs->push_back(dest_reg);
        string oprand1= "v"+Int2Str(dest_reg);
        return oprand1;
}
string InstructionProcess::F12xoperand2(unsigned short regs, vector<unsigned short>* all_regs)
{
	unsigned short sou_reg = (regs & 0xf0) >> 4;
	if (all_regs != NULL)
		all_regs->push_back(sou_reg);
        string oprand2 = "v"+Int2Str(sou_reg);
        return oprand2;
}

ASMInstruction* InstructionProcess::fillOperandandins_F12x(DexOpcode opcode,string op,unsigned short regs,vector<unsigned short>* all_regs)
{
        string operand1 = F12xoperand1(regs,all_regs);
        string operand2 = F12xoperand2(regs,all_regs);
	vector<unsigned short>::reverse_iterator it = all_regs->rbegin(); //we start from the RHS register
	vector<unsigned short>::iterator it1 = all_regs->begin(); //start from the First register inside the ins; for int-to-long(cast ins)
	map <unsigned int, ClassDefInfo*>* ins_type = new map <unsigned int, ClassDefInfo*>();
	unsigned int firstreg = 0;
	ClassDefInfo* classdef = NULL;
	vector<unsigned short>* in_use = NULL;
	if(in_use_flag)
		in_use = new vector<unsigned short>();
	if((op == "move") || (op == "move-wide") || (op == "move-object"))
	{
		ClassDefInfo* rht = NULL;	    
		for(; it != all_regs->rend(); it++) //first reg should be RSH (v0 = v1) e.x, v1 is first reg
		{
			assert(_declared_map->count(*it)>0);
			assert(_var_type->count(*it) > 0);
			if(it == all_regs->rbegin()) // the RSH reg
			{
				if(in_use_flag)
					in_use->push_back(*it);
			    if((*_declared_map)[*it] != NULL)
			    {
				(*ins_type)[*it] = (*_declared_map)[*it]; 
			    }else if((*it) >= _codeheader->getFirstArgu()){ //parameters
			    	assert((*_var_type)[*it] != NULL);
			    	(*ins_type)[*it] = (*_var_type)[*it];
			    }
			    else if(USING_TYPE_INFER)
			    {
				(*ins_type)[*it] = this->_invalid_type;
			    }
			    else
			    {
				assert((*_var_type)[*it]!= NULL); //if not found by declared type, it should in _var_type
				(*ins_type)[*it] = (*_var_type)[*it]; 
			    }
			    rht = (*ins_type)[*it]; //hold the in use register for example(v1)
			}
			else //the LSH reg, the defined register inside the move instruction
			{
			    if((*_declared_map)[*it] != NULL)
			    {
				(*ins_type)[*it] = (*_declared_map)[*it]; 
			    	if(*it < _codeheader->getFirstArgu()) //if possible, update the _var_type map
					(*_var_type)[*it] = (*ins_type)[*it];
			    }
			    else if(USING_TYPE_INFER)
			    {
				 (*ins_type)[*it] = this->_invalid_type;
			    }
			    else
			    {
				assert(rht != NULL);
				(*ins_type)[*it] = rht; // use v1 to infer the type of v0(v0 = v1) 
			    	if(*it < _codeheader->getFirstArgu()) //if possible, update the _var_type map
				(*_var_type)[*it] = (*ins_type)[*it];
			    }
			    //if(*it < _codeheader->getFirstArgu()) //if possible, update the _var_type map
			//	(*_var_type)[*it] = (*ins_type)[*it];
			}
			/*
			if(it == all_regs->begin()) //first register
			{
				firstreg = (*it);
			}
			else //second register
			{
				if((*_var_type)[*it] != NULL)
				{
					(*ins_type)[firstreg] = (*_var_type)[*it];
					(*ins_type)[*it] = (*_var_type)[*it]; 
					if(firstreg < _codeheader->getFirstArgu())
					    (*_var_type)[firstreg] = (*_var_type)[*it]; //update
				}
				else
				{
				    cout << "second register for move is NULL, bail out" << endl;
				    exit(-1);
				}
			}
			*/
		}
	}
	else if(op == "array-length")
	{
		if(!GLOBAL_MAP)
			classdef = new ClassDefInfo("I");
		else
			classdef = _dfr->getClassDefByName("I");
		//12/20
		unsigned int typeidx = _dfr->getTypeId("I");
		classdef->setTypeIdx(typeidx);
		//12/20
		unsigned int rhsr;
		for(; it != all_regs->rend(); it++)
		{
			assert(_declared_map->count(*it)>0);
			assert(_var_type->count(*it) > 0);
			if(it == all_regs->rbegin()) // the RSH reg in_use
			{
				if(in_use_flag)
					in_use->push_back(*it);
		        rhsr = *it;
			    if((*_declared_map)[*it] != NULL)
			    {
				(*ins_type)[*it] = (*_declared_map)[*it]; 
			    }
			    else if(USING_TYPE_INFER)
			    {
				(*ins_type)[*it] = this->_invalid_type;
			    }
			    else
			    {
				assert((*_var_type)[*it]!= NULL); //if not found by declared type, it should in _var_type
				(*ins_type)[*it] = (*_var_type)[*it]; 
			    }
			}
			else //the LSH reg, the defined register inside the move instruction
			{
			    //assert(*it != rhsr); //to check if the two registers are the same,lhs register equals to rhs;
			    if(*it == rhsr)
			    {
				(*ins_type)[DUPLICATE_REGISTER] = (*ins_type)[rhsr]; 
                            }
			    
			    if((*_declared_map)[*it] != NULL)
			    {
				(*ins_type)[*it] = (*_declared_map)[*it]; 
			    }
			    else if(USING_TYPE_INFER)
			    {
				(*ins_type)[*it] = this->_invalid_type;
                            }
			    else
			    {
				(*ins_type)[*it] = classdef; // the size of array,which should be interger
			    }
			    if(*it < _codeheader->getFirstArgu()) //if possible, update the _var_type map
				(*_var_type)[*it] = classdef;
			}
			//////////////
			/*if(it == all_regs->rbegin()) //RSH
			{
				classdef = new ClassDefInfo("I");
				(*ins_type)[*it] = classdef;
				firstreg = *it;
			}
			else 
			{
				in_use->push_back(*it);
				if(*it == firstreg)
				{
				    if((*_var_type)[*it]!= NULL)
					(*ins_type)[2000] = (*_var_type)[*it];
				    else
					cout << "bail out, no register exists!" << endl;
				}
				else
				{
				    if((*_var_type)[*it]!= NULL)
					(*ins_type)[*it] = (*_var_type)[*it];
				    else
					cout << "bail out, no register exists!!" << endl;
				}
				if(firstreg < _codeheader->getFirstArgu())
				    (*_var_type)[firstreg] = classdef; //update
			}*/
			/////
		}
	}
	else if ((op == "neg-int") || (op == "not-int") || (op == "add-int/2addr")
			|| (op == "sub-int/2addr") || (op == "mul-int/2addr")
			|| (op == "div-int/2addr") || (op == "rem-int/2addr")
			|| (op == "and-int/2addr") || (op == "or-int/2addr")
			|| (op == "xor-int/2addr") || (op == "shl-int/2addr")
			|| (op == "shr-int/2addr") || (op == "ushr-int/2addr")
			|| (op == "neg-long") || (op == "not-long")
			|| (op == "add-long/2addr") || (op == "sub-long/2addr")
			|| (op == "mul-long/2addr") || (op == "div-long/2addr")
			|| (op == "rem-long/2addr") || (op == "and-long/2addr")
			|| (op == "or-long/2addr") || (op == "xor-long/2addr")
			|| (op == "shl-long/2addr") || (op == "shr-long/2addr")
			|| (op == "ushr-long/2addr") || (op == "neg-float")
			|| (op == "add-float/2addr") || (op == "sub-float/2addr")
			|| (op == "mul-float/2addr") || (op == "div-float/2addr")
			|| (op == "rem-float/2addr") || (op == "neg-double")
			|| (op == "add-double/2addr") || (op == "sub-double/2addr")
			|| (op == "mul-double/2addr") || (op == "div-double/2addr")
			|| (op == "rem-double/2addr")) {

		int lhs_reg = 0;
		for (; it != all_regs->rend(); it++) {


			assert(_declared_map->count(*it)>0);
			assert(_var_type->count(*it) > 0);
			if (it == all_regs->rbegin()) //the RHS register(in_use)
					{
				//note that in_use_flag is false;
				lhs_reg = *it;
				if (in_use_flag)
					in_use->push_back(*it); // in_use register


				if ((*_declared_map)[*it] != NULL) {
					(*ins_type)[*it] = (*_declared_map)[*it];
				} else if(((*_declared_map)[*it] == NULL) && ((*_var_type)[*it] != NULL)){
					//cout << "*_var_type is " << (*_var_type)[*it]->getTypename() << endl;
					(*ins_type)[*it] = (*_var_type)[*it];
				}else if (USING_TYPE_INFER) {
					(*ins_type)[*it] = this->_invalid_type;
				} else {
					assert((*_var_type)[*it] != NULL);
					// must being defined before
					(*ins_type)[*it] = (*_var_type)[*it];
				}
				classdef = (*ins_type)[*it]; // keep the RHS register type
			} else { //the LHS register
				if ((op == "neg-int") || (op == "not-int") || (op == "neg-long")
						|| (op == "not-long") || (op == "neg-float")
						|| (op == "neg-double")) // the rhs register (in_def) it is possible that the register is reused.
						{

					if ((*_declared_map)[*it] != NULL) {
						(*ins_type)[*it] = (*_declared_map)[*it];
					}else if(*it == lhs_reg){ //it is possible that same register is redefined here;
						(*ins_type)[*it] = classdef;
					}else if (USING_TYPE_INFER) {
						(*ins_type)[*it] = this->_invalid_type;
					} else { //use the infer type from the RHS
						assert(classdef != NULL);
						(*ins_type)[*it] = classdef;
					}
					if (*it < _codeheader->getFirstArgu()) //if possible, update the _var_type map
						(*_var_type)[*it] = (*ins_type)[*it];

				} else { //the rhs register first used as the source reg, and then used as destination reg and two times of same reg should have same type
					if (in_use_flag)
						in_use->push_back(*it);
					if ((*_declared_map)[*it] != NULL) {
						(*ins_type)[*it] = (*_declared_map)[*it];
					}else if(((*_declared_map)[*it] == NULL) && ((*_var_type)[*it] != NULL)){
						(*ins_type)[*it] = (*_var_type)[*it];
					}else if (USING_TYPE_INFER) {
						(*ins_type)[*it] = this->_invalid_type;
					} else { //inside the _var_type map
						assert((*_var_type)[*it] != NULL);
						(*ins_type)[*it] = (*_var_type)[*it];
					}
					if (*it < _codeheader->getFirstArgu()) //if possible, update the _var_type map
						(*_var_type)[*it] = (*ins_type)[*it];
				}
			}
		}
		/*
		 if(it== all_regs->rbegin()) //the RHS register(in_use)
		 {
		 if((op != "neg-int")&& (op != "not-int"))
		 in_use->push_back(*it);
		 firstreg = *it;
		 (*ins_type)[*it] = classdef;
		 }
		 else
		 {
		 in_use->push_back(*it);
		 if(*it == firstreg)
		 (*ins_type)[2000] = classdef;
		 else
		 (*ins_type)[*it] = classdef;
		 if(firstreg < _codeheader->getFirstArgu())
		 (*_var_type)[firstreg] = classdef;
		 }
		 */

	}
/*
	if((op == "neg-long") | (op == "not-long")|(op =="add-long/2addr")|(op =="sub-long/2addr")| (op == "mul-long/2addr")| (op =="div-long/2addr")|(op =="rem-long/2addr")|(op =="and-long/2addr")|(op =="or-long/2addr")|(op =="xor-long/2addr")|(op =="shl-long/2addr")|(op =="shr-long/2addr")|(op =="ushr-long/2addr")| )
	{
                classdef = new ClassDefInfo("J");
                for(; it != all_regs->end(); it++)
                {
                        if(it== all_regs->begin())
                        {
			    if((op != "neg-long")&&(op != "not-long"))
				in_use->push_back(*it);
                            firstreg = *it;
                            (*ins_type)[*it] = classdef;
                        } 
                        else
                        {
				
				in_use->push_back(*it);
                                if(*it == firstreg)
                                        (*ins_type)[2000] = classdef;
                                else
                                        (*ins_type)[*it] = classdef;
				if(firstreg < _codeheader->getFirstArgu())
                                (*_var_type)[firstreg] = classdef;
                        }

                }

        }
	if((op == "neg-float") |(op =="add-float/2addr")|(op =="sub-float/2addr")| (op == "mul-float/2addr")| (op =="div-float/2addr")|(op =="rem-float/2addr"))
	{
                classdef = new ClassDefInfo("F");
                for(; it != all_regs->end(); it++)
                {
                        if(it== all_regs->begin())
                        {
			    if(op != "neg-float")
				in_use->push_back(*it);
                            firstreg = *it;
                            (*ins_type)[*it] = classdef;
                        }
                        else
                        {
			    in_use->push_back(*it);
                                if(*it == firstreg)
                                        (*ins_type)[2000] = classdef;
                                else
                                        (*ins_type)[*it] = classdef;
				if(firstreg < _codeheader->getFirstArgu())
				    (*_var_type)[firstreg] = classdef;
                        }

                }

        }
	if((op == "neg-double") |(op =="add-double/2addr")|(op =="sub-double/2addr")| (op == "mul-double/2addr")| (op =="div-double/2addr")|(op =="rem-double/2addr"))
        {
                classdef = new ClassDefInfo("D");
                for(; it != all_regs->end(); it++)
                {
                        if(it== all_regs->begin())
                        {
			    if(op != "neg-double")
				in_use->push_back(*it);
                            firstreg = *it;
                            (*ins_type)[*it] = classdef;
                        }
                        else
                        {
			    in_use->push_back(*it);
                                if(*it == firstreg)
                                        (*ins_type)[2000] = classdef;
                                else
                                        (*ins_type)[*it] = classdef;
				if(firstreg < _codeheader->getFirstArgu())
				    (*_var_type)[firstreg] = classdef;
                        }

                }

        }
*/
	else
	{
	    	for(; it1 != all_regs->end(); it1++)
		{
			if(it1== all_regs->begin())
			{
				if((op == "long-to-int")||(op == "float-to-int")||(op == "double-to-int"))
				{
					if (!GLOBAL_MAP)
						classdef = new ClassDefInfo("I");
					else
						classdef = _dfr->getClassDefByName("I");
					//12/20
					unsigned int typeidx = _dfr->getTypeId("I");
					classdef->setTypeIdx(typeidx);
					//12/20
				}
				if((op == "int-to-long")||(op == "float-to-long")||(op == "double-to-long"))
				{
					if (!GLOBAL_MAP)
						classdef = new ClassDefInfo("J");
					else
						classdef = _dfr->getClassDefByName("J");
					//12/20
					unsigned int typeidx = _dfr->getTypeId("J");
					classdef->setTypeIdx(typeidx);
					//12/20
				}
				if((op == "int-to-float")||(op == "long-to-float")||(op == "double-to-float"))
				{
					if (!GLOBAL_MAP)
						classdef = new ClassDefInfo("F");
					else
						classdef = _dfr->getClassDefByName("F");
					//12/20
					unsigned int typeidx = _dfr->getTypeId("F");
					classdef->setTypeIdx(typeidx);
					//12/20
				}
				if((op == "int-to-double")||(op == "long-to-double")||(op == "float-to-double"))
				{
					if (!GLOBAL_MAP)
						classdef = new ClassDefInfo("D");
					else
						classdef = _dfr->getClassDefByName("D");
					//12/20
					unsigned int typeidx = _dfr->getTypeId("D");
					classdef->setTypeIdx(typeidx);
					//12/20
				}
				if(op == "int-to-byte")
				{
					if (!GLOBAL_MAP)
						classdef = new ClassDefInfo("B");
					else
						classdef = _dfr->getClassDefByName("B");
					//12/20
					unsigned int typeidx = _dfr->getTypeId("B");
					classdef->setTypeIdx(typeidx);
					//12/20
				}
				if(op == "int-to-char")
				{
					if (!GLOBAL_MAP)
						classdef = new ClassDefInfo("C");
					else
						classdef = _dfr->getClassDefByName("C");
					//12/20
					unsigned int typeidx = _dfr->getTypeId("C");
					classdef->setTypeIdx(typeidx);
					//12/20
				}
				if(op == "int-to-short")
				{
					if (!GLOBAL_MAP)
						classdef = new ClassDefInfo("S");
					else
						classdef = _dfr->getClassDefByName("S");
					//12/20
					unsigned int typeidx = _dfr->getTypeId("S");
					classdef->setTypeIdx(typeidx);
					//12/20
				}
				firstreg = *it1; //in_def reg
				if((*_declared_map)[*it1] != NULL)
				{
				   //(*ins_type)[*it1] = classdef; //type information from the instruction
				   	(*ins_type)[*it1] = (*_declared_map)[*it1];
				}
				else if(USING_TYPE_INFER)
					(*ins_type)[*it1] = this->_invalid_type;
				else
				        (*ins_type)[*it1] = classdef; 
			}
			else //second register(in_use)
			{
				if(in_use_flag)
					in_use->push_back(*it1);
				//cout << "need register  :: " << *it1 << "Exist !!!!!!!! " << endl;
				//assert(((*_var_type)[*it1] != NULL)); //should be defined before;

				if(*it1 == firstreg) //register reused by the same instruction;
				{
				    if((*_declared_map)[*it1] != NULL)
					(*ins_type)[DUPLICATE_REGISTER] = (*_declared_map)[*it1];
				    else if(USING_TYPE_INFER)
					(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
				    else
				    	(*ins_type)[DUPLICATE_REGISTER]= (*_var_type)[*it1]; //as previous
				    /*
				    else
				    {
					if(op.substr(0,4) == "long")
					    classdef2 = new ClassDefInfo("J");
					if(op.substr(0,3) == "int")
					    classdef2 = new ClassDefInfo("I");
					if(op.substr(0,5) == "float")
					    classdef2 = new ClassDefInfo("F");
					if(op.substr(0,6) == "double")
					    classdef2 = new ClassDefInfo("D");
					(*ins_type)[2000] = classdef2;
				    */
				}
				else //register not reused
				{
				    if((*_declared_map)[*it1] != NULL)
					(*ins_type)[*it1]= (*_declared_map)[*it1];
				    else if(USING_TYPE_INFER)
					(*ins_type)[*it1]= this->_invalid_type;
				    else		
				    	(*ins_type)[*it1]= (*_var_type)[*it1];
				    /*
				    else
				    {
					if(op.substr(0,4) == "long")
					    classdef2 = new ClassDefInfo("J");
					if(op.substr(0,3) == "int")
					    classdef2 = new ClassDefInfo("I");
					if(op.substr(0,5) == "float")
					    classdef2 = new ClassDefInfo("F");
					if(op.substr(0,6) == "double")
					    classdef2 = new ClassDefInfo("D");
					(*ins_type)[*it1] = classdef2;
					if(*it < _codeheader->getFirstArgu())
					    (*_var_type)[*it1] = classdef2;
				    }
				    */
				}
				if(firstreg < _codeheader->getFirstArgu())
				    (*_var_type)[firstreg] = classdef; //update 
			}
		}
	}
        ASMInstruction* ins = new ASMInstruction(_codeheader,_format,opcode, op, ins_type, all_regs,operand1,operand2);
        if(in_use_flag)
        	ins->setInUseRegs(in_use);
	//delete in_use;
        return ins;
}



ASMInstruction* InstructionProcess::processF12x(DataIn* in,DexOpcode opcode, unsigned int address)
{
	in->move(address);
	in->skip(1); //skip the opcode
	unsigned short regs = in->readByte();
	vector<unsigned short>* allregs = new vector<unsigned short>();
	switch(opcode)
	{	
		case OP_MOVE:
		{
			string ope = "move"; //done
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_MOVE_WIDE:
		{
			string ope = "move-wide";//done
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_MOVE_OBJECT:
		{
			string ope = "move-object"; //done
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_ARRAY_LENGTH:
		{	
			string ope = "array-length"; //done
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_NEG_INT:
		{	
			string ope = "neg-int";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
			return asm_ins;
		}
        	case OP_NOT_INT:
		{
			string ope = "not-int";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_NEG_LONG:
		{
			string ope = "neg-long";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_NOT_LONG:
		{
			string ope = "not-long";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_NEG_FLOAT:
		{
			string ope = "neg-float";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_NEG_DOUBLE:
		{
			string ope = "neg-double";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_INT_TO_LONG:
		{
			string ope = "int-to-long";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_INT_TO_FLOAT:
		{
			string ope = "int-to-float";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_INT_TO_DOUBLE:
		{
			string ope = "int-to-double";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_LONG_TO_INT:
		{
			string ope = "long-to-int";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_LONG_TO_FLOAT:
		{
			string ope = "long-to-float";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_LONG_TO_DOUBLE:
		{
			string ope = "long-to-double";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_FLOAT_TO_INT:
		{
			string ope = "float-to-int";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_FLOAT_TO_LONG:
		{
			string ope = "float-to-long";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_FLOAT_TO_DOUBLE:
		{
			string ope = "float-to-double";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
			
		}
        	case OP_DOUBLE_TO_INT:
		{
			string ope = "double-to-int";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_DOUBLE_TO_LONG:
		{
			string ope = "double-to-long";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_DOUBLE_TO_FLOAT:
		{
			string ope = "double-to-float";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_INT_TO_BYTE:
		{
			string ope = "int-to-byte";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_INT_TO_CHAR:
		{
			string ope = "int-to-char";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_INT_TO_SHORT:
		{
			string ope = "int-to-short";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_ADD_INT_2ADDR:
		{
			string ope = "add-int/2addr"; //done
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_SUB_INT_2ADDR:
		{
			string ope = "sub-int/2addr"; //done
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_MUL_INT_2ADDR:
		{
			string ope = "mul-int/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_DIV_INT_2ADDR:
		{
			string ope = "div-int/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_REM_INT_2ADDR:
		{
			string ope = "rem-int/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_AND_INT_2ADDR:
		{
			string ope = "and-int/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_OR_INT_2ADDR:
		{
			string ope = "or-int/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_XOR_INT_2ADDR:
		{
			string ope = "xor-int/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_SHL_INT_2ADDR:
		{
			string ope = "shl-int/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_SHR_INT_2ADDR:
		{	
			string ope = "shr-int/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_USHR_INT_2ADDR:
		{
			string ope = "ushr-int/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_ADD_LONG_2ADDR:
		{
			string ope = "add-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_SUB_LONG_2ADDR:
		{
			string ope = "sub-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_MUL_LONG_2ADDR:
		{
			string ope = "mul-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_DIV_LONG_2ADDR:
		{
			string ope = "div-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_REM_LONG_2ADDR:
		{
			string ope = "rem-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_AND_LONG_2ADDR:
		{
			string ope = "and-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_OR_LONG_2ADDR:
		{
			string ope = "or-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_XOR_LONG_2ADDR:
		{
			string ope = "xor-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_SHL_LONG_2ADDR:
		{
			string ope = "shl-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_SHR_LONG_2ADDR:
		{
			string ope = "shr-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_USHR_LONG_2ADDR:
		{
			string ope = "ushr-long/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_ADD_FLOAT_2ADDR:
		{
			string ope = "add-float/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
		case OP_SUB_FLOAT_2ADDR:
		{
			string ope = "sub-float/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_MUL_FLOAT_2ADDR:
		{
			string ope = "mul-float/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_DIV_FLOAT_2ADDR:
		{
			string ope = "div-float/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_REM_FLOAT_2ADDR:
		{
			string ope = "rem-float/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_ADD_DOUBLE_2ADDR:
		{
			string ope = "add-double/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_SUB_DOUBLE_2ADDR:
		{
			string ope = "sub-double/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_MUL_DOUBLE_2ADDR:
		{
			string ope = "mul-double/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_DIV_DOUBLE_2ADDR:
		{
			string ope = "div-double/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
        	case OP_REM_DOUBLE_2ADDR:
		{
			string ope = "rem-double/2addr";
			ASMInstruction* asm_ins = fillOperandandins_F12x(opcode,ope,regs,allregs);
                        return asm_ins;
		}
		default:
		{
			return NULL;
		}
	}
}

//goto/16 +AAAA how it works for 20t? 2 words? //done
ASMInstruction* InstructionProcess::processF20t(DataIn* in,DexOpcode opcode, unsigned int address)
{
	in->move(address);
	in->skip(2); //skip opcode(one byte) and another adjacent byte
	switch(opcode)
	{
		case OP_GOTO_16:
		{
			string ope = "goto/16";
			short goto_addr = in->readShortx();
                        unsigned int addr = address + goto_addr*2;
                        char buffer[50]; 
                        sprintf(buffer,"%u",addr);
                        string tempstr = buffer;
                        tempstr = "+" + tempstr;
                        string operand1 = tempstr;
                        //ClassDefInfo* vartype = new ClassDefInfo("Label");
                        map<unsigned int, ClassDefInfo*>* type = new map<unsigned int, ClassDefInfo*>();
                        //(*type)[1000] = vartype;
                        return new ASMInstruction(_codeheader,_format,opcode, ope,type, NULL,operand1);
		}
		default:
			return NULL;
	}
}

//for 21c //done
string InstructionProcess::F21cOperand1(DataIn* in, vector<unsigned short>* regs)
{  
	string oprand1 = "v";
	unsigned short reg = in->readByte();
	regs->push_back(reg);
	oprand1 += Int2Str(reg);
	return oprand1;
	
}

string InstructionProcess::F21cOperand2_string(DataIn* in)
{
	string oprand2;
	unsigned short stridx = in->readShortx();
        oprand2 = _dfr->getString(stridx);
	oprand2 = "\"" + oprand2 + "\"";
	return oprand2;
}

string InstructionProcess::F21cOperand2_type(unsigned int typeidx)
{
       //return _dfr->getType(typeidx);
	//using mapping unify
	return _dfr->getTypename(typeidx);
}

// hzhu add 4/24/2012
unsigned int InstructionProcess::F21cOperand2_typeidx(DataIn* in)
{
        string oprand2;
        unsigned int typeidx = in->readShortx();
        return typeidx;
}

//hzhu add 4/29/2012
unsigned int InstructionProcess::F21cOperand2_fieldidx(DataIn* in)
{
	unsigned int fieldidx = in->readShortx();
	return fieldidx;
}
//hzhu end 4/29/2012

string InstructionProcess::F21cOperand2_field( unsigned int fieldidx) //hzhu changed the prototype
{
	return _dfr->getField(fieldidx)->toString();
	
}
//hzhu added 4/29/2012 done of the liveness
void InstructionProcess::F21cGetOperandTypes(DataIn* in,vector<unsigned short>* regs,map<unsigned int, ClassDefInfo*>* ins_type,unsigned int fie_idx, string ope)
{
	ClassDefInfo* classdef;
	Field* field = _dfr->getField(fie_idx);
	unsigned int typeidx = field->getTypeIdx();
	classdef = type2ClassDef(_dfr,typeidx);
	///////liveness
	unsigned int reg = *regs->begin();
	assert(_declared_map->count(reg) > 0);
	assert(_var_type->count(reg) > 0);
	if(ope.substr(0,4) =="sget") //sget
	{
	    if((*_declared_map)[reg] != NULL) //def register
	    {
		(*ins_type)[reg] = (*_declared_map)[reg];
	    }
	    else if(USING_TYPE_INFER)
	    {
		(*ins_type)[reg] = this->_invalid_type;
	    }
	    else //using infer from instruction
		(*ins_type)[reg] = classdef;
	    if(reg < _codeheader->getFirstArgu())
		(*_var_type)[reg] = (*ins_type)[reg] ; //update
	}
	else //sput
	{
	    if((*_declared_map)[reg] != NULL) //def register
	    {
		(*ins_type)[reg] = (*_declared_map)[reg];
	    }
	    else if(USING_TYPE_INFER)
		(*ins_type)[reg] = this->_invalid_type;
	    else
	    {
		assert((*_var_type)[reg] != NULL);
		(*ins_type)[reg] = (*_var_type)[reg];
	    }

	}
}
//hzhu end 4/29/2012
ASMInstruction* InstructionProcess::processF21c(DataIn* in,DexOpcode opcode, unsigned int address)
{
	in->move(address);
	//printf ("HERE: %x\n", in->getCurrentPosition() );
        in->skip(1); // this byte for opcode
	vector<unsigned short>* reg = new vector<unsigned short>();
	vector<unsigned short>* in_use = NULL;
	if(in_use_flag)
		in_use = new vector<unsigned short>();
	switch(opcode)
	{
		case OP_CONST_STRING:
		{
		string ope = "const-string"; //don't care
		string operand1 = F21cOperand1(in, reg);
		string operand2 = F21cOperand2_string(in);
		vector<unsigned short>::iterator it = reg->begin();
		map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int,
				ClassDefInfo*>();
		ClassDefInfo* vartype = NULL;
		if (!GLOBAL_MAP)
			vartype = new ClassDefInfo("string");
		else
			vartype = _dfr->getClassDefByName("string");
		//12/20
		unsigned int typeidx = _dfr->getTypeId("string");
		vartype->setTypeIdx(typeidx);
		//12/20
		unsigned short reg_n = *it;

		assert( _declared_map->count(reg_n) >0);
		assert( _var_type->count(reg_n) >0);

		if ((*_declared_map)[reg_n] != NULL) { // found
			vartype = (*_declared_map)[reg_n]; // change temp_type to found one
		} else if (USING_TYPE_INFER)
			vartype = this->_invalid_type;
		(*ins_type)[reg_n] = vartype;
		if (reg_n < _codeheader->getFirstArgu())
			(*_var_type)[reg_n] = vartype;

		return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type,
				reg, operand1, operand2);

	}
        	case OP_CONST_CLASS:
		{
		string ope = "const-class"; //don't care , almost set is invalid, determined by type inference phase
		string operand1 = F21cOperand1(in, reg);
		//string operand2 = F21cOperand2_type(in);
		unsigned int typeidx = F21cOperand2_typeidx(in);
		string operand2 = F21cOperand2_type(typeidx);

		//ClassDefInfo* vartype = type2ClassDef(_dfr,typeidx); //type
		ClassDefInfo* vartype = NULL;
		if (!GLOBAL_MAP)
			vartype = new ClassDefInfo("Ljava/lang/Class;");
		else
			vartype = _dfr->getClassDefByName("Ljava/lang/Class;");
		//12/20
		unsigned int typeidx1 = _dfr->getTypeId("Ljava/lang/Class;");
		vartype->setTypeIdx(typeidx1);
		//12/20
		unsigned int reg_n = (*reg->begin()); //register
		map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int,
				ClassDefInfo*>();

		assert( _declared_map->count(reg_n) >0);
		assert( _var_type->count(reg_n) >0);

		if ((*_declared_map)[reg_n] != NULL) { // found
			vartype = (*_declared_map)[reg_n]; // change temp_type to found one
		} else if (USING_TYPE_INFER) {
			vartype = this->_invalid_type;
		}
		(*ins_type)[reg_n] = vartype;
		if (reg_n < _codeheader->getFirstArgu())
			(*_var_type)[reg_n] = vartype;

		return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type,
				reg, operand1, operand2);
	}
        	case OP_CHECK_CAST: //done with the liveness
	{

		string ope = "check-cast"; //don't care
		string operand1 = F21cOperand1(in, reg);
		unsigned int typeidx = F21cOperand2_typeidx(in);
		string operand2 = F21cOperand2_type(typeidx);
		ClassDefInfo* classdef = type2ClassDef(_dfr, typeidx); //type
		unsigned int reg_num = (*reg->begin()); //register
		map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int,
				ClassDefInfo*>();
		(*ins_type)[reg_num] = classdef;

		assert((*ins_type)[reg_num] != NULL);



		if (((*_declared_map)[reg_num] == NULL)
				&& (reg_num >= _codeheader->getFirstArgu())) { //not defined and it coming from parameter
			//cout << "HHHHHHHHHHH 1 " << endl;
			(*ins_type)[reg_num] = (*_var_type)[reg_num];
		}else if(reg_num >= _codeheader->getFirstArgu()){
			//cout << "HHHHHHHHHHHHHHHH " << endl;
			(*ins_type)[reg_num] = (*_declared_map)[reg_num];
		}else if (USING_TYPE_INFER) {
			//cout << "HHHHHHHHH 2 " << endl;
			(*ins_type)[reg_num] = this->_invalid_type;
		} else{
			//cout << "HHHHHHHH 3 " << endl;
			(*ins_type)[reg_num] = (*_var_type)[reg_num];
		}


		if (reg_num < _codeheader->getFirstArgu())
			(*_var_type)[reg_num] = classdef; //update for the method local variables
		//check-cast v2, [Ljava/lang/String; ==>Types::::  v2:[Ljava/lang/Object;;(here v2 is a parameter)
		//update declared_map, which will changed the parameter's type
		if (reg_num >= _codeheader->getFirstArgu())
			(*_declared_map)[reg_num] = classdef;
		ASMInstruction* asm_ins = new ASMInstruction(_codeheader, _format,
				opcode, ope, ins_type, reg, operand1, operand2);
		assert(classdef != NULL);
		asm_ins->setCheckCastClassDef(classdef);
		return asm_ins;
	}
        	case OP_NEW_INSTANCE: //add the liveness check
		{
			string ope = "new-instance"; //don't care
			string operand1 = F21cOperand1(in,reg);
			unsigned int typeidx = F21cOperand2_typeidx(in);
			string operand2 = F21cOperand2_type(typeidx);
			ClassDefInfo* classdef = type2ClassDef(_dfr,typeidx); //type
			//ClassDefInfo* cdf_from_inst = classdef;
			unsigned int reg_num = (*reg->begin()); //register
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();



			//use the dynamic type to udpate the declared_map(which would makes sense for pointer analysis procedure)
			//update the declared_map(if it exists)
			//bool flag = false;
//			if ((_declared_map->count(reg_num) > 0) && ((*_declared_map)[reg_num] != NULL)){
//				cout << "_delcared_map type is for reg: "  <<reg_num << " "
//						<< (*_declared_map)[reg_num]->getTypename() << endl;
//
//				(*_declared_map)[reg_num] = classdef;
//				cout << "set it as " << classdef->getTypename() << endl;
//				flag = true;
//			}

			//if defined, we use dynamic type instead of declared type
			if ((_declared_map->count(reg_num) > 0) && ((*_declared_map)[reg_num] != NULL)){ // if found in declared_map, change the pointer
				//classdef = (*_declared_map)[reg_num];
				(*_declared_map)[reg_num] = classdef;

			}
			else if(USING_TYPE_INFER){
				classdef = this->_invalid_type;
			}

			///////////////haiyan first check the liveness of the register end
			(*ins_type)[reg_num] = classdef;

			if(reg_num < _codeheader->getFirstArgu())
			    (*_var_type)[reg_num] = classdef; //update for the method local variables


			return new ASMInstruction(_codeheader, _format,opcode,ope, ins_type,reg, operand1, operand2);
		}
        	case OP_SGET:
		{
			string ope = "sget"; 
			string operand1 = F21cOperand1(in,reg);
                        unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
			return new ASMInstruction(_codeheader,_format,opcode,ope, ins_type ,reg, operand1, operand2);
		}
        	case OP_SGET_WIDE:
		{
			string ope = "sget-wide"; 
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
			return new ASMInstruction(_codeheader,_format,opcode,ope,ins_type,reg, operand1, operand2);
		}
        	case OP_SGET_OBJECT:
		{
			string ope = "sget-object"; 
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
                        return new ASMInstruction(_codeheader,_format,opcode, ope,ins_type, reg, operand1, operand2);
		}
        	case OP_SGET_BOOLEAN:
		{
			string ope = "sget-boolean"; 
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
                        return new ASMInstruction(_codeheader, _format,opcode,ope,ins_type, reg, operand1, operand2);
		}
        	case OP_SGET_BYTE:
		{
			string ope = "sget-byte";
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
                        return new ASMInstruction(_codeheader, _format,opcode, ope, ins_type, reg, operand1, operand2);
		}
        	case OP_SGET_CHAR:
		{
			string ope = "sget-char";
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
                        return new ASMInstruction(_codeheader, _format,opcode,ope, ins_type, reg, operand1, operand2);
		}
        	case OP_SGET_SHORT:
		{
			string ope = "sget-short";
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
                        return new ASMInstruction(_codeheader, _format,opcode,ope, ins_type, reg, operand1, operand2);
		}
        	case OP_SPUT:
		{
			string ope = "sput";
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
			assert(reg->size()>0);
			if(in_use_flag)
				in_use->push_back(reg->at(0));
            ASMInstruction* ins = new ASMInstruction(_codeheader,_format,opcode, ope, ins_type, reg, operand1, operand2);
            if(in_use_flag)
            	ins->setInUseRegs(in_use);
			return ins;
		}
        	case OP_SPUT_WIDE:
		{
			string ope = "sput-wide";
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
			assert(reg->size()>0);
			if(in_use_flag)
				in_use->push_back(reg->at(0));
            ASMInstruction* ins = new ASMInstruction(_codeheader,_format,opcode, ope, ins_type, reg, operand1, operand2);
            if(in_use_flag)
            	ins->setInUseRegs(in_use);
			return ins;
		}
        	case OP_SPUT_OBJECT:
		{
			string ope = "sput-object";
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
                        //return new ASMInstruction(_codeheader, _format,opcode,ope, ins_type, reg, operand1, operand2);
			assert(reg->size()>0);
			if(in_use_flag)
				in_use->push_back(reg->at(0));
            ASMInstruction* ins = new ASMInstruction(_codeheader,_format,opcode, ope, ins_type, reg, operand1, operand2);
            if(in_use_flag)
            	ins->setInUseRegs(in_use);
			return ins;
		}
        	case OP_SPUT_BOOLEAN:
		{
			string ope = "sput-boolean";
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
			assert(reg->size()>0);
			if(in_use_flag)
				in_use->push_back(reg->at(0));
            ASMInstruction* ins = new ASMInstruction(_codeheader,_format,opcode, ope, ins_type, reg, operand1, operand2);
            if(in_use_flag)
            	ins->setInUseRegs(in_use);
			return ins;
		}
        	case OP_SPUT_BYTE:
		{
			string ope = "sput-byte";
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
			assert(reg->size()>0);
			if(in_use_flag)
				in_use->push_back(reg->at(0));
            ASMInstruction* ins = new ASMInstruction(_codeheader,_format,opcode, ope, ins_type, reg, operand1, operand2);
            if(in_use_flag)
            	ins->setInUseRegs(in_use);
			return ins;
		}
        	case OP_SPUT_CHAR:
		{
			string ope = "sput-char";
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
                        //return new ASMInstruction(_codeheader, _format,opcode,ope,ins_type, reg, operand1, operand2);
			assert(reg->size()>0);
			if(in_use_flag)
				in_use->push_back(reg->at(0));
            ASMInstruction* ins = new ASMInstruction(_codeheader,_format,opcode, ope, ins_type, reg, operand1, operand2);
            if(in_use_flag)
            	ins->setInUseRegs(in_use);
			return ins;
		}
        	case OP_SPUT_SHORT:
		{
			string ope = "sput-short";
                        string operand1 = F21cOperand1(in,reg);
			unsigned int fieldidx = F21cOperand2_fieldidx(in);
                        string operand2 = F21cOperand2_field(fieldidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			F21cGetOperandTypes(in,reg,ins_type,fieldidx,ope);
                        //return new ASMInstruction(_codeheader, _format,opcode,ope, ins_type, reg, operand1, operand2);
			assert(reg->size()>0);
			if(in_use_flag)
				in_use->push_back(reg->at(0));
            ASMInstruction* ins = new ASMInstruction(_codeheader,_format,opcode, ope, ins_type, reg, operand1, operand2);
            if(in_use_flag)
            	ins->setInUseRegs(in_use);
			return ins;
		}
		default:
			return NULL;
	}
}

//for 21h const/high16 vaa, #+ BBBB0000 or const-wide/high16 vaa, #+BBBB000000000000  //done
string InstructionProcess::F21hoperand1(DataIn* in,vector<unsigned short>* all_reg)
{
        unsigned short reg = in->readByte();
	all_reg->push_back(reg);
        string oprand1 = "v" + Int2Str(reg);
        return oprand1;
}

//might wrong
string InstructionProcess::F21hoperand2_const(DataIn* in)
{
        short value = in->readShortx();
	int tempvalue = value << 16;
	float* p =(float*)&tempvalue;
	string oprand2 = "#+" + Float2Str(*p);	//hzhu 4.25/2012 changed
        return oprand2;
}

//might wrong
string InstructionProcess::F21hoperand2_const_wide(DataIn* in)
{
        short value = in->readShortx();
        long int tempvalue = ((long int) value) << 48;
        double * p =(double*)&tempvalue;
        string oprand2 = "#+" + Double2Str(*p); //hzhu 4.25/2012 changed
        return oprand2;
}

ASMInstruction* InstructionProcess::processF21h(DataIn* in,DexOpcode opcode, unsigned int address)
{
	in->move(address);
	in->skip(1); //skip the opcode one byte
	vector<unsigned short>* allreg = new vector<unsigned short>();
	switch(opcode)
	{
		case OP_CONST_HIGH16:
		{
			string ope = "const/high16";
			string operand1 = F21hoperand1(in,allreg);
			string operand2 = F21hoperand2_const(in);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			ClassDefInfo* vartype = NULL;
			if(!GLOBAL_MAP)
				vartype = new ClassDefInfo("F");
			else
				vartype = _dfr->getClassDefByName("F");
			//12/20
			unsigned int typeidx = _dfr->getTypeId("F");
			vartype->setTypeIdx(typeidx);
			//12/20
			unsigned short reg = allreg->at(0);

			assert( _declared_map->count(reg) >0);
			assert( _var_type->count(reg) >0);


			if((*_declared_map)[reg] != NULL){// found
			    vartype = (*_declared_map)[reg]; // change temp_type to found one
			}else if(USING_TYPE_INFER){
			    vartype = this->_invalid_type;	
			}
			(*ins_type)[reg] =  vartype;
			if( reg < _codeheader->getFirstArgu())
			    (*_var_type)[reg] = vartype;

			/*(*ins_type)[*allreg->begin()] = classdef;
			if((*(allreg->begin())) < _codeheader->getFirstArgu())
			    (*_var_type)[*allreg->begin()] = classdef;*/
			return new ASMInstruction(_codeheader, _format, opcode,ope,ins_type, allreg,operand1,operand2);
		}
        	case OP_CONST_WIDE_HIGH16:
		{
		string ope = "const-wide/high16";
		string operand1 = F21hoperand1(in, allreg);
		string operand2 = F21hoperand2_const_wide(in);
		map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int,
				ClassDefInfo*>();

		ClassDefInfo* vartype = NULL;
		if (!GLOBAL_MAP)
			vartype = new ClassDefInfo("D");
		else
			vartype = _dfr->getClassDefByName("D");
		//12/20
		unsigned int typeidx = _dfr->getTypeId("D");
		vartype->setTypeIdx(typeidx);
		//12/20
		unsigned short reg = allreg->at(0);

		assert( _declared_map->count(reg) >0);
		assert( _var_type->count(reg) >0);

		if ((*_declared_map)[reg] != NULL) { // found
			vartype = (*_declared_map)[reg]; // change temp_type to found one
		} else if (USING_TYPE_INFER) {
			vartype = this->_invalid_type;
		}
		(*ins_type)[reg] = vartype;
		if (reg < _codeheader->getFirstArgu())
			(*_var_type)[reg] = vartype;

		return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type,
				allreg, operand1, operand2);
	}
		default:
			return NULL;
	}
}

//for 21s const/16 vaa, #+BBBB //done
string InstructionProcess::F21soperand1(DataIn* in,vector<unsigned short>* allregs)
{
	unsigned short reg = in->readByte();
	allregs->push_back(reg);
	string oprand1 = "v" + Int2Str(reg);
	return oprand1;
}

string InstructionProcess::F21soperand2(DataIn* in)
{
	short value = in->readShortx();
	char buffer[50];
	sprintf(buffer,"%d",value);
	string oprand2 = buffer;
	oprand2 = "#+" + oprand2;
	return oprand2;
}

ASMInstruction* InstructionProcess::fillOperandandins_F21s(DexOpcode opcode,string ope,DataIn* in,vector<unsigned short>* allregs)
{
	string operand1 = F21soperand1(in, allregs);
	string operand2 = F21soperand2(in);
	//hzhu begin 4/25/2012
	vector<unsigned short>::iterator it = allregs->begin();
	ClassDefInfo* classdef = NULL;
	string name;
	if (!GLOBAL_MAP) {
		if (opcode == OP_CONST_16) {
			classdef = new ClassDefInfo("I"); //"I" means int
			name = "I";
		} else {
			classdef = new ClassDefInfo("D");
			name = "D";
		}
	} else {
		if (opcode == OP_CONST_16) {
			classdef = _dfr->getClassDefByName("I"); //"I" means int
			name = "I";
		} else {
			classdef = _dfr->getClassDefByName("D");
			name = "D";
		}

	}
	//12/20
	unsigned int typeidx = _dfr->getTypeId(name);
	classdef->setTypeIdx(typeidx);
	//12/20
	map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int,
			ClassDefInfo*>();

	unsigned int reg = (*it);

	assert(_declared_map->count(reg) > 0);
	assert(_var_type->count(reg) > 0);

	if ((*_declared_map)[reg] != NULL) {
		//cout << "found in declared map " << endl;
		(*ins_type)[reg] = (*_declared_map)[reg];
		//update
		(*_var_type)[reg] = (*ins_type)[reg];
	} else if (USING_TYPE_INFER) {
		(*ins_type)[reg] = this->_invalid_type;
	} else {
		(*ins_type)[reg] = classdef;
		//cout << "reg " << reg << "first arg " << _codeheader->getFirstArgu()<< endl;
		if (reg < _codeheader->getFirstArgu())
			(*_var_type)[reg] = classdef; //update the method's local variable types
	}

	/*for(it = allregs->begin(); it!= allregs->end();it++)
	 {
	 (*ins_type)[*it] =  classdef;
	 if(*it < _codeheader->getFirstArgu())
	 (*_var_type)[*it] = classdef;//update the method's local variable types
	 }*/
	//(*ins_type)[1000] = classdef;
	//hzhu end 4/25/2012
	ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode, ope,
			ins_type, allregs, operand1, operand2);
	return ins;
}


ASMInstruction* InstructionProcess::processF21s(DataIn* in,DexOpcode opcode, unsigned int address)
{
	in->move(address);
	in->skip(1); //skip the opcode
	vector<unsigned short>* allreg = new vector<unsigned short>();
	switch(opcode)
	{
		case OP_CONST_16:
		{
			string ope = "const/16"; // sign extended to 32 bits
			ASMInstruction* asm_ins = fillOperandandins_F21s(opcode, ope,in,allreg);
			return asm_ins;
		}
        	case OP_CONST_WIDE_16:
		{
			string ope = "const-wide/16"; //sign extended to 64 bits
			ASMInstruction* asm_ins = fillOperandandins_F21s(opcode,ope,in,allreg);
                        return asm_ins;
		}
		default:
			return NULL;
	}
}
//for 21t if-testz vaa, +vbbbb (8+8+16) //done
string InstructionProcess::F21toperand1(unsigned short reg, vector<unsigned short>* all_regs)
{
	unsigned short test_reg = reg;
	all_regs->push_back(test_reg);
        return "v"+ Int2Str(test_reg);
}

string InstructionProcess::F21toperand2(DataIn* in, unsigned int address)
{
        short offset = in->readShortx();
//	cout << "offset :: " <<offset <<endl;
        unsigned int abs_off =(unsigned int) ((address + offset*2)& 0xffffffff);
//	cout << "operd :: " << abs_off <<endl;
	string oprand2 = "+" + Int2Str(abs_off);
	
/*
        char buffer[50]; // treat it as c string
        sprintf(buffer, "%u",abs_off);
        string oprand2 = buffer;
        oprand2 = "+" + oprand2;
*/
#ifdef DEBUG
        //for test
        in->move(abs_off);
        unsigned short opco = in->readByte()& 0x00ff;
        cout << "add-int/li8 ? opcode is d8 ?"<< opco << endl;
        printf("opcode : %.2x\n",opco);
#endif
        return oprand2;
}

//if-testz vAA, +BBBB
ASMInstruction* InstructionProcess::fillOperandandins_F21t(DexOpcode opcode,string op,unsigned short reg, DataIn* in, unsigned int address,vector<unsigned short>* all_regs)
{
	string oprand1 = F21toperand1(reg,all_regs);
	string oprand2 = F21toperand2(in,address);
	map<unsigned int, ClassDefInfo*>* ins_type =  new map<unsigned int, ClassDefInfo*>();
	
	assert(_var_type->count(reg)>0);
	assert(_declared_map->count(reg)>0);

	ClassDefInfo* temp = NULL;
	if((*_var_type)[reg]!= NULL)
	{
		//(*ins_type)[reg] = (*_var_type)[reg];
	    temp = (*_var_type)[reg];
	}
	if ((*_declared_map)[reg] != NULL){
	    temp = (*_declared_map)[reg];
	}else if(USING_TYPE_INFER){
	    temp = this->_invalid_type;
	}else if((*_var_type)[reg] != NULL){
	    temp = (*_var_type)[reg];
	}
	else
	{
	    cout << "no definition register before on this in use " << reg << endl;
	    if(PRINT_INFO)
	    {
	    	cout << "opcode :" << opcode << endl;
	    	cout << "bail out! No existing register!" << endl;
	    }
	    assert(false);
	}
	(*ins_type)[reg] = temp;
	if(reg < _codeheader->getFirstArgu())
	    (*_var_type)[reg] = (*ins_type)[reg];//update the method's local variable types  
	//(*ins_type)[1000] = new ClassDefInfo("Lable");
	ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode, op,ins_type, all_regs,oprand1,oprand2);
	//cout << "asm :: " << ins->toString()<< endl;
	return ins;
}

ASMInstruction* InstructionProcess::processF21t(DataIn* in,DexOpcode opcode, unsigned int address)
{
	in->move(address);
	in->skip(1); // skip the opcode
	unsigned short reg = in->readByte();
	vector<unsigned short>* allreg = new vector<unsigned short>();
	vector<unsigned short>* in_use = NULL;
	if(in_use_flag)
		in_use = new vector<unsigned short>();
	switch(opcode)
	{	
		case OP_IF_EQZ:
		{
			string ope = "if-eqz";
			ASMInstruction* asm_ins = fillOperandandins_F21t(opcode,ope,reg,in,address,allreg);
			if(in_use_flag){
			in_use->push_back(allreg->at(0));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
			return asm_ins;
		}
	        case OP_IF_NEZ:
		{
			string ope = "if-nez";
			ASMInstruction* asm_ins = fillOperandandins_F21t(opcode,ope,reg,in,address,allreg);
			if(in_use_flag){
			in_use->push_back(allreg->at(0));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
	        case OP_IF_LTZ:
		{
			string ope = "if-ltz";
			ASMInstruction* asm_ins = fillOperandandins_F21t(opcode,ope,reg,in,address,allreg);
			if(in_use_flag){
			in_use->push_back(allreg->at(0));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
	        case OP_IF_GEZ:
		{
			string ope = "if-gez";
			ASMInstruction* asm_ins = fillOperandandins_F21t(opcode,ope,reg,in,address,allreg);
			if(in_use_flag){
			in_use->push_back(allreg->at(0));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
	        case OP_IF_GTZ:
		{
			string ope = "if-gtz";
			ASMInstruction* asm_ins = fillOperandandins_F21t(opcode,ope,reg,in,address,allreg);
			if(in_use_flag){
			in_use->push_back(allreg->at(0));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
	        case OP_IF_LEZ:
		{
			string ope = "if-lez";
			ASMInstruction* asm_ins = fillOperandandins_F21t(opcode,ope,reg,in,address,allreg);
			if(in_use_flag){
			in_use->push_back(allreg->at(0));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
			
		}
                default:
			return NULL;
	}
}




string InstructionProcess::F22boperand1(DataIn* in,vector<unsigned short>* all_regs)
{
        unsigned short dest_reg = in->readByte()& 0x00ff;
	all_regs->push_back(dest_reg);
	string oprand1= "v"+Int2Str(dest_reg);
        return oprand1;
}
string InstructionProcess::F22boperand2(DataIn* in, vector<unsigned short>* all_regs)
{
        unsigned short  sou_reg = in->readByte() & 0x00ff;
	all_regs->push_back(sou_reg);
        string oprand2 = "v"+Int2Str(sou_reg);
        return oprand2;
}

string InstructionProcess::F22boperand3(DataIn* in)
{
        int constant = in->readByte();
        string oprand3 = "#+"+ Int2Str(constant);
        return oprand3;
}

ASMInstruction* InstructionProcess::fillOperandandins_F22b(DexOpcode opcode, string op, DataIn* in,vector<unsigned short>* all_regs)
{
	string oprand1 = F22boperand1(in,all_regs);	
	string oprand2 = F22boperand2(in,all_regs);	
	string oprand3 = F22boperand3(in);	
	vector<unsigned short>::iterator it = all_regs->begin();
	map<unsigned int, ClassDefInfo*>* ins_var = new map<unsigned int, ClassDefInfo*>();
	ClassDefInfo* classdef = NULL;
	if(!GLOBAL_MAP)
		classdef = new ClassDefInfo("I");
	else
		classdef = _dfr->getClassDefByName("I");
	//12/20
	unsigned int typeidx = _dfr->getTypeIdx("I");
	classdef->setTypeIdx(typeidx);
	//12/20
	unsigned int firstreg = 0;
	for(; it != all_regs->end(); it ++)
	{
		if(it == all_regs->begin())
		{
			firstreg = *it;
			if((*_declared_map)[*it] != NULL)
				(*ins_var)[*it] = (*_declared_map)[*it];
			else if(USING_TYPE_INFER)
			{
			        (*ins_var)[*it] = this->_invalid_type;
			}
			else
				(*ins_var)[*it] = classdef;
			
		}
		else
		{
			if((*_declared_map)[*it] != NULL)
				 (*ins_var)[*it] = (*_declared_map)[*it];
			else if((*it) >= _codeheader->getFirstArgu())
				(*ins_var)[*it] = (*_var_type)[*it];
			else if(USING_TYPE_INFER)
                        {
                                 (*ins_var)[*it] = this->_invalid_type;
                        }
			else
				(*ins_var)[*it] = classdef;
			if(*it < _codeheader->getFirstArgu())
			    (*_var_type)[*it] = classdef; //update
		}
		if(firstreg < _codeheader->getFirstArgu())
		    (*_var_type)[firstreg] = classdef; //update
		
	}
	//(*ins_var)[1000] = classdef;
	ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode,op, ins_var, all_regs,oprand1,oprand2,oprand3);
	return ins;
	
}
//binop/lit8 vAA, vBB, #+CC
ASMInstruction* InstructionProcess::processF22b(DataIn* in,DexOpcode opcode, unsigned int address)
{
	in->move(address);
	in->skip(1); // skip the opcode;
	vector<unsigned short>* allreg = new vector<unsigned short>();
	vector<unsigned short>* in_use = NULL;
	if(in_use_flag)
		in_use = new vector<unsigned short>();
	switch(opcode)
	{
		case OP_ADD_INT_LIT8:
		{
			string ope = "add-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode, ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
			return asm_ins;
		}
        	case OP_RSUB_INT_LIT8:
		{
			string ope = "rsub-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode,ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_MUL_INT_LIT8:
		{
			string ope = "mul-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode,ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_DIV_INT_LIT8:
		{
			string ope = "div-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode,ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_REM_INT_LIT8:
		{
			string ope = "rem-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode,ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_AND_INT_LIT8:
		{
			string ope = "and-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode,ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_OR_INT_LIT8:
		{
			string ope = "or-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode,ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_XOR_INT_LIT8:
		{
			string ope = "xor-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode,ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_SHL_INT_LIT8:
		{
			string ope = "shl-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode,ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_SHR_INT_LIT8:
		{
			string ope = "shr-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode,ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_USHR_INT_LIT8:
		{	
			 string ope = "ushr-int/lit8";
			ASMInstruction* asm_ins = fillOperandandins_F22b(opcode,ope,in,allreg);
			assert(allreg->size()>1);
			if(in_use_flag){
			in_use->push_back(allreg->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}		
		default:
			return NULL;
	}
}


//iget va, vb, field@CCCC //done
string InstructionProcess::F22coperand1(unsigned short regs, vector<unsigned short>* all_regs)
{
	unsigned short desti_reg = regs & 0x0f;
	all_regs->push_back(desti_reg);
	return "v"+Int2Str(desti_reg);
}

string InstructionProcess::F22coperand2(unsigned short regs,vector<unsigned short>* all_regs)
{
	unsigned short source_reg = (regs >> 4)&0x0f;
	all_regs->push_back(source_reg);
	return "v"+ Int2Str(source_reg);
}

//hzhu added 4/25/2012
unsigned int InstructionProcess::F22coperand3_typeidx(DataIn* in)
{
	unsigned short typeidx  = in->readShortx();
	return typeidx;
}
//hzhu added 4/25/2012

string InstructionProcess::F22coperand3_type(unsigned int typeidx) //hzhu 4/25/2012 changed the prototype
{
	//string oprand3 = _dfr->getType(typeidx);
	//using map unify type name
	string oprand3 = _dfr->getTypename(typeidx);
	return oprand3;
}

//hzhu added 4/25/2012
unsigned int InstructionProcess::F22coperand3_fieldidx(DataIn* in)
{
	unsigned short fieldidx  = in->readShortx();
	return fieldidx;
}
//hzhu edn 4/25/2012
string InstructionProcess::F22coperand3_field(unsigned int fieldidx) //hzhu  4/25/2012 changed the prototype 
{
        string oprand3 = _dfr->getField(fieldidx)->toString();
        return oprand3;
}

ASMInstruction* InstructionProcess::processF22c(DataIn* in,DexOpcode opcode,unsigned int address)
{	
	in->move(address);
	in->skip(1); //skip the first 8 bits for opcode;
	unsigned short regs = in->readByte();
	vector<unsigned short>* allregs = new vector<unsigned short>();
	vector<unsigned short>* in_use = NULL;
	if(in_use_flag)
		in_use = new vector<unsigned short>();
	switch(opcode)
	{
		case OP_INSTANCE_OF: //done  with the liveness
	{
		string ope = "instance-of";
		string operand1 = F22coperand1(regs, allregs);
		string operand2 = F22coperand2(regs, allregs);
		//hzhu added 4.25.2012
		unsigned int typeidx = F22coperand3_typeidx(in);
		string operand3 = F22coperand3_type(typeidx);
		//hzhu added 4.29.2012
		map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int,
				ClassDefInfo*>();
		vector<unsigned short>::iterator it = allregs->begin();
		unsigned int firstreg = 0;
		ClassDefInfo* classdef = NULL;
		if(!GLOBAL_MAP)
			classdef = new ClassDefInfo("I"); //infer type
		else
			classdef = _dfr->getClassDefByName("I");
		//12/20
		unsigned int typeidx1 = _dfr->getTypeId("I");
		classdef->setTypeIdx(typeidx1);
		//12/20
		for (; it != allregs->end(); it++) {
			assert(_declared_map->count(*it) > 0);
			assert(_var_type->count(*it) > 0);
			if (it == allregs->begin()) {
				firstreg = *it;
				if ((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				else if (USING_TYPE_INFER)
					(*ins_type)[*it] = this->_invalid_type;
				else {
					(*ins_type)[*it] = classdef; //infer type
				}
			} else //the second register in_use
			{
				if (*it == firstreg) {
					if ((*_declared_map)[*it] != NULL)
						(*ins_type)[DUPLICATE_REGISTER] = (*_declared_map)[*it];
					else if (USING_TYPE_INFER) {
						(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
					} else {
						assert((*_var_type)[*it] != NULL);
						(*ins_type)[DUPLICATE_REGISTER] = (*_var_type)[*it];
					}
				} else //not duplicated register
				{
					if ((*_declared_map)[*it] != NULL){
						(*ins_type)[*it] = (*_declared_map)[*it];
					}
					else if (USING_TYPE_INFER) {
						(*ins_type)[*it] = this->_invalid_type;
					} else {
						assert((*_var_type)[*it] != NULL);
						(*ins_type)[*it] = (*_var_type)[*it];
					}
				}
				if (firstreg < _codeheader->getFirstArgu())
					(*_var_type)[firstreg] = classdef; //update
			}
		}
		assert(allregs->size()> 1);
		if(in_use_flag)
			in_use->push_back(allregs->at(1));
		return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type,
				allregs, operand1, operand2, operand3);
	}
        	case OP_NEW_ARRAY: //23 new-array vx vy, type_idx //vx defined here and vy in use
		{
			string ope = "new-array";
			string operand1 = F22coperand1(regs,allregs);
			string operand2 = F22coperand2(regs,allregs);
			//hzhu added 4.25.2012
                        unsigned int typeidx = F22coperand3_typeidx(in);
                        string operand3 = F22coperand3_type(typeidx);
			
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>;
                        ClassDefInfo* classdef = NULL;
			ClassDefInfo* temp1 = NULL;

			typedef vector<unsigned short>::iterator pos;
			pos begin(allregs->begin());                                 
  			pos end(allregs->end());
			reverse_iterator<pos> rev_end (begin);                      
  			reverse_iterator<pos> rev_iterator (end);
			unsigned int count = 1;
			unsigned int lastreg = 0; //nothing affect, just initilize
			while(rev_iterator < rev_end)
			{
				assert(_declared_map->count(*rev_iterator) > 0);
				assert(_var_type->count(*rev_iterator) > 0);
				if(count == 1) // the tail of the register
				{
					lastreg = *rev_iterator; //keep it
					if((*_declared_map)[*rev_iterator] != NULL)
					    temp1 = (*_declared_map)[*rev_iterator];
					else if(USING_TYPE_INFER){
					     temp1 = this->_invalid_type;
					}
					else
					{   
					    if((*_var_type)[*rev_iterator] != NULL)
					    temp1 = (*_var_type)[*rev_iterator]; //keep it, because it may duplicated register
					}
					//(*ins_type)[*rev_iterator] = (*_var_type)[*rev_iterator];
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					
					if(*rev_iterator == lastreg) //duplicated register
					{
						if((*_declared_map)[*rev_iterator] != NULL)
						{
						    (*ins_type)[*rev_iterator] = (*_declared_map)[*rev_iterator];
						}else if(USING_TYPE_INFER){
							(*ins_type)[*rev_iterator] = this->_invalid_type;
						}
						else //use infered type from the instruction
						{
						    assert(classdef != NULL);
						    (*ins_type)[*rev_iterator] = classdef; //same register number, the other denoted as 2000
						}
						(*ins_type)[DUPLICATE_REGISTER] = temp1;
					}
					else //not duplicate register
					{
						if((*_declared_map)[*rev_iterator] != NULL)
						{
						    (*ins_type)[*rev_iterator] = (*_declared_map)[*rev_iterator];
						}else if(USING_TYPE_INFER){
						   (*ins_type)[*rev_iterator] = this->_invalid_type;
						}
						else //use infered type from the instruction
						{
						    assert(classdef != NULL);
						    (*ins_type)[*rev_iterator] = classdef; //same register number, the other denoted as 2000
						}
						//(*ins_type)[*rev_iterator] = classdef;
						(*ins_type)[lastreg] = temp1;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = (*ins_type)[*rev_iterator];
					rev_iterator ++;
					count ++;
				}
			}
                        //hzhu ended 4.25.2012
			assert(allregs->size()>1);
			if(in_use_flag)
				in_use->push_back(allregs->at(1));
			//return new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IGET://52  iget vaa, vbb ,fieldid
		{
			string ope = "iget";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
			//hzhu 4.25.2012
			unsigned int fieldidx = F22coperand3_fieldidx(in);
                	string operand3 = F22coperand3_field(fieldidx);
			Field* fie = _dfr->getField(fieldidx);
			unsigned int typeidx = fie->getTypeIdx();
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>;
                        ClassDefInfo* classdef = NULL;
			ClassDefInfo* temp = NULL;

			typedef vector<unsigned short>::iterator pos;
			pos begin(allregs->begin());                                 
  			pos end(allregs->end());
			reverse_iterator<pos> rev_end (begin);                      
  			reverse_iterator<pos> rev_iterator (end);
			unsigned int count = 1;
			unsigned int lastreg = 0; //nothing affect
			while(rev_iterator < rev_end)
			{
				assert(_declared_map->count(*rev_iterator) > 0);
				assert(_var_type->count(*rev_iterator) > 0);
				if(count == 1) // the tail of the register vbb in_use
				{
					lastreg = *rev_iterator; //keep it
					if((*_declared_map)[*rev_iterator] != NULL)
					{
					    temp = (*_declared_map)[*rev_iterator];
					}else if(USING_TYPE_INFER){
					    temp = this->_invalid_type;
					}
					else
					{
					    assert((*_var_type)[*rev_iterator] != NULL);
					    temp = (*_var_type)[*rev_iterator];
					}
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg) //duplicated register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER){
						(*ins_type)[*rev_iterator] = this->_invalid_type;
					    }
					    else
						(*ins_type)[*rev_iterator] = classdef;
                                            (*ins_type)[DUPLICATE_REGISTER] = temp; //same register number, the other denoted as 200
					}
					else  // different register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER){
						(*ins_type)[*rev_iterator] =  this->_invalid_type;
					    }
					    else //infer
						(*ins_type)[*rev_iterator] = classdef;
					    (*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = (*ins_type)[*rev_iterator] ;
					rev_iterator ++;
					count ++;
				}
			}
                        //hzhu ended 4.25.2012
                	//return new ASMInstruction(_codeheader, _format,opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			assert(allregs->size()>1);
			if(in_use_flag)
				in_use->push_back(allregs->at(1));
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IGET_WIDE:
		{	
			string ope = "iget-wide";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
			//hzhu 4.25.2012
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
			//hzhu 4.25.2012
			Field* fie = _dfr->getField(fieldidx);
			unsigned int typeidx = fie->getTypeIdx();
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>;
                        ClassDefInfo* classdef = NULL;
			ClassDefInfo* temp = NULL;
			typedef vector<unsigned short>::iterator pos;
			pos begin(allregs->begin());                                 
  			pos end(allregs->end());
			reverse_iterator<pos> rev_end (begin);                      
  			reverse_iterator<pos> rev_iterator (end);
			unsigned int count = 1;
			unsigned int lastreg = 0;//nothing affect
			while(rev_iterator < rev_end)
			{
				assert(_declared_map->count(*rev_iterator) > 0);
				assert(_var_type->count(*rev_iterator) > 0);
				if(count == 1) // the tail of the register vbb in_use
				{
					lastreg = *rev_iterator; //keep it
					if((*_declared_map)[*rev_iterator] != NULL)
					{
					    temp = (*_declared_map)[*rev_iterator];
					}else if(USING_TYPE_INFER){
					    temp = this->_invalid_type;
					}
					else
					{
					    assert((*_var_type)[*rev_iterator] != NULL);
					    temp = (*_var_type)[*rev_iterator];
					}
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg) //duplicated register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER){
						 (*ins_type)[*rev_iterator] = this->_invalid_type;
                                            }
					    else
						(*ins_type)[*rev_iterator] = classdef;
                                            (*ins_type)[DUPLICATE_REGISTER] = temp; //same register number, the other denoted as 2000
					}
					else  // different register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER){
						(*ins_type)[*rev_iterator] = this->_invalid_type;
					    }
					    else
						(*ins_type)[*rev_iterator] = classdef;
					    (*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = (*ins_type)[*rev_iterator] ;
					rev_iterator ++;
					count ++;
				}
				/*
				if(count == 1) // the tail of the register
				{
					lastreg = *rev_iterator; //keep it
					if((*_var_type)[*rev_iterator] != NULL)
					    temp = (*_var_type)[*rev_iterator];
					else
					{
					    //cout << "In else " << endl;
					    temp = _codeheader->getClassDefInfo();
					    assert(temp != NULL);
					    if(lastreg < _codeheader->getFirstArgu())
						(*_var_type)[lastreg] = temp;
					}
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg)
					{
						(*ins_type)[*rev_iterator] = classdef;
                                        	(*ins_type)[2000] = temp; //same register number, the other denoted as 200
					}
					else
					{
						(*ins_type)[*rev_iterator] = classdef;
						(*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = classdef;
					rev_iterator ++;
					count ++;
				}
				*/
			}
                        //hzhu ended 4.25.2012
                	//return new ASMInstruction(_codeheader, _format,opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			assert(allregs->size()>1);
			if(in_use_flag)
				in_use->push_back(allregs->at(1));
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IGET_OBJECT:
		{
			string ope = "iget-object";
			string operand1 = F22coperand1(regs,allregs);
            string operand2 = F22coperand2(regs,allregs);
			//hzhu 4.25.2012
            unsigned int fieldidx = F22coperand3_fieldidx(in);
            string operand3 = F22coperand3_field(fieldidx);
            //cout << "operand3  is " << operand3 << endl;


            /**********
             * To fix corner cases;
             * I would like to add flag to indicate if it is library class, if it is, I would like to
             * use ASMInstruction's "type" information as it declared type(not based on debug information),
             *  because .........
             * e.g A->B(A is base type)
             * B has field b, but A doesn't, if we never
             * see iget/iput instructions use A->b, we never know A has field b, when in type-inference,
             * if v0 = v1->b (said, v1 is A type, if fails to find the v1->b type and propagate it to lhs(v0))
             */
            bool lib_flag = false;
            ClassDefInfo* object_cdi = NULL;
            if(operand3.find("Ljava") != string::npos)
            	lib_flag = true;

            if(operand3.find("Landroid") != string::npos)
            	lib_flag = true;


            //hzhu 4.25.2012
			
			//hzhu 4.25.2012
			Field* fie = _dfr->getField(fieldidx);
			unsigned int typeidx = fie->getTypeIdx();

			if(lib_flag){
				object_cdi = type2ClassDef(_dfr, fie->getClassOwnerIdx());;
			}


			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>;
            ClassDefInfo* classdef = NULL;
			ClassDefInfo* temp = NULL;
			typedef vector<unsigned short>::iterator pos;
			pos begin(allregs->begin());                                 
  			pos end(allregs->end());
			reverse_iterator<pos> rev_end (begin);                      
  			reverse_iterator<pos> rev_iterator (end);
			unsigned int count = 1;
			unsigned int lastreg = 0; //nothing affect
			while(rev_iterator < rev_end) //iget-object v6,v13, ~
			{
				assert(_declared_map->count(*rev_iterator) > 0);
				assert(_var_type->count(*rev_iterator) > 0);
				if(count == 1) // the tail of the register vbb in_use
				{
					lastreg = *rev_iterator; //keep it
					if((*_declared_map)[*rev_iterator] != NULL)
					{
					    temp = (*_declared_map)[*rev_iterator];
					}else if(lib_flag){
						assert(object_cdi != NULL);
						//cout << "set as " << object_cdi->getTypename() << endl;
						temp = object_cdi;
					}
					else if(USING_TYPE_INFER){
					    temp = this->_invalid_type;
					}
					else
					{
					    //cout << "need to find  " << *rev_iterator << " also last reg " <<lastreg << endl; 
					    assert((*_var_type)[*rev_iterator] != NULL);
					    temp = (*_var_type)[*rev_iterator];
					}
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg) //duplicated register
					{
						if((*_declared_map)[*rev_iterator] != NULL)
							(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
						else if(lib_flag){
							(*ins_type)[*rev_iterator] = classdef;
						}
						else if(USING_TYPE_INFER){
							(*ins_type)[*rev_iterator] = this->_invalid_type;
						}
						else
							(*ins_type)[*rev_iterator] = classdef;
						(*ins_type)[DUPLICATE_REGISTER] = temp; //same register number, the other denoted as 200
					}
					else  // different register
					{
						if((*_declared_map)[*rev_iterator] != NULL)
							(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
						else if(lib_flag)
							(*ins_type)[*rev_iterator] = classdef;
						else if(USING_TYPE_INFER)
							(*ins_type)[*rev_iterator] = this->_invalid_type;
						else
							(*ins_type)[*rev_iterator] = classdef;
						(*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = (*ins_type)[*rev_iterator] ;
					rev_iterator ++;
					count ++;
				}
				/*
				if(count == 1) // the tail of the register v13 object
				{
					lastreg = *rev_iterator; //keep it
					//cout << "iget-object object reg :" << lastreg << endl;
					if((*_var_type)[*rev_iterator] != NULL)
					{
					  //  cout << "iget-object has existing object register" << *rev_iterator <<endl;
					    temp = (*_var_type)[*rev_iterator];
					}
					else
					{
					    //cout << "In else " << endl;
					    temp = _codeheader->getClassDefInfo();
					    assert(temp != NULL);
					    if(lastreg < _codeheader->getFirstArgu())
						(*_var_type)[lastreg] = temp;
					}
					rev_iterator++;
					count ++;
				}
				else //the first register v6 field
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					//cout << "FIELD::" <<classdef->getTypename()<<endl;
					if(*rev_iterator == lastreg)
					{
						(*ins_type)[*rev_iterator] = classdef;
                                        	(*ins_type)[2000] = temp; //same register number, the other denoted as 200
					}
					else
					{
						(*ins_type)[*rev_iterator] = classdef;
						(*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = classdef;
					//cout << "update the register :" << *rev_iterator <<"->" <<classdef->getTypename()<<endl;
					rev_iterator ++;
					count ++;
				}
				*/
			}
                        //hzhu ended 4.25.2012
                	//return new ASMInstruction(_codeheader, _format,opcode, ope, ins_type, allregs, operand1, operand2,operand3);
			assert(allregs->size()>1);
			if(in_use_flag)
				in_use->push_back(allregs->at(1));
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IGET_BOOLEAN:
		{
			string ope = "iget-boolean";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
                	//string operand3 = F22coperand3_field(in);

			//hzhu 4.25.2012
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
                        //hzhu 4.25.2012

                        //hzhu begin 4.25.2012
			Field* fie = _dfr->getField(fieldidx);
			unsigned int typeidx = fie->getTypeIdx();
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>;
                        ClassDefInfo* classdef = NULL;
			ClassDefInfo* temp = NULL;
			typedef vector<unsigned short>::iterator pos;
			pos begin(allregs->begin());                                 
  			pos end(allregs->end());
			reverse_iterator<pos> rev_end (begin);                      
  			reverse_iterator<pos> rev_iterator (end);
			unsigned int count = 1;
			unsigned int lastreg = 0; //nothing affect
			while(rev_iterator < rev_end)
			{
				assert(_declared_map->count(*rev_iterator) > 0);
				assert(_var_type->count(*rev_iterator) > 0);
				if(count == 1) // the tail of the register vbb in_use
				{
					lastreg = *rev_iterator; //keep it
					if((*_declared_map)[*rev_iterator] != NULL)
					{
					    temp = (*_declared_map)[*rev_iterator];
					}
					else if(USING_TYPE_INFER){
					   temp = this->_invalid_type;
					}
					else
					{
					    assert((*_var_type)[*rev_iterator] != NULL);
					    temp = (*_var_type)[*rev_iterator];
					}
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg) //duplicated register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER){
						 (*ins_type)[*rev_iterator] = this->_invalid_type;
					    }
					    else
						(*ins_type)[*rev_iterator] = classdef;
                                            (*ins_type)[DUPLICATE_REGISTER] = temp; //same register number, the other denoted as 200
					}
					else  // different register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER){
						(*ins_type)[*rev_iterator] = this->_invalid_type;
					    }
					    else
						(*ins_type)[*rev_iterator] = classdef;
					    (*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = (*ins_type)[*rev_iterator] ;
					rev_iterator ++;
					count ++;
				}
				/*
				if(count == 1) // the tail of the register
				{
					lastreg = *rev_iterator; //keep it
					if((*_var_type)[*rev_iterator] != NULL)
					    temp = (*_var_type)[*rev_iterator];
					else
					{
					    temp = _codeheader->getClassDefInfo();
					    assert(temp != NULL);
					    if(lastreg < _codeheader->getFirstArgu())
						(*_var_type)[lastreg] = temp;
					}
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg)
					{
						(*ins_type)[*rev_iterator] = classdef;
                                        	(*ins_type)[2000] = temp; //same register number, the other denoted as 200
					}
					else
					{
						(*ins_type)[*rev_iterator] = classdef;
						(*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = classdef;
					rev_iterator ++;
					count ++;
				}
				*/
			}
                        //hzhu ended 4.25.2012
                	//return new ASMInstruction(_codeheader, _format,opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			assert(allregs->size()>1);
			if(in_use_flag)
				in_use->push_back(allregs->at(1));
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IGET_BYTE:
		{
			string ope = "iget-byte";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
                	//string operand3 = F22coperand3_field(in);
			
			//hzhu 4.25.2012
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
                        //hzhu 4.25.2012
			
                        //hzhu begin 4.25.2012
			Field* fie = _dfr->getField(fieldidx);
			unsigned int typeidx = fie->getTypeIdx();
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>;
                        ClassDefInfo* classdef = NULL;
			ClassDefInfo* temp = NULL;
			typedef vector<unsigned short>::iterator pos;
			pos begin(allregs->begin());                                 
  			pos end(allregs->end());
			reverse_iterator<pos> rev_end (begin);                      
  			reverse_iterator<pos> rev_iterator (end);
			unsigned int count = 1;
			unsigned int lastreg = 0; 
			while(rev_iterator < rev_end)
			{   
				assert(_declared_map->count(*rev_iterator) > 0);
				assert(_var_type->count(*rev_iterator) > 0);
				if(count == 1) // the tail of the register vbb in_use
				{
					lastreg = *rev_iterator; //keep it
					if((*_declared_map)[*rev_iterator] != NULL)
					{
					    temp = (*_declared_map)[*rev_iterator];
					}else if(USING_TYPE_INFER){
					    temp = this->_invalid_type;
					}
					else
					{
					    assert((*_var_type)[*rev_iterator] != NULL);
					    temp = (*_var_type)[*rev_iterator];
					}
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg) //duplicated register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER){
						(*ins_type)[*rev_iterator] =  this->_invalid_type;
					    }
					    else
						(*ins_type)[*rev_iterator] = classdef;
                                            (*ins_type)[DUPLICATE_REGISTER] = temp; //same register number, the other denoted as 200
					}
					else  // different register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER){
						(*ins_type)[*rev_iterator] = this->_invalid_type;
					    }
					    else
						(*ins_type)[*rev_iterator] = classdef;
					    (*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = (*ins_type)[*rev_iterator] ;
					rev_iterator ++;
					count ++;
				}
				/*
				if(count == 1) // the tail of the register
				{
					lastreg = *rev_iterator; //keep it
					if((*_var_type)[*rev_iterator] != NULL)
					    temp = (*_var_type)[*rev_iterator];
					else
					{
					    temp = _codeheader->getClassDefInfo();
					    assert(temp != NULL);
					    if(lastreg < _codeheader->getFirstArgu())
						(*_var_type)[lastreg] = temp;
					}
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg)
					{
						(*ins_type)[*rev_iterator] = classdef;
                                        	(*ins_type)[2000] = temp; //same register number, the other denoted as 200
					}
					else
					{
						(*ins_type)[*rev_iterator] = classdef;
						(*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = classdef;
					rev_iterator ++;
					count ++;
				}
				*/
			}
                        //hzhu ended 4.25.2012
                	//return new ASMInstruction(_codeheader,_format, opcode, ope, ins_type, allregs, operand1, operand2,operand3);
			assert(allregs->size()>1);
			if(in_use_flag)
				in_use->push_back(allregs->at(1));
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IGET_CHAR:
		{
			string ope = "iget-char";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
                	//string operand3 = F22coperand3_field(in);
			//hzhu 4.25.2012
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
                        //hzhu 4.25.2012

                        //hzhu begin 4.25.2012
			Field* fie = _dfr->getField(fieldidx);
			unsigned int typeidx = fie->getTypeIdx();
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>;
                        ClassDefInfo* classdef = NULL;
			ClassDefInfo* temp = NULL;
			typedef vector<unsigned short>::iterator pos;
			pos begin(allregs->begin());                                 
  			pos end(allregs->end());
			reverse_iterator<pos> rev_end (begin);                      
  			reverse_iterator<pos> rev_iterator (end);
			unsigned int count = 1;
			unsigned int lastreg = 0;
			while(rev_iterator < rev_end)
			{
				assert(_declared_map->count(*rev_iterator) > 0);
				assert(_var_type->count(*rev_iterator) > 0);
				if(count == 1) // the tail of the register vbb in_use
				{
					lastreg = *rev_iterator; //keep it
					if((*_declared_map)[*rev_iterator] != NULL)
					{
					    temp = (*_declared_map)[*rev_iterator];
					}
					else if(USING_TYPE_INFER){
					    temp = this->_invalid_type;
					}
					else
					{
					    assert((*_var_type)[*rev_iterator] != NULL);
					    temp = (*_var_type)[*rev_iterator];
					}
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg) //duplicated register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER){
						(*ins_type)[*rev_iterator] = this->_invalid_type;
					    }
					    else
						(*ins_type)[*rev_iterator] = classdef;
                                            (*ins_type)[DUPLICATE_REGISTER] = temp; //same register number, the other denoted as 200
					}
					else  // different register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER)
						(*ins_type)[*rev_iterator] = this->_invalid_type;
					    else
						(*ins_type)[*rev_iterator] = classdef;
					    (*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = (*ins_type)[*rev_iterator] ;
					rev_iterator ++;
					count ++;
				}
				/*
				if(count == 1) // the tail of the register
				{
					lastreg = *rev_iterator; //keep it
					if((*_var_type)[*rev_iterator] != NULL)
					    temp = (*_var_type)[*rev_iterator];
					else
					{
					    temp = _codeheader->getClassDefInfo();
					    assert(temp != NULL);
					    if(lastreg < _codeheader->getFirstArgu())
						(*_var_type)[lastreg] = temp;
					}
				//	(*ins_type)[*rev_iterator] = (*_var_type)[*rev_iterator];
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg)
					{
						(*ins_type)[*rev_iterator] = classdef;
                                        	(*ins_type)[2000] = temp; //same register number, the other denoted as 200
					}
					else
					{
						(*ins_type)[*rev_iterator] = classdef;
						(*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = classdef;
					rev_iterator ++;
					count ++;
				}
				*/
			}
                        //hzhu ended 4.25.2012
                	//return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type, allregs, operand1, operand2,operand3);
			assert(allregs->size()>1);
			if(in_use_flag)
				in_use->push_back(allregs->at(1));
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IGET_SHORT://58
		{
			string ope = "iget-short";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
			//hzhu 4.25.2012
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
                        //hzhu 4.25.2012

                        //hzhu begin 4.25.2012
			Field* fie = _dfr->getField(fieldidx);
			unsigned int typeidx = fie->getTypeIdx();
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>;
                        ClassDefInfo* classdef = NULL;
			ClassDefInfo* temp = NULL;
			typedef vector<unsigned short>::iterator pos;
			pos begin(allregs->begin());                                 
  			pos end(allregs->end());
			reverse_iterator<pos> rev_end (begin);                      
  			reverse_iterator<pos> rev_iterator (end);
			unsigned int count = 1;
			unsigned int lastreg = 0;
			while(rev_iterator < rev_end)
			{
				assert(_declared_map->count(*rev_iterator) > 0);
				assert(_var_type->count(*rev_iterator) > 0);
				if(count == 1) // the tail of the register vbb in_use
				{
					lastreg = *rev_iterator; //keep it
					if((*_declared_map)[*rev_iterator] != NULL)
					{
					    temp = (*_declared_map)[*rev_iterator];
					}
					else if(USING_TYPE_INFER){
					    temp = this->_invalid_type;
					}
					else
					{
					    assert((*_var_type)[*rev_iterator] != NULL);
					    temp = (*_var_type)[*rev_iterator];
					}
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg) //duplicated register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER){
						(*ins_type)[*rev_iterator] = this->_invalid_type;
					    }
					    else
						(*ins_type)[*rev_iterator] = classdef;
                                            (*ins_type)[DUPLICATE_REGISTER] = temp; //same register number, the other denoted as 200
					}
					else  // different register
					{
					    if((*_declared_map)[*rev_iterator] != NULL)
						(*ins_type)[*rev_iterator] =(*_declared_map)[*rev_iterator] ;
					    else if(USING_TYPE_INFER)
						(*ins_type)[*rev_iterator] = this->_invalid_type;
					    else
						(*ins_type)[*rev_iterator] = classdef;
					    (*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = (*ins_type)[*rev_iterator] ;
					rev_iterator ++;
					count ++;
				}
				/*
				if(count == 1) // the tail of the register
				{
					lastreg = *rev_iterator; //keep it
					//temp = (*_var_type)[*rev_iterator];
					if((*_var_type)[*rev_iterator] != NULL)
					    temp = (*_var_type)[*rev_iterator];
					else
					{
					    temp = _codeheader->getClassDefInfo();
					    assert(temp != NULL);
					    if(lastreg < _codeheader->getFirstArgu())
						(*_var_type)[lastreg] = temp;
					}
				//	(*ins_type)[*rev_iterator] = (*_var_type)[*rev_iterator];
					rev_iterator++;
					count ++;
				}
				else
				{
					classdef = type2ClassDef(_dfr,typeidx); //get the type info for the first operand
					if(*rev_iterator == lastreg)
					{
						(*ins_type)[*rev_iterator] = classdef;
                                        	(*ins_type)[2000] = temp; //same register number, the other denoted as 2000
					}
					else
					{
						(*ins_type)[*rev_iterator] = classdef;
						(*ins_type)[lastreg] = temp;
					}
					if(*rev_iterator < _codeheader->getFirstArgu())
					    (*_var_type)[*rev_iterator] = classdef;
					rev_iterator ++;
					count ++;
				}
				*/
			}
                        //hzhu ended 4.25.2012
                	//return new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			assert(allregs->size()>1);
			if(in_use_flag)
				in_use->push_back(allregs->at(1));
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IPUT:
		{
			string ope = "iput";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
			//hzhu 4.25.2012
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
			///Field* fie = _dfr->getField(fieldidx);
                        string operand3 = F22coperand3_field(fieldidx);
                        //hzhu 4.25.2012
			//hzhu begin 4.25.2012 //this should be settled for all the registers //store it to memory
			//ClassDefInfo* classdef = new ClassDefInfo("I");
			vector<unsigned short>::iterator it = allregs->begin();
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			assert(allregs->at(0) != allregs->at(1));
			for(; it!= allregs->end(); it ++) //two registers are the both in use
			{
				assert(_declared_map->count(*it) > 0);
				assert(_var_type->count(*it) > 0);
				if(it == allregs->begin())// the first reg is the field register
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
				else //second reg is the object register
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER)
					(*ins_type)[*it] = this->_invalid_type;
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
			}
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));}
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
			//return asm_ins;
		}
        	case OP_IPUT_WIDE:
		{
			string ope = "iput-wide";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
                	//string operand3 = F22coperand3_field(in);
			//hzhu 4.25.2012
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
                        //hzhu 4.25.2012

                        //hzhu 4.25.2012
			Field* fie = _dfr->getField(fieldidx);
                        unsigned int typeidx = fie->getTypeIdx();
			//string cur_class_str = _dfr->getType(typeidx);
			//using map unify type name
			string cur_class_str = _dfr->getTypename(typeidx);
			/*
                        ClassDefInfo* classdef = NULL;
			if(_dfr->insideClassDef(cur_class_str))
			    classdef = _dfr->getClassDef(cur_class_str);
			else
			    classdef = new ClassDefInfo(cur_class_str);
			*/
			vector<unsigned short>::iterator it = allregs->begin();
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			assert(allregs->at(0) != allregs->at(1));
			for(; it!= allregs->end(); it ++)
			{
				assert(_declared_map->count(*it) > 0);
				assert(_var_type->count(*it) > 0);
				if(it == allregs->begin())// the first reg is the field register
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
                                        (*ins_type)[*it] = this->_invalid_type;													
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
				else //second reg is the object register
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER)
					 (*ins_type)[*it] = this->_invalid_type;
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
				
				/*
				if(it == allregs->begin())
				{
					(*ins_type)[*it] = classdef;
				}
				else
				{
				    if((*_var_type)[*it] != NULL)
					(*ins_type)[*it] = (*_var_type)[*it];
				    else
				    {
					unsigned int classOwnerid = fie->getClassOwnerIdx();
					ClassDefInfo* classtype = type2ClassDef(_dfr, classOwnerid);
					assert(classtype != NULL);
					(*ins_type)[*it] = classtype;
					if(*it < _codeheader->getFirstArgu())
					    (*_var_type)[*it] = classtype;
					//cout << "||||||||||||||||||||||||" <<endl;
					//cout << "iput-wide" << endl;
					//cout <<"bail out!!" << endl;
				    }
				}
				*/
			}
			//if( (*(allregs->begin()))< _codeheader->getFirstArgu())
			//    (*_var_type)[*allregs->begin()] = classdef;
			//hzhu end 4.25.2012
			//ASMInstruction* asm_ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
                	//return new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));}
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
			//return asm_ins;
		}
        	case OP_IPUT_OBJECT:
		{
			string ope = "iput-object";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
                	//string operand3 = F22coperand3_field(in);
			//hzhu 4.25.2012
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
                        //hzhu 4.25.2012
			//Field* fie = _dfr->getField(fieldidx);
                        //unsigned int typeidx = fie->getTypeIdx();
                        //ClassDefInfo* classdef = type2ClassDef(_dfr, typeidx);
                        vector<unsigned short>::iterator it = allregs->begin();
                        map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			//assert(allregs->at(0) != allregs->at(1));
                        for(; it!= allregs->end(); it ++)
                        {
				assert(_declared_map->count(*it) > 0);
				assert(_var_type->count(*it) > 0);
				if(it == allregs->begin())// the first reg is source reg
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
				else //second reg is object register
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
				
				/*
				if(it == allregs->begin())
				{
					(*ins_type)[*it] = classdef;
				}
				else
				{
				    if((*_var_type)[*it] != NULL)
					(*ins_type)[*it] = (*_var_type)[*it];
				    else
				    {
					unsigned int classOwnerid = fie->getClassOwnerIdx();
					ClassDefInfo* classtype = type2ClassDef(_dfr, classOwnerid);
					assert(classtype != NULL);
					(*ins_type)[*it] = classtype;
					if(*it < _codeheader->getFirstArgu())
					    (*_var_type)[*it] = classtype;
					//cout << "||||||||||||||||||||||||" <<endl;
					//cout << "iput-wide" << endl;
					//cout <<"bail out!!" << endl;
				    }
				}
				*/
			}
			//if( (*(allregs->begin()))< _codeheader->getFirstArgu())
			//    (*_var_type)[*allregs->begin()] = classdef;
			//hzhu end 4.25.2012
			//ASMInstruction* asm_ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
                	//return new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));}
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
			//return asm_ins;
		}
        	case OP_IPUT_BOOLEAN:
		{
			string ope = "iput-boolean";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
                	//string operand3 = F22coperand3_field(in);
			//hzhu 4.25.2012
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
			///Field* fie = _dfr->getField(fieldidx);
                        //hzhu 4.25.2012
                        vector<unsigned short>::iterator it = allregs->begin();
                        map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			///ClassDefInfo* classdef = new ClassDefInfo("Z");
			assert(allregs->at(0) != allregs->at(1));
                        for(; it!= allregs->end(); it ++)
                        {
				assert(_declared_map->count(*it) > 0);
				assert(_var_type->count(*it) > 0);
				if(it == allregs->begin())// the first reg is source reg
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){	
					(*ins_type)[*it] = this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
				else //second reg is object register
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
			    /*
				if(it == allregs->begin())
				{
					(*ins_type)[*it] = classdef;
				}
                                else
				{
				    if((*_var_type)[*it] != NULL)
					(*ins_type)[*it] = (*_var_type)[*it];
				    else
				    {
					unsigned int classOwnerid = fie->getClassOwnerIdx();
					ClassDefInfo* classtype = type2ClassDef(_dfr, classOwnerid);
					assert(classtype != NULL);
					(*ins_type)[*it] = classtype;
					if(*it < _codeheader->getFirstArgu())
					    (*_var_type)[*it] = classtype;
				    }
				}
				*/
                        }
                	//return new ASMInstruction(_codeheader,_format, opcode, ope, ins_type, allregs, operand1, operand2,operand3);
			//ASMInstruction* asm_ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			//return asm_ins;
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));}
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IPUT_BYTE:
		{
			string ope = "iput-byte";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
                	//string operand3 = F22coperand3_field(in);
			//hzhu 4.25.2012
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
			///Field* fie = _dfr->getField(fieldidx);
                        //hzhu 4.25.2012
                        vector<unsigned short>::iterator it = allregs->begin();
                        map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			//ClassDefInfo* classdef = new ClassDefInfo("B");
                        for(; it!= allregs->end(); it ++)
                        {
				assert(_declared_map->count(*it) > 0);
				assert(_var_type->count(*it) > 0);
				if(it == allregs->begin())// the first reg is source reg
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
				else //second reg is object register
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					 (*ins_type)[*it] =this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}

				/*
				if(it == allregs->begin())
				{
					(*ins_type)[*it] = classdef;
				}
                                else
				{
				    if((*_var_type)[*it] != NULL)
					(*ins_type)[*it] = (*_var_type)[*it];
				    else
				    {
					unsigned int classOwnerid = fie->getClassOwnerIdx();
					ClassDefInfo* classtype = type2ClassDef(_dfr, classOwnerid);
					assert(classtype != NULL);
					(*ins_type)[*it] = classtype;
					if(*it < _codeheader->getFirstArgu())
					    (*_var_type)[*it] = classtype;
				    }
				}
				*/
                        }
			//if((*(allregs->begin())) < _codeheader->getFirstArgu())
			//    (*_var_type)[*allregs->begin()] = classdef;
                	//return new ASMInstruction(_codeheader, _format, opcode,ope, ins_type,allregs, operand1, operand2,operand3);
			//ASMInstruction* asm_ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			//return asm_ins;
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));}
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IPUT_CHAR:
		{
			string ope = "iput-char";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
			///Field* fie = _dfr->getField(fieldidx);
                        vector<unsigned short>::iterator it = allregs->begin();
                        map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			//ClassDefInfo* classdef = new ClassDefInfo("C");
			assert(allregs->at(0) != allregs->at(1));
                        for(; it!= allregs->end(); it ++)
                        {
				assert(_declared_map->count(*it) > 0);
				assert(_var_type->count(*it) > 0);
				if(it == allregs->begin())// the first reg is source reg
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
				else //second reg is object register
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					 (*ins_type)[*it] = this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
			    /*
				if(it == allregs->begin())
				{
					(*ins_type)[*it] = classdef;
				}
                                else
				{
				    if((*_var_type)[*it] != NULL)
					(*ins_type)[*it] = (*_var_type)[*it];
				    else
				    {
					unsigned int classOwnerid = fie->getClassOwnerIdx();
					ClassDefInfo* classtype = type2ClassDef(_dfr, classOwnerid);
					assert(classtype != NULL);
					(*ins_type)[*it] = classtype;
					if(*it < _codeheader->getFirstArgu())
					    (*_var_type)[*it] = classtype;
				    }
				}
				*/
                        }
			//if((*(allregs->begin())) < _codeheader->getFirstArgu())
			//    (*_var_type)[*allregs->begin()] = classdef;
                	//return new ASMInstruction(_codeheader, _format, opcode,ope,ins_type,allregs, operand1, operand2,operand3);
			//ASMInstruction* asm_ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			//return asm_ins;
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));}
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_IPUT_SHORT:
		{
			string ope = "iput-short";
			string operand1 = F22coperand1(regs,allregs);
                	string operand2 = F22coperand2(regs,allregs);
                        unsigned int fieldidx = F22coperand3_fieldidx(in);
                        string operand3 = F22coperand3_field(fieldidx);
			////Field* fie = _dfr->getField(fieldidx);
                        vector<unsigned short>::iterator it = allregs->begin();
                        map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			//ClassDefInfo* classdef = new ClassDefInfo("S");
			assert(allregs->at(0) != allregs->at(1));
                        for(; it!= allregs->end(); it ++)
                        {
				assert(_declared_map->count(*it) > 0);
				assert(_var_type->count(*it) > 0);
				if(it == allregs->begin())// the first reg is source reg
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
				else //second reg is object register
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				    }
				    else
				    {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				    }
				}
			    /*
				if(it == allregs->begin())
				{
					(*ins_type)[*it] = classdef;
				}
                                else
				{
				    if((*_var_type)[*it] != NULL)
					(*ins_type)[*it] = (*_var_type)[*it];
				    else
				    {
					unsigned int classOwnerid = fie->getClassOwnerIdx();
					ClassDefInfo* classtype = type2ClassDef(_dfr, classOwnerid);
					assert(classtype != NULL);
					(*ins_type)[*it] = classtype;
					if(*it < _codeheader->getFirstArgu())
					    (*_var_type)[*it] = classtype;
				    }
				}
				*/
                        }
			//if((*(allregs->begin())) < _codeheader->getFirstArgu())
			//    (*_var_type)[*allregs->begin()] = classdef;
                	//return new ASMInstruction(_codeheader, _format, opcode,ope,ins_type,allregs, operand1, operand2,operand3);
			//ASMInstruction* asm_ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			//return asm_ins;
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));}
			ASMInstruction* ins =  new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs, operand1, operand2,operand3);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
		default:
			return NULL;
	}
}


string InstructionProcess::F22soperand1(DataIn* in, vector<unsigned short>* all_regs)
{
	unsigned short reg = (in->readByte()&0x0f);
	all_regs->push_back(reg);
	string oprand1 = "v" + Int2Str(reg);
	return oprand1;
}
string InstructionProcess::F22soperand2(DataIn* in, unsigned int pos,vector<unsigned short>* all_regs)
{
	in->move(pos);
	unsigned short reg = (in->readByte()&0xf0) >>4;
	all_regs->push_back(reg);
	string oprand2 = "v" + Int2Str(reg);
	return oprand2;
}
string InstructionProcess::F22soperand3(DataIn* in)
{
	short constantval = in->readShortx();
	string oprand3 = "#+" + Int2Str(constantval);
	return oprand3;
}
//binop/lit16 vA, vB, #+CCCC
ASMInstruction* InstructionProcess::fillOperandandins_F22s(DexOpcode opcode,string ope,vector<unsigned short>* all_regs, string operand1, string operand2, string operand3)
{
	vector<unsigned short>::iterator it = all_regs->begin();
	map<unsigned int, ClassDefInfo*>* ins_var = new map<unsigned int, ClassDefInfo*>();
	ClassDefInfo* classdef = NULL;
	if(!GLOBAL_MAP)
		classdef = new ClassDefInfo("I");
	else
		classdef = _dfr->getClassDefByName("I");
	//12/20
	unsigned int typeidx = _dfr->getTypeId("I");
	classdef->setTypeIdx(typeidx);
	//12/20
	unsigned int firstreg = 0;
	for(; it != all_regs->end(); it ++)
	{
		if(it == all_regs->begin())
		{
			firstreg = *it;
			if((*_declared_map)[*it] != NULL)
			   (*ins_var)[*it] = (*_declared_map)[*it];
			else if(USING_TYPE_INFER){
			    (*ins_var)[*it] = this->_invalid_type;
			}
			else
			    (*ins_var)[*it] = classdef;
			
		}
		else
		{
			/*if(firstreg == *it)
			{
				(*ins_var)[2000] = classdef;
			}
			else
			{*/
			if((*_declared_map)[*it] != NULL)
				(*ins_var)[*it] = (*_declared_map)[*it];
			else if(USING_TYPE_INFER)
				(*ins_var)[*it] = this->_invalid_type;
			else
				(*ins_var)[*it] = classdef;
			if(*it < _codeheader->getFirstArgu())
			    (*_var_type)[*it] = classdef; //update
			//}
		}
		if(firstreg < _codeheader->getFirstArgu())
		    (*_var_type)[firstreg] = classdef; //update
		
	}
	//(*ins_var)[1000] = classdef;
	return new ASMInstruction(_codeheader,_format, opcode, ope, ins_var, all_regs, operand1,operand2,operand3);
}
ASMInstruction* InstructionProcess::processF22s(DataIn* in,DexOpcode opcode,unsigned int address)
{
	in->move(address);
	in->skip(1); //skip the opcode
	unsigned int curr_pos  = in->getCurrentPosition();
	vector<unsigned short>* allregs = new vector<unsigned short>();
	vector<unsigned short>* in_use = NULL;
	if(in_use_flag)
		in_use = new vector<unsigned short>();
	switch(opcode)
	{
		case OP_ADD_INT_LIT16:
		{
			string ope = "add-int/lit16";
			string operd1 = F22soperand1(in,allregs);
			string operd2 = F22soperand2(in,curr_pos,allregs);
			string operd3 = F22soperand3(in);
			ASMInstruction* asm_ins = fillOperandandins_F22s(opcode,ope,allregs,operd1,operd2,operd3);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
			return asm_ins;
			
		}
        	case OP_RSUB_INT:
		{
			string ope = "rsub-int/lit16"; //or just "sub-int/lit16"
			string operd1 = F22soperand1(in,allregs);
			string operd2 = F22soperand2(in,curr_pos,allregs);
			string operd3 = F22soperand3(in);
			ASMInstruction* asm_ins = fillOperandandins_F22s(opcode,ope,allregs,operd1,operd2,operd3);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			return asm_ins;
			
		}
        	case OP_MUL_INT_LIT16:
		{
			string ope = "mul-int/lit16";
			string operd1 = F22soperand1(in,allregs);
			string operd2 = F22soperand2(in,curr_pos,allregs);
			string operd3 = F22soperand3(in);
			ASMInstruction* asm_ins = fillOperandandins_F22s(opcode,ope,allregs,operd1,operd2,operd3);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			return asm_ins;
		}
        	case OP_DIV_INT_LIT16:
		{
			string ope = "div-int/lit16";
			string operd1 = F22soperand1(in,allregs);
			string operd2 = F22soperand2(in,curr_pos,allregs);
			string operd3 = F22soperand3(in);
			ASMInstruction* asm_ins = fillOperandandins_F22s(opcode,ope,allregs,operd1,operd2,operd3);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			return asm_ins;
		}
        	case OP_REM_INT_LIT16:
		{
			string ope = "rem-int/lit16";
			string operd1 = F22soperand1(in,allregs);
			string operd2 = F22soperand2(in,curr_pos,allregs);
			string operd3 = F22soperand3(in);
			ASMInstruction* asm_ins = fillOperandandins_F22s(opcode,ope,allregs,operd1,operd2,operd3);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			return asm_ins;
		}
        	case OP_AND_INT_LIT16:
		{
			string ope = "and-int/lit16";
			string operd1 = F22soperand1(in,allregs);
			string operd2 = F22soperand2(in,curr_pos,allregs);
			string operd3 = F22soperand3(in);
			ASMInstruction* asm_ins = fillOperandandins_F22s(opcode,ope,allregs,operd1,operd2,operd3);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			return asm_ins;
		}
        	case OP_OR_INT_LIT16:
		{
			string ope = "or-int/lit16";
			string operd1 = F22soperand1(in,allregs);
			string operd2 = F22soperand2(in,curr_pos,allregs);
			string operd3 = F22soperand3(in);
			ASMInstruction* asm_ins = fillOperandandins_F22s(opcode,ope,allregs,operd1,operd2,operd3);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			return asm_ins;
		}
        	case OP_XOR_INT_LIT16:
		{
			string ope = "xor-int/lit16";
			string operd1 = F22soperand1(in,allregs);
			string operd2 = F22soperand2(in,curr_pos,allregs);
			string operd3 = F22soperand3(in);
			ASMInstruction* asm_ins = fillOperandandins_F22s(opcode,ope,allregs,operd1,operd2,operd3);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			return asm_ins;
		}
		default:
			return NULL;
	}
}


//format 22t if-test va, vb, +cccc (8+4+4+16) //done
string InstructionProcess::F22toperand1(DataIn* in, unsigned short regs, vector<unsigned short>* all_regs)
{

        unsigned short desti_reg = regs & 0x000f;
	all_regs->push_back(desti_reg);
        return "v"+ Int2Str(desti_reg);
}

string InstructionProcess::F22toperand2(DataIn* in, unsigned short regs,vector<unsigned short>* all_regs)
{

        unsigned short sou_reg = (regs & 0x00f0) >> 4;
	all_regs->push_back(sou_reg);
        return "v"+ Int2Str(sou_reg);
}

string InstructionProcess::F22toperand3(DataIn* in, unsigned int address)
{
	short offset = in->readShortx();
	//short abs_off = address + offset*2;
	unsigned int abs_off =(unsigned int) ((address + offset*2)& 0xffffffff);
	string oprand3 = "+" + Int2Str(abs_off);

	/*
	char buffer[50]; // treat it as c string
        sprintf(buffer, "%u",abs_off);
	string oprand3 = buffer;
	oprand3 = "+" + oprand3;
	*/
#ifdef DEBUG
	//for test
	in->move(abs_off);
	unsigned short opco = in->readByte()& 0x00ff;
	cout << "add-int/li8 ? opcode is d8 ?"<< opco << endl;
	printf("opcode : %.2x\n",opco);
#endif
        return oprand3;
}

ASMInstruction* InstructionProcess::fillOperandandins_F22t(DexOpcode opcode,string op, DataIn* in, unsigned int address,unsigned short regs,vector<unsigned short>* all_regs)
{	
	string ope = op;
	string operand1 = F22toperand1(in,regs,all_regs);
	string operand2 = F22toperand2(in,regs,all_regs);
	string operand3 = F22toperand3(in,address);
	//ClassDefInfo* classdef = new ClassDefInfo("Lable");
	vector<unsigned short>::iterator it = all_regs->begin();
	map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
	ClassDefInfo* first_classdef = NULL;
	ClassDefInfo* second_classdef = NULL;
	for(; it != all_regs->end(); it ++)
	{
	    assert(_declared_map->count(*it)>0);
	    assert(_var_type->count(*it)>0);

		if ((*_declared_map)[*it] != NULL) {
			(*ins_type)[*it] = (*_declared_map)[*it];
		} else if (USING_TYPE_INFER) {
			(*ins_type)[*it] = this->_invalid_type;
		}
	    else {
			assert((*_var_type)[*it] != NULL);
			(*ins_type)[*it] = (*_var_type)[*it];
			if (*it < _codeheader->getFirstArgu())
				(*_var_type)[*it] = (*ins_type)[*it]; //update the method's local variable types
		}
		if(it == all_regs->begin())
			first_classdef = (*ins_type)[*it];
		else
			second_classdef = (*ins_type)[*it];
	    
		/*if((*_var_type)[*it]!= NULL)
			(*ins_type)[*it] = (*_var_type)[*it];
		else
		{
		    cout <<"------------------------------------" << endl;
		    cout << "bail out!!" << endl;
		}*/
	}
	//(*ins_type)[1000] = classdef;
	if(first_classdef != second_classdef){
		if(first_classdef->getTypename() == "I")
			(*ins_type)[all_regs->at(0)] = (*ins_type)[all_regs->at(1)];
		else
			(*ins_type)[all_regs->at(1)] = (*ins_type)[all_regs->at(0)];

	}

	ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode,op, ins_type, all_regs,operand1, operand2, operand3);
	return ins;
}

ASMInstruction* InstructionProcess::processF22t(DataIn* in,DexOpcode opcode,unsigned int address)
{
	in->move(address);
	in->skip(1);
	unsigned short regs = in->readByte()&0x00ff;
	vector<unsigned short>* allregs = new vector<unsigned short>();
	vector<unsigned short>* in_use = NULL;
	if(in_use_flag)
		in_use = new vector<unsigned short>();
	switch(opcode)
	{
		case OP_IF_EQ:
		{
			string ope= "if-eq";
			ASMInstruction* asm_ins = fillOperandandins_F22t(opcode,ope,in,address,regs,allregs);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_IF_NE:
		{
			string ope = "if-ne";
			ASMInstruction* asm_ins = fillOperandandins_F22t(opcode,ope,in,address,regs,allregs);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_IF_LT:
		{
			string ope= "if-lt";
			ASMInstruction* asm_ins = fillOperandandins_F22t(opcode,ope,in,address,regs,allregs);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
			return asm_ins;
		}
        	case OP_IF_GE:
		{
			string ope= "if-ge";
			ASMInstruction* asm_ins = fillOperandandins_F22t(opcode,ope,in,address,regs,allregs);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_IF_GT:
		{
			string ope = "if-gt";
			ASMInstruction* asm_ins = fillOperandandins_F22t(opcode,ope,in,address,regs,allregs);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
        	case OP_IF_LE:
		{
			string ope = "if-le";
			ASMInstruction* asm_ins = fillOperandandins_F22t(opcode,ope,in,address,regs,allregs);
			assert(allregs->size()>1);
			if(in_use_flag){
			in_use->push_back(allregs->at(0));
			in_use->push_back(allregs->at(1));
			asm_ins->setInUseRegs(in_use);}
			//delete in_use;
                        return asm_ins;
		}
		default:
			return NULL;
	}
}
//for 22x(8+8+16) move/from16,vaa,vbbbb //done
string InstructionProcess::F22xoperand1(DataIn* in, vector<unsigned short>* all_regs)
{
	unsigned short reg = in->readByte();
	all_regs->push_back(reg);
	string oprand1 = "v" + Int2Str(reg);
	return oprand1;
}
string InstructionProcess::F22xoperand2(DataIn* in,vector<unsigned short>* all_regs)
{
	unsigned short reg = in->readShortx();
	all_regs->push_back(reg);
	string oprand2 = "v" + Int2Str(reg);
	return oprand2;
}
ASMInstruction* InstructionProcess::processF22x(DataIn* in,DexOpcode opcode,unsigned int address)
{
	in->move(address);
	in->skip(1); //skip opcode
	vector<unsigned short>* allregs = new vector<unsigned short>();
	switch(opcode)
	{
		case OP_MOVE_FROM16:
		{
			string ope = "move/from16";
			string operd1 = F22xoperand1(in,allregs);
			string operd2 = F22xoperand2(in,allregs);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			vector<unsigned short>::iterator it = allregs->begin();
			unsigned int firstreg = 0;
			ClassDefInfo* classdef = NULL;
			bool no_dec = false;
			for(; it != allregs->end(); it ++) //move-object/from 16 vaa, vbbb
			{
			    assert(_declared_map->count(*it) > 0);
			    assert(_var_type->count(*it) > 0);
			    if(it == allregs->begin())
			    {
				firstreg = *it;
				if((*_declared_map)[*it] != NULL)
				    (*ins_type)[*it] = (*_declared_map)[*it];
				else if(USING_TYPE_INFER){
				   (*ins_type)[*it] = this->_invalid_type;
				}
				else
				    no_dec = true;
			    }
			    else
			    {
				if((*_declared_map)[*it] != NULL)
				    (*ins_type)[*it] = (*_declared_map)[*it];
				else if(USING_TYPE_INFER){
				    (*ins_type)[*it] = this->_invalid_type;
				}
				else
				{
				    assert((*_var_type)[*it] != NULL);
				    (*ins_type)[*it] = (*_var_type)[*it];
				}
				classdef = (*ins_type)[*it]; //get infered type for definition register
				if(no_dec)
				    (*ins_type)[firstreg] = classdef;
				 //update method's level type table
				if(firstreg < _codeheader->getFirstArgu())
				    (*_var_type)[firstreg] = (*ins_type)[firstreg]; //update
					
			    }
			}
			return new ASMInstruction(_codeheader, _format, opcode,ope, ins_type,allregs,operd1,operd2);
			
		}
        	case OP_MOVE_WIDE_FROM16:
		{
			string ope = "move-wide/from16";
			string operd1 = F22xoperand1(in,allregs);
			string operd2 = F22xoperand2(in,allregs);
			map<unsigned int, ClassDefInfo*>* ins_type = new  map<unsigned int, ClassDefInfo*>();
			vector<unsigned short>::iterator it = allregs->begin();
			unsigned int firstreg = 0;
			ClassDefInfo* classdef = NULL;
			bool no_dec = false;
			for(; it != allregs->end(); it ++) //move-object/from 16 vaa, vbbb
			{
			    assert(_declared_map->count(*it) > 0);
			    assert(_var_type->count(*it) > 0);
			    if(it == allregs->begin())
			    {
				firstreg = *it;
				if((*_declared_map)[*it] != NULL)
				    (*ins_type)[*it] = (*_declared_map)[*it];
				else if(USING_TYPE_INFER){
				    (*ins_type)[*it] =  this->_invalid_type;
				}
				else
				    no_dec = true;
			    }
			    else
			    {
				if((*_declared_map)[*it] != NULL)
				    (*ins_type)[*it] = (*_declared_map)[*it];
				else if(USING_TYPE_INFER){
				    (*ins_type)[*it] = this->_invalid_type;
				}
				else
				{
				    assert((*_var_type)[*it] != NULL);
				    (*ins_type)[*it] = (*_var_type)[*it];
				}
				classdef = (*ins_type)[*it]; //get infered type for definition register
				if(no_dec)
				    (*ins_type)[firstreg] = classdef;
				 //update method's level type table
				if(firstreg < _codeheader->getFirstArgu())
				    (*_var_type)[firstreg] = (*ins_type)[firstreg]; //update
					
			    }
			}
			/*
			for(; it != allregs->end(); it ++)
			{
				if(it == allregs->begin())
					firstreg = *it;
				else if((*_var_type)[*it] != NULL)
				{
					//classdef = new ClassDefInfo("D");
					classdef = (*_var_type)[*it];
					if (*it == firstreg)
						(*ins_type)[2000] = classdef;
					else
						(*ins_type)[*it] = classdef;
				}
				else
				{
				    cout << "bail out !! move-wide/from16 because no source type " << *it<<endl;
				    exit(-1);
				}
				
			}
			(*ins_type)[firstreg] = classdef;
			if(firstreg < _codeheader->getFirstArgu())
			    (*_var_type)[firstreg] = classdef; //update
			*/
			return new ASMInstruction(_codeheader, _format, opcode,ope, ins_type,allregs,operd1,operd2);

		}
        	case OP_MOVE_OBJECT_FROM16:
		{
			string ope = "move-object/from16";
			string operd1 = F22xoperand1(in,allregs);
			string operd2 = F22xoperand2(in,allregs);
			map<unsigned int, ClassDefInfo*>* ins_type = new  map<unsigned int, ClassDefInfo*>();
			vector<unsigned short>::iterator it = allregs->begin();
			//int reg_size = allregs->size();
			/*
			cout << "|+|+|+|+|+|+|+|+|+|+" << endl;
			for(int i; i < reg_size; i ++)
			{
			    cout << allregs->at(i)<<endl;
			}
			cout << "|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+|+" << endl;
			*/
			unsigned int firstreg = 0;
			ClassDefInfo* classdef = NULL;
			bool no_dec = false;
			for(; it != allregs->end(); it ++) //move-object/from 16 vaa, vbbb
			{
			    assert(_declared_map->count(*it) > 0);
			    assert(_var_type->count(*it) > 0);
			    if(it == allregs->begin())
			    {
				firstreg = *it;
				if((*_declared_map)[*it] != NULL)
				    (*ins_type)[*it] = (*_declared_map)[*it];
				else if(USING_TYPE_INFER){
				    (*ins_type)[*it] = this->_invalid_type;
				}
				else
				    no_dec = true;
			    }
			    else
			    {
				if((*_declared_map)[*it] != NULL)
				    (*ins_type)[*it] = (*_declared_map)[*it];
				else if(USING_TYPE_INFER){
				    (*ins_type)[*it] = this->_invalid_type;
				}
				else
				{
				    assert((*_var_type)[*it] != NULL);
				    (*ins_type)[*it] = (*_var_type)[*it];
				}
				classdef = (*ins_type)[*it]; //get infered type for definition register
				if(no_dec)
				    (*ins_type)[firstreg] = classdef;
				 //update method's level type table
				if(firstreg < _codeheader->getFirstArgu())
				    (*_var_type)[firstreg] = (*ins_type)[firstreg]; //update
					
			    }
			}
			/*
			for(; it != allregs->end(); it ++)
			{
				if(it == allregs->begin()) // the destination register
					firstreg = *it;
				else if((*_var_type)[*it] != NULL)//source register
				{
					classdef = (*_var_type)[*it];
					if (*it == firstreg)
						(*ins_type)[2000] = classdef;
					else
						(*ins_type)[*it] = classdef;
				}
				else
				{
				    //ClassDefInfo* thisp_t = _codeheader->getClassDefInfo();
				    cout << "bail out, move-object/from 16 because no source type " <<*it <<endl;
				    exit(-1);
				    
				    classdef = _codeheader->getClassDefInfo();
				    assert(classdef != NULL);
				    (*ins_type)[*it] = classdef;
				    if(*it < _codeheader->getFirstArgu())
					(*_var_type)[*it] = classdef; //update
				    

				    //cout << "register:: " << *it <<",,," <<classdef->getTypename()<<endl;
				    //cout << "bail out!!!! move-object/from16 ->file::" <<_codeheader->getClassDefInfo()->getTypename() << "method::" << _codeheader->getMethod()->toString()<<endl;
				}
			}
			(*ins_type)[firstreg] = classdef;
			if(firstreg < _codeheader->getFirstArgu())
			    (*_var_type)[firstreg] = classdef; //update
			*/
			return new ASMInstruction(_codeheader, _format, opcode,ope, ins_type,allregs,operd1,operd2);

		}
		default:
			return NULL;
	}
}

//for 23x format //done
string InstructionProcess::F23xoperand1(DataIn* in,vector<unsigned short>* all_regs)
{
	unsigned short reg = (unsigned short)in->readByte();
	all_regs->push_back(reg);
	string oprand1= "v"+Int2Str(reg);
	return oprand1;
} 
string InstructionProcess::F23xoperand2(DataIn* in,vector<unsigned short>* all_regs)
{
	unsigned short reg = (unsigned short)in->readByte();
        all_regs->push_back(reg);
	string oprand2 = "v"+Int2Str(reg);
	return oprand2;
}
string InstructionProcess::F23xoperand3(DataIn* in,vector<unsigned short>* all_regs)
{	
	unsigned short reg = (unsigned short)in->readByte();
        all_regs->push_back(reg);
        string oprand3 = "v"+Int2Str(reg);
	return oprand3;
}

ASMInstruction* InstructionProcess::fillOperandandins_F23x(DexOpcode opcode,string op, DataIn* in)
{      
	vector<unsigned short>* allregs = new vector<unsigned short>();
	vector<unsigned short>* in_use = NULL;
	if(in_use_flag)
		in_use = new vector<unsigned short>();
	string operand1 = F23xoperand1(in,allregs);
	string operand2 = F23xoperand2(in,allregs);
	string operand3 = F23xoperand3(in,allregs);
	//hzhu added 4/27/2012
	vector<unsigned short>::iterator it = allregs->begin();
	map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
	unsigned int firstreg = 0;
	ClassDefInfo* classdef = NULL;
	ClassDefInfo* infer_t = NULL;
	string arraytype;
	string eletype;
	ClassDefInfo* ele_cdf = NULL;
	if((op == "cmpl-float")||(op == "cmpg-float")||(op == "cmpl-double")||(op == "cmpg-double")||(op == "cmp-long")||(op=="aget")||(op=="aget-wide")||(op=="aget-object")||(op == "aget-boolean")||(op == "aget-byte")||(op == "aget-char")||(op == "aget-short")) ////cmpl-float vaa, vbb, vcc (int)vaa = float(vbb) >/< float(vcc)
	{
		int i= 1;
		bool ele_flag = false; // if we can not get the declared type for aget dest register
		for(it= allregs->begin(); it!= allregs->end(); it ++) //for example aget-object v4,v1,v0 where v4 is element of array and v1 is the array reference and v0 is the index
		{//haiyan added
				
			assert(_declared_map->count(*it) > 0);
			assert(_var_type->count(*it) > 0);
			if(i == 1) //first register hold the element of the array or the result of compared source regs(in_def)  // we need to update the *_var_type later after go through each regiters
			{
				firstreg = *it; // ele registe is 6
				if(PRINT_INFO)
				    cout << "firstreg : " << firstreg << endl;
				if(op.substr(0,3) == "cmp")
				{
					if (!GLOBAL_MAP)
						classdef = new ClassDefInfo("I");
					else
						classdef = _dfr->getClassDefByName("I");
					//12/20
					unsigned int typeidx = _dfr->getTypeId("I");
					classdef->setTypeIdx(typeidx);
					//12/20
					if ((*_declared_map)[firstreg] != NULL) //first reg in def
						(*ins_type)[firstreg] = (*_declared_map)[firstreg];
					else if (USING_TYPE_INFER) {
						(*ins_type)[firstreg] = this->_invalid_type;
					} else { //use the infer type
						(*ins_type)[firstreg] = classdef;
					}
					if (firstreg < _codeheader->getFirstArgu()) {
						assert((*ins_type)[firstreg] != NULL);
						(*_var_type)[firstreg] = (*ins_type)[firstreg];
					}
				}else{ // aget, the type dest register may no clue
				    if((*_declared_map)[firstreg] != NULL) //happy to find the declared type
				    {
					//cout << " happy to find the declared type !! " << firstreg << endl;
					(*ins_type)[firstreg] =(*_declared_map)[firstreg] ;
					ele_cdf = (*ins_type)[firstreg];
				    }else if(USING_TYPE_INFER){
					(*ins_type)[firstreg] = this->_invalid_type;
				    }	
				    else
				    {
					//cout << "didn't find  the declared type, so need to add later " << endl; 
					ele_flag = true;
				    }
				}
				////////////we need to get from the source register's type (use the infered type)
			}
			else  //second or third registers, source regs
			{	if(in_use_flag)
					in_use->push_back(*it);
				if(i == 2) // i = 2 the second register might be [object  also 6
				{
				    if(op.substr(0, 3) == "cmp") // source register
				    {
					if((op =="cmpl-float")||(op == "cmpg-float")){
						if(!GLOBAL_MAP)
							infer_t = new ClassDefInfo("F");
						else
							infer_t = _dfr->getClassDefByName("F");
					}
					else if((op == "cmpl-double") || (op == "cmpg-double") ){
						if(!GLOBAL_MAP)
							infer_t = new ClassDefInfo("D");
						else
							infer_t = _dfr->getClassDefByName("D");
					}
					else{
						if(!GLOBAL_MAP)
							infer_t = new ClassDefInfo("J");
						else
							infer_t = _dfr->getClassDefByName("J");
					}


					if((*it) != firstreg) //use different register by source and dest
					{
					    if((*_declared_map)[*it] != NULL)
						(*ins_type)[*it] = (*_declared_map)[*it];
					    else if(USING_TYPE_INFER){
						 (*ins_type)[*it] = this->_invalid_type;
					    }else
					    	(*ins_type)[*it] = infer_t;
					}else{ //duplicated register
				            if((*_declared_map)[*it] != NULL)
						(*ins_type)[DUPLICATE_REGISTER] = (*_declared_map)[*it];
					    else if(USING_TYPE_INFER)
						(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
					    else
					    	(*ins_type)[DUPLICATE_REGISTER] = infer_t; // 2000->type
					}
					
					if(*it < _codeheader->getFirstArgu())
					    (*_var_type)[*it] = infer_t;

				    }
				    else //aget vaa, vbb, vcc (here vbb, the array register)
				    {
					ClassDefInfo* array_classdef = NULL;
					if((*it) != firstreg)
					{ 
					    if((*_declared_map)[*it] != NULL)
					    {
						(*ins_type)[*it] = (*_declared_map)[*it];
						array_classdef = (*_declared_map)[*it];
					    }
					    else if(USING_TYPE_INFER)
					    {
						(*ins_type)[*it] = this->_invalid_type;
                                            }
					    else
					    {
						assert((*_var_type)[*it] != NULL);
						(*ins_type)[*it] = (*_var_type)[*it];
						array_classdef = (*_var_type)[*it];
					    }
					}
					else //duplicated register aget va, va, vb //overwrite register va;
					{
					   if((*_declared_map)[*it] != NULL){
						 (*ins_type)[DUPLICATE_REGISTER] = (*_declared_map)[*it];
						 array_classdef = (*_declared_map)[*it];
					   }
					   else if(USING_TYPE_INFER){
						(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
					   }else{
						assert((*_var_type)[*it] != NULL);
						(*ins_type)[DUPLICATE_REGISTER] = (*_var_type)[*it];
						array_classdef = (*_var_type)[*it];
					   }
					    //(*ins_type)[DUPLICATE_REGISTER] = array_classdef;
					}
					if(array_classdef != NULL){
						arraytype = array_classdef->getTypename(); //real array type  //possible prepare for element type
						if (arraytype == "[string")
					    		arraytype = "[Ljava/lang/String;";
						int str_len = arraytype.size();
						//cout << "arraytype ??       ?? " << arraytype << endl;
						eletype = arraytype.substr(1,str_len-1); //real element type
					}
				    }
				}
				else // i = 3 
				{
					if(op.substr(0, 3) == "cmp") // the same as the second infer type
					{
					    if(*it == firstreg)
					    {
						assert (infer_t != NULL);
						if((*_declared_map)[*it] != NULL)
							(*ins_type)[DUPLICATE_REGISTER] = (*_declared_map)[*it];
						else if(USING_TYPE_INFER){
							(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
						}else
							(*ins_type)[DUPLICATE_REGISTER] = infer_t;
					    }
					    else //not duplicated register
					    {
						
						if((*_declared_map)[*it] != NULL)
							 (*ins_type)[*it] =  (*_declared_map)[*it];
						else if(USING_TYPE_INFER)
							(*ins_type)[*it] = this->_invalid_type;
						else
							(*ins_type)[*it] = infer_t;
						//if(*it < _codeheader->getFirstArgu())
						//    (*_var_type)[*it] = classdef; //update
					    }

					    if(*it < _codeheader->getFirstArgu())
						(*_var_type)[*it] = infer_t;
					}
					else  //aget..... 3rd register for the index which should be int type
					{
					    if(*it == firstreg)
					    {
						if((*_declared_map)[*it] != NULL)
							(*ins_type)[DUPLICATE_REGISTER] = (*_declared_map)[*it];
						else if(USING_TYPE_INFER){
							(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
						}
						else if((*_var_type)[*it] != NULL)
						{
						   //cout << "reg :: " << *it << " type is " << (*_var_type)[*it]->getTypename()<< endl;
						   //assert((*_var_type)[*it]->getTypename() == "I" ) ;  // index register should be integer, or byte or short, any of them are reasonable
						  (*ins_type)[DUPLICATE_REGISTER] = (*_var_type)[*it];
						}
					    }
					    else
					    {
						if((*_declared_map)[*it] != NULL)
							(*ins_type)[*it] = (*_declared_map)[*it];
						else if(USING_TYPE_INFER)
							(*ins_type)[*it] = this->_invalid_type;
						else if((*_var_type)[*it] != NULL)
						{ 
						   // assert((*_var_type)[*it]->getTypename() == "I" ) ;  // index register should be integer
						    (*ins_type)[*it] = (*_var_type)[*it];
						}
					    }
					    if((ele_flag)&&(op.substr(0,4) == "aget"))
					    {
						if(!GLOBAL_MAP){
						if (_dfr->insideClassDef(eletype))
						    ele_cdf = _dfr->getClassDef(eletype);
						else
						    ele_cdf = new ClassDefInfo(eletype);
						}else{
							ele_cdf = _dfr->getClassDefByName(eletype);
						}

						(*ins_type)[firstreg] = ele_cdf;
					    }

					    if((firstreg < _codeheader->getFirstArgu())&&(ele_cdf != NULL)){
						//cout << "in again: " << firstreg << endl;
						//assert(ele_cdf!=NULL);
						(*_var_type)[firstreg] = ele_cdf;
					    }
					}
				} // end if i = 3;
			}// end of else (i =2 and i  =3 )
			i ++;
		}
		
	}
	else if((op=="aput")||(op=="aput-wide")||(op=="aput-object")||(op=="aput-boolean")||(op=="aput-byte")||(op=="aput-char")||(op=="aput-short")) // the 1st and 3rd reg may be the same
	{
		///unsigned int firstreg = 0;
		///int count = 0;
		///ClassDefInfo* ele_classdef = NULL;
		///bool flag = false;
		///bool first_found_f = false;
		//ClassDefInfo* f_classdef = NULL;
		//////////////////////////////////////make sure the register won't be used twice in this type of ins
		assert(allregs->at(0) != allregs->at(1));
		assert(allregs->at(1) != allregs->at(2));
		assert(allregs->size() == 3);
		int i = 0;
		//assert(allregs->at(0) != allregs->at(2)); aput v8, v0, v8, we don't care here, v8 must has same type here, we won't use V2000 to denoted duplicated register
		for(; it!=allregs->end();it++) 
		{
			i++;
			if(in_use_flag)
				in_use->push_back(*it);
			assert(_declared_map->count(*it) > 0);
			assert(_var_type->count(*it) >0);
			if ((i == 3) && (allregs->at(0) == allregs->at(2))) {
				if ((*_declared_map)[*it] != NULL) {
					(*ins_type)[DUPLICATE_REGISTER] = (*_declared_map)[*it];
				} else if (USING_TYPE_INFER) {
					(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
				} else {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[DUPLICATE_REGISTER] = (*_var_type)[*it];
				}
			}
			else {
				if ((*_declared_map)[*it] != NULL) {
					(*ins_type)[*it] = (*_declared_map)[*it];
				} else if (USING_TYPE_INFER) {
					(*ins_type)[*it] = this->_invalid_type;
				} else {
					assert((*_var_type)[*it] != NULL);
					(*ins_type)[*it] = (*_var_type)[*it];
				}
			}
			/*
			if(count == 0)
			{
				firstreg = *it;
				if(PRINT_INFO)
				    cout << "firstreg " << firstreg << endl;
				if(flag)
				{
				    (*ins_type)[*it] = ele_classdef;
				    if(*it < _codeheader->getFirstArgu())
					(*_var_type)[*it] = ele_classdef;
				}
				else if((*_var_type)[*it] != NULL)
				{
				    if(PRINT_INFO)
					cout << "firstreg exists " <<endl;
				    first_found_f = true;
				    f_classdef = (*_var_type)[*it];
				    (*ins_type)[*it] = (*_var_type)[*it];
				}
			}
			if(count == 1) //destination register //aput-wide && aput-object
			{
				if(flag) //ele type already know 
				{
				    //(*ins_type)[*it] = (*_var_type)[*it];
				    string tname = "["+ ele_classdef->getTypename();
				    ClassDefInfo* arr_classdef = NULL;
				    if(_dfr->insideClassDef(tname))
				    {
					arr_classdef = _dfr->getClassDef(tname);
				    }
				    else
					arr_classdef = new ClassDefInfo(tname);
				    (*ins_type)[*it] = arr_classdef;
				    if(*it < _codeheader->getFirstArgu())
					(*_var_type)[*it] = arr_classdef; // need to update global
				}
				else if(first_found_f)//ele has then try make up the array type
				{
				    if(PRINT_INFO)
					cout << "inside socond register doesn't exist but first one does" << endl;
				    string f_classdef_str = f_classdef->getTypename();
				   // cout << f_classdef_str << endl;
				    string second_classdef_str = "["+ f_classdef_str;
				    if(_dfr->insideClassDef(second_classdef_str))
					(*ins_type)[*it] =  _dfr->getClassDef(second_classdef_str);
				    else
					(*ins_type)[*it] = new ClassDefInfo(second_classdef_str);
				    if(*it < _codeheader->getFirstArgu())
				    {
					if(_dfr->insideClassDef(second_classdef_str))
					    (*_var_type)[*it] =  _dfr->getClassDef(second_classdef_str);
					else
					    (*_var_type)[*it] = new ClassDefInfo(second_classdef_str);
				    }
				}
				else if((*_var_type)[*it] != NULL) //ele type doesn't know
				{
				    (*ins_type)[*it] = (*_var_type)[*it]; 
				    if(PRINT_INFO)
					cout << "inside second register found ! " << *it <<endl;
				    string arr_t_str = (*_var_type)[*it]->getTypename();
				    if(PRINT_INFO)
					cout << "arr_t_str " << arr_t_str <<endl;
				    string ele_t_str = arr_t_str.substr(1, arr_t_str.size()-1);
				    if(PRINT_INFO)
					cout << "ele_t_str " << ele_t_str << endl;
				    if(_dfr->insideClassDef(ele_t_str))
					(*ins_type)[firstreg] = _dfr->getClassDef(ele_t_str);
				    else
					(*ins_type)[firstreg] = new ClassDefInfo(ele_t_str); 
				    if(firstreg < _codeheader->getFirstArgu())
				    {
					if(_dfr->insideClassDef(ele_t_str))
					    (*_var_type)[firstreg] = _dfr->getClassDef(ele_t_str);
					else	
					    (*_var_type)[firstreg] = new ClassDefInfo(ele_t_str);//update global
				    }
				}
				else
				    cout << "bail out! NO clue about the type of aput instruction" << endl;
			}
			if(count == 2) //the 3rd register which is an index
			{
				if(firstreg == *it)
					(*ins_type)[2000] = new ClassDefInfo("I");
				else
					(*ins_type)[*it] = (*_var_type)[*it];	
			}
			count ++;
			*/
		}
	}
	else if((op.substr(4,3)=="int")||(op.substr(3,3)=="int")||(op.substr(5,3)=="int"))
	{
		unsigned int firstreg = 0;
		if(!GLOBAL_MAP)
			classdef = new ClassDefInfo("I");
		else
			classdef = _dfr->getClassDefByName("I");
		for(; it!=allregs->end();it++)
		{
			if(it == allregs->begin())
			{
				firstreg = *it;
				if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				else if(USING_TYPE_INFER){
					 (*ins_type)[*it] = this->_invalid_type;
				}else
					(*ins_type)[*it] = classdef;
				//(*_var_type)[*it] = classdef;
			}
			else
			{
				if(in_use_flag)
					in_use->push_back(*it);
				if(*it == firstreg)
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[DUPLICATE_REGISTER] =  (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
				    }else	
				    	(*ins_type)[DUPLICATE_REGISTER] = classdef;
				}
				else
				{
				   if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				   else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				   }else{
				    //	assert(classdef->getTypename() == ((*_var_type)[*it])->getTypename());
				    	(*ins_type)[*it] = classdef;
				   }
				}
			}
		}
		if(firstreg < _codeheader->getFirstArgu())
		    (*_var_type)[firstreg] = classdef;//update
	}
	else if((op.substr(4,4)=="long")||(op.substr(3,4)=="long")||(op.substr(5,4)=="long")) 
	{
		unsigned int firstreg = 0;
		if(!GLOBAL_MAP)
			classdef = new ClassDefInfo("J");
		else
			classdef = _dfr->getClassDefByName("J");
		for(; it!=allregs->end();it++)
		{
			if(it == allregs->begin())
			{
				firstreg = *it;
				if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				else if(USING_TYPE_INFER){
					 (*ins_type)[*it] = this->_invalid_type;
				}else
					(*ins_type)[*it] = classdef;
				//(*ins_type)[*it] = classdef;
				//(*_var_type)[*it] = classdef;
			}
			else
			{
				if(in_use_flag)
					in_use->push_back(*it);
				if(*it == firstreg)
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[DUPLICATE_REGISTER] =  (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
				    }else	
				    	(*ins_type)[DUPLICATE_REGISTER] = classdef;
				    //(*ins_type)[DUPLICATE_REGISTER] = classdef;
				}
				else
				{
				   if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				   else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				   }else{
				    //	assert(classdef->getTypename() == ((*_var_type)[*it])->getTypename());
				    	(*ins_type)[*it] = classdef;
				   }
				   // assert(classdef->getTypename() == ((*_var_type)[*it])->getTypename());
				    //(*ins_type)[*it] = classdef;
				}
			}
		}
		if(firstreg < _codeheader->getFirstArgu())
		    (*_var_type)[firstreg] = classdef; //update
	}
	else if((op.substr(4,5)=="float"))
	{
		unsigned int firstreg = 0;
		if(!GLOBAL_MAP)
			classdef = new ClassDefInfo("F");
		else
			classdef = _dfr->getClassDefByName("F");
		for(; it!=allregs->end();it++)
		{
			if(it == allregs->begin())
			{
				firstreg = *it;
				if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				else if(USING_TYPE_INFER){
					 (*ins_type)[*it] = this->_invalid_type;
				}else
					(*ins_type)[*it] = classdef;
				//(*ins_type)[*it] = classdef;
			}
			else
			{
				if(in_use_flag)
					in_use->push_back(*it);
				if(*it == firstreg)
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[DUPLICATE_REGISTER] =  (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
				    }else	
				    	(*ins_type)[DUPLICATE_REGISTER] = classdef;
					
				//	(*ins_type)[DUPLICATE_REGISTER] = classdef;
				}
				else
				{
				   if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				   else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				   }else{
				//	cout << "classdef->getTypename() " << classdef->getTypename()<< endl;
				//	cout << "_var_type[*it] type " <<  ((*_var_type)[*it])->getTypename()<< endl;
				  //  	assert(classdef->getTypename() == ((*_var_type)[*it])->getTypename());
				    	(*ins_type)[*it] = classdef;
				   }
				   // assert(classdef->getTypename() == ((*_var_type)[*it])->getTypename());
				    //(*ins_type)[*it] = classdef;
				}
			}
		}
		if(firstreg < _codeheader->getFirstArgu())
		    (*_var_type)[firstreg] = classdef;//udpate
	}
	else
	{
		unsigned int firstreg = 0;
		if(!GLOBAL_MAP)
			classdef = new ClassDefInfo("D");
		else
			classdef = _dfr->getClassDefByName("D");
		for(; it!=allregs->end();it++)
		{
			if(it == allregs->begin())
			{
				firstreg = *it;
				if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				else if(USING_TYPE_INFER){
					 (*ins_type)[*it] = this->_invalid_type;
				}else
					(*ins_type)[*it] = classdef;
			//	(*ins_type)[*it] = classdef;
				//(*_var_type)[*it] = classdef;
			}
			else
			{
				if(in_use_flag)
					in_use->push_back(*it);
				if(*it == firstreg)
				{
				    if((*_declared_map)[*it] != NULL)
					(*ins_type)[DUPLICATE_REGISTER] =  (*_declared_map)[*it];
				    else if(USING_TYPE_INFER){
					(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
				    }else	
				    	(*ins_type)[DUPLICATE_REGISTER] = classdef;
				//	(*ins_type)[DUPLICATE_REGISTER] = classdef;
				}
				else
				{
				   if((*_declared_map)[*it] != NULL)
					(*ins_type)[*it] = (*_declared_map)[*it];
				   else if(USING_TYPE_INFER){
					(*ins_type)[*it] = this->_invalid_type;
				   }else{
				    	//assert(classdef->getTypename() == ((*_var_type)[*it])->getTypename()); do not gurantee, because of const uncertainty problem
				    	(*ins_type)[*it] = classdef;
				   }
				   // assert(classdef->getTypename() == ((*_var_type)[*it])->getTypename());
				    //(*ins_type)[*it] = classdef;
				}
			}
		}
		if(firstreg < _codeheader->getFirstArgu())
			(*_var_type)[firstreg] = classdef; //update
	}
	//haiyan added 8.23
	/*if (op == "aget-object"){
	    if ((*_var_type)[1] != NULL){
		cout << (*_var_type)[1] -> getTypename() << endl ;
	}*/
	ASMInstruction* ins = new ASMInstruction(_codeheader,_format, opcode,op, ins_type ,allregs,operand1,operand2,operand3);
	if(in_use_flag)
		ins->setInUseRegs(in_use);
	return ins;
}
ASMInstruction* InstructionProcess::processF23x(DataIn* in,DexOpcode opcode,unsigned int address)
{
	in->move(address);
	in->skip(1); //skip the opcode
	switch(opcode)
	{
		case OP_CMPL_FLOAT:
		{
			string ope = "cmpl-float";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
		case OP_CMPG_FLOAT:
		{
			string ope = "cmpg-float";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_CMPL_DOUBLE:
		{
			string ope = "cmpl-double";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_CMPG_DOUBLE:
		{
			string ope = "cmpg-double";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_CMP_LONG:
		{
			string ope = "cmp-long";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_AGET:
		{
			string ope = "aget";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_AGET_WIDE:
		{
			string ope = "aget-wide";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_AGET_OBJECT:
		{
			string ope = "aget-object";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_AGET_BOOLEAN:
		{
			string ope = "aget-boolean";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_AGET_BYTE:
		{
			string ope = "aget-byte";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_AGET_CHAR:
		{
			string ope = "aget-char";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_AGET_SHORT:
		{
			string ope = "aget-short";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins; 
		}
        	case OP_APUT:
		{
			string ope = "aput";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
			return ins;
		}
        	case OP_APUT_WIDE:
		{
			string ope = "aput-wide";
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_APUT_OBJECT:
		{
			string ope = "aput-object";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}   
		case OP_APUT_BOOLEAN:
		{
			string ope = "aput-boolean";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_APUT_BYTE:
		{
			string ope = "aput-byte";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_APUT_CHAR:
		{
			string ope = "aput-char";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_APUT_SHORT:
		{
			string ope = "aput-short";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}		
        	case OP_ADD_INT:
		{
			string ope = "add-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_SUB_INT:
		{
			string ope = "sub-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_MUL_INT:
		{
			string ope = "mul-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_DIV_INT:
		{
			string ope = "div-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_REM_INT:
		{
			string ope = "rem-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_AND_INT:
		{
			string ope = "and-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_OR_INT:
		{
			string ope = "or-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_XOR_INT:
		{
			string ope = "xor-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_SHL_INT:
		{
			string ope = "shl-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_SHR_INT:
		{	
			string ope= "shr-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_USHR_INT:
		{
			string ope = "ushr-int";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_ADD_LONG:
		{	
			string ope = "add-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_SUB_LONG:
		{
			string ope = "sub-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_MUL_LONG:
		{
			string ope = "mul-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_DIV_LONG:
		{
			string ope = "div-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_REM_LONG:
		{
			string ope = "rem-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;	
		}
        	case OP_AND_LONG:
		{
			string ope = "and-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_OR_LONG:
		{
			string ope = "or-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_XOR_LONG:
		{
			string ope = "xor-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_SHL_LONG:
		{
			string ope = "shl-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_SHR_LONG:
		{
			string ope = "shr-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_USHR_LONG:
		{
			string ope = "ushr-long";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_ADD_FLOAT:
		{
			string ope = "add-float";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_SUB_FLOAT:
		{
			string ope = "sub-float";
			//ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_MUL_FLOAT:
		{
			string ope = "mul-float";
                        //ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_DIV_FLOAT:
		{
			string ope = "div-float";
                        //ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_REM_FLOAT:
		{
			string ope = "rem-float";
                        //ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
		case OP_ADD_DOUBLE:
		{
			string ope = "add-double";
                        //ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_SUB_DOUBLE:
		{
			string ope = "sub-double";
                        //ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_MUL_DOUBLE:
		{
			string ope = "mul-double";
                        //ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_DIV_DOUBLE:
		{
			string ope = "div-double";
                        //ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
        	case OP_REM_DOUBLE:
		{
			string ope = "rem_double";
                        //ASMInstruction*ins = fillOperandandins_F23x(ope, in);
			ASMInstruction*ins = fillOperandandins_F23x(opcode,ope, in);
                        return ins;
		}
		default:
			return NULL;
	}
}

ASMInstruction* InstructionProcess::processF30t(DataIn* in,DexOpcode opcode,unsigned int address)
{
	return NULL;
}


ASMInstruction* InstructionProcess::processF31c(DataIn* in,DexOpcode opcode,unsigned int address)
{
	return NULL;
}

//for format 31i const vaa, #+bbbbbbbb(32bit) //done
string InstructionProcess::F31iOperand1(unsigned short reg)
{
	string oprand1 = "v" + Int2Str(reg);
	return oprand1;
}

string InstructionProcess::F31iOperand2(DataIn* in,ClassDefInfo* vartype)
{
	string oprand2;
	//printf("const value is: %x\n",value);
	if(vartype->getTypename() == "I")
	{
		int value = in->readIntx();
		oprand2 = Int2Str(value);
	}
	else if(vartype->getTypename() == "F")
	{
		float value = in->readFloatx();	
		
		oprand2 = Float2Str(value);
	}
	/*
	char buffer[50];
	sprintf(buffer,"%d",value);
	oprand2 = buffer;
	*/
	oprand2 = "#+" + oprand2;
	return oprand2;

}

ASMInstruction* InstructionProcess::fillOperandandins_F31i(DexOpcode opcode,string ope,unsigned short reg, DataIn* in)
{
	vector<unsigned short>* allregs = new vector<unsigned short>();
	allregs->push_back(reg);
	//string oprd1 = F31iOperand1(reg);
	//string oprd2 = F31iOperand2(in);
	 //hzhu begin 4/25/2012
    vector<unsigned short>::iterator it;
    ClassDefInfo* vartype = NULL;
    if(!GLOBAL_MAP)
        vartype = new ClassDefInfo("I"); //"I" means int
    else
    	vartype = _dfr->getClassDefByName("I");
    map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
	
	assert( _declared_map->count(reg) >0);
	assert( _var_type->count(reg) >0);


	if((*_declared_map)[reg] != NULL){// found
	     vartype = (*_declared_map)[reg]; // change temp_type to found one
	}else if(USING_TYPE_INFER){
	     vartype = this->_invalid_type;
	}
        (*ins_type)[reg] =  vartype;
	if( reg < _codeheader->getFirstArgu())
	    (*_var_type)[reg] = vartype;
	
	string oprd1 = F31iOperand1(reg);
	string oprd2 = F31iOperand2(in,vartype);
	/*
        for(it = allregs->begin(); it!= allregs->end();it++)
        {
                (*ins_type)[*it] =  vartype;
                (*_var_type)[*it] = vartype;//update the method's local variable types  
        }
        (*ins_type)[1000] = vartype;
	*/
        //hzhu end 4/25/2012
	ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode, ope,ins_type, allregs,oprd1,oprd2);
	return ins;
		
}
ASMInstruction* InstructionProcess::processF31i(DataIn* in,DexOpcode opcode,unsigned int address)
{
	in->move(address);
	in->skip(1); //skip the opcode
	unsigned short reg = in->readByte()&0xff;
	
	switch(opcode)
	{
		case OP_CONST:
		{
			string ope = "const";//no sign extension
			ASMInstruction* asm_ins = fillOperandandins_F31i(opcode, ope,reg,in);
			return asm_ins;
			
		}
        	case OP_CONST_WIDE_32:
		{
			string ope = "const-wide/32"; //sign extended to 64bits
			ASMInstruction* asm_ins = fillOperandandins_F31i(opcode, ope,reg,in);
                        return asm_ins;
		}
		default:
			return NULL;
	}
}


//f31t format fill-array-data vaa, +bbbbbbbb (8+8+32)
ASMInstruction* InstructionProcess::fillInstructiondata_F31t(DexOpcode opcode,DataIn* in, string ope, string operand1,string operand2, int addr, int sec_address, vector<unsigned short>* all_regs)
{
    in->move(sec_address);//move to the place store the data formed as certain format
	unsigned short ident = in->readShortx();// has the ident /opcode check
	Data* data = NULL;

	Switch* swit = NULL;
	ASMInstruction* ins = NULL;
	
	vector<unsigned int>* target = NULL;
	
	//also need the switched branch object;
	
	unsigned short firstshort = in->readShortx();
	switch(ident)
	{
		case 0x0100: //packed-switch format
		{
			unsigned short size = firstshort;
			//cout << "the number of entries in this table :" << size << endl;
			int firstkey = in->readIntx();
			//cout << "the fist key is :" << firstkey << endl;
			target = new vector<unsigned int>();
			//cout << "just for test to see what the branch information" <<endl;
			for(int i = 0; i < size; i++)
			{
				unsigned int rela_addr = in->readIntx()*2 + addr;
				target->push_back(rela_addr);
			}
			swit = new PackedSwitch(ident,size,target,firstkey);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			unsigned int reg = *all_regs->begin();

			assert(_declared_map->count(reg) > 0);
			assert(_var_type->count(reg) > 0);

			if ((*_declared_map)[reg] != NULL){
			    (*ins_type)[reg] = (*_declared_map)[reg];
			}
			else if(USING_TYPE_INFER){
			    (*ins_type)[reg] = this->_invalid_type;
			}
			else if((*_var_type)[reg] != NULL){
			    (*ins_type)[reg] = (*_var_type)[reg];
			}
			else{
			    cout << "bail out for no existing reg!" << endl;
			    assert (false);
			}
			ins = new ASMInstruction(_codeheader, _format, opcode, ope, ins_type, all_regs,operand1,operand2,"",NULL,swit);
			return ins;
			
			
		}
		case 0x0200: //sparse-switch format
		{
			//the size for sparse-switch
			unsigned short size = firstshort;
			// the keys for sparse-switch
			vector<int>* keys = new vector<int>();
			for(int i = 0; i< size; i++)
			{
				int tempkey = in->readIntx();
				keys->push_back(tempkey);
			}
			
			//the target for spase-switch
			target = new vector<unsigned int>();
			for(int j=0; j<size; j++)
			{
				unsigned int temptarget = in->readIntx()*2 + addr;
				target->push_back(temptarget);
			}
			swit = new SparseSwitch(ident,size,target,keys);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			unsigned int reg = *all_regs->begin();
			
			
			assert(_declared_map->count(reg) > 0);
			assert(_var_type->count(reg) > 0);

			if ((*_declared_map)[reg] != NULL){
			    (*ins_type)[reg] = (*_declared_map)[reg];
			}else if(USING_TYPE_INFER){
				 (*ins_type)[reg] = this->_invalid_type;
			}
			else if((*_var_type)[reg] != NULL){
			    (*ins_type)[reg] = (*_var_type)[reg];
			}
			else{
			    cout << "bail out for no existing reg!" << endl;
			    assert (false);
			}
			
			/*if((*_var_type)[reg] != NULL)
			    (*ins_type)[reg] = (*_var_type)[reg];
			else
			    cout << "bail out for no existing reg!" << endl;*/
                        ins = new ASMInstruction(_codeheader, _format, opcode,ope,ins_type, all_regs,operand1,operand2,"",NULL,swit);
                        return ins;
		}
		case 0x0300: //fill-array-data
		{
			unsigned short element_width = firstshort;
			//cout<< "the number of bytes in each element : " <<element_width <<endl;
                        unsigned int size = in->readIntx();
                        //cout << "size: " << size << endl;
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int,ClassDefInfo*>();
			unsigned int reg = *all_regs->begin();
			if((*_var_type)[reg] != NULL)
			    (*ins_type)[reg] = (*_var_type)[reg];
			else
			    cout << "bail out for no existing reg!" << endl;
			//(*ins_type)[1000] = new ClassDefInfo("Label"); //we don't need to jump to the data place
			ClassDefInfo* classdef = NULL;
			assert(_declared_map->count(reg) > 0);
			if((*_declared_map)[reg] != NULL)
			    classdef = (*_declared_map)[reg];
			else if(USING_TYPE_INFER){
			    classdef = this->_invalid_type;
			}
			else
			{
			    string arraytype = (*_var_type)[reg]->getTypename();
			    unsigned int typesize = arraytype.size();
			    string eletype = arraytype.substr(1,typesize-1);
			if (!GLOBAL_MAP) {
				if (_dfr->insideClassDef(eletype))
					classdef = _dfr->getClassDef(eletype);
				else
					classdef = new ClassDefInfo(eletype);
			}else
			    	classdef = _dfr->getClassDefByName(eletype);
			    /*
			    //hzhu added 4/29/2012
			    map<int, int>* type2class = _dfr->getClassToTypeidx();
			    map<int,int>::iterator it2 = type2class->begin();

			    for(; it2 != type2class->end(); it2++)
			    {
				unsigned int typeidx = it2->first;
				string tmpstr = _dfr->getType(typeidx);
				if(tmpstr == eletype)
				{
					classdef = _dfr->getClassDefInfo(it2->second);
				}
				else
				{
				    classdef = new ClassDefInfo(eletype);
				}
			    }
			   */
			} 
			//hzhu end 4/29/2012		
			vector <ArrayData>* realdata = new vector <ArrayData>();
			if (element_width == 1){
				for(unsigned int i=0; i< size; i++)
                                {
					ArrayData temp;
					temp._uint8_data = in->readByte();
                                        realdata->push_back(temp);
                                }
                                data = new Data(element_width, size, realdata, ARRAY8,classdef);
                                ins = new ASMInstruction(_codeheader, _format, opcode, ope,ins_type, all_regs, operand1,operand2,"",data);
			}else if (element_width == 2){
				for(unsigned int i=0; i< size; i++)
                                {
					ArrayData temp;
					temp._uint16_data = in->readShortx();
                                        realdata->push_back(temp);
                                }
                                data = new Data(element_width, size, realdata, ARRAY16,classdef);
                                ins = new ASMInstruction(_codeheader, _format, opcode, ope,ins_type,all_regs,operand1,operand2,"",data);
			}else if (element_width == 4){
				for(unsigned int i= 0; i < size; i++)
                                {
					ArrayData temp;
					temp._uint32_data = in->readIntx();
                                        realdata->push_back(temp);
                                }
                                data = new Data(element_width,size,realdata, ARRAY32,classdef);
                                ins = new ASMInstruction(_codeheader,_format,opcode, ope,ins_type, all_regs,operand1,operand2,"",data);
			}else if (element_width == 8){
				for(unsigned int i= 0; i < size; i++)
                                {
					ArrayData temp;
					temp._uint64_data = in->readLongx();
                                        realdata->push_back(temp);
                                }
                                data = new Data(element_width,size,realdata, ARRAY64,classdef);
                                ins = new ASMInstruction(_codeheader,_format,opcode, ope,ins_type, all_regs,operand1,operand2,"",data);
			}

			return ins;
		}
		default:
		{
			cout << "doesn't match corresponding opcode, should be 0x0100/0x0200/0x0300, exit!" <<endl;
			return NULL;
		}
	}
}

ASMInstruction* InstructionProcess::processF31t(DataIn* in,DexOpcode opcode,unsigned int address)
{
	in->move(address); //address == current instruction position
	in->skip(1); // skip opcode;
	//prepare for the operand1;
	unsigned short reg = in->readByte();
	string operand1 = "v" + Int2Str(reg);
	vector<unsigned short>* allreg = new vector<unsigned short>();
	allreg->push_back(reg);
	//the end of operand1

	//prepare for the operand2
	int rela_addr = in->readIntx();
	//cout << "rela_addr: " << rela_addr <<endl;
	int addr = rela_addr*2 + address;
	//printf("ab_addr address of content is : %.8x\n", addr);
	char buffer[50]; // treat it as c string, we do not need it to be a hex, just int is OK/convenient
        sprintf(buffer, "%u",addr);
        string operand2 = buffer;
	operand2 = "+"+operand2; //"+" means the address!
	//the end of operand2
        
	//next need to process the data that stored at the place of operand2
	ASMInstruction* asm_ins = NULL;

	switch(opcode)
	{
		case OP_FILL_ARRAY_DATA:
		{
			string op = "fill-array-data";
			asm_ins = fillInstructiondata_F31t(opcode,in, op, operand1,operand2,address, addr,allreg);
			return asm_ins;
		}
        	case OP_PACKED_SWITCH:
		{
			string op = "packed-switch";
			asm_ins = fillInstructiondata_F31t(opcode,in, op, operand1,operand2, address, addr,allreg);
			return asm_ins;
		}
        	case OP_SPARSE_SWITCH:
		{
                        string op = "sparse-switch";
                        asm_ins = fillInstructiondata_F31t(opcode,in, op, operand1,operand2, address, addr, allreg);
                        return asm_ins;
		}
		default:
			return NULL;
	}
}
	
ASMInstruction* InstructionProcess::processF32x(DataIn* in,DexOpcode opcode,unsigned int address)
{
	return NULL;
}

/**
 * if we encouter this type of instruction, we process it later
 * */
string InstructionProcess::F35cOperand1(DataIn* in, vector<unsigned short>* all_regs)
{
	
	string oprand1 = "{"; 
	//read the first bytes after the opcode
	int pandr = in->readByte(); //for example 53
	//printf("parameters and register : %.2x\n", pandr);
	int psize = pandr >> 4; //get total 5 registers
	int lastreg = 0;
	if(psize == 5) // there are 5 registers need to parse
	{
	    //cout << "HELLO !!" << endl;
	    lastreg = pandr & 0x0f; //lastreg contain the last register for example 3 here
	}
	in->skip(2); //skip the operand2
	unsigned short registers = in->readShortx();
	//vector<unsigned short>* r = new vector<unsigned short>();

	unsigned short base = 0x000f;
	unsigned int count = 0;
        for(int i = 0; i < psize ; i++)
        {
		//printf("base: %.4x\n",base);
		if(count == 4)
		    break;
         	unsigned short num = registers & base;
                unsigned int dividend = pow(16,i);
                num = num /dividend;
                //cout << "register :" << num << endl;
                all_regs->push_back(num);
                base = (base << 4);
		count ++;
        }
	if(psize==5)
	{
		all_regs->push_back(lastreg);
	}
	
	vector<unsigned short>::iterator it;
        for( it= all_regs->begin(); it <= all_regs->end(); it++)
        {
        	//cout << "register -> " << *it <<endl;
               // stringstream ss;
               // ss << *it;
                if(it != all_regs->end())
                {
                	if(it == all_regs->end()-1)
                        {
				oprand1 += "v" + Int2Str(*it);
                        	//oprand1  += "v" + ss.str();
                        }
                        else
                       	{
                                oprand1 += "v" + Int2Str(*it)+ ", ";
				//oprand1  += "v" + ss.str()+", ";
                        }
		}
                else
                        oprand1 +="}";

         }

	return oprand1;
	
}

//hzhu added 4/30/2012
unsigned int InstructionProcess::F35cOperand2_typeidx(DataIn* in, int pos)
{
	in->move(pos);
	in->skip(1);
	return in->readShortx();
}
//hzhu added 4/30/2012
string InstructionProcess::F35cOperand2_type( unsigned int typeidx)
{
	//return _dfr->getType(typeidx);
	//using map unify type name
	return _dfr->getTypename(typeidx);
}


//hzhu begin 4/22/2012
unsigned int InstructionProcess::F35cOperand2_methodidx(DataIn* in, int pos)
{
	in->move(pos);
	in->skip(1); //skip for the 2nd byte
	return in->readShortx();
}
//hzhu end 4/22/2012

string InstructionProcess::F35cOperand2_method( unsigned int methodidx) //hzhu changed the prototype
{
	return _dfr->getMethod(methodidx)->toString(); // return pointer to method
}

//hzhu begin 4/24/2012
void InstructionProcess::F35c_processTypes(DataIn* in, map<unsigned int, ClassDefInfo*>* instructiontypes, vector<unsigned short>* allregs, Method* method, DexOpcode opcode, vector<unsigned short>* in_use)
{//add local variable liveness
	ClassDefInfo* temp_type = NULL;
	vector<unsigned int>* para_type_ids = method->getParaTypeIds();
	//unsigned int firstreg = 0;
	if(PRINT_INFO)
	{
	    map<unsigned int, ClassDefInfo*>::iterator it = _var_type->begin();
	    //cout << endl;
	    for(; it != _var_type->end(); it ++)
	    {
		//cout << "reg---- " << it->first<<endl;
		if(it->second != NULL)
		    cout << "type==== " <<it->second->getTypename()<<endl;
	    }
	}
	if((opcode == OP_INVOKE_VIRTUAL) ||(opcode == OP_INVOKE_SUPER) || (opcode == OP_INVOKE_DIRECT) || (opcode == OP_INVOKE_INTERFACE) )
	{
		if(allregs!= NULL)
		{	
			vector<unsigned short>::iterator it;
			vector<unsigned int>::iterator it1;
			unsigned int para_number =0;
			if(para_type_ids!= NULL)
			{
				it1 = para_type_ids->begin();
				para_number = para_type_ids->size(); // number of parameters at method definition. do not include this pointer
				//cout << "para_number: " << para_number << endl;
			}
			for(it = allregs->begin(); it != allregs->end(); it ++) //for example v0, v1 denoted by 0, 1
			{
				if(in_use_flag)
					in_use->push_back(*it);
				if(it == allregs->begin())
				{
					assert(_declared_map->count(*it) > 0); // should be true
					assert(_var_type->count(*it) > 0);
					//firstreg = *it;
					if((*_declared_map)[*it] != NULL){// found
					    (*instructiontypes)[*it] = (*_declared_map)[*it];
					    //firstreg = *it;
					}else if(USING_TYPE_INFER)
					{
						(*instructiontypes)[*it] = this->_invalid_type;
					}
					else if((*_var_type)[*it] != NULL) // found 
					{
					    (*instructiontypes)[*it] = (*_var_type)[*it]; //get it from the _var_type map
					    //firstreg = *it;
					}
					else // do not find in the both maps
					{
					    cout << "SHOULD NOT BE HERE" << endl;
					    assert(false);
					    //string classowner = method->getClassOwner();
					    ClassDefInfo* thisp = _codeheader->getClassDefInfo();
					    
					    //Init();
					    //ClassDefInfo* classdef = (*_str2type)[classowner];
					    (*instructiontypes)[*it] = thisp;
					    /*if(*it < _codeheader->getFirstArgu())
						(*_var_type)[*it] = thisp ; //update the method's level */
					    //cout << "this pointer equals to NULL, bail out!" <<endl;
					}
				}
				else if( para_number != 0 )//parameters for invoke this method
				{
					//temp_type = type2ClassDef(_dfr,*it1);
					ClassDefInfo* para_type = type2ClassDef(_dfr,*it1);

					assert(_declared_map->count(*it) > 0);
					if((*_declared_map)[*it] != NULL){// found
					    temp_type = (*_declared_map)[*it]; // change temp_type to found one
					}else if(USING_TYPE_INFER){
					    temp_type = this->_invalid_type;
					}
					else if ((*_var_type)[*it] != NULL)
					{
						temp_type = (*_var_type)[*it];
					}
					else
					{
						cout << "parameter is not declared and defined !" << endl;
						assert(false);
					}
					(*instructiontypes)[*it] = temp_type;


					/*if(it1 == para_type_ids->begin()) //first parameter
					{
						temp_type = type2ClassDef(_dfr,*it1); //get type from parameter list

						assert(_declared_map->count(*it) > 0); // should be true
						if((*_declared_map)[*it] != NULL){// found
						    temp_type = (*_declared_map)[*it]; // change temp_type to found one
						}

						if(*it != firstreg)
						{
							(*instructiontypes)[*it] = temp_type;

							if(*it < _codeheader->getFirstArgu())// update when reg is not a parameter reg
							    (*_var_type)[*it] = temp_type;

							firstreg = *it; //hzhu added this
						}
						else
						{
							(*instructiontypes)[DUPLICATE_REGISTER] = temp_type;
						}
					}
					else //2nd or 3rd .. parameters
					{
						temp_type = type2ClassDef(_dfr,*it1);
						if((firstreg == *it))
						{
							(*instructiontypes)[2000] = temp_type;
						}
						else
						{
							(*instructiontypes)[*it] = temp_type;
							firstreg = *it;
							if(*it < _codeheader->getFirstArgu())
							    (*_var_type)[*it] = temp_type;
						}	
					}*/

					//if((temp_type->getTypename() == "D")||(temp_type->getTypename() == "J"))
					if((para_type->getTypename() == "D")||(para_type->getTypename() == "J"))
					{
						it++; // do not deal with next register.
					}
					it1 ++; // go to next parameter
					para_number --;
				}
			}
			assert(para_number == 0);
		}
	}
	
	if(opcode == OP_INVOKE_STATIC)
	{
		if(para_type_ids != NULL)
		{
			vector<unsigned short>::iterator it = allregs->begin();
			vector<unsigned int>::iterator it1 = para_type_ids->begin();
			unsigned int para_number = para_type_ids->size();
			//cout << "para_number ======= " << para_number << endl;
			///unsigned int lastreg = 0;
			//cout << "allregs  === " << allregs->size() << endl;
			for(; it != allregs->end(); it ++)
                	{
				if(in_use_flag)
					in_use->push_back(*it);
				assert(_declared_map->count(*it) > 0);
				if((*_declared_map)[*it] != NULL){// found
				    temp_type = (*_declared_map)[*it]; // change temp_type to found one
				}else if(USING_TYPE_INFER){
				     temp_type = this->_invalid_type;
				}
				else if ((*_var_type)[*it] != NULL)
				{
					temp_type = (*_var_type)[*it];
				}
				else
				{
					cout << "parameter is not declared and defined !" << endl;
                                        assert(false);
				}
				(*instructiontypes)[*it] = temp_type;
				//here should based on the signature's type but the variable type
				ClassDefInfo* para_type = type2ClassDef(_dfr,*it1);					
				//if((temp_type->getTypename() == "D") ||(temp_type->getTypename() == "J"))
				if((para_type->getTypename() == "D") ||(para_type->getTypename() == "J"))
				{
					it ++;
				}
				it1 ++;
				para_number --;
			}
			assert(para_number == 0);

		}
	}	
}
//hzhu end 4/24/2012

//hzhu begin 4/24/2012
void InstructionProcess::F35c_SetReturn(Method* method, ClassDefInfo* return_type)
{
	unsigned int returnidx = method->getReturnidx(); //we need return type(that is return index)
	map<int, int>* mymap = _dfr->getClassToTypeidx();
	if (mymap->count(returnidx) > 0) {
		unsigned int typeidx = (*mymap)[returnidx];
		return_type = (*_dfr->getClassDefIdx2Ele())[typeidx];
	} else {
		//return_type = new ClassDefInfo(_dfr->getTypename(returnidx));
		return_type = _dfr->getClassDefByName(_dfr->getTypename(returnidx));
	}
	(*_var_type)[RETURNREG] = return_type; //10000 reserved for the return type's register number, which is fake for the purpose of identify
}
//hzhu end 4/24/2012
ASMInstruction* InstructionProcess::processF35c(DataIn* in, DexOpcode opcode,unsigned int address)
{
	in->move(address);
	in->skip(1); // this byte for opcode
	int pos = in->getCurrentPosition();
	vector<unsigned short>* allregs = new vector<unsigned short>();
	vector<unsigned short>* in_use = NULL;
	if(in_use_flag)
		in_use = new vector<unsigned short>();
	switch(opcode)
	{
		case OP_FILLED_NEW_ARRAY:
		{
			string ope = "filled-new-array";

			string operand1 = F35cOperand1(in,allregs);
			unsigned int typeidx = F35cOperand2_typeidx(in,pos);
			string operand2 = F35cOperand2_type(typeidx);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			string eletype = operand2.substr(1, operand2.length()-1); //element type
			map<int, int>* type2class = _dfr->getClassToTypeidx();
			map<int, int>::iterator pos = type2class->begin();
			ClassDefInfo* classdef = NULL; // update with initialization to NULL pointer;
			for(; pos != type2class->end(); pos++)
			{
				unsigned typeID = pos->first;
				//string tempstr = _dfr->getType(typeID);
				//using map unify type name
				string tempstr = _dfr->getTypename(typeID);
				if(tempstr == eletype)
				{

					classdef = (*_dfr->getClassDefIdx2Ele())[pos->second];

				}
				else
				{
					if(!GLOBAL_MAP)
						classdef = new ClassDefInfo(eletype);
					else
						classdef = _dfr->getClassDefByName(eletype);
				}
			}
			vector<unsigned short>::iterator it = allregs->begin();
			unsigned int lastreg = 0;
			for(; it != allregs->end(); it ++)
			{
				if(it == allregs->begin())
				{
					if((*_declared_map)[*it] != NULL)
						(*ins_type)[*it] = (*_declared_map)[*it];
					else if(USING_TYPE_INFER){
						 (*ins_type)[*it] = this->_invalid_type;
					}else
						(*ins_type)[*it] = classdef;
					lastreg = *it;
					if(*it < _codeheader->getFirstArgu())
					    (*_var_type)[*it] = (*ins_type)[*it]; //update
				}
				else
				{
					if(*it == lastreg)
					{
						if((*_declared_map)[*it] != NULL)
							(*ins_type)[DUPLICATE_REGISTER] = (*_declared_map)[*it];
						else if(USING_TYPE_INFER){
							(*ins_type)[DUPLICATE_REGISTER] = this->_invalid_type;
						}else
							(*ins_type)[DUPLICATE_REGISTER] = classdef;
					}
					else
					{
						if((*_declared_map)[*it] != NULL)
							(*ins_type)[*it] = (*_declared_map)[*it] ;
						else if(USING_TYPE_INFER){
							(*ins_type)[*it] = this->_invalid_type;
						}else
							(*ins_type)[*it] = classdef;
						lastreg = *it;
						if(*it < _codeheader->getFirstArgu())
						    (*_var_type)[*it] = classdef; //update
					}

				}
			}
			ClassDefInfo* returntype = type2ClassDef(_dfr,typeidx); //followed by move-result_object instruction
			assert(returntype != NULL);
			//cout << "pay attention, fill-new-array 's type is " << returntype->getTypename()<< endl;	
			(*_var_type)[10000] = returntype;
			return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type ,allregs, operand1, operand2);
		}
        	case OP_INVOKE_VIRTUAL:
		{
			string ope = "invoke-virtual";
			string operand1 = F35cOperand1(in,allregs);
			unsigned int methodidx = F35cOperand2_methodidx(in,pos);
			Method* method = _dfr->getMethod(methodidx);
			string operand2 = F35cOperand2_method( methodidx);
			map<unsigned int, ClassDefInfo*>* instruction_type = new map<unsigned int, ClassDefInfo*>();
			if(in_use_flag)
				F35c_processTypes(in,instruction_type, allregs, method, opcode,in_use);
			else
				F35c_processTypes(in,instruction_type, allregs, method, opcode,NULL);
			ClassDefInfo* returntype = NULL;
			F35c_SetReturn( method,returntype);
			/*for(int i = 0; i < allregs->size(); i++)
			{
			    in_use->push_back(allregs->at(i));
			}*/
			//return new ASMInstruction(_codeheader, _format, opcode, ope, instruction_type, allregs, operand1, operand2,"",NULL,NULL,methodidx,method);
			ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode, ope, instruction_type, allregs, operand1, operand2,"",NULL,NULL,methodidx,method);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_INVOKE_SUPER:
		{
			string ope = "invoke-super";
			string operand1 = F35cOperand1(in,allregs);
			unsigned int methodidx = F35cOperand2_methodidx(in,pos);
                        Method* method = _dfr->getMethod(methodidx);
			string operand2 = F35cOperand2_method(methodidx);	
                        map<unsigned int, ClassDefInfo*>* instruction_type = new map<unsigned int, ClassDefInfo*>();
            if(in_use_flag)
                        F35c_processTypes(in,instruction_type, allregs, method, opcode,in_use);
            else
            	F35c_processTypes(in,instruction_type, allregs, method, opcode,NULL);
                        ClassDefInfo* returntype = NULL;
			F35c_SetReturn( method,returntype);
			//return new ASMInstruction(_codeheader, _format, opcode, ope, instruction_type,allregs, operand1, operand2,"",NULL,NULL,methodidx,method);
			/*for(int i = 0; i < allregs->size(); i++)
			{
			    in_use->push_back(allregs->at(i));
			}*/
			ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode, ope, instruction_type, allregs, operand1, operand2,"",NULL,NULL,methodidx,method);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_INVOKE_DIRECT:
		{
			string ope = "invoke-direct";
                        string operand1 = F35cOperand1(in,allregs);
			unsigned int methodidx = F35cOperand2_methodidx(in,pos);
                        Method* method = _dfr->getMethod(methodidx);
			string operand2 = F35cOperand2_method(methodidx);	
                        map<unsigned int, ClassDefInfo*>* instruction_type = new map<unsigned int, ClassDefInfo*>();
            if(in_use_flag)
                        F35c_processTypes(in,instruction_type, allregs, method, opcode,in_use);
            else
            	F35c_processTypes(in,instruction_type, allregs, method, opcode,NULL);
			ClassDefInfo* returntype = NULL;
                        F35c_SetReturn( method,returntype);
			//return new ASMInstruction(_codeheader, _format, opcode, ope, instruction_type,allregs, operand1, operand2,"",NULL,NULL,methodidx,method);
			/*for(int i = 0; i < allregs->size(); i++)
			{
			    in_use->push_back(allregs->at(i));
			}*/
			ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode, ope, instruction_type, allregs, operand1, operand2,"",NULL,NULL,methodidx,method);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;

		}
        	case OP_INVOKE_STATIC:
		{
			string ope = "invoke-static";
			string operand1 = F35cOperand1(in,allregs);
			unsigned int methodidx = F35cOperand2_methodidx(in,pos);
                        Method* method = _dfr->getMethod(methodidx);
			string operand2 = F35cOperand2_method(methodidx);	
                        map<unsigned int, ClassDefInfo*>* instruction_type = new map<unsigned int, ClassDefInfo*>();
            if(in_use_flag)
                F35c_processTypes(in,instruction_type, allregs, method, opcode,in_use);
            else
            	F35c_processTypes(in,instruction_type, allregs, method, opcode,NULL);
			ClassDefInfo* returntype = NULL;
                        F35c_SetReturn( method,returntype);
			//return new ASMInstruction(_codeheader, _format,opcode, ope, instruction_type,allregs, operand1, operand2,"",NULL,NULL,methodidx,method);
			/*for(int i = 0; i < allregs->size(); i++)
			{
			    in_use->push_back(allregs->at(i));
			}*/
			ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode, ope, instruction_type, allregs, operand1, operand2,"",NULL,NULL,methodidx,method);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_INVOKE_INTERFACE:
		{
			string ope = "invoke-interface";
                        string operand1 = F35cOperand1(in,allregs);
			unsigned int methodidx = F35cOperand2_methodidx(in,pos);
                        Method* method = _dfr->getMethod(methodidx);
			string operand2 = F35cOperand2_method(methodidx);	
                        map<unsigned int, ClassDefInfo*>* instruction_type = new map<unsigned int, ClassDefInfo*>();
            if(in_use_flag)
                F35c_processTypes(in,instruction_type, allregs, method, opcode, in_use);
            else
            	F35c_processTypes(in,instruction_type, allregs, method, opcode, NULL);
			ClassDefInfo* returntype = NULL;
                        F35c_SetReturn( method,returntype);
			//return new ASMInstruction(_codeheader, _format, opcode, ope,instruction_type,allregs, operand1, operand2,"",NULL,NULL,methodidx,method);
            /*
			for(int i = 0; i < allregs->size(); i++)
			{
			    in_use->push_back(allregs->at(i));
			}*/
			ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode, ope, instruction_type, allregs, operand1, operand2,"",NULL,NULL,methodidx,method);
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
		default:
			return NULL;
	}
}


//for 3rc invoke-virtual/range {vcccc .. vnnnn}, meth@BBBB (1+1+2+2) bytes
string InstructionProcess::F3rcOperand1(DataIn* in, unsigned short reg_size,vector<unsigned short>* all_regs)
{
	in->skip(2); //skip the operand2 info
	unsigned short first_reg = in->readShortx();
	string oprand1 = "{";
	if(reg_size == 1)
	{
		oprand1 += "v";
		oprand1 += Int2Str(first_reg) + "}";
		all_regs->push_back(first_reg);
	}
	else if(reg_size ==2)
	{
		for(int i=0; i < reg_size; i++)
		{
			oprand1 += "v";
			unsigned short curr_reg = first_reg + i;
			all_regs->push_back(curr_reg);
			if(i< (reg_size-1))
			{
				oprand1 += Int2Str(curr_reg)+ " ";
			}
			else
				oprand1 += Int2Str(curr_reg);
		}
		oprand1 +="}";
	}
	else
	{
		oprand1 +="v" + Int2Str(first_reg) + ".." + "v" + Int2Str(first_reg+reg_size-1)+"}";
		for(int i = 0;i< reg_size; i++)
		{
			all_regs->push_back(first_reg + i);
		}
	}

	return oprand1;
}
//hzhu added 4/30/2012
unsigned int InstructionProcess::F3rcOperand2_typeidx(DataIn* in, unsigned int pos)
{
	in->move(pos);
	return in->readShortx();
}
//hzhu end 4/30/2012
string InstructionProcess::F3rcOperand2_type( unsigned int typeidx) //hzhu changed the prototype
{
	//return _dfr->getType(typeidx);
	//using map unify type name
	return _dfr->getTypename(typeidx);
}
//hzhu added 4/30/2012
unsigned int InstructionProcess::F3rcOperand2_methodidx(DataIn* in, unsigned int pos)
{
	in->move(pos);
	return in->readShortx();
}
//hzhu end 4/30/2012
string InstructionProcess::F3rcOperand2_method(unsigned int methodidx) //hzhu changed the prototype
{
	return _dfr->getMethod(methodidx)->toString();
}

void InstructionProcess::F3rc_processTypes(DataIn* in, map<unsigned int, ClassDefInfo*>* ins_type, vector<unsigned short>*allregs, Method* method, DexOpcode opcode,vector<unsigned short>* in_use)
{
	ClassDefInfo* classdef = NULL;
	vector<unsigned int>* para_type_ids = method->getParaTypeIds(); //the ins_type contains the type of variables(regs) , not the declared type
	bool static_flag = false;
	if(opcode == OP_INVOKE_STATIC_RANGE)
	    static_flag = true;
	if(allregs != NULL) //all register should in_use
	{
	    vector<unsigned short>::iterator it; //iterator to registers
	    vector <unsigned int>::iterator it1; //interator to declared types
	    //unsigned int count = 0;
	    if(para_type_ids != NULL)
	    {
		it1 = para_type_ids->begin();
	    }
	    if(static_flag) //static
	    {
		for(it = allregs->begin(); it!= allregs->end(); it++)
		{
			if(in_use_flag)
				in_use->push_back(*it);
		    classdef = type2ClassDef(_dfr,*it1); //infered type to check if one type uses 2 adjacent registers
		    assert(_declared_map->count(*it) > 0);
		    assert(_var_type->count(*it) > 0);
		    if((*_declared_map)[*it] != NULL) //found by debug declaration
		    {
			(*ins_type)[*it] =(*_declared_map)[*it] ;
		    }else if(USING_TYPE_INFER){
			(*ins_type)[*it] = this->_invalid_type;
		    }
		    else if((*_var_type)[*it] != NULL)
		    {
			(*ins_type)[*it] =(*_var_type)[*it] ;
		    }
		    else
		    {
			cout << "parameter used but not defined !!" << endl;
			assert(false);
		    }
		    if((classdef->getTypename()=="D")||(classdef->getTypename()=="J"))
		    {
			it ++;
		    }
		    it1++;
		}
	    }
	    else //not static fun call
	    {
		for(it = allregs->begin(); it!= allregs->end(); it++)
		{
			if(in_use_flag)
				in_use->push_back(*it);
		    assert(_declared_map->count(*it) > 0);
		    assert(_var_type->count(*it) > 0);
		    if(it == allregs->begin()) //this pointer, not declared function siganature
		    {
			if((*_declared_map)[*it] != NULL) //found by debug declaration
			{
			    (*ins_type)[*it] =(*_declared_map)[*it] ;
			}else if(USING_TYPE_INFER){
				(*ins_type)[*it] = this->_invalid_type;
			}
			else if ((*_var_type)[*it] != NULL)
			{
			    (*ins_type)[*it] =(*_var_type)[*it] ;
			}
			else
			{
				cout << "this pointer inside the parameter used but not defined !!" << endl;
                                assert(false);
			}
		    }
		    else
		    {
			classdef = type2ClassDef(_dfr,*it1);
			if((*_declared_map)[*it] != NULL) //found by debug declaration
			{
			    (*ins_type)[*it] =(*_declared_map)[*it] ;
			}else if(USING_TYPE_INFER){
				(*ins_type)[*it] = this->_invalid_type;
			}
			else if((*_var_type)[*it] != NULL)
			{
			    (*ins_type)[*it] =(*_var_type)[*it] ;
			}
			else
			{
				cout << "parameter used but not defined !!" << endl;
                        	assert(false);

			}
			if((classdef->getTypename()=="D")||(classdef->getTypename()=="J"))
			{
			    it ++;
			}
			it1++;
		    }
		}
	    }
	}
	/*
	else //not static invoke ins, has this pointer
	{
		//printf("invoke opcode :: %x\n", opcode);
		if(allregs != NULL)
		{
			vector <unsigned short>::iterator it;
			vector <unsigned int>::iterator it1;
			unsigned int count = 0;
			if(para_type_ids != NULL)
			{
				it1 = para_type_ids->begin();
				count = para_type_ids->size();
			}
			for(it = allregs->begin(); it!= allregs->end(); it++)
			{
				if(it == allregs->begin()) //this pointer
				{
					if((*_var_type)[*it] != NULL)
					{
						(*ins_type)[*it] = (*_var_type)[*it];
						lastreg = *it;
					}
					else
					{
						ClassDefInfo* thisp = _codeheader->getClassDefInfo();
						assert(thisp != NULL);
						(*ins_type)[*it] = thisp;
						if(*it < _codeheader->getFirstArgu())
						    (*_var_type)[*it] = thisp;
					    	//cout << "no this pointer, bail out! NO register existing!" << endl;
					}
				}
				else if(count != 0) //parameters
				{
					classdef = type2ClassDef(_dfr,*it1);
					if(*it != lastreg)
					{
						(*ins_type)[*it] = classdef;
						lastreg = *it;
						if(*it < _codeheader->getFirstArgu())
						    (*_var_type)[*it] = classdef; //update
					}
					else
					{
						(*ins_type)[2000] = classdef;
					}
					count --;
					if((classdef->getTypename()=="D")||(classdef->getTypename()=="J"))
					{
						it ++;
					}
					it1 ++;
				}

		}
		
	}
	*/
}
//hzhu added 5.14
void InstructionProcess::F3rc_SetReturn(Method* method, ClassDefInfo* return_type)
{
	unsigned int returnidx = method->getReturnidx(); //we need return type(that is return index)
        map<int, int>* mymap = _dfr->getClassToTypeidx();
        if(!GLOBAL_MAP){
        if(mymap->count(returnidx) > 0)
        {
        	unsigned int typeidx = (*mymap)[returnidx];
        	//HZHU
        	return_type =(*_dfr->getClassDefIdx2Ele())[typeidx];
        }
        else
        {
	    //return_type = new ClassDefInfo(_dfr->getType(returnidx));
	    //using map unify type name
	    return_type = new ClassDefInfo(_dfr->getTypename(returnidx));
        }}else{
        	string return_name1 = _dfr->getType(returnidx);
        	string return_name = _dfr->getTypename(returnidx);
        	//cout << "TT return_name1 " << return_name1 << "\t return_name " << return_name << endl;
        	return_type = _dfr->getClassDefByName(return_name);
        }

        (*_var_type)[RETURNREG] = return_type; //10000 reserved for the return type's register number, which is fake for the purpose of identify   
}
//hzhu end 5.14
ASMInstruction* InstructionProcess::processF3rc(DataIn* in,DexOpcode opcode,unsigned int address)
{
	in->move(address);
	in->skip(1); // skip the opcode
	unsigned short num_reg = in->readByte();
	unsigned int current_pos = in->getCurrentPosition();
	vector<unsigned short>* allregs = new vector<unsigned short>();

	vector<unsigned short>* in_use = NULL;
	if(in_use_flag)
		in_use = new vector<unsigned short>();

	switch(opcode)
	{
		case OP_FILLED_NEW_ARRAY_RANGE:
		{
			string ope = "filled-new-array/range";
			cout << " =========== haven't parse it yet waiting for exmaples " <<ope << endl;
			assert(false);

			string operand1 = F3rcOperand1(in, num_reg,allregs);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
			unsigned int typeidx = F3rcOperand2_typeidx(in, current_pos);
			string operand2 = F3rcOperand2_type( typeidx);
			ClassDefInfo* classdef = type2ClassDef(_dfr, typeidx);
			vector<unsigned short>::iterator pos = allregs->begin();
			for(; pos != allregs->end(); pos ++)
			{
				(*ins_type)[*pos] = classdef;
				if(*pos < _codeheader->getFirstArgu())
				    (*_var_type)[*pos] = classdef;
			}
			return new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs,operand1,operand2);
		}
        	case OP_INVOKE_VIRTUAL_RANGE:
		{
			string ope = "invoke-virtual/range" ;
			string operand1 = F3rcOperand1(in, num_reg,allregs);
                        unsigned int methodidx = F3rcOperand2_methodidx(in, current_pos);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
                        string operand2 = F3rcOperand2_method( methodidx);
			Method* method = _dfr->getMethod(methodidx);
			if(in_use_flag)
				F3rc_processTypes(in, ins_type,allregs,method,opcode, in_use);
			else
				F3rc_processTypes(in, ins_type,allregs,method,opcode, NULL);

			ClassDefInfo* returntype = NULL;
                        F3rc_SetReturn( method,returntype);
			//return new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs,operand1,operand2, "", NULL,NULL, methodidx, method);
			ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs,operand1,operand2, "", NULL,NULL, methodidx, method);
			/*
			for(map<unsigned int, ClassDefInfo*>::iterator type_it = ins_type->begin(); type_it != ins_type->end(); type_it++)
			{
			    unsigned int reg_n = type_it->first;
			    in_use->push_back(reg_n);
			}*/
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;

		}
        	case OP_INVOKE_SUPER_RANGE:
		{
			string ope = "invoke-super/range" ;
			string operand1 = F3rcOperand1(in, num_reg,allregs);
                        unsigned int methodidx = F3rcOperand2_methodidx(in, current_pos);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
                        string operand2 = F3rcOperand2_method( methodidx);
			Method* method = _dfr->getMethod(methodidx);
			if(in_use_flag)
				F3rc_processTypes(in, ins_type,allregs,method,opcode,in_use);
			else
				F3rc_processTypes(in, ins_type,allregs,method,opcode,NULL);

			ClassDefInfo* returntype = NULL;
                        F3rc_SetReturn( method,returntype);
			//return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type, allregs,operand1,operand2 ,"", NULL,NULL, methodidx, method);
			ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs,operand1,operand2, "", NULL,NULL, methodidx, method);
			/*for(map<unsigned int, ClassDefInfo*>::iterator type_it = ins_type->begin(); type_it != ins_type->end(); type_it++)
			{
			    unsigned int reg_n = type_it->first;
			    in_use->push_back(reg_n);
			}*/
			/*
			for(int i = 0; i < allregs->size(); i++)
			{
			    in_use->push_back(allregs->at(i));
			}
			*/
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_INVOKE_DIRECT_RANGE:
		{
			string ope = "invoke-direct/range";
			string operand1 = F3rcOperand1(in, num_reg,allregs);
                        unsigned int methodidx = F3rcOperand2_methodidx(in, current_pos);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
                        string operand2 = F3rcOperand2_method( methodidx);
			Method* method = _dfr->getMethod(methodidx);
			if(in_use_flag)
				F3rc_processTypes(in, ins_type,allregs,method,opcode,in_use);
			else
				F3rc_processTypes(in, ins_type,allregs,method,opcode,NULL);
			ClassDefInfo* returntype = NULL;
                        F3rc_SetReturn( method,returntype);
			//return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type, allregs,operand1,operand2 ,"", NULL,NULL, methodidx, method);
			ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs,operand1,operand2, "", NULL,NULL, methodidx, method);
			/*for(map<unsigned int, ClassDefInfo*>::iterator type_it = ins_type->begin(); type_it != ins_type->end(); type_it++)
			{
			    unsigned int reg_n = type_it->first;
			    in_use->push_back(reg_n);
			}*/
			/*
			for(int i = 0; i < allregs->size(); i++)
			{
			    in_use->push_back(allregs->at(i));
			}
			*/
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}

        	case OP_INVOKE_STATIC_RANGE:
		{
			string ope = "invoke-static/range" ;
			string operand1 = F3rcOperand1(in, num_reg,allregs);
                        unsigned int methodidx = F3rcOperand2_methodidx(in, current_pos);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
                        string operand2 = F3rcOperand2_method( methodidx);
			Method* method = _dfr->getMethod(methodidx);
			if(in_use_flag)
				F3rc_processTypes(in, ins_type,allregs,method,opcode, in_use);
			else
				F3rc_processTypes(in, ins_type,allregs,method,opcode,NULL);
			ClassDefInfo* returntype = NULL;
                        F3rc_SetReturn( method,returntype);
			//return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type, allregs,operand1,operand2,"", NULL,NULL, methodidx, method);
			ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs,operand1,operand2, "", NULL,NULL, methodidx, method);
			/*for(map<unsigned int, ClassDefInfo*>::iterator type_it = ins_type->begin(); type_it != ins_type->end(); type_it++)
			{
			    unsigned int reg_n = type_it->first;
			    in_use->push_back(reg_n);
			}*/
			/*
			for(int i = 0; i < allregs->size(); i++)
			{
			    in_use->push_back(allregs->at(i));
			}
			*/
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
        	case OP_INVOKE_INTERFACE_RANGE:
		{
			string ope = "invoke-interface/range";
			string operand1 = F3rcOperand1(in, num_reg,allregs);
                        unsigned int methodidx = F3rcOperand2_methodidx(in, current_pos);
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();
                        string operand2 = F3rcOperand2_method( methodidx);
			Method* method = _dfr->getMethod(methodidx);
			if(in_use_flag)
				F3rc_processTypes(in, ins_type,allregs,method,opcode, in_use);
			else
				F3rc_processTypes(in, ins_type,allregs,method,opcode,NULL);
			ClassDefInfo* returntype = NULL;
                        F3rc_SetReturn( method,returntype);
			//return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type, allregs,operand1,operand2,"", NULL,NULL, methodidx, method);
			ASMInstruction* ins = new ASMInstruction(_codeheader, _format, opcode,ope, ins_type, allregs,operand1,operand2, "", NULL,NULL, methodidx, method);
			/*for(map<unsigned int, ClassDefInfo*>::iterator type_it = ins_type->begin(); type_it != ins_type->end(); type_it++)
			{
			    unsigned int reg_n = type_it->first;
			    in_use->push_back(reg_n);
			}*/
			/*
			for(int i = 0; i < allregs->size(); i++)
			{
			    in_use->push_back(allregs->at(i));
			}
			*/
			if(in_use_flag)
				ins->setInUseRegs(in_use);
			//delete in_use;
			return ins;
		}
		default:
			return NULL;
	}
}
/*
ASMInstruction* InstructionProcess::processF40sc(DataIn* in,DexOpcode opcode,unsigned int address)
{
	return NULL;
}
*/

ASMInstruction* InstructionProcess::processF51l(DataIn* in,DexOpcode opcode,unsigned int address)
{
	in->move(address);
	in->skip(1);
	switch(opcode)
	{
		case OP_CONST_WIDE:
		{
			string ope = "const-wide";
			char buffer1[50];
			unsigned short reg = in->readByte();
			vector<unsigned short>* allregs = new vector<unsigned short>();
			allregs->push_back(reg);
			sprintf(buffer1, "%d", reg);
			string operand1 = "v" + string(buffer1);
			//HZHU changed 
			/*
			int cur_pos = in->getCurrentPosition();
			
			long value = in->readLongx();
                        char buffer[50];
                        sprintf(buffer, "%ld",value);
                        string operand2 = buffer;
			
			//or parepare double value HZHU changed
			in->move(cur_pos);
			*/
			double valued = in->readDoublex();
			string valued_str = Double2Str(valued);
			
                        string operand2 = "#+" + valued_str;
			//operand2 = "#+" + operand2;
			map<unsigned int, ClassDefInfo*>* ins_type = new map<unsigned int, ClassDefInfo*>();

			ClassDefInfo* vartype = NULL;
			if(!GLOBAL_MAP)
				vartype = new ClassDefInfo("J");
			else
				vartype = _dfr->getClassDefByName("J");

			assert( _declared_map->count(reg) >0);
			assert( _var_type->count(reg) >0);


			if((*_declared_map)[reg] != NULL){// found
			    vartype = (*_declared_map)[reg]; // change temp_type to found one

			}else if(USING_TYPE_INFER){
			   vartype = this->_invalid_type;
			}
			(*ins_type)[reg] =  vartype;
			if( reg < _codeheader->getFirstArgu())
			    (*_var_type)[reg] = vartype;

			return new ASMInstruction(_codeheader, _format, opcode, ope, ins_type, allregs, operand1, operand2);
			
		}
		default:
			return NULL;
	}
}


CodeHeader* InstructionProcess::getCodeH()
{
    return _codeheader;
}
