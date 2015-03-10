#include "InstructionMapping.h"

#include "Ins2Debug.h"
#include "DexOpcode.h"
#include "RVT.h"
#include "string_const_exp.h"
#include "Format.h"

#define ANNOTATION_ADDR false
#define STATIC_FIELD_CHECK false
#define INS_MAPPING_CHECK false
//#define SAIL_INS_PRINT true
#define SCRATCH_INF false
#define EXCEPTION_TEST true
#define DEX_INS_PRINT true
#define EXCEPTION_LABEL_PRINT false
#define CHECK_TYPE false
#define PRINT_FUNCALL false

#define NON_DECLARED_RETURN_TYPE true

#define LIB_HAS_EXCEPTION false

#define PRINT_PROCESS_EXCEPTION false

InstructionMapping::InstructionMapping(DexFileReader* dfr,
		map<string, ClassDefInfo*>* typemap, il::type* return_type) {
	_dfr = dfr;
	_sail_ins = new vector<sail::Instruction*>();
	_type_map = typemap;

	_static_field_addr = new map<string, unsigned int>();
	_newest_addr = rand() % 10000 + 200;

	_static_var = new map<unsigned int, sail::Variable*>();

	_str_label = new map<string, sail::Label*>();

	//use _random to get an initial value, then increase it per use;
	_random = rand() % 1000 + 1001;

	_backward_latest_line = -1;

	_fields_temp_var_map = new map<pair<sail::Variable*, unsigned int>,
			sail::Variable*>();

	//USING_TYPE_INFER is true, we do not need to do it at all
	if (NON_DECLARED_RETURN_TYPE &&(!USING_TYPE_INFER)) {
		_fun_declared_return_var = NULL;
		if (return_type != NULL) {
			_fun_declared_return_var = new pair<unsigned int, il::type*>();
			_fun_declared_return_var->second = return_type;
			_return_ins_addr = 0;
			_different_branch_return_type = NULL;
			_different_branch_return_var = NULL;
			_return_ins_has_label = false;
			_return_ins = NULL;

		}
	}

	_label_for_annotation_throw = NULL;
	_annotation_labelled = false;
}
InstructionMapping::~InstructionMapping() {
	delete _sail_ins;
	delete _static_field_addr;
	delete _static_var;
	delete _fields_temp_var_map;
}


//1/5 add last parameter to indicate if it is a clinit
void InstructionMapping::setsailInstruction(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr, bool clinit_flag) {
	asm_ins->getMthCodeH()->setSailIns(_sail_ins);
	Format format = asm_ins->getOpFormat();
	switch (format) {
	case F10t:
		InstructionMapping_F10t(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F10x:
		InstructionMapping_F10x(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F11n:
		InstructionMapping_F11n(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F11x:
		InstructionMapping_F11x(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F12x:
		InstructionMapping_F12x(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F20t:
		InstructionMapping_F20t(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F21c:{//const-class and sget/sput is here
		InstructionMapping_F21c(ins_debug, instruction_debug, asm_ins, addr, clinit_flag);
	}
		break;
	case F21h:
		InstructionMapping_F21h(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F21s:
		InstructionMapping_F21s(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F21t:
		InstructionMapping_F21t(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F22b:
		InstructionMapping_F22b(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F22c:
		InstructionMapping_F22c(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F22s:
		InstructionMapping_F22s(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F22t:
		InstructionMapping_F22t(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F22x:
		InstructionMapping_F22x(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F23x:
		InstructionMapping_F23x(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F30t:
		InstructionMapping_F30t(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F31c:
		InstructionMapping_F31c(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F31i:
		InstructionMapping_F31i(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F31t:
		InstructionMapping_F31t(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F32x:
		InstructionMapping_F32x(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F35c:
		InstructionMapping_F35c(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F3rc:
		InstructionMapping_F3rc(ins_debug, instruction_debug, asm_ins, addr);
		break;
	case F51l:
		InstructionMapping_F51l(ins_debug, instruction_debug, asm_ins, addr);
		break;
	default:
		return;

	}

}

vector<sail::Instruction*>* InstructionMapping::getSailInstruction() {
	updateSailInstruction();
	return _sail_ins;
}



void InstructionMapping::InstructionMapping_constantAssignment(
		Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	vector<unsigned short>* regs = asm_ins->getRegs();

	//register number and 1000 denotes const or addr, 2000 denotes Mul Reg.
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();

	il::type* t = NULL;


	//do not have the instruction and debuginformation mapping for this method
	if (instruction_debug == NULL) {
		if (SCRATCH_INF)
			cout << "instruction_debug mapping is NULL";
	}

	DebugInfo* debug = NULL;
	DexOpcode opcode = asm_ins->getOpcode();

	//only have one reg represent the variable
	unsigned short reg = regs->at(0);

	sail::Variable* sl = NULL;

	//get the first classdef inside the map
	ClassDefInfo* classdef = ins_var_type->begin()->second;
	t = getType(0, _dfr, classdef, _type_map);
	sl = asm_ins->getMthCodeH()->getRegVarEle(reg, t, _type_map, false, false);


	if (instruction_debug->count(asm_ins) > 0) {
		debug = (*instruction_debug)[asm_ins];
	}

	int startline = -1;
	string var_name = "";
	string reg_name = "v" + Int2Str(reg);
	if (debug == NULL) {
		if(0)
			cout << "debug information is NULL " << endl;
	} else {
		startline = debug->getLine();
		if (startline != -1) {
			asm_ins->getMthCodeH()->setLatestLine(startline);
		}

		vector<RVT*>* rvt_list = debug->getRVTList();
		if (rvt_list == NULL) {
			if(0)
				cout << "rvt list is empty" << endl;
		} else {
			RVT* rvt = *rvt_list->begin();
			var_name = rvt->getVarName();

		}
	}

	if (startline == -1)
		startline = asm_ins->getMthCodeH()->getLatestLine();

	//prepare done, then construct the instruction with variable or symbol
	//left is a variable
	il::location lo(startline, startline); //location

	//right is a constant string or a constant number
	string operd2 = asm_ins->getOperand2();
	long constant = 0;
	sail::Constant* sr = NULL;

	switch (opcode) {
	case OP_CONST_4:
	case OP_CONST_16:
	case OP_CONST_WIDE_16:
	case OP_CONST_WIDE: //51l
	case OP_CONST:
	case OP_CONST_WIDE_32:
	case OP_CONST_HIGH16:
	case OP_CONST_WIDE_HIGH16: {
			string prepstr = operd2.substr(2, operd2.size() - 2);
			constant = atoi(prepstr.c_str());
			//HZHU end of change
			sr = new sail::Constant(constant, true, 32); //right side is a digital constant
	}
		break;
	case OP_ARRAY_LENGTH: //array-length v1, v2 (v1 is the destinatin register, v2 is the array register)
	{

		//operd2 is the array register name
		map<string, long>* array_len_map = asm_ins->getMthCodeH()->getArrSize();
		if (array_len_map->count(operd2) > 0) {
			if (SCRATCH_INF)
				cout << "the array length found !" << endl;
			constant = (*array_len_map)[operd2];
			sr = new sail::Constant(constant, true, 32);
		} else {
			if (SCRATCH_INF)
				cout << "NO<<<<<<<<<<<<<<<<<<" << endl;
			sr = new sail::Constant(0, true, 32);
		}
	}
		break;
	default:
		return;

	}

	//haiyan added 5.30
	pair<string, unsigned int> p("v" + Int2Str(reg), constant);
	asm_ins->getMthCodeH()->updateConst(p.first, p.second); //register name and constant
	//haiyan ended 5.30

	il::string_const_exp* original = NULL;
	original = new il::string_const_exp(var_name, t, lo);

	//hzhu added 5.24
	sl->set_original(original);
	if (var_name != "") {
		if (opcode == OP_ARRAY_LENGTH)
			sl->set_alias_name(var_name + ".length");
		else
			sl->set_alias_name(var_name);
	} else {
		if (opcode == OP_ARRAY_LENGTH)
			sl->set_alias_name(reg_name + ".length");
		else
			sl->set_alias_name(reg_name);
	}

	assert(sl != NULL);

	string ins_str = "";
	il::string_const_exp* temp = new il::string_const_exp(ins_str, t, lo);
	sail::Assignment* inst = new sail::Assignment(sl, sr, temp, startline);
	if (SAIL_INS_PRINT) {
		cout << "test assignement pp:::" << inst->to_string(true) << endl;
		cout << "test assignement:::" << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);
	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)&&(!USING_TYPE_INFER)) //if UINSG_TYPE_INFER is true, we don't do it
		updateAnotherBranchReturnType(addr, reg, t, sl);
}

void InstructionMapping::InstructionMapping_constantStringAddr(
		Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	vector<unsigned short>* regs = asm_ins->getRegs();
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar(); //register number and 1000 denotes const or addr, 2000 denotes Mul Reg.
	//sail::Instruction* ins = NULL;
	il::type* t = NULL;


	//do not have the instruction and debuginformation mapping for this method
	if (instruction_debug == NULL){
		if (SCRATCH_INF)
			cout << "instruction_debug mapping is NULL";
	}


	DebugInfo* debug = NULL;

	unsigned short reg = regs->at(0); //only have one reg represent the variable
	ClassDefInfo* classdef = ins_var_type->begin()->second;

	t = getType(0, _dfr, classdef, _type_map); //deal with the type
	if (instruction_debug->count(asm_ins) > 0) {
		//cout << "Found! " << endl;
		debug = (*instruction_debug)[asm_ins];
	}


	int startline = -1;
	string var_name = "";
	string reg_name = "v" + Int2Str(reg);
	if (debug == NULL) {
		if (SCRATCH_INF)
			cout << "debug information is NULL " << endl;
	} else {
		startline = debug->getLine();
		if (startline != -1) {
			asm_ins->getMthCodeH()->setLatestLine(startline);
		}

		vector<RVT*>* rvt_list = debug->getRVTList();
		if (rvt_list == NULL) {
			if (SCRATCH_INF)
				cout << "rvt list is empty" << endl;
		} else {
			RVT* rvt = *rvt_list->begin();
			var_name = rvt->getVarName();
			reg_name = rvt->getRegName();
		}
	}
	if (startline == -1)
		startline = asm_ins->getMthCodeH()->getLatestLine();

	//hzhu added 5.25
	sail::Variable* sl = NULL;
	//destination

	sl = asm_ins->getMthCodeH()->getRegVarEle(reg, t, _type_map, false, false);



	il::location lo(startline, startline); //location

	//override hzhu 5.25 begin
	il::string_const_exp* original = new il::string_const_exp(var_name, t, lo);
	sl->set_original(original);
	if (var_name != "")
		sl->set_alias_name(var_name);
	else
		sl->set_alias_name(reg_name);
	//sl->set_type(t);
	//override hzhu 5.25 end

	//right is a constant string or a constant number
	string operd2 = asm_ins->getOperand2();
	operd2 = operd2.substr(1, operd2.size() - 2);

	il::string_const_exp* sr = new il::string_const_exp(operd2, t, lo);

	string str = sl->to_string(true) + "=" + sr->to_string();
	sail::AddressString* i = new sail::AddressString(sl, sr,
			new il::string_const_exp(str, t, lo), startline);
	if (SAIL_INS_PRINT) {
		cout << "test addrstring pp:::" << i->to_string(true) << endl;
		cout << "test addrstring:::" << i->to_string(false) << endl;
	}
	_sail_ins->push_back(i);

	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)&&(!USING_TYPE_INFER)) //we do not do it if USING_TYPE_INFER is true
		updateAnotherBranchReturnType(addr, reg, t, sl);

}

void InstructionMapping::InstructionMapping_varAssignment(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	//<<<<>>>>>>haiyan added 4.26
	//map<unsigned int, pair<bool, sail::Variable*>>* var_map = asm_ins->getMthCodeH()->getRegVar();
	//assert(var_map != NULL);
	//<<<<>>>>>>haiyan end 4.26

	//DataIn* in = ins_debug->getDataIn();
	vector<unsigned short>* regs = asm_ins->getRegs();
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar(); //register number and 1000 denotes const or addr, 2000 denotes Mul Reg.

	if (0) {
		for (map<unsigned int, ClassDefInfo*>::iterator it =
				ins_var_type->begin(); it != ins_var_type->end(); it++) {
			cout << "HZHU register: " << it->first << endl;
			cout << "HZHU type:     " << it->second->getTypename() << endl;
		}
	}

	//sail::Instruction* ins = NULL;
	il::type* tl = NULL;
	il::type* tr = NULL;
	//map<ASMInstruction*, DebugInfo*>* instruction_debug = codeh->mgetIns2Debug(); //one ASMInstruction* one debuginfo* for the passing in method codeh
	if (instruction_debug == NULL) //do not have the instruction and debuginformation mapping for this method
	{
		if (SCRATCH_INF)
			cout << "instruction_debug mapping is NULL and exit";
		//exit(1);	
	}
	DebugInfo* debug = NULL;
	//DexOpcode opcode = asm_ins->getOpcode();
	//unsigned short regl = *regs->begin(); //only have one reg represent the variable
	//unsigned short regr =*(++regs->begin());
	unsigned short regl = regs->at(0); //only have one reg represent the variable
	unsigned short regr = regs->at(1);
	//the two variable are the same type, so just need to fetch one of the type information to retrieve all
	ClassDefInfo* classdefl = (*ins_var_type)[regl];
	ClassDefInfo* classdefr = (*ins_var_type)[regr];
	assert(classdefl != NULL);
	assert(classdefr != NULL);
	//cout << "type name :" << classdef->getTypename()<<endl;
	//t = TypeMapping(0, _dfr , classdef); //deal with the type
	//t = TypeMapping(0, _dfr , classdef,_type_map); //deal with the type
	tl = getType(0, _dfr, classdefl, _type_map);
	tr = getType(0, _dfr, classdefr, _type_map); //deal with the type
	//t = TypeMapping(_dfr , classdef); //deal with the type
	if (instruction_debug->count(asm_ins) > 0) {
		//cout << "Found! " << endl;
		debug = (*instruction_debug)[asm_ins];
	}
	int startline = -1;
	string var_namel = "";
	string reg_namel = "v" + Int2Str(regl);
	string var_namer = "";
	string reg_namer = "v" + Int2Str(regr);
	if (debug == NULL) {
		if (SCRATCH_INF)
			cout << "debug information is NULL " << endl;
	} else {
		startline = debug->getLine();
		if (startline != -1) {
			asm_ins->getMthCodeH()->setLatestLine(startline);
		}
		vector<RVT*>* rvt_list = debug->getRVTList();
		if (rvt_list == NULL) {
			if (SCRATCH_INF)
				cout << "rvt list is empty" << endl;
		} else {
			vector<RVT*>::iterator it = rvt_list->begin();
			for (; it != rvt_list->end(); it++) {
				unsigned short reg = (*it)->getReg();
				if (reg == regl)
					var_namel = (*it)->getVarName();
				else
					var_namer = (*it)->getVarName();
			}

		}
	}
	if (startline == -1)
		startline = asm_ins->getMthCodeH()->getLatestLine();
	//prepare done, then construct the instruction with variable or symbol
	//left is a variable
	//prepare for the constructor;
	//Variable(il::node* original, string alias_name, il::type*t) //node* is high level
	//il::node*
	il::location lo(startline, startline); //location
	//il::string_const_exp* originall = new il::string_const_exp(var_namel, t, lo);
	//cout << "reg_name left ===> " << reg_namel << endl;
	//sail::Variable* sl = new sail::Variable(originall, reg_namel,t);

	//sail::Variable* sr = asm_ins->getMthCodeH()->getRegVarEle(regr, _type_map);//source 2nd
	sail::Variable* sr = asm_ins->getMthCodeH()->getRegVarEle(regr, tr,
			_type_map, true, false); //source 2nd
	//destination
	//sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(regl, _type_map);//destination 1st
	sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(regl, tl,
			_type_map, false, false); //destination 1st

	//right side is the same as in the left side
	il::string_const_exp* originalr = new il::string_const_exp(var_namer, tr,
			lo);
	//sail::Variable* sr = new sail::Variable(originalr, reg_namer,t);
	sr->set_original(originalr);
	if (var_namer != "")
		sr->set_alias_name(var_namer);
	else
		sr->set_alias_name(reg_namer);
	sr->set_type(tr);
	il::string_const_exp* originall = new il::string_const_exp(var_namel, tl,
			lo);
	sl->set_original(originall);
	sl->set_type(tl);
	//pair<bool, sail::Variable*> p2(true, sl);
	//(*var_map)[regl] = p2;
	//cout << "reg_name left ===> " << reg_namel << endl;
	//sail::Variable* sl = new sail::Variable(originall, reg_namel,t);
	//construct the il::Instruction which is an assignment
	//
	string ins_str = sl->to_string(true) + " = " + sr->to_string(true);
	il::string_const_exp* temp = new il::string_const_exp(ins_str, tl, lo);
	sail::Assignment* inst = new sail::Assignment(sl, sr, temp, startline);
	if (SAIL_INS_PRINT) {
		cout << "test assignment pp:::" << inst->to_string(true) << endl;
		cout << "test assignment:::" << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);

	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)&&(!USING_TYPE_INFER))//we do not do it if last one is true
		updateAnotherBranchReturnType(addr, regl, tl, sl);

}

void InstructionMapping::InstructionMapping_F10x(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	switch (opcode) {
	case OP_NOP:
		break;
	case OP_RETURN_VOID: {
		//build and label instruction
		//sail::Label* l = new sail::Label("__return_label", NULL);
		sail::Label* l = get_label("__return_label");
		sail::Jump* jump = new sail::Jump(l, NULL);
		_sail_ins->push_back(jump);
		// _sail_ins->push_back(l);
		if (SAIL_INS_PRINT) {
			cout << "test return void  PP::::" << jump->to_string(true) << endl;
			cout << "test return void  " << jump->to_string(false) << endl;
			cout << "test return label pp::::" << l->to_string(true) << endl;
			cout << "test return label " << l->to_string(false) << endl;
		}
	}
		break;
	default:
		break;
	}

}

//opcode is OP_GOTO
void InstructionMapping::InstructionMapping_F10t(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	//cout<< endl;
	if (0) {
		if (asm_ins->is_label())
			cout << "sucks goto!" << endl;
	}
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	string operd = asm_ins->getOperand1();
	string add_operd = operd.substr(1, operd.size() - 1);
	string name = "goto_" + add_operd; //goto the place of goto_**

	//if USING_TYPE_INFER is true, we do not do anything for adding assignment before return!
	if (!USING_TYPE_INFER) {
		if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)&&(!USING_TYPE_INFER)) { //we don't do it if last one is true
			unsigned int jump_addr = Str2Int(add_operd);
			addingOneAssignmentBeforeReturn(jump_addr,
					asm_ins->getMthCodeH()->getLatestLine());
		}
	}
	sail::Label *l = get_label(name);
	//(*_label_list)[label_addr] = l; //prepare for the Label* instruction to be push_back later
	sail::Jump* j = new sail::Jump(l, NULL);
	if (SAIL_INS_PRINT) {
		//cout << endl;
		cout << "test goto pp::" << j->to_string(true) << endl;
		cout << "test goto::" << j->to_string(false) << endl;
	}
	_sail_ins->push_back(j);

}

void InstructionMapping::InstructionMapping_F11n(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	InstructionMapping_constantAssignment(ins_debug, instruction_debug, asm_ins,
			addr);
}

void InstructionMapping::InstructionMapping_F11x(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();
	sail::Variable* ret_var = NULL;
	//map<unsigned int, pair<bool, sail::Variable*>>* var_map = asm_ins->getMthCodeH()->getRegVar();
	//assert(var_map != NULL);

	switch (opcode) {
	case OP_MOVE_RESULT:
	case OP_MOVE_RESULT_WIDE:
	case OP_MOVE_RESULT_OBJECT:
		break;
	case OP_MOVE_EXCEPTION: //we can't just ignore it because what if the definition variable used later but not defined?
	{
		assert(asm_ins->getTypeofVar()->size() > 0);
		ClassDefInfo* classdef = asm_ins->getTypeofVar()->begin()->second;
		il::type* t = getType(0, _dfr, classdef, _type_map);
		assert(asm_ins->getRegs()->size()>0);
		unsigned short reg = asm_ins->getRegs()->at(0);
		string reg_name = "v" + Int2Str(reg);
		string reg_var_name = reg_name;
		DebugInfo* debug = NULL;
		int line = -1;
		if (instruction_debug != NULL) {
			if (instruction_debug->count(asm_ins) > 0)
				debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1) {
				asm_ins->getMthCodeH()->setLatestLine(line);
			}
			vector<RVT*>* rvt_list = debug->getRVTList();
			if (rvt_list != NULL) {
				RVT* rvt = *rvt_list->begin();
				reg_var_name = rvt->getVarName();
			}
		}
		if (line == -1)
			line = asm_ins->getMthCodeH()->getLatestLine();

		sail::Variable* exception_var = asm_ins->getMthCodeH()->getRegVarEle(
				reg, t, _type_map, false, false);
		exception_var->set_alias_name(reg_var_name);
		//int line = asm_ins->getMthCodeH()->getLatestLine();
		il::string_const_exp* sr = new il::string_const_exp("", t,
				il::location(line, line));
		string str = exception_var->to_string(true) + " = " + sr->to_string();
		//sail::AddressString* ins = new sail::AddressString(exception_var, sr, new il::string_const_exp(str, t, il::location(line,line)), line);
		sail::Assignment* ins = new sail::Assignment(exception_var,
				ins_debug->type_of_exception_var(), sr, line);
		if (SAIL_INS_PRINT) {
			cout << "test move-exception  pp ::: " << ins->to_string(true)
					<< endl;
			cout << "test move-exception   ::: " << ins->to_string(false)
					<< endl;
		}
		_sail_ins->push_back(ins);

	}
		break;
	case OP_RETURN:
	case OP_RETURN_WIDE:
	case OP_RETURN_OBJECT: {

		if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)&&(!USING_TYPE_INFER)) //when USING_TYPE_INFER is false, we do it,other wise boyang do it
			_return_ins_has_label = asm_ins->is_label();
		assert(ins_var_type->size() > 0);
		ClassDefInfo* classdef = ins_var_type->begin()->second;
		//cout << "sanity check " << classdef->getField()->size()<< endl;
		//il::type* t = TypeMapping(0,_dfr,classdef,_type_map);
		il::type* t = getType(0, _dfr, classdef, _type_map);
		ret_var = new sail::Variable(t);

		unsigned short reg = (*asm_ins->getRegs()->begin());
		string reg_var_name = "";

		//added by haiyan 9.30
		bool add_flag = false;
		//when USING_TYPE_INFER is false, we do it,otherwise boyang will do it by typeinference!
		if ((NON_DECLARED_RETURN_TYPE) && (_return_ins_has_label)
				&& (_fun_declared_return_var != NULL) &&(!USING_TYPE_INFER)) {
			_return_ins_addr = addr;
			//only has one register and it is the register will be return;
			assert(asm_ins->getRegs()->size() == 1);
			_fun_declared_return_var->first = asm_ins->getRegs()->at(0);

			//check if return type is the same as the type in the return ins, if they are not the same, add a cast inst
			if (t != _fun_declared_return_var->second) {
				add_flag = true;
			}

		}
		//ended by haiyan 9.30

		//now build an assignment instruction and then jump to return label;
		//to the register inside the instruction, use the default way to generate its variable
		sail::Variable* reg_var = NULL;
		DebugInfo* debug = NULL;
		int line = -1;
		if (instruction_debug != NULL) {
			if (instruction_debug->count(asm_ins) > 0)
				debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1) {
				asm_ins->getMthCodeH()->setLatestLine(line);
			}
			vector<RVT*>* rvt_list = debug->getRVTList();
			if (rvt_list != NULL) {
				RVT* rvt = *rvt_list->begin();
				reg_var_name = rvt->getVarName();
			}
		}
		if (line == -1)
			line = asm_ins->getMthCodeH()->getLatestLine();
		il::string_const_exp* reg_var_exp = new il::string_const_exp(
				reg_var_name, t, il::location(line, line));
		//reg_var = asm_ins->getMthCodeH()->getRegVarEle(reg, _type_map);
		reg_var = asm_ins->getMthCodeH()->getRegVarEle(reg, t, _type_map, true,
				false);
		reg_var->set_original(reg_var_exp);
		if (reg_var_name != "")
			reg_var->set_alias_name(reg_var_name);
		else
			reg_var->set_alias_name("v" + Int2Str(reg));
		//reg_var->set_type(t);

		//string ins_str = ret_var->to_string(true) + " = "
		//		+ reg_var->to_string(true);

		//added an assignment instruction before assigned to return, lhs has the same type as declared return type
		sail::Variable* temp_return = NULL;
		if (NON_DECLARED_RETURN_TYPE &&(!USING_TYPE_INFER)) { //when USING_TYPE_INFER is true, boyang do it
			if ((add_flag) && (_fun_declared_return_var != NULL)) {
				temp_return =
						asm_ins->getMthCodeH()->getRegVarEle(getRandom(),
								_fun_declared_return_var->second, _type_map,
								false, false);
				il::string_const_exp* origi = new il::string_const_exp("",
						_fun_declared_return_var->second,
						il::location(line, line));
				sail::Assignment* ass_inst_temp = new sail::Assignment(
						temp_return, reg_var, origi, line);
				_sail_ins->pop_back();
				_sail_ins->push_back(ass_inst_temp);
				add_labels(asm_ins);

				if (SAIL_INS_PRINT) {
					cout << "test adding return assign pp ::: "
							<< ass_inst_temp->to_string(true) << endl;
					cout << "test adding return assign  ::: "
							<< ass_inst_temp->to_string(false) << endl;
				}
				reg_var = temp_return;
			}
		}
		il::string_const_exp* temp = new il::string_const_exp("", t,
				il::location(line, line));
		sail::Assignment* ass_inst = NULL;
		if((NON_DECLARED_RETURN_TYPE)&&(add_flag)&&(_fun_declared_return_var != NULL)&&(!USING_TYPE_INFER))//we don't do it if last one is true
		{
				ret_var->set_type(_fun_declared_return_var->second);
				ass_inst = new sail::Assignment(ret_var, temp_return, temp, line);

		}
		else{
			ass_inst = new sail::Assignment(ret_var, reg_var,
				temp, line);
			cout << "************ set it as return instruction ! " << endl;
			ass_inst->set_as_return_instruction();
		}
		_return_ins = ass_inst;
		if (SAIL_INS_PRINT) {
			cout << "test return assign pp ::: " << ass_inst->to_string(true)
					<< endl;
			cout << "test return assign  ::: " << ass_inst->to_string(false)
					<< endl;
		}
		_sail_ins->push_back(ass_inst);

		//sail::Label* l = new sail::Label("__return_label", NULL);
		sail::Label* return_l = get_label("__return_label");
		sail::Jump* j = new sail::Jump(return_l, NULL);
		_sail_ins->push_back(j);
		if (SAIL_INS_PRINT) {
			cout << "test return jump pp ::: " << j->to_string(true) << endl;
			cout << "test return jump  ::: " << j->to_string(false) << endl;
		}
		//_sail_ins->push_back(l);
		//cout << "test return label pp ::: " << l->to_string(true) <<endl;
		//cout << "test return label  ::: " << l->to_string(false) <<endl;

	}
		break;

	case OP_MONITOR_ENTER: {
		//cout << "---------------------------------------------" << endl ;
		//cout << "asm: " << asm_ins->toString()<<endl;
		//cout << "is label : " << asm_ins->is_label()<<endl;
		//cout << "inside method: " <<asm_ins->getMthCodeH()->getMethod()->toString()<<endl;
		//add_labels(asm_ins);
		//fake instructions
		string regn = asm_ins->getOperand1().substr(1,
				asm_ins->getOperand1().size() - 1);
		int reg = atoi(regn.c_str());
		assert(ins_var_type->size() != 0);
		assert((*ins_var_type)[reg] != NULL);
		ClassDefInfo* classdef = (*ins_var_type)[reg];
		il::type* t = getType(0, _dfr, classdef, _type_map);
		//sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(reg,_type_map);
		DebugInfo* debug = NULL;
		int line = -1;
		if (instruction_debug != NULL) {
			if (instruction_debug->count(asm_ins) > 0)
				debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1) {
				asm_ins->getMthCodeH()->setLatestLine(line);
			}
		}
		if (line == -1)
			line = asm_ins->getMthCodeH()->getLatestLine();
		sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(reg, t,
				_type_map, true, false);
		il::string_const_exp* sr = new il::string_const_exp("monitor-enter", t,
				il::location(line, line));
		string str = sl->to_string(true) + " = " + sr->to_string();
		sail::AddressString* ins = new sail::AddressString(sl, sr,
				new il::string_const_exp(str, t, il::location(line, line)),
				line);
		_sail_ins->push_back(ins);
	}
		break;
	case OP_MONITOR_EXIT: {
		//cout << "---------------------------------------------" << endl ;
		//cout << "asm: " << asm_ins->toString()<<endl;
		//cout << "is label : " << asm_ins->is_label()<<endl;
		//cout << "inside method: " <<asm_ins->getMthCodeH()->getMethod()->toString()<<endl;
		//add_labels(asm_ins);
		string regn = asm_ins->getOperand1().substr(1,
				asm_ins->getOperand1().size() - 1);
		int reg = atoi(regn.c_str());
		assert(ins_var_type->size() != 0);
		assert((*ins_var_type)[reg] != NULL);
		ClassDefInfo* classdef = (*ins_var_type)[reg];
		il::type* t = getType(0, _dfr, classdef, _type_map);
		//sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(reg,_type_map);
		DebugInfo* debug = NULL;
		int line = -1;
		if (instruction_debug != NULL) {
			if (instruction_debug->count(asm_ins) > 0)
				debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1) {
				asm_ins->getMthCodeH()->setLatestLine(line);
			}
		}
		if (line == -1)
			line = asm_ins->getMthCodeH()->getLatestLine();

		sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(reg, t,
				_type_map, false, false);
		il::string_const_exp* sr = new il::string_const_exp("monitor-exit", t,
				il::location(line, line));
		string str = sl->to_string(true) + " = " + sr->to_string();
		sail::AddressString* ins = new sail::AddressString(sl, sr,
				new il::string_const_exp(str, t, il::location(line, line)),
				line);
		_sail_ins->push_back(ins);
		//fake instruction
	}
		break;
	case OP_THROW: {
		assert(ins_var_type->size()> 0);
		ClassDefInfo* classdef = ins_var_type->begin()->second;
		//il::type* t = TypeMapping(0,_dfr, classdef, _type_map);
		il::type* t = getType(0, _dfr, classdef, _type_map);
		DebugInfo* debug = NULL;
		int line = -1;
		if (instruction_debug != NULL) {
			if (instruction_debug->count(asm_ins) > 0)
				debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1) {
				asm_ins->getMthCodeH()->setLatestLine(line);
			}
		}
		if (line == -1)
			line = asm_ins->getMthCodeH()->getLatestLine();

		// cout << "---------------------------------------------" << endl ;
		// cout << "asm: " << asm_ins->toString()<<endl;
		// cout << "is label : " << asm_ins->is_label()<<endl;
		// cout << "inside method: " <<asm_ins->getMthCodeH()->getMethod()->toString()<<endl;
		// add_labels(asm_ins);
		/*
		 string regn = asm_ins->getOperand1().substr(1, asm_ins->getOperand1().size()-1);
		 int reg = atoi(regn.c_str());
		 sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(reg, _type_map);
		 il::string_const_exp* sr = new il::string_const_exp("throw", il::get_integer_type(), il::location(0,0));
		 string str = sl->to_string(true) + " = " + sr->to_string();
		 */
		//sail::AddressString* ins = new sail::AddressString(sl, sr, new il::string_const_exp(str, il::get_integer_type(), il::location(0,0)), 0);
		//create an assignment for cur_exception  = vaa;
		string regn = asm_ins->getOperand1().substr(1,
				asm_ins->getOperand1().size() - 1);
		int reg = Str2Int(regn);
		//sail::Variable* sl = ins_debug->cur_exception_var();
		sail::Variable* sl = ins_debug->throw_exception_var();
		sl->set_type(t);
		//sail::Variable* sr = asm_ins->getMthCodeH()->getRegVarEle(reg, _type_map);
		sail::Variable* sr = asm_ins->getMthCodeH()->getRegVarEle(reg, t,
				_type_map, true, false);
		if(0){
		string ins_str_assig = sl->to_string(true) + " = "
				+ sr->to_string(true);
		il::string_const_exp* assig_exp = new il::string_const_exp(
				ins_str_assig, t, il::location(line, line));
		sail::Assignment* inst_assig = new sail::Assignment(sl, sr, assig_exp,
				line);
		_sail_ins->push_back(inst_assig);
		if (SAIL_INS_PRINT) {
			cout << "test throw assignement pp:: "
					<< inst_assig->to_string(true) << endl;
			cout << "test throw assignement :: " << inst_assig->to_string(false)
					<< endl;
		}
		}
		sail::ExceptionReturn* e_r = new sail::ExceptionReturn();
		_sail_ins->push_back(e_r);
		if (SAIL_INS_PRINT) {
			cout << "test throw return  pp:: " << e_r->to_string(true) << endl;
			cout << "test throw return :: " << e_r->to_string(false) << endl;
		}
		//fake instruction

	}
		break;

	default:
		break;
	}

}

void InstructionMapping::InstructionMapping_F12x_unop_neg(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, il::unop_type unop, unsigned int addr) {
	//neg-int va, vb;
	//cout << endl;
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	//map<unsigned int, pair<bool, sail::Variable*>>* var_map = asm_ins->getMthCodeH()->getRegVar();
	//assert(var_map != NULL);

	vector<unsigned short>* regs = asm_ins->getRegs();
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();
	//sail::Instruction* ins = NULL;
	il::type* lt = NULL;
	il::type* rt = NULL;
	//map<ASMInstruction*, DebugInfo*>* instruction_debug = codeh->mgetIns2Debug(); //one ASMInstruction* one debuginfo* for the passing in method codeh
	if (instruction_debug == NULL) //do not have the instruction and debuginformation mapping for this method
	{
		if (SCRATCH_INF)
			cout << "instruction_debug mapping is NULL and exit";
		//exit(1);
	}
	DebugInfo* debug = NULL;

	//DexOpcode opcode = asm_ins->getOpcode();
	//unsigned short regl = *regs->begin(); //only have one reg represent the variable
	//unsigned short regr =*(++regs->begin());
	unsigned short regl = regs->at(0); //only have one reg represent the variable
	unsigned short regr = regs->at(1);
	//the two variable are the same type, so just need to fetch one of the type information to retrieve all
	ClassDefInfo* classdefl = (*ins_var_type)[regl];//ins_var_type->begin()->second;
	ClassDefInfo* classdefr = (*ins_var_type)[regr];
	//t = TypeMapping(0, _dfr , classdef); //deal with the type
	lt = getType(0, _dfr, classdefl, _type_map); //deal with the type
	rt = getType(0, _dfr, classdefr, _type_map);
	//t = TypeMapping(_dfr , classdef); //deal with the type
	if (instruction_debug->count(asm_ins) > 0) {
		//cout << "Found! " << endl;
		debug = (*instruction_debug)[asm_ins];
	}
	int startline = -1;
	string var_namel = "";
	string reg_namel = "v" + Int2Str(regl);
	string var_namer = "";
	string reg_namer = "v" + Int2Str(regr);
	if (debug == NULL) {
		if (SCRATCH_INF)
			cout << "debug information is NULL " << endl;
	} else {
		startline = debug->getLine();
		if (startline != -1)
			asm_ins->getMthCodeH()->setLatestLine(startline);
		//cout << "line: " << startline << endl;
		vector<RVT*>* rvt_list = debug->getRVTList();
		if (rvt_list == NULL) {
			if (SCRATCH_INF)
				cout << "rvt list is empty" << endl;
		} else {
			vector<RVT*>::iterator it = rvt_list->begin();
			for (; it != rvt_list->end(); it++) {
				unsigned short reg = (*it)->getReg();
				if (reg == regl)
					var_namel = (*it)->getVarName();
				else
					var_namer = (*it)->getVarName();
			}
		}
	}
	if (startline == -1)
		startline = asm_ins->getMthCodeH()->getLatestLine();

	//prepare done, then construct the instruction with variable or symbol
	//left is a variable
	//prepare for the constructor;
	//Variable(il::node* original, string alias_name, il::type*t) //node* is high level
	//il::node*
	il::location lo(startline, startline); //location

	il::string_const_exp* originalr = new il::string_const_exp(var_namer, rt,
			lo);
	//sail::Variable* sr = new sail::Variable(originalr, reg_namer,t);
	//sail::Variable* sr = asm_ins->getMthCodeH()->getRegVarEle(regr, _type_map);
	sail::Variable* sr = asm_ins->getMthCodeH()->getRegVarEle(regr, rt,
			_type_map, true, false);
	sr->set_original(originalr);
	if (var_namer != "")
		sr->set_alias_name(var_namer);
	else
		sr->set_alias_name(reg_namer);
	//sr->set_type(t);

	il::string_const_exp* originall = new il::string_const_exp(var_namel, lt,
			lo);
	//sail::Variable* sl = new sail::Variable(originall, reg_namel,t);
	//destination
	//sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(regl, _type_map);
	sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(regl, lt,
			_type_map, false, false);
	sl->set_original(originall);
	if (var_namel != "")
		sl->set_alias_name(var_namel);
	else
		sl->set_alias_name(reg_namel);
	//sl->set_type(t);
	//pair<bool, sail::Variable*> p(true, sl);
	//(*var_map)[regl] = p;

	//construct the il::Instruction which unop
	string ins_str;
	if (asm_ins->getOperator().substr(0, 3) == "neg")
		ins_str = sl->to_string(true) + " = -" + sr->to_string(true);
	else
		ins_str = sl->to_string(true) + " = ~" + sr->to_string(true);
	il::string_const_exp* temp = new il::string_const_exp(ins_str, rt, lo);
	//cout << "check line: " << startline << endl;
	//HZHU added for invariant;
	processInvariant(sl, sr, NULL, startline);
	assert( _tempvar_for_invariant != NULL);
	sail::Unop* inst = new sail::Unop(sl, _tempvar_for_invariant, unop, temp,
			startline);
	//HZHU ended for invariant
	//sail::Unop* inst = new sail::Unop(sl, sr, unop, temp, startline);
	if (SAIL_INS_PRINT) {
		cout << "unop test pp:::" << inst->to_string(true) << endl;
		cout << "unop test :::" << inst->to_string(false) << endl;
		//cout << "RHS before process invariant " << sr->get_type()->to_string() << endl;
		cout << "RHS " << _tempvar_for_invariant->to_string() << " type :: " << _tempvar_for_invariant->get_type()->to_string() << endl;
		cout << "LHS  " <<  sl->to_string() << "type :: " <<sl->get_type()->to_string() << endl;
	}
	_sail_ins->push_back(inst);

	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)&&(!USING_TYPE_INFER)) //when USING_TYPE_INFER is true, boyang will do it
		updateAnotherBranchReturnType(addr, regl, lt, sl);
}

void InstructionMapping::cast(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr, il::type* cast_type) { //add-int va, vb, vc
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	vector<unsigned short>* regs = asm_ins->getRegs();
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();
	bool assi_flag = false;


	if (asm_ins->getOperator() != "check-cast") {
		assert(regs->size() == 2);
		assert(ins_var_type->size() == 2);
		ClassDefInfo* fir = ins_var_type->begin()->second;
		ClassDefInfo* sec = (++ins_var_type->begin())->second;
		if (0) {
			cout << "first type is " << fir->getTypename() << endl;
			cout << "second type is " << sec->getTypename() << endl;
		}
		assert(fir != NULL);
		assert(sec!= NULL);
		if ((!asm_ins->is_label()) && (regs->at(0) == regs->at(1))
				&& (fir->isDigits() == sec->isDigits())) //do nothing,just int to int and no effect on CFG
			return;
		if ((regs->at(0) != regs->at(1) && (fir->isDigits() && sec->isDigits()))) {
			//doing assginment
			assi_flag = true;
		}
	}

	//OK, following should deal with the cast instruction;
	if (instruction_debug == NULL) //do not have the instruction and debuginformation mapping for this method
	{
		if (SCRATCH_INF)
			cout << "instruction_debug mapping is NULL and exit";
	}
	DebugInfo* debug = NULL;

	unsigned short regl = regs->at(0); //only have one reg represent the variable
	unsigned short regr = regl;
	//if ((asm_ins->getOperator() != "check-cast")
	//		&& (asm_ins->getOperator().find("const") == string::npos))
	if (asm_ins->getOperator() != "check-cast") {
		assert(regs->size() >= 2);
		regr = regs->at(1);
	}
	//done with the prepare the registers number

	//ClassDefInfo* orig_classdef = NULL;
	//il::type* origi_t = NULL;
	ClassDefInfo* cast_classdef = NULL;
	il::type* cast_t = cast_type;

	ClassDefInfo* origi_classdef = NULL;
	il::type* origi_t = NULL;

	assert(ins_var_type->count(regr) > 0);
	if (asm_ins->getOperator() != "check-cast") {
		if (regr != regl)
			origi_classdef = (*ins_var_type)[regr];
		else
			origi_classdef = (*ins_var_type)[2000];
	} else
		//check-cast
		origi_classdef = (*ins_var_type)[regr];

	origi_t = getType(0, _dfr, origi_classdef, _type_map);
	if (asm_ins->getOperator() == "check-cast") {
		cast_classdef = asm_ins->getCheckCastClassDef();
		assert(cast_classdef != NULL);
		cast_t = getType(0, _dfr, cast_classdef, _type_map);
	}

	if (instruction_debug->count(asm_ins) > 0) {
		debug = (*instruction_debug)[asm_ins];
	}
	int startline = -1;

	string reg_namel = "v" + Int2Str(regl);
	string var_namel = reg_namel;

	string reg_namer = "v" + Int2Str(regr);
	string var_namer = reg_namer;

	if (debug == NULL) {
		if (SCRATCH_INF)
			cout << "debug information is NULL " << endl;
	} else {
		startline = debug->getLine();
		if (startline != -1)
			asm_ins->getMthCodeH()->setLatestLine(startline);
		vector<RVT*>* rvt_list = debug->getRVTList();
		if (rvt_list == NULL) {
			if (SCRATCH_INF)
				cout << "rvt list is empty" << endl;
		} else {
			vector<RVT*>::iterator it = rvt_list->begin();
			for (; it != rvt_list->end(); it++) {
				unsigned int reg = (*it)->getReg();
				if (reg == regl)
					var_namel = (*it)->getVarName();

				if (reg == regr)
					var_namer = (*it)->getVarName();
			}
		}
	}
	if (startline == -1)
		startline = asm_ins->getMthCodeH()->getLatestLine();

	il::location lo(startline, startline); //location

	sail::Variable* rhs_v = NULL;
	//if (opcode != OP_CHECK_CAST) {
	rhs_v = asm_ins->getMthCodeH()->getRegVarEle(regr, origi_t, _type_map, true,
			false);
	rhs_v->set_alias_name(var_namer);

	sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(regl, cast_t,
			_type_map, false, false);
	sl->set_alias_name(var_namel);

	il::string_const_exp* temp = new il::string_const_exp("", cast_t, lo);

	if (!assi_flag) {
		sail::Cast* cast = NULL;
		//HZHU add invariant
		assert(sl != NULL);
		if (0) {
			processInvariant(sl, rhs_v, NULL, startline);
			assert(_tempvar_for_invariant != NULL);
			assert(cast_t != NULL);
			if (0)
				cout << "temp var for invariant "
						<< _tempvar_for_invariant->to_string() << " type == "
						<< _tempvar_for_invariant->get_type()->to_string()
						<< endl;
			cast = new sail::Cast(sl, _tempvar_for_invariant, cast_t, temp,
					startline);
		}else
			cast = new sail::Cast(sl, rhs_v, cast_t, temp, startline);

		if (0) {
			cout << "origi_t " << origi_t->to_string() << endl;
			cout << "end check " << endl;
		}

		if (SAIL_INS_PRINT) {
			cout << "test cast pp:::" << cast->to_string(true) << endl;
			cout << "test cast:::" << cast->to_string(false) << endl;
		}
		_sail_ins->push_back(cast);
	} else {
		cast_assignment(sl, rhs_v, temp, startline);
	}

	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)
			&& (!USING_TYPE_INFER)) //boyang will do it if USING_TYPE_INFER is true
		updateAnotherBranchReturnType(addr, regl, origi_t, sl);

}

void InstructionMapping::cast_assignment(sail::Variable* lhs, sail::Variable* rhs, il::string_const_exp* original, unsigned int line){
	sail::Assignment* inst = new sail::Assignment(lhs, rhs, original,
				line);

		if (SAIL_INS_PRINT) {
			cout << "test global load pp::" << inst->to_string(true) << endl;
			cout << "test global load::" << inst->to_string(false) << endl;
		}
		_sail_ins->push_back(inst);

}
/**
 *** array-length v0, v1 : load(v1, -4) (-4 store the length of array),
 *** then we get the total size of array,
 *** then we use it to divide the length of each ele, wo get the length of array
 **/
void InstructionMapping::InstructionMapping_arrayLength(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {

	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	vector<unsigned short>* regs = asm_ins->getRegs();
	assert(regs->size() == 2);
	unsigned int regl = regs->at(0);
	unsigned int regr = regs->at(1);
	//assert(regl != regr); //to check if it hold; this may not hold

	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();
	assert(ins_var_type != NULL);
	assert(ins_var_type->size() == 2);
	int startline = -1;
	string reg_namel = "v" + Int2Str(regl);
	string var_namel = reg_namel;
	string reg_namer = "v" + Int2Str(regr);
	string var_namer = reg_namer;

	if (instruction_debug == NULL) //do not have the instruction and debuginformation mapping for this method
	{
		if (SCRATCH_INF)
			cout << "instruction_debug mapping is NULL and exit";
	}
	DebugInfo* debug = NULL;

	if (instruction_debug->count(asm_ins) > 0) {
		debug = (*instruction_debug)[asm_ins];
	}
	if (debug != NULL) //retrive line number and variable name from debug information
	{
		startline = debug->getLine();
		if (startline != -1)
			asm_ins->getMthCodeH()->setLatestLine(startline);
		vector<RVT*>* rvt_list = debug->getRVTList();
		if (rvt_list == NULL) {
			if (SCRATCH_INF)
				cout << "rvt list is empty" << endl;
		} else {
			vector<RVT*>::iterator it = rvt_list->begin();
			for (; it != rvt_list->end(); it++) {
				unsigned int reg = (*it)->getReg();
				if (reg == regl)
					var_namel = (*it)->getVarName();
				if (reg == regr)
					var_namer = (*it)->getVarName();
			}
		}

	}
	if (startline == -1)
		startline = asm_ins->getMthCodeH()->getLatestLine();
	il::location lo(startline, startline); //location

	//now retrieve types from the type map;
	ClassDefInfo* lhscdi = (*ins_var_type)[regl];
	ClassDefInfo* rhscdi = NULL;
	if (regl != regr)
		rhscdi = (*ins_var_type)[regr];
	else
		rhscdi = (*ins_var_type)[2000];
	assert(lhscdi != NULL);
	assert(rhscdi != NULL);
	il::type* lhst = getType(0, _dfr, lhscdi, _type_map); //length regsister
	il::type* rhst = getType(0, _dfr, rhscdi, _type_map); // array register
	//making variables for them
	sail::Variable* rhs_va = asm_ins->getMthCodeH()->getRegVarEle(regr, rhst,
			_type_map, true, false); //in_use, not check_cast
	rhs_va->set_alias_name(var_namer);




	vector<sail::Symbol*>* args_v = new vector<sail::Symbol*>();
		args_v->push_back(rhs_va);

	//build one temp variable to retrive the total number size of the arra
	sail::Variable* total_array_size = new sail::Variable("total_array_size",
			il::get_integer_type(), true, false, -1, false); //temp, not_argu, -1, not global
	total_array_size->set_var_name_as_temp_name();

	total_array_size->set_alias_name(total_array_size->get_var_name());

	//build instructions that used by
	//load sail::inst
	il::string_const_exp* original = new il::string_const_exp("", lhst,
			lo);
	string fn_name = "buffer_size";
	vector<il::type*> args;
	args.push_back(rhst);
	il::type* fn_signature = il::function_type::make(il::get_integer_type(),
			args, false);
	//cout << "fn_signature ::: " << fn_signature->to_string() << endl;


	sail::FunctionCall* inst = new sail::FunctionCall(total_array_size, fn_name,
			il::namespace_context(), fn_signature, args_v, false, false, false,
			false, original, startline);
	//sail::Load* load_array_size = new sail::Load(total_array_size, rhs_va, -4,"", original, startline);
	if (SAIL_INS_PRINT) {
		cout << "array-length buffer_size PP :: " << inst->to_string(true)
				<< endl;
		cout << "array-length buffer_size:: " << inst->to_string() << endl;
	}
	_sail_ins->push_back(inst);

	//now build up an binop instruction
	//calculate the size of ele array

	sail::Variable* lhs_va = asm_ins->getMthCodeH()->getRegVarEle(regl, lhst,
				_type_map, false, false); //in_def, not check_cast
	lhs_va->set_alias_name(var_namel);

	unsigned int ele_size = 0;
	string ele_type = rhscdi->getTypename();
	ele_type = ele_type.substr(1, ele_type.length() - 1);
	//cout << "ele_type " << ele_type << endl;
	string first_char = ele_type.substr(0, 1);
	//cout << "first_char " << first_char << endl;
	if (first_char == "L")
		ele_size = 8;
	else
		ele_size = 4;

	sail::Constant* ele_size_const = new sail::Constant(ele_size, false, 32);
	il::binop_type divid = il::_DIV;
	il::string_const_exp* temp = new il::string_const_exp("", lhst, lo);
	sail::Binop* binop = new sail::Binop(lhs_va, total_array_size,
			ele_size_const, divid, temp, startline);
	if (SAIL_INS_PRINT) {
		cout << "array-length length PP :: " << binop->to_string(true) << endl;
		cout << "array-length lengh:: " << binop->to_string() << endl;
	}
	_sail_ins->push_back(binop);
	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)&&(!USING_TYPE_INFER)) //boyang will do it if UINSG_TYPE_INFER is true
		updateAnotherBranchReturnType(addr, regl, lhst, lhs_va);

}

void InstructionMapping::InstructionMapping_F12x(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opc = asm_ins->getOpcode();
	il::type* cast_t = NULL;
	switch (opc) {
	case OP_MOVE:
	case OP_MOVE_WIDE:
	case OP_MOVE_OBJECT:
		InstructionMapping_varAssignment(ins_debug, instruction_debug, asm_ins,
				addr);
		break;
	case OP_ARRAY_LENGTH: {
		//InstructionMapping_constantAssignment(ins_debug, instruction_debug, asm_ins, addr);
		InstructionMapping_arrayLength(ins_debug, instruction_debug, asm_ins,
				addr);
	}
		break;
		//unop_type : _NEGATE
	case OP_NEG_INT: {
		il::unop_type unop = il::_NEGATE;
		InstructionMapping_F12x_unop_neg(ins_debug, instruction_debug, asm_ins,
				unop, addr);
	}
		break;
		//unop_type: _BITWISE_NOT
	case OP_NOT_INT: {
		il::unop_type unop = il::_BITWISE_NOT;
		InstructionMapping_F12x_unop_neg(ins_debug, instruction_debug, asm_ins,
				unop, addr);
	}
		break;
	case OP_NEG_LONG: {
		il::unop_type unop = il::_NEGATE;
		InstructionMapping_F12x_unop_neg(ins_debug, instruction_debug, asm_ins,
				unop, addr);
	}
		break;
	case OP_NOT_LONG: {
		il::unop_type unop = il::_BITWISE_NOT;
		InstructionMapping_F12x_unop_neg(ins_debug, instruction_debug, asm_ins,
				unop, addr);
	}
		break;
	case OP_NEG_FLOAT: {
		il::unop_type unop = il::_NEGATE;
		InstructionMapping_F12x_unop_neg(ins_debug, instruction_debug, asm_ins,
				unop, addr);
	}
		break;
	case OP_NEG_DOUBLE: {
		il::unop_type unop = il::_NEGATE;
		InstructionMapping_F12x_unop_neg(ins_debug, instruction_debug, asm_ins,
				unop, addr);
	}
		break;
	case OP_INT_TO_LONG: //cast
	{
		cast_t = il::get_integer_type();
		//cast_t = il::base_type::make("long int", 64, 64, true, il::INTEGER, "");
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_INT_TO_FLOAT: {
		cast_t = il::get_integer_type();
		//cast_t = il::base_type::make("ieee_float", 32, 32, true, il::IEEE_FLOAT,
		//		"");
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_INT_TO_DOUBLE: {
		cast_t = il::get_integer_type();
		//cast_t = il::base_type::make("ieee_double", 64, 64, true,
		//		il::IEEE_FLOAT, "");
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_LONG_TO_INT: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_LONG_TO_FLOAT: {
		cast_t = il::get_integer_type();
		//cast_t = il::base_type::make("ieee_float", 32, 32, true, il::IEEE_FLOAT,
		//		"");
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_LONG_TO_DOUBLE: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_FLOAT_TO_INT: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_FLOAT_TO_LONG: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_FLOAT_TO_DOUBLE: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_DOUBLE_TO_INT: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_DOUBLE_TO_LONG: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_DOUBLE_TO_FLOAT: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_INT_TO_BYTE: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_INT_TO_CHAR: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_INT_TO_SHORT: {
		cast_t = il::get_integer_type();
		cast(ins_debug, instruction_debug, asm_ins, addr, cast_t);
	}
		break;
	case OP_ADD_INT_2ADDR: {
		il::binop_type bt = il::_PLUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_SUB_INT_2ADDR: {
		il::binop_type bt = il::_MINUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_MUL_INT_2ADDR: {
		il::binop_type bt = il::_MULTIPLY;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_DIV_INT_2ADDR: {
		il::binop_type bt = il::_DIV;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_REM_INT_2ADDR: {
		il::binop_type bt = il::_MOD;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_AND_INT_2ADDR: {
		il::binop_type bt = il::_BITWISE_AND;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_OR_INT_2ADDR: {
		il::binop_type bt = il::_BITWISE_OR;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_XOR_INT_2ADDR: {
		il::binop_type bt = il::_BITWISE_XOR;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_SHL_INT_2ADDR: {
		il::binop_type bt = il::_LEFT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_SHR_INT_2ADDR: {
		il::binop_type bt = il::_RIGHT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_USHR_INT_2ADDR: {
		il::binop_type bt = il::_RIGHT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_ADD_LONG_2ADDR: {
		il::binop_type bt = il::_PLUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_SUB_LONG_2ADDR: {
		il::binop_type bt = il::_MINUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_MUL_LONG_2ADDR: {
		il::binop_type bt = il::_MULTIPLY;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_DIV_LONG_2ADDR: {
		il::binop_type bt = il::_DIV;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_REM_LONG_2ADDR: {
		il::binop_type bt = il::_MOD;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_AND_LONG_2ADDR: {
		il::binop_type bt = il::_BITWISE_AND;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_OR_LONG_2ADDR: {
		il::binop_type bt = il::_BITWISE_OR;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_XOR_LONG_2ADDR: {
		il::binop_type bt = il::_BITWISE_XOR;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_SHL_LONG_2ADDR: {
		il::binop_type bt = il::_LEFT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_SHR_LONG_2ADDR: {
		il::binop_type bt = il::_RIGHT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_USHR_LONG_2ADDR: {
		il::binop_type bt = il::_RIGHT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_ADD_FLOAT_2ADDR: {
		il::binop_type bt = il::_PLUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_SUB_FLOAT_2ADDR: {
		il::binop_type bt = il::_MINUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_MUL_FLOAT_2ADDR: {
		il::binop_type bt = il::_MULTIPLY;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_DIV_FLOAT_2ADDR: {
		il::binop_type bt = il::_DIV;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_REM_FLOAT_2ADDR: {
		il::binop_type bt = il::_MOD;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_ADD_DOUBLE_2ADDR: {
		il::binop_type bt = il::_PLUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_SUB_DOUBLE_2ADDR: {
		il::binop_type bt = il::_MINUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_MUL_DOUBLE_2ADDR: {
		il::binop_type bt = il::_MULTIPLY;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_DIV_DOUBLE_2ADDR: {
		il::binop_type bt = il::_DIV;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	case OP_REM_DOUBLE_2ADDR: {
		il::binop_type bt = il::_MOD;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, false, addr);
	}
		break;
	default:
		return;
	}

}

//goto/16 + AA
void InstructionMapping::InstructionMapping_F20t(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	if (0) {
		if (asm_ins->is_label())
			cout << "sucks goto/16!" << endl;
	}

	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	string operd = asm_ins->getOperand1();
	string add_operd = operd.substr(1, operd.size() - 1);
	string name = "goto_" + add_operd; //goto the place of goto_**

	sail::Label *l = get_label(name);
	sail::Jump* j = new sail::Jump(l, NULL);

	if (SAIL_INS_PRINT) {
		cout << "test goto pp::" << j->to_string(true) << endl;
		cout << "test goto::" << j->to_string(false) << endl;
	}
	_sail_ins->push_back(j);
}


void InstructionMapping::InstructionMapping_constClass(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {

	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	map<string, sail::Variable*>* global_vars = NULL;
	if(!USING_CLINIT)
		global_vars = _dfr->getGlobalVars();
	else
		global_vars = ins_debug->getGlobalVars();

	string opd2 = asm_ins->getOperand2() + "->TYPE"; //this is the alias_name of
	//cout << "CHECK opd2 " << opd2 << endl;
	//assert(global_vars->count(opd2));
	sail::Variable* global_var = NULL;
	if(global_vars->count(opd2) > 0)
		global_var = (*global_vars)[opd2];
	else { //didn't find
		string special_temp_name = "v" + Int2Str(_dfr->getGlobalVariableReg());
		ClassDefInfo* classdef = _dfr->getClassDefByName("Ljava/lang/Class;");
		il::type* t = getType(0, _dfr, classdef, _type_map);
		global_var = new sail::Variable(special_temp_name,
												t, false, false, -1, true);
		(*global_vars)[opd2] = global_var;
	}

	il::type* rhs_t = global_var->get_type();

	map<unsigned int, ClassDefInfo*>* ins_type_map = asm_ins->getTypeofVar();
	assert(ins_type_map->size() == 1);
	ClassDefInfo* lhs_classdef = ins_type_map->begin()->second;
	il::type* lhs_t = getType(0, _dfr, lhs_classdef, _type_map);
	//assert(rhs_t == lhs_t);
	assert(asm_ins->getRegs()->size() ==1);
	unsigned int reg = asm_ins->getRegs()->at(0);
	string reg_name = "v" + Int2Str(reg);
	string reg_var_name = reg_name;
	int line = -1;
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;
	if (instruction_debug != NULL) {
		if (instruction_debug->count(asm_ins) > 0)
			debug = (*instruction_debug)[asm_ins];
	}
	if (debug != NULL) {
		line = debug->getLine();
		if (line != -1)
			asm_ins->getMthCodeH()->setLatestLine(line);
		rvt_list = debug->getRVTList();
	}
	if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
		for (unsigned int i = 0; i < rvt_list->size(); i++) {
			if (rvt_list->at(i)->getReg() == reg)
				reg_var_name = rvt_list->at(i)->getVarName();
		}
	}

	if (line == -1)
		line = asm_ins->getMthCodeH()->getLatestLine();
	il::location lo(line, line);

	sail::Variable* reg_var = asm_ins->getMthCodeH()->getRegVarEle(reg, rhs_t,
			_type_map, false, false);
	reg_var->set_alias_name(reg_var_name);
	il::string_const_exp* temp = new il::string_const_exp("", lhs_t, lo);

	sail::Assignment* inst = new sail::Assignment(reg_var, global_var, temp,
			line);

	if (SAIL_INS_PRINT) {
		cout << "test global load pp::" << inst->to_string(true) << endl;
		cout << "test global load::" << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);

	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)&&(!USING_TYPE_INFER)) //boyang will do if if USING_TYPE_INFER is true
		updateAnotherBranchReturnType(addr, reg, rhs_t, reg_var);
}

//sget
void InstructionMapping::load(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr1) {
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr1);
	add_labels(asm_ins);

	//HZHU changed to retrive global variable;
	map<string, sail::Variable*>* global_vars = NULL;
	if(!USING_CLINIT)
		global_vars = _dfr->getGlobalVars();
	else
		global_vars = ins_debug->getGlobalVars();
	string field_name = asm_ins->getOperand2();
	StringSplit* ssp = new StringSplit(field_name, ":");
	vector<string>* sub_strs = ssp->getSubStrs();
	assert(sub_strs->size() == 2);
	field_name = sub_strs->at(0);
	string type_name = sub_strs->at(1);
	type_name = type_name.substr(1, type_name.size()-1);



	sail::Variable* global_var = NULL;
	if(global_vars->count(field_name)){
		global_var = (*global_vars)[field_name];
	}else{
		//or we can get its super class name's static fields
		StringSplit* ssp1 = new StringSplit(field_name, "->");
		vector<string>* sub_str2 = ssp1->getSubStrs();
		assert(sub_str2->size() == 2);
		string classname = sub_str2->at(0);
		string fname = sub_str2->at(1);
		ClassDefInfo* base_type = _dfr->getClassDefByName(classname);
		string base = base_type->getSuperClass();
		string base_field = base + "-" + fname;


		if(global_vars->count(base_field)){
			global_var = (*global_vars)[base_field];
		} else {
			string special_temp_name = "v"
					+ Int2Str(_dfr->getGlobalVariableReg());
			ClassDefInfo* classdef = _dfr->getClassDefByName(type_name);
			il::type* t = getType(0, _dfr, classdef, _type_map);
			global_var = new sail::Variable(special_temp_name, t, false, false,
					-1, true);
			(*global_vars)[field_name] = global_var;
		}
	}

	//now prepare the LHS(source register's variable)
	sail::Variable* reg_var = NULL; // the faked register
	assert(asm_ins->getRegs()->size() > 0);
	unsigned short reg = asm_ins->getRegs()->at(0);

	string reg_name = "v" + Int2Str(reg);
	string reg_var_name = reg_name;
	int line = -1;
	//ClassDefInfo* field_classdef = NULL;
	//il::type* reg_t = NULL;
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;

	//if possible update var_name
	if (instruction_debug != NULL) {
		if (instruction_debug->count(asm_ins) > 0) {
			debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1)
				asm_ins->getMthCodeH()->setLatestLine(line);
			rvt_list = debug->getRVTList();
		}
		if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
			for (unsigned int i = 0; i < rvt_list->size(); i++) {
				if (rvt_list->at(i)->getReg() == reg)
					reg_var_name = rvt_list->at(i)->getVarName();
			}
		}
	}
	if (line == -1)
		line = asm_ins->getMthCodeH()->getLatestLine();
	il::location lo(line, line);
	map<unsigned int, ClassDefInfo*>* ins_type_map = asm_ins->getTypeofVar();
	assert(ins_type_map->count(reg) > 0);
	ClassDefInfo* lhs_classdef = (*ins_type_map)[reg];
	il::type* lhs_t = getType(0, _dfr, lhs_classdef, _type_map);

	reg_var = asm_ins->getMthCodeH()->getRegVarEle(reg, lhs_t, _type_map, false,
			false); //in_use, check_cast for two bool value, we set in_def, because it is global, we don't care


	reg_var->set_alias_name(reg_var_name);

	il::string_const_exp* temp = new il::string_const_exp("", lhs_t, lo);

	sail::Assignment* inst = new sail::Assignment(reg_var, global_var, temp,
			line);
	//  sail::Store* inst = new sail::Store(field_var, reg_var, 0, "", temp,line);
	if (SAIL_INS_PRINT) {
		cout << "test global load pp::" << inst->to_string(true) << endl;
		cout << "test global load::" << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);

	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL) &&(!USING_TYPE_INFER)) //boyang will do it if USING_TYPE_INFER is true
		updateAnotherBranchReturnType(addr1, reg, lhs_t, reg_var);

}




/************************************************************************
 * add clinit_flag as the last parameter //Please do sth. on based on it
 ************************************************************************/
void InstructionMapping::store(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr1, bool clinit_flag) {
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr1);
	add_labels(asm_ins);

	//HZHU changed to retrive global variable;
	map<string, sail::Variable*>* global_vars = NULL;
	if (!USING_CLINIT)
		global_vars = _dfr->getGlobalVars();
	else
		global_vars = ins_debug->getGlobalVars();
	string field_name = asm_ins->getOperand2();
	StringSplit* ssp = new StringSplit(field_name, ":");
	vector<string>* sub_strs = ssp->getSubStrs();
	assert(sub_strs->size() == 2);
	field_name = sub_strs->at(0);
	string type_name = sub_strs->at(1); //use this to hint the ClassDefInfo*
	type_name = type_name.substr(1,type_name.size()-1);
	ClassDefInfo* cdi_hint_for_global_var = _dfr->getClassDefByName(type_name);
	il::type* il_hint_type_for_global_var = getType(0, _dfr, cdi_hint_for_global_var,
			_type_map);
	sail::Variable* global_var = NULL;
	il::type* lhs_t = NULL;
	//cout << "field_name " << field_name << endl;
	if ((USING_CLINIT) && (!(global_vars->count(field_name) > 0))) { //if sput is inside clinit method;
		//first build up the global variables 必须不是和library 相关的的静态域
		global_var = new sail::Variable(
				"v" + Int2Str(_dfr->getGlobalVariableReg()),
				il_hint_type_for_global_var, false, false, -1, true);
		(*global_vars)[field_name] = global_var; //set it to the global map
	} else { //two cases : in clinit methods or not
		assert(global_vars->count(field_name) > 0);
		global_var = (*global_vars)[field_name];
	}
	lhs_t = global_var->get_type();
	//now prepare the LHS(source register's variable)
	sail::Variable* reg_var = NULL; // the faked register
	assert(asm_ins->getRegs()->size() > 0);
	unsigned short reg = asm_ins->getRegs()->at(0);

	string reg_name = "v" + Int2Str(reg);
	string reg_var_name = reg_name;
	int line = -1;
	//ClassDefInfo* field_classdef = NULL;
	//il::type* reg_t = NULL;
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;

	//if possible update var_name
	if (instruction_debug != NULL) {
		if (instruction_debug->count(asm_ins) > 0) {
			debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1)
				asm_ins->getMthCodeH()->setLatestLine(line);
			rvt_list = debug->getRVTList();
		}
		if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
			for (unsigned int i = 0; i < rvt_list->size(); i++) {
				if (rvt_list->at(i)->getReg() == reg)
					reg_var_name = rvt_list->at(i)->getVarName();
			}
		}
	}
	if (line == -1)
		line = asm_ins->getMthCodeH()->getLatestLine();
	il::location lo(line, line);
	map<unsigned int, ClassDefInfo*>* ins_type_map = asm_ins->getTypeofVar();
	assert(ins_type_map->count(reg) > 0);
	ClassDefInfo* rhs_classdef = (*ins_type_map)[reg];
	il::type* rhs_t = getType(0, _dfr, rhs_classdef, _type_map);
	if (0) {
		cout << "rhs_t " << rhs_t->to_string() << endl;
		cout << "lhs_t " << lhs_t->to_string() << endl;
	}

	//assert(rhs_t == lhs_t);, we can not account on ins_var type now ,because there are a bunch of invalid_type
	reg_var = asm_ins->getMthCodeH()->getRegVarEle(reg, rhs_t, _type_map, true,
			false); //in_use, check_cast for two bool value


	reg_var->set_alias_name(reg_var_name);
	string ins_str = "";
	il::string_const_exp* temp = new il::string_const_exp(ins_str, lhs_t, lo);


	sail::Assignment* inst = new sail::Assignment(global_var, reg_var, temp,
			line);

	if (SAIL_INS_PRINT) {
		cout << "test global store pp::" << inst->to_string(true) << endl;
		cout << "test global store::" << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);

}

//1/5 add the last paraeter indicating the clinit method
void InstructionMapping::InstructionMapping_F21c(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr, bool clinit_flag) {

	DexOpcode opc = asm_ins->getOpcode();

	switch (opc) {
	case OP_CONST_STRING:
		InstructionMapping_constantStringAddr(ins_debug, instruction_debug,
				asm_ins, addr);
		break;
		//case OP_CONST_CLASS:
		//case OP_CONST_CLASS: InstructionMapping_constantStringAddr(ins_debug, instruction_debug, asm_ins,addr); break;
	case OP_CONST_CLASS:
		InstructionMapping_constClass(ins_debug, instruction_debug, asm_ins,
				addr);
		break;
	case OP_CHECK_CAST:
		cast(ins_debug, instruction_debug, asm_ins, addr, NULL);
		break;
	case OP_NEW_INSTANCE: // return an instance reference
	{
		New_functioncall(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	case OP_SGET:
	case OP_SGET_WIDE:
	case OP_SGET_OBJECT:
	case OP_SGET_BOOLEAN:
	case OP_SGET_BYTE:
	case OP_SGET_CHAR:
	case OP_SGET_SHORT: {
		load(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	case OP_SPUT:
	case OP_SPUT_WIDE:
	case OP_SPUT_OBJECT:
	case OP_SPUT_BOOLEAN:
	case OP_SPUT_BYTE:
	case OP_SPUT_CHAR:
	case OP_SPUT_SHORT: {
		//store(ins_debug, instruction_debug, asm_ins, addr);
		store(ins_debug, instruction_debug, asm_ins, addr,clinit_flag);
	}
		break;
	default:
		return;
	}
}

void InstructionMapping::InstructionMapping_F21h(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	InstructionMapping_constantAssignment(ins_debug, instruction_debug, asm_ins,
			addr);
}
void InstructionMapping::InstructionMapping_F21s(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	InstructionMapping_constantAssignment(ins_debug, instruction_debug, asm_ins,
			addr);
}

void InstructionMapping::branch(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) //for each instructon, you need to first build an binop instruction, an unop instrucion, then branch, then label, but the label instruction will be last until it hit the address of the label to push_back to the intruction vector
		{
	//cout << endl;
	if(0){
		if (asm_ins->is_label())
			cout << "sucks if! " << endl;
	}
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	//first need to process an binop instruction and push_back vaa == 0;
	//hzhu added 5.25
	//map<unsigned int, pair<bool, sail::Variable*>>* var_map = asm_ins->getMthCodeH()->getRegVar();
	//assert(var_map != NULL);
	//hzhu end 5.25

	il::binop_type bt = il::_EQ;
	DexOpcode opcode = asm_ins->getOpcode();
	bool flag = false;
	switch (opcode) {
	case OP_IF_EQZ:
		bt = il::_EQ;
		break;
	case OP_IF_EQ: {
		bt = il::_EQ;
		flag = true;
	}
		break;
	case OP_IF_NEZ:
		bt = il::_NEQ;
		break;
	case OP_IF_NE: {
		bt = il::_NEQ;
		flag = true;
	}
		break;
	case OP_IF_LTZ:
		bt = il::_LT;
		break;
	case OP_IF_LT: {
		bt = il::_LT;
		flag = true;
	}
		break;
	case OP_IF_GEZ:
		bt = il::_GEQ;
		break;
	case OP_IF_GE: {
		bt = il::_GEQ;
		flag = true;
	}
		break;
	case OP_IF_GTZ:
		bt = il::_GT;
		break;
	case OP_IF_GT: {
		bt = il::_GT;
		flag = true;
	}
		break;
	case OP_IF_LEZ:
		bt = il::_LEQ;
		break;
	case OP_IF_LE: {
		bt = il::_LEQ;
		flag = true;
	}
		break;
	default:
		break;
	}
	int line = -1;
	//left t1 = binop(op1,op2)
	//first build op1 the register inside the instruction
	assert(asm_ins->getRegs()->size()>0);
	unsigned short op1_reg = asm_ins->getRegs()->at(0);
	assert(asm_ins->getTypeofVar()->size()>0);
	assert(asm_ins->getTypeofVar()->count(op1_reg) >0);
	ClassDefInfo* op1_classdef = (*asm_ins->getTypeofVar())[op1_reg];
	//il::type* op1_t = TypeMapping(0,_dfr, op1_classdef,_type_map);
	il::type* op1_t = getType(0, _dfr, op1_classdef, _type_map);
	string op1_reg_name = "v" + Int2Str(op1_reg);
	string op1_var_name = "";
	unsigned short op2_reg = 0;
	string op2_reg_name = "";
	string op2_var_name = "";
	if (flag) {
		assert(asm_ins->getRegs()->size()>1);
		op2_reg = asm_ins->getRegs()->at(1);
		op2_reg_name = "v" + Int2Str(op2_reg);
	}
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;
	if (instruction_debug->count(asm_ins) > 0) {
		debug = (*instruction_debug)[asm_ins];
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1)
				asm_ins->getMthCodeH()->setLatestLine(line);
			rvt_list = debug->getRVTList();
		}
		if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
			for (unsigned int i = 0; i < rvt_list->size(); i++) {
				if (rvt_list->at(i)->getReg() == op1_reg) {
					op1_var_name = rvt_list->at(i)->getVarName();
					continue;
				}
				if ((flag) && rvt_list->at(i)->getReg() == op2_reg) {
					op2_var_name = rvt_list->at(i)->getVarName();
					continue;
				}

			}
		}

	}
	if (line == -1)
		line = asm_ins->getMthCodeH()->getLatestLine();
	il::location lo(line, line);
	il::type* bool_type = il::get_integer_type();
	il::string_const_exp* ope1_exp = new il::string_const_exp(op1_var_name,
			op1_t, lo);
	sail::Variable* op1_var = asm_ins->getMthCodeH()->getRegVarEle(op1_reg,
			op1_t, _type_map, true, false);

	op1_var->set_original(ope1_exp);
	if (op1_var_name != "")
		op1_var->set_alias_name(op1_var_name);
	else
		op1_var->set_alias_name(op1_reg_name);
	//op1_var->set_type(op1_t);

	sail::Variable* op2_var = NULL;

	sail::Constant* const_zero = NULL;
	if (!flag) {
		const_zero = new sail::Constant(0, true, 32);
	} else {
		il::string_const_exp* ope2_exp = new il::string_const_exp(op2_var_name,
				op1_t, lo); // same type as op1
		//op2_var = new sail::Variable(ope2_exp, op2_reg_name, op1_t);
		op2_var = asm_ins->getMthCodeH()->getRegVarEle(op2_reg, op1_t,
				_type_map, true, false);
		op2_var->set_original(ope2_exp);
		//cout << "op2_var_name " << op2_var_name << endl;
		if (op2_var_name != "")
			op2_var->set_alias_name(op2_var_name);
		else
			op2_var->set_alias_name(op2_reg_name);
		//op2_var->set_type(op1_t);
	}
	//il::string_const_exp* con_1_exp = new il::string_const_exp("t1", bool_type,lo);

	//sail::Variable * con_1_var = new sail::Variable(con_1_exp, "t1", bool_type);
	//hzhu added 5.27
	//destination
	//sail::Variable* con_1_var = asm_ins->getMthCodeH()->getRegVarEle(getRandom(), _type_map); //we use 1000 to fake the unused register;
	sail::Variable* con_1_var = asm_ins->getMthCodeH()->getRegVarEle(
			getRandom(), bool_type, _type_map, false, false); //we use 1000 to fake the unused register;
	//con_1_var->set_original(con_1_exp);
	// cout << "Test get_var_name " << con_1_var->get_var_name()<<endl;
	con_1_var->set_alias_name(con_1_var->get_var_name());
	//con_1_var->set_type(bool_type);
	//hzhu ended 5.27

	string binop_str = "";
	il::string_const_exp* temp = NULL;

	sail::Binop* ins_binop = NULL;
	if (flag) {
		binop_str = con_1_var->to_string(true) + "=" + op1_var->to_string(true)
				+ il::binop_expression::binop_to_string(bt)
				+ op2_var->to_string(true);
		temp = new il::string_const_exp(binop_str, bool_type, lo);
		ins_binop = new sail::Binop(con_1_var, op1_var, op2_var, bt, temp,
				line);
	} else {
		binop_str = con_1_var->to_string(true) + "=" + op1_var->to_string(true)
				+ il::binop_expression::binop_to_string(bt)
				+ const_zero->to_string(true);
		temp = new il::string_const_exp(binop_str, bool_type, lo);
		ins_binop = new sail::Binop(con_1_var, op1_var, const_zero, bt, temp,
				line);
	}
	if (SAIL_INS_PRINT) {
		cout << "test branch_binop pp :: " << ins_binop->to_string(true)
				<< endl;
		cout << "test branch_binop :: " << ins_binop->to_string(false) << endl;
	}
	_sail_ins->push_back(ins_binop);

	//now build an unop variable
	string else_var_name = "!(" + con_1_var->to_string(true) + ")";
	//il::string_const_exp* else_exp = new il::string_const_exp(else_var_name,bool_type,lo);
	//sail::Variable* else_var = new sail::Variable(else_exp, "t2", bool_type);
	//hzhu added 5.27
	//destination

	//sail::Variable* else_var = asm_ins->getMthCodeH()->getRegVarEle(getRandom(), _type_map); //we use randon register to fake the unused register;
	sail::Variable* else_var = asm_ins->getMthCodeH()->getRegVarEle(getRandom(),
			bool_type, _type_map, false, false); //we use randon register to fake the unused register;
	//else_var->set_original(else_exp);
	//if(else_var_name != "")
	//	else_var->set_alias_name(else_var_name);
	//else
	//cout << "ele_var->get_var_name()" << ele_var->get_var_name()<<endl;
	else_var->set_alias_name(else_var->get_var_name());
	//else_var->set_type(bool_type);
	//hzhu ended 5.27
	string unop_str = else_var->to_string() + " != " + con_1_var->to_string();
	il::string_const_exp* unop_exp = new il::string_const_exp(unop_str,
			bool_type, lo);
	//cout << "check line: " << line << endl;
	sail::Unop* ins_unop = new sail::Unop(else_var, con_1_var, il::_LOGICAL_NOT,
			unop_exp, line);
	if (SAIL_INS_PRINT) {
		cout << "test branch_unop pp :: " << ins_unop->to_string(true) << endl;
		cout << "test branch_unop :: " << ins_unop->to_string(false) << endl;
	}
	_sail_ins->push_back(ins_unop);

	//processing branch instruction
	string operd = asm_ins->getOperand2();
	if (flag)
		operd = asm_ins->getOperand3();
	//unsigned int operd_addr = atoi(operd.substr(1, operd.size()-1).c_str());
	//cout << "operd2 :: " << operd2 <<endl;
	string then_label_str = "con_" + operd.substr(1, operd.size() - 1);
	//cout << "con_label :: " << then_label_str <<endl;

	unsigned int nextaddr = addr + 2 * 2;
	string else_label_str = "con_" + Int2Str(nextaddr);
	//cout << "else_label ::" << else_label_str << endl;

	//sail::Label* then_label = new sail::Label(then_label_str, NULL);
	sail::Label* then_label = get_label(then_label_str);
	//(*_label_list)[operd_addr] = then_label; //the jump to label

	//sail::Label* else_label = new sail::Label(else_label_str,NULL);
	sail::Label* else_label = get_label(else_label_str);
	//(*_label_list)[nextaddr] = else_label; //the address of next intruction

	string ins_branch_str = "if(" + con_1_var->to_string(true) + ")";
	ins_branch_str += "then goto" + then_label->to_string(true);
	ins_branch_str += "else goto" + else_label->to_string(true);
	//il::string_const_exp* original = new il::string_const_exp(ins_branch_str, il::base_type::make("bool",8,8,false, il::BOOL, "BOOLEAN"), lo); // I have no idea about the type of the branch's string_const_exp, bool_type just for test
	//il::string_const_exp* original = new il::string_const_exp(ins_branch_str, il::get_integer_type(), lo); // I have no idea about the type of the branch's string_const_exp, bool_type just for test

	//sail::Label* then_jump_label = new sail::Label("jump_to_for"+ con_1_var->to_string(true), NULL);
	sail::Label* then_jump_label = get_label(
			"jump_to_for" + con_1_var->to_string(true));

	sail::Branch* ins_branch = new sail::Branch(con_1_var, else_var,
			then_jump_label, else_label, NULL);
	if (SAIL_INS_PRINT) {
		cout << "test branch_branch pp :: " << ins_branch->to_string(true)
				<< endl;
		cout << "test branch_branch :: " << ins_branch->to_string(false)
				<< endl;
	}
	_sail_ins->push_back(ins_branch);

	_sail_ins->push_back(then_jump_label);
	if (SAIL_INS_PRINT) {
		cout << "test branch_jump_label pp :: "
				<< then_jump_label->to_string(true) << endl;
		cout << "test branch_jump_label :: "
				<< then_jump_label->to_string(false) << endl;
	}
	sail::Jump* ins_jump = new sail::Jump(then_label, NULL);
	if (SAIL_INS_PRINT) {
		cout << "test branch_jump pp :: " << ins_jump->to_string(true) << endl;
		cout << "test branch_jump :: " << ins_jump->to_string(false) << endl;
	}
	_sail_ins->push_back(ins_jump);
	_sail_ins->push_back(else_label);
	if (SAIL_INS_PRINT) {
		cout << "test branch_jump else_label pp :: "
				<< else_label->to_string(true) << endl;
		cout << "test branch_jump else_label:: " << else_label->to_string(false)
				<< endl;
	}

}
void InstructionMapping::InstructionMapping_F21t(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	switch (opcode) {
	case OP_IF_EQZ:
	case OP_IF_NEZ:
	case OP_IF_LTZ:
	case OP_IF_GEZ:
	case OP_IF_GTZ:
	case OP_IF_LEZ: {
		branch(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	default:
		return;
	}
}

void InstructionMapping::InstructionMapping_F22b(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	switch (opcode) {
	case OP_ADD_INT_LIT8: {
		il::binop_type bt = il::_PLUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_RSUB_INT_LIT8: {
		il::binop_type bt = il::_MINUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_MUL_INT_LIT8: {
		il::binop_type bt = il::_MULTIPLY;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_DIV_INT_LIT8: {
		il::binop_type bt = il::_DIV;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_REM_INT_LIT8: {
		il::binop_type bt = il::_MOD;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_AND_INT_LIT8: {
		il::binop_type bt = il::_BITWISE_AND;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_OR_INT_LIT8: {
		il::binop_type bt = il::_BITWISE_OR;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_XOR_INT_LIT8: {
		il::binop_type bt = il::_BITWISE_XOR;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_SHL_INT_LIT8: {
		il::binop_type bt = il::_LEFT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_SHR_INT_LIT8: {
		il::binop_type bt = il::_RIGHT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_USHR_INT_LIT8: {
		il::binop_type bt = il::_RIGHT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	default:
		return;
	}
}

void InstructionMapping::InstanceOf(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	//InstanceOf(Variable* v, Symbol* s, il::type* t, il::node* original, int line);
	assert(asm_ins->getRegs()->size() == 2);
	unsigned int lhs_reg = asm_ins->getRegs()->at(0);
	unsigned int rhs_reg = asm_ins->getRegs()->at(1);
	string lhs_reg_name = "v" + Int2Str(lhs_reg);
	string rhs_reg_name = "v" + Int2Str(rhs_reg);
	string lhs_var_name = lhs_reg_name;
	string rhs_var_name = rhs_reg_name;
	ClassDefInfo* lhs_classdef = NULL;
	ClassDefInfo* rhs_classdef = NULL;
	map<unsigned int, ClassDefInfo*>* ins_type_map = asm_ins->getTypeofVar();
	assert(ins_type_map->count(lhs_reg) > 0);
	lhs_classdef = (*ins_type_map)[lhs_reg];
	if (ins_type_map->count(rhs_reg) > 0) {
		rhs_classdef = (*ins_type_map)[rhs_reg];
	} else { //duplicated register
		assert(ins_type_map->count(2000) > 0);
		rhs_classdef = (*ins_type_map)[2000];
	}
	il::type* lhs_t = getType(0, _dfr, lhs_classdef, _type_map);
	il::type* rhs_t = getType(0, _dfr, rhs_classdef, _type_map);

	int line = -1;
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;
	if (instruction_debug != NULL) {
		if (instruction_debug->count(asm_ins) > 0)
			debug = (*instruction_debug)[asm_ins];
	}
	if (debug != NULL) {
		line = debug->getLine();
		if (line != -1)
			asm_ins->getMthCodeH()->setLatestLine(line);
		rvt_list = debug->getRVTList();
	}
	if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
		for (unsigned int i = 0; i < rvt_list->size(); i++) {
			if (rvt_list->at(i)->getReg() == lhs_reg)
				lhs_var_name = rvt_list->at(i)->getVarName();
			if (rvt_list->at(i)->getReg() == rhs_reg)
				rhs_var_name = rvt_list->at(i)->getVarName();
		}
	}

	if (line == -1)
		line = asm_ins->getMthCodeH()->getLatestLine();
	il::location lo(line, line);
	sail::Variable* lhs_var = asm_ins->getMthCodeH()->getRegVarEle(
			lhs_reg, lhs_t, _type_map, false, false);
	lhs_var->set_alias_name(lhs_var_name);
	sail::Variable* rhs_var = asm_ins->getMthCodeH()->getRegVarEle(rhs_reg, rhs_t,
			_type_map, true, false);
	rhs_var->set_alias_name(rhs_var_name);

	ClassDefInfo* check_type_classdef = NULL;
	string check_type_str = asm_ins->getOperand3();
	if(!GLOBAL_MAP){
	if(_dfr->insideClassDef(check_type_str))
		check_type_classdef = _dfr->getClassDef(check_type_str);
	else
		check_type_classdef = new ClassDefInfo(check_type_str);
	}else
		check_type_classdef = _dfr->getClassDefByName(check_type_str);
	//12/20
	unsigned int typeidx = _dfr->getTypeId(check_type_str);
	check_type_classdef->setTypeIdx(typeidx);
	//12/20
	il::type* check_type = getType(0,_dfr, check_type_classdef, _type_map);

	il::string_const_exp* original = new il::string_const_exp("", lhs_t, lo);
	sail::InstanceOf* instance_of = new sail::InstanceOf(lhs_var, rhs_var, check_type,
			original, line);
	_sail_ins->push_back(instance_of);

	if (SAIL_INS_PRINT) {
			cout << "test instance-of ins pp :: "
					<< instance_of->to_string(true) << endl;
			cout << "test instance-of :: " << instance_of->to_string(false)
					<< endl;
	}

	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL) &&(!USING_TYPE_INFER)) //boyang will do it if USING_TYPE_INFER is true
			updateAnotherBranchReturnType(addr, lhs_reg, lhs_t, lhs_var);
}

void InstructionMapping::InstructionMapping_F22c(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	switch (opcode) {
	//case OP_INSTANCE_OF: and case OP_NEW_ARRAY: belong to the new function call
	case OP_INSTANCE_OF: //
	{
		//add_labels(asm_ins);
		InstanceOf(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	case OP_NEW_ARRAY: //23 new an array which has a continous memory in heap
	{
		New_functioncall(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	case OP_IGET: //52
	case OP_IGET_WIDE:
	case OP_IGET_OBJECT:
	case OP_IGET_BOOLEAN:
	case OP_IGET_BYTE:
	case OP_IGET_CHAR:
	case OP_IGET_SHORT: //58
	{
		loadField(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	case OP_IPUT:
	case OP_IPUT_WIDE:
	case OP_IPUT_OBJECT:
	case OP_IPUT_BOOLEAN:
	case OP_IPUT_BYTE:
	case OP_IPUT_CHAR:
	case OP_IPUT_SHORT: {
		storeField(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	default:
		return;
	}
}

void InstructionMapping::InstructionMapping_F22s(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	switch (opcode) {
	case OP_ADD_INT_LIT16: {
		il::binop_type bt = il::_PLUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_RSUB_INT: {
		il::binop_type bt = il::_MINUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_MUL_INT_LIT16: {
		il::binop_type bt = il::_MULTIPLY;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_DIV_INT_LIT16: {
		il::binop_type bt = il::_DIV;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_REM_INT_LIT16: {
		il::binop_type bt = il::_MOD;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_AND_INT_LIT16: {
		il::binop_type bt = il::_BITWISE_AND;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_OR_INT_LIT16: {
		il::binop_type bt = il::_BITWISE_OR;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	case OP_XOR_INT_LIT16: {
		il::binop_type bt = il::_BITWISE_XOR;
		binop(ins_debug, instruction_debug, asm_ins, bt, false, true, addr);
	}
		break;
	default:
		return;
	}
}

void InstructionMapping::InstructionMapping_F22t(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	switch (opcode) {
	case OP_IF_EQ:
	case OP_IF_NE:
	case OP_IF_LT:
	case OP_IF_GE:
	case OP_IF_GT:
	case OP_IF_LE: {
		branch(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	default:
		return;
	}
}
void InstructionMapping::InstructionMapping_F22x(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	InstructionMapping_varAssignment(ins_debug, instruction_debug, asm_ins,
			addr);
}

void InstructionMapping::binop(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, il::binop_type bt, bool reg_flag,
		bool const_flag, unsigned int addr) {
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	vector<unsigned short>* regs = asm_ins->getRegs();
	vector<unsigned short>::iterator itr = regs->begin();
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();

	//do not have the instruction and debuginformation mapping for this method
	if (instruction_debug == NULL) {
		if (SCRATCH_INF)
			cout << "instruction_debug mapping is NULL and exit";
	}

	DebugInfo* debug = NULL;

	unsigned short regl = *itr; //1st
	unsigned short regr1 = *(itr + 1); //2nd

	//3rd may not exist this number of register will never exists
	unsigned short regr2 = 9000;
	string var_namer2 = "";
	string reg_namer2;

	ClassDefInfo* classdef = ins_var_type->begin()->second;

	il::type* t = getType(0, _dfr, classdef, _type_map);
	if (instruction_debug->count(asm_ins) > 0) {
		//cout << "Found! " << endl;
		debug = (*instruction_debug)[asm_ins];
	}
	int startline = -1;
	//1st
	string var_namel = "";
	string reg_namel = "v" + Int2Str(regl);
	//2nd
	string var_namer1 = "";
	string reg_namer1 = "v" + Int2Str(regr1); //default not for 2addr

	DexOpcode opcode = asm_ins->getOpcode();
	switch (opcode) {
	case OP_ADD_INT:
	case OP_SUB_INT:
	case OP_MUL_INT:
	case OP_DIV_INT:
	case OP_REM_INT:
	case OP_AND_INT:
	case OP_OR_INT:
	case OP_XOR_INT:
	case OP_SHL_INT:
	case OP_SHR_INT:
	case OP_USHR_INT:
	case OP_ADD_LONG:
	case OP_SUB_LONG:
	case OP_MUL_LONG:
	case OP_DIV_LONG:
	case OP_REM_LONG:
	case OP_AND_LONG:
	case OP_OR_LONG:
	case OP_XOR_LONG:
	case OP_SHL_LONG:
	case OP_SHR_LONG:
	case OP_USHR_LONG:
	case OP_ADD_FLOAT:
	case OP_SUB_FLOAT:
	case OP_MUL_FLOAT:
	case OP_DIV_FLOAT:
	case OP_REM_FLOAT:
	case OP_ADD_DOUBLE:
	case OP_SUB_DOUBLE:
	case OP_MUL_DOUBLE:
	case OP_DIV_DOUBLE:
	case OP_REM_DOUBLE: {
		regr2 = *(itr + 2); //3rd
		reg_namer2 = "v" + Int2Str(regr2);
	}
		break;
	case OP_ADD_INT_LIT16: //22s //8
	case OP_RSUB_INT:
	case OP_MUL_INT_LIT16:
	case OP_DIV_INT_LIT16:
	case OP_REM_INT_LIT16:
	case OP_AND_INT_LIT16:
	case OP_OR_INT_LIT16:
	case OP_XOR_INT_LIT16:
	case OP_ADD_INT_LIT8: //22b //11
	case OP_RSUB_INT_LIT8:
	case OP_MUL_INT_LIT8:
	case OP_DIV_INT_LIT8:
	case OP_REM_INT_LIT8:
	case OP_AND_INT_LIT8:
	case OP_OR_INT_LIT8:
	case OP_XOR_INT_LIT8:
	case OP_SHL_INT_LIT8:
	case OP_SHR_INT_LIT8:
	case OP_USHR_INT_LIT8:
		break;
	default: { //2addr
		regr2 = regr1;
		regr1 = regl;
		reg_namer1 = "v" + Int2Str(regr1); //the same as 1st
		reg_namer2 = "v" + Int2Str(regr2); //3rd reg equals to 2nd
	}
	}

	if (debug == NULL) {
		if (SCRATCH_INF)
			cout << "debug information is NULL " << endl;
	} else {
		startline = debug->getLine();
		if (startline != -1)
			asm_ins->getMthCodeH()->setLatestLine(startline);
		//cout << "line: " << startline << endl;
		vector<RVT*>* rvt_list = debug->getRVTList();
		if (rvt_list == NULL) {
			if (SCRATCH_INF)
				cout << "rvt list is empty" << endl;
		} else {
			vector<RVT*>::iterator it = rvt_list->begin();
			for (; it != rvt_list->end(); it++) {
				unsigned int reg = (*it)->getReg(); //for each register in the rvt list;
				if (reg_flag) //if it has 3 registers including that two of them are the same
				{
					if (reg == regl)
						var_namel = (*it)->getVarName();
					if (reg == regr1)
						var_namer1 = (*it)->getVarName();
					if (reg == regr2)
						var_namer2 = (*it)->getVarName();
				}
				if (const_flag) //3rd is an constant, so just need to process 2 registers
				{
					if (reg == regl)
						var_namel = (*it)->getVarName();
					if (reg == regr1)
						var_namer1 = (*it)->getVarName();
				} else {
					if (reg == regl) {
						var_namel = (*it)->getVarName();
						var_namer1 = (*it)->getVarName();
					}
					if (reg == regr2)
						var_namer2 = (*it)->getVarName();
				}

			}
		}
	}
	if (startline == -1)
		startline = asm_ins->getMthCodeH()->getLatestLine();

	//prepare done, then construct the instruction with variable or symbol
	//left is a variable(type)va
	//prepare for the constructor;

	il::location lo(startline, startline); //location
	//binop_expression(expression* exp1, expression* exp2, binop_type op);
//	il::binop_expression bin = 

	//left side
	//il::string_const_exp* originall = new il::string_const_exp(var_namel,t,lo);
	//sail::Variable* sl = new sail::Variable(originall, reg_namel, t);

	//right side op1+ op2
	//il::string_const_exp* op1_exp = new il::string_const_exp(var_namer1,t,lo);
	//sail::Variable* op1 = new sail::Variable(op1_exp, reg_namer1,t);
	//sail::Variable* op1 = asm_ins->getMthCodeH()->getRegVarEle(regr1, _type_map);
	////1/8
	ClassDefInfo* op1_cdi = (*ins_var_type)[regr1];
	il::type* op1_t = getType(0, _dfr, op1_cdi, _type_map);
	sail::Variable* op1 = asm_ins->getMthCodeH()->getRegVarEle(regr1, op1_t,
			_type_map, true, false);
	//op1->set_original(op1_exp);
	if (var_namer1 != "")
		op1->set_alias_name(var_namer1);
	else
		op1->set_alias_name(reg_namer1);
	il::string_const_exp* op1_exp = new il::string_const_exp(var_namer1, t, lo);
	op1->set_original(op1_exp);
	//op1->set_type(t);

	il::string_const_exp* op2_exp = NULL;
	sail::Variable* op2 = NULL;
	sail::Constant* op2_c = NULL;
	string ins_str = "";
	/////////////////////////
	//1/8
	ClassDefInfo* op2_cdi = NULL;
	il::type* op2_t = NULL;
	if (!const_flag) {
		//1/8
		op2_cdi = (*ins_var_type)[regr2];
		op2_t = getType(0, _dfr, op2_cdi, _type_map);

		//op2_exp = new il::string_const_exp(var_namer2, t, lo);
		//op2_exp = new il::string_const_exp(var_namer2, op2_t, lo);
		//op2 = new sail::Variable(op2_exp, reg_namer2, t);
		//op2 = asm_ins->getMthCodeH()->getRegVarEle(regr2, _type_map);
		op2 = asm_ins->getMthCodeH()->getRegVarEle(regr2, op2_t, _type_map,
				true, false);
		//op2->set_original(op2_exp);
		if (var_namer2 != "")
			op2->set_alias_name(var_namer2);
		else
			op2->set_alias_name(reg_namer2);
		op2_exp = new il::string_const_exp(var_namer2, t, lo);
		//	op2->set_type(t);
		//ins_str = sl->to_string(true) +  "=" + op1->to_string(true) + il::binop_expression::binop_to_string(bt) + op2->to_string(true);
	} else //3rd is an constant
	{
		string operd2 = asm_ins->getOperand3();
		long constant = atoi((operd2.substr(2, operd2.size() - 2)).c_str());
		op2_c = new sail::Constant(constant, true, 32);
		//ins_str = sl->to_string(true) +  "=" + op1->to_string(true) + il::binop_expression::binop_to_string(bt) + op2_c->to_string(true);
	}
	//construct the il::Instruction which is binop
	//string ins_str = sl->to_string(true) +  "=" + op1->to_string(true) + il::binop_expression::binop_to_string(bt) + op2->to_string(true);
	//left side
	il::string_const_exp* originall = new il::string_const_exp(var_namel, t,
			lo);
	//sail::Variable* sl = new sail::Variable(originall, reg_namel, t);
	//destination
	sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(regl, t,
			_type_map, false, false);
	//sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(regl, _type_map);
	//cout << "regl::" << regl <<endl;
	//update
	sl->set_original(originall);
	if (var_namel != "")
		sl->set_alias_name(var_namel);
	else
		sl->set_alias_name(reg_namel);
	//sl->set_type(t);

	if (!const_flag)
		ins_str = sl->to_string(true) + "=" + op1->to_string(true)
				+ il::binop_expression::binop_to_string(bt)
				+ op2->to_string(true);
	else
		ins_str = sl->to_string(true) + "=" + op1->to_string(true)
				+ il::binop_expression::binop_to_string(bt)
				+ op2_c->to_string(true);

	//for print
	il::string_const_exp* temp = new il::string_const_exp(ins_str, t, lo);
	sail::Binop* inst = NULL;
	if (!const_flag) // two possible way that, two register in use
	{
		//HZHU add invariant
		//cout << "op1 " << op1->get_type()->to_string() << endl;
		processInvariant(sl, op1, NULL, startline);
		assert( _tempvar_for_invariant != NULL);
		//cout << "_tempvar_for_invariant " << _tempvar_for_invariant->to_string()
		//<< endl;
		sail::Variable* op1_temp = _tempvar_for_invariant;
		////////////////////////////////////////////////////////
		//cout << "op2 " << op2->get_type()->to_string() << endl;
		processInvariant(sl, op2, NULL, startline);
		assert( _tempvar_for_invariant != NULL);
		//cout << "_tempvar_for_invariant " << _tempvar_for_invariant->to_string()
		//<< endl;
		sail::Variable* op2_temp = _tempvar_for_invariant;
		//HZHU end invariant

		//inst = new sail::Binop(sl, op1, op2, bt, temp, startline);
		inst = new sail::Binop(sl, op1_temp, op2_temp, bt, temp, startline);
		//cout << " HHZHU :: op1_temp  type " << op1_temp->get_type()->to_string() << endl;
		//cout << " HHZHU :: op1_temp type" << op2_temp->get_type()->to_string() << endl;
	} else {
		//HZHU add invariant
		processInvariant(sl, op1, NULL, startline);
		assert( _tempvar_for_invariant != NULL);
		//cout << "_tempvar_for_invariant " << _tempvar_for_invariant->to_string()
		//<< endl;

		//HZHU end invariant
		//inst = new sail::Binop(sl, op1, op2_c, bt, temp, startline);
		inst = new sail::Binop(sl, _tempvar_for_invariant, op2_c, bt, temp,
				startline);

	}
	assert(inst != NULL);
	if (SAIL_INS_PRINT) {
		cout << "test binop pp :::" << inst->to_string(true) << endl;
		cout << "test binop :::" << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);

	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)
			&& (!USING_TYPE_INFER)) //boyang will do it if USING_TYPE_INFER is true
		updateAnotherBranchReturnType(addr, regl, t, sl);
}

void InstructionMapping::cmp_branch(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	//first need to process an binop instruction and push_back vaa == 0;
	il::binop_type bt = il::_EQ;
	il::binop_type btl = il::_LT;
	il::binop_type btg = il::_GT;
	//il::type* binop_t = NULL;
	il::type* des_t = NULL;
	il::type* op1_t = NULL;
	il::type* op2_t = NULL;
	//DexOpcode opcode = asm_ins->getOpcode();

	int line = -1;
	//left t1 = binop(op1,op2)
	//first build op1 the register inside the instruction
	assert(asm_ins->getRegs()->size()>2);
	unsigned short des_reg = asm_ins->getRegs()->at(0);
	unsigned short op1_reg = asm_ins->getRegs()->at(1);
	unsigned short op2_reg = asm_ins->getRegs()->at(2);
	string des_reg_name = "v" + Int2Str(des_reg);
	string des_var_name = des_reg_name;
	string op1_reg_name = "v" + Int2Str(op1_reg);
	string op1_var_name = op1_reg_name;
	string op2_reg_name = "v" + Int2Str(op2_reg);
	string op2_var_name = op2_reg_name;
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;

	map<unsigned int, ClassDefInfo*>* var_type = asm_ins->getTypeofVar();
	ClassDefInfo* des_cdf = (*var_type)[des_reg];
	des_t = getType(0, _dfr, des_cdf, _type_map);
	ClassDefInfo* op1_cdf = NULL;
	ClassDefInfo* op2_cdf = NULL;
	if (des_reg != op1_reg)
		op1_cdf = (*var_type)[op1_reg];
	else
		op1_cdf = (*var_type)[2000];
	op1_t = getType(0, _dfr, op1_cdf, _type_map);
	if (des_reg != op2_reg)
		op2_cdf = (*var_type)[op2_reg];
	else
		op2_cdf = (*var_type)[2000];
	op2_t = getType(0, _dfr, op2_cdf, _type_map);

	if (instruction_debug->count(asm_ins) > 0) {
		debug = (*instruction_debug)[asm_ins];
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1)
				asm_ins->getMthCodeH()->setLatestLine(line);
			rvt_list = debug->getRVTList();
		}
		if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
			for (unsigned int i = 0; i < rvt_list->size(); i++) {
				if (rvt_list->at(i)->getReg() == op1_reg)
					op1_var_name = rvt_list->at(i)->getVarName();
				if (rvt_list->at(i)->getReg() == op2_reg)
					op2_var_name = rvt_list->at(i)->getVarName();
				if (rvt_list->at(i)->getReg() == des_reg)
					des_var_name = rvt_list->at(i)->getVarName();
			}
		}
	}
	if (line == -1)
		line = asm_ins->getMthCodeH()->getLatestLine();
	il::location lo(line, line);
	il::string_const_exp* ope1_exp = new il::string_const_exp(op1_var_name,
			op1_t, lo);

	sail::Variable* op1_var = asm_ins->getMthCodeH()->getRegVarEle(op1_reg,
			op1_t, _type_map, true, false);
	op1_var->set_original(ope1_exp);
	op1_var->set_alias_name(op1_var_name);

	il::string_const_exp* ope2_exp = new il::string_const_exp(op2_var_name,
			op2_t, lo);

	sail::Variable* op2_var = asm_ins->getMthCodeH()->getRegVarEle(op2_reg,
			op2_t, _type_map, true, false);

	op2_var->set_alias_name(op2_var_name);
	op2_var->set_original(ope2_exp);

	il::type* bool_type = il::get_integer_type();
	il::string_const_exp* return_exp_eq = new il::string_const_exp("t1",
			bool_type, lo);
	il::string_const_exp* return_exp_lt = new il::string_const_exp("t2",
			bool_type, lo);
	il::string_const_exp* return_exp_gt = new il::string_const_exp("t3",
			bool_type, lo);
	//sail::Variable * return_var = new sail::Variable(return_exp, "t1", bool_type);
	int randon_eq = getRandom();
	int randon_lt = getRandom();
	int randon_gt = getRandom();
	//Destination
	//sail::Variable* return_var = asm_ins->getMthCodeH()->getRegVarEle(randon,_type_map);
	sail::Variable* return_var_eq = asm_ins->getMthCodeH()->getRegVarEle(
			randon_eq, bool_type, _type_map, false, false);
	return_var_eq->set_original(return_exp_eq);
	return_var_eq->set_alias_name("v" + Int2Str(randon_eq));

	sail::Variable* return_var_lt = asm_ins->getMthCodeH()->getRegVarEle(
			randon_lt, bool_type, _type_map, false, false);
	return_var_lt->set_original(return_exp_lt);
	return_var_lt->set_alias_name("v" + Int2Str(randon_lt));

	sail::Variable* return_var_gt = asm_ins->getMthCodeH()->getRegVarEle(
			randon_gt, bool_type, _type_map, false, false);
	return_var_gt->set_original(return_exp_gt);
	return_var_gt->set_alias_name("v" + Int2Str(randon_gt));
	//return_var->set_type(bool_type);

	/*
	 string binop_str_eq = return_var_eq->to_string(true) + "="
	 + op1_var->to_string(true)
	 + il::binop_expression::binop_to_string(bt)
	 + op2_var->to_string(true);
	 string binop_str_lt = return_var_lt->to_string(true) + "="
	 + op1_var->to_string(true)
	 + il::binop_expression::binop_to_string(btl)
	 + op2_var->to_string(true);
	 string binop_str_gt = return_var_gt->to_string(true) + "="
	 + op1_var->to_string(true)
	 + il::binop_expression::binop_to_string(btg)
	 + op2_var->to_string(true);*/

	il::string_const_exp* temp_eq = new il::string_const_exp(/*binop_str_eq*/"",
			bool_type, lo);
	il::string_const_exp* temp_lt = new il::string_const_exp(/*binop_str_lt*/"",
			bool_type, lo);
	il::string_const_exp* temp_gt = new il::string_const_exp(/*binop_str_gt*/"",
			bool_type, lo);

	sail::Binop* ins_binop_eq = new sail::Binop(return_var_eq, op1_var, op2_var,
			bt, temp_eq, line);
	sail::Binop* ins_binop_lt = new sail::Binop(return_var_lt, op1_var, op2_var,
			btl, temp_lt, line);
	sail::Binop* ins_binop_gt = new sail::Binop(return_var_gt, op1_var, op2_var,
			btg, temp_gt, line);
	if (SAIL_INS_PRINT) {
		cout << "test cmp_binop pp :: " << ins_binop_eq->to_string(true)
				<< endl;
		cout << "test cmp_binop :: " << ins_binop_eq->to_string(false) << endl;
		cout << "test cmp_binop pp :: " << ins_binop_lt->to_string(true)
				<< endl;
		cout << "test cmp_binop :: " << ins_binop_lt->to_string(false) << endl;
		cout << "test cmp_binop pp :: " << ins_binop_gt->to_string(true)
				<< endl;
		cout << "test cmp_binop :: " << ins_binop_gt->to_string(false) << endl;
	}
	_sail_ins->push_back(ins_binop_eq);
	_sail_ins->push_back(ins_binop_lt);
	_sail_ins->push_back(ins_binop_gt);

	string str_label_eq = "eq_compare_in_" + Int2Str(addr);
	string str_label_lt = "lt_compare_in_" + Int2Str(addr);
	string str_label_gt = "gt_compare_in_" + Int2Str(addr);
	sail::Label* eq_label = get_label(str_label_eq);
	sail::Label* lt_label = get_label(str_label_lt);
	sail::Label* gt_label = get_label(str_label_gt);
	if (SAIL_INS_PRINT) {
		cout << "test cmp_label pp :: " << eq_label->to_string(true) << endl;
		cout << "test cmp_label :: " << eq_label->to_string(false) << endl;
		cout << "test cmp_label pp :: " << lt_label->to_string(true) << endl;
		cout << "test cmp_label :: " << lt_label->to_string(false) << endl;
		cout << "test cmp_label pp :: " << gt_label->to_string(true) << endl;
		cout << "test cmp_label :: " << gt_label->to_string(false) << endl;
	}

	//constructor an assignment instruction

	unsigned int nextaddr = addr + 2 * 2;
	string nextins_label_str = "con_cmp_" + Int2Str(nextaddr);
	//sail::Label* else_label = new sail::Label(else_label_str, NULL);
	sail::Label* next_ins_label = get_label(nextins_label_str);
	if (SAIL_INS_PRINT) {
		cout << "test cmp_label pp :: " << next_ins_label->to_string(true)
				<< endl;
		cout << "test cmp_label :: " << next_ins_label->to_string(false)
				<< endl;
	}
	//(*_label_list)[nextaddr] = else_label;

	//now build branch instruction
	////
	/*
	 string ins_branch_str = "if(" + return_var->to_string(true) + ")";
	 if(SAIL_INS_PRINT)
	 {
	 ins_branch_str += "then goto" + then_label->to_string(true);
	 ins_branch_str += "else goto"+ else_label->to_string(true);
	 }
	 //il::string_const_exp* original = new il::string_const_exp(ins_branch_str, il::get_integer_type(), lo); // I have no idea about the type of the branch's string_const_exp, bool_type just for test
	 */
	//prepare the vector of target
	vector<pair<sail::Symbol*, sail::Label*>>* target = new vector<
			pair<sail::Symbol*, sail::Label*>>();
	//fill target

	pair<sail::Symbol*, sail::Label*> p1(return_var_eq, eq_label);
	target->push_back(p1);
	pair<sail::Symbol*, sail::Label*> p2(return_var_lt, lt_label);
	target->push_back(p2);
	pair<sail::Symbol*, sail::Label*> p3(return_var_gt, gt_label);
	target->push_back(p3);

	//sail::Branch* ins_branch = new sail::Branch(return_var, else_var, then_label, else_label, NULL);
	sail::Branch* ins_branch = new sail::Branch(target, NULL);
	if (SAIL_INS_PRINT) {
		cout << "test branch_branch pp :: " << ins_branch->to_string(true)
				<< endl;
		cout << "test branch_branch :: " << ins_branch->to_string(false)
				<< endl;
	}
	_sail_ins->push_back(ins_branch);

	//////jump instruction

	sail::Jump* jump_nextins_eq = new sail::Jump(next_ins_label, NULL);
	sail::Jump* jump_nextins_lt = new sail::Jump(next_ins_label, NULL);
	sail::Jump* jump_nextins_gt = new sail::Jump(next_ins_label, NULL);

	//fake an interger to the assignment instruction

	il::string_const_exp* l_orig = new il::string_const_exp("s1",
	/*il::get_integer_type()*/des_t, lo);
	//sail::Variable* sl = new sail::Variable(l_orig, "v"+ Int2Str(des_reg), il::get_integer_type());
	//Destination
	//sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(des_reg,_type_map);
	sail::Variable* sl = asm_ins->getMthCodeH()->getRegVarEle(des_reg,
	/*il::get_integer_type()*/des_t, _type_map, false, false);
	sl->set_original(l_orig);
	sl->set_alias_name(sl->get_var_name());
	//sl->set_type(il::get_integer_type());
	//pair<bool,sail::Variable*> p(true, sl);
	//(*var_map)[des_reg] = p;

	sail::Constant* sr_const_zero = new sail::Constant(0, true, 32);
	sail::Constant* sr_const_neg = new sail::Constant(-1, true, 32);
	sail::Constant* sr_const_pos = new sail::Constant(1, true, 32);

	/*
	 string ins_str_assig_zero = sl->to_string(true) + " = "
	 + sr_const_zero->to_string(true);
	 string ins_str_assig_neg = sl->to_string(true) + " = "
	 + sr_const_neg->to_string(true);
	 string ins_str_assig_pos = sl->to_string(true) + " = "
	 + sr_const_pos->to_string(true);*/

	il::string_const_exp* assig_exp_zero = new il::string_const_exp(
	/*ins_str_assig_zero*/"", il::get_integer_type(), lo);
	il::string_const_exp* assig_exp_neg = new il::string_const_exp(
	/*ins_str_assig_neg*/"", il::get_integer_type(), lo);
	il::string_const_exp* assig_exp_pos = new il::string_const_exp(
	/*ins_str_assig_pos*/"", il::get_integer_type(), lo);

	sail::Assignment* inst_assigzero = new sail::Assignment(sl, sr_const_zero,
			assig_exp_zero, line);
	sail::Assignment* inst_assigneg = new sail::Assignment(sl, sr_const_neg,
			assig_exp_neg, line);
	sail::Assignment* inst_assigpos = new sail::Assignment(sl, sr_const_pos,
			assig_exp_pos, line);
	if (SAIL_INS_PRINT) {
		cout << "test cmp_assignment eq pp:::"
				<< inst_assigzero->to_string(true) << endl;
		cout << "test cmp_assignment eq:::" << inst_assigzero->to_string(false)
				<< endl;
		cout << "test cmp_assignment lt pp:::" << inst_assigneg->to_string(true)
				<< endl;
		cout << "test cmp_assignment lt :::" << inst_assigneg->to_string(false)
				<< endl;
		cout << "test cmp_assignment gt pp:::" << inst_assigpos->to_string(true)
				<< endl;
		cout << "test cmp_assignment gt:::" << inst_assigpos->to_string(false)
				<< endl;
	}
	_sail_ins->push_back(eq_label);
	_sail_ins->push_back(inst_assigzero);
	_sail_ins->push_back(jump_nextins_eq);
	//add an jump to the next ins_label;
	_sail_ins->push_back(lt_label);
	_sail_ins->push_back(inst_assigneg);
	_sail_ins->push_back(jump_nextins_lt);
	//add an jump to the next ins_labe;
	_sail_ins->push_back(gt_label);
	_sail_ins->push_back(inst_assigpos);
	_sail_ins->push_back(jump_nextins_gt);
	_sail_ins->push_back(next_ins_label);
	/*
	 _sail_ins->push_back(then_label); //need to push back here
	 _sail_ins->push_back(inst_assig);
	 _sail_ins->push_back(else_label);
	 */
}
void InstructionMapping::InstructionMapping_F23x(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	switch (opcode) {
	case OP_CMPL_FLOAT:
	case OP_CMPG_FLOAT:
	case OP_CMPL_DOUBLE:
	case OP_CMPG_DOUBLE:
	case OP_CMP_LONG: {
		cmp_branch(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	case OP_AGET:
	case OP_AGET_WIDE:
	case OP_AGET_OBJECT:
	case OP_AGET_BOOLEAN:
	case OP_AGET_BYTE:
	case OP_AGET_CHAR:
	case OP_AGET_SHORT: {
		loadFromArray(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	case OP_APUT:
	case OP_APUT_WIDE:
	case OP_APUT_OBJECT:
	case OP_APUT_BOOLEAN:
	case OP_APUT_BYTE:
	case OP_APUT_CHAR:
	case OP_APUT_SHORT: {
		store2array(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	case OP_ADD_INT: {
		il::binop_type bt = il::_PLUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_SUB_INT: {
		il::binop_type bt = il::_MINUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_MUL_INT: {
		il::binop_type bt = il::_MULTIPLY;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_DIV_INT: {
		il::binop_type bt = il::_DIV;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_REM_INT: {
		il::binop_type bt = il::_MOD;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_AND_INT: {
		il::binop_type bt = il::_BITWISE_AND;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_OR_INT: {
		il::binop_type bt = il::_BITWISE_OR;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_XOR_INT: {
		il::binop_type bt = il::_BITWISE_XOR;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_SHL_INT: {
		il::binop_type bt = il::_LEFT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_SHR_INT: {
		il::binop_type bt = il::_RIGHT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_USHR_INT: {
		il::binop_type bt = il::_RIGHT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_ADD_LONG: {
		il::binop_type bt = il::_PLUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_SUB_LONG: {
		il::binop_type bt = il::_MINUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_MUL_LONG: {
		il::binop_type bt = il::_MULTIPLY;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_DIV_LONG: {
		il::binop_type bt = il::_DIV;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_REM_LONG: {
		il::binop_type bt = il::_MOD;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_AND_LONG: {
		il::binop_type bt = il::_BITWISE_AND;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_OR_LONG: {
		il::binop_type bt = il::_BITWISE_OR;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_XOR_LONG: {
		il::binop_type bt = il::_BITWISE_XOR;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_SHL_LONG: {
		il::binop_type bt = il::_LEFT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_SHR_LONG: {
		il::binop_type bt = il::_RIGHT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_USHR_LONG: {
		il::binop_type bt = il::_RIGHT_SHIFT;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_ADD_FLOAT: {
		il::binop_type bt = il::_PLUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_SUB_FLOAT: {
		il::binop_type bt = il::_MINUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_MUL_FLOAT: {
		il::binop_type bt = il::_MULTIPLY;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_DIV_FLOAT: {
		il::binop_type bt = il::_DIV;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_REM_FLOAT: {
		il::binop_type bt = il::_MOD;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_ADD_DOUBLE: {
		il::binop_type bt = il::_PLUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_SUB_DOUBLE: {
		il::binop_type bt = il::_MINUS;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_MUL_DOUBLE: {
		il::binop_type bt = il::_MULTIPLY;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_DIV_DOUBLE: {
		il::binop_type bt = il::_DIV;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	case OP_REM_DOUBLE: {
		il::binop_type bt = il::_MOD;
		binop(ins_debug, instruction_debug, asm_ins, bt, true, false, addr);
	}
		break;
	default:
		return;
	}
}

//goto/32 +AAAAAAAA
void InstructionMapping::InstructionMapping_F30t(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {

	if (asm_ins->is_label())
		cout << "sucks goto/32!" << endl;
	string operd = asm_ins->getOperand1();
	string add_operd = operd.substr(1, operd.size() - 1);
	string name = "goto_" + add_operd; //goto the place of goto_**

	sail::Label *l = get_label(name);
	sail::Jump* j = new sail::Jump(l, NULL);
	if (SAIL_INS_PRINT) {
		cout << "check goto::" << j->to_string() << endl;
	}
	_sail_ins->push_back(j);
}

void InstructionMapping::InstructionMapping_F31c(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
}

void InstructionMapping::InstructionMapping_F31i(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	InstructionMapping_constantAssignment(ins_debug, instruction_debug, asm_ins,
			addr);
}

void InstructionMapping::switch_branch(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	assert(asm_ins->hasSwitch());
	Switch* swit = asm_ins->getSwitch();

	bool flag_packed = swit->isPackedSwitch();

	unsigned int branch_size = swit->getSize(); //settle the size
	int first_key = 0;
	vector<int>* all_key = NULL;
	if (flag_packed) //packed switch
	{
		first_key = ((PackedSwitch*) swit)->getFirstKey();
		all_key = new vector<int>();
		for (unsigned int i = 0; i < branch_size; i++)
			all_key->push_back(first_key + i);
	} else {
		all_key = ((SparseSwitch*) swit)->getKeys();
	}
	//done of the all_key vector
	//based on the all_key(case) inside the register, we jump to its corresponding branch

	//first build all the unop intruction the size of unop is branch_size+1
	assert(asm_ins->getRegs()->size() >0);
	unsigned short reg = asm_ins->getRegs()->at(0);
	string reg_name = "v" + Int2Str(reg); //lower level, and check it higher level 
	string reg_var_name = "";

	assert(asm_ins->getTypeofVar()->size()>0);
	assert(asm_ins->getTypeofVar()->count(reg) >0);
	ClassDefInfo* reg_classdef = (*asm_ins->getTypeofVar())[reg];
	//il::type* reg_t = TypeMapping(0,_dfr, reg_classdef, _type_map);
	il::type* reg_t = getType(0, _dfr, reg_classdef, _type_map);
	//cout << "type of register ==> " << reg_t->to_string() << endl;

	int line = -1;
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;
	if (instruction_debug->count(asm_ins) > 0) {
		debug = (*instruction_debug)[asm_ins];
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1)
				asm_ins->getMthCodeH()->setLatestLine(line);
			rvt_list = debug->getRVTList();
		}
		if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
			for (unsigned int i = 0; i < rvt_list->size(); i++) {
				if (rvt_list->at(i)->getReg() == reg) {
					reg_var_name = rvt_list->at(i)->getVarName();
					break;
				}
			}
		}

	}
	if (line == -1)
		line = asm_ins->getMthCodeH()->getLatestLine();

	il::location lo(line, line);
	//il::type* bool_type = il::base_type::make("bool", 8, 8, false, il::BOOL,
	//		"BOOLEAN");
	il::type* bool_type = il::get_integer_type();

	il::string_const_exp* ope1_exp = new il::string_const_exp(reg_var_name,
			reg_t, lo);
	//sail::Variable* op1_var = new sail::Variable(ope1_exp, reg_name, reg_t);
	//sail::Variable* op1_var = asm_ins->getMthCodeH()->getRegVarEle(reg, _type_map);
	sail::Variable* op1_var = asm_ins->getMthCodeH()->getRegVarEle(reg, reg_t,
			_type_map, true, false);
	op1_var->set_original(ope1_exp);
	if (reg_var_name != "")
		op1_var->set_alias_name(reg_var_name);
	else
		op1_var->set_alias_name(reg_name);
	//op1_var->set_type(reg_t);

	//now build number of branch_size+1's binop
	int switch_case = 0; //used to as a op2_var for binop
	il::binop_type bt = il::_EQ;
	il::string_const_exp* return_exp = NULL;
	sail::Variable* return_var = NULL;
	sail::Constant* op2_var = NULL;
	vector<sail::Constant*>* case_list = new vector<sail::Constant*>();
	sail::Binop* ins_binop = NULL;
	//sail::Unop* ins_unop = NULL;
	vector<sail::Variable*>* all_return_var = new vector<sail::Variable*>();
	//sail::Variable* neg_all_case_var = NULL;
	vector<sail::Binop*>* binop_list = new vector<sail::Binop*>();
	unsigned int fakeregn = 1000;
	for (unsigned int k = 0; k <= branch_size; k++) {
		fakeregn++;
		if (k != branch_size) {
			switch_case = all_key->at(k);
			//cout << "case is :: " << switch_case << endl;
			op2_var = new sail::Constant(switch_case, true, 32);
			case_list->push_back(op2_var);

			return_exp = new il::string_const_exp("t" + Int2Str(k + 1),
					bool_type, lo);
			//return_var = new sail::Variable(return_exp, "t"+Int2Str(k+1), bool_type);
			//Destination
			//return_var = asm_ins->getMthCodeH()->getRegVarEle(fakeregn, _type_map);
			return_var = asm_ins->getMthCodeH()->getRegVarEle(fakeregn,
					bool_type, _type_map, false, false);
			return_var->set_original(return_exp);
			//return_var->set_alias_name("v" + Int2Str(fakeregn));
			return_var->set_alias_name(return_var->get_var_name());
			//return_var->set_type(bool_type);

			all_return_var->push_back(return_var);

			string binop_str = return_var->to_string(true) + " = "
					+ op1_var->to_string(true)
					+ il::binop_expression::binop_to_string(bt)
					+ op2_var->to_string(true);
			il::string_const_exp* temp = new il::string_const_exp(binop_str,
					bool_type, lo);
			ins_binop = new sail::Binop(return_var, op1_var, op2_var, bt, temp,
					line);
			binop_list->push_back(ins_binop);

			if (SAIL_INS_PRINT) {
				cout << "test switch binop pp ==>" << ins_binop->to_string(true)
						<< endl;
				cout << "test switch binop ==>" << ins_binop->to_string(false)
						<< endl;
			}
			_sail_ins->push_back(ins_binop);
		} else {
			//found the smallest and biggest case then create its variables
			//il:: binop_type default_bt = il::_NEQ;
			int start_size = branch_size + 1;
			int small_case = all_key->front(); //
			int biggest_case = all_key->back();
			//build two binop instruction , less than smallest and greater than biggest

			return_exp = new il::string_const_exp("t" + Int2Str(start_size),
					bool_type, lo);
			//return_var = new sail::Variable(return_exp, "t"+Int2Str(start_size), bool_type);
			//Destination
			//return_var = asm_ins->getMthCodeH()->getRegVarEle(fakeregn, _type_map);
			return_var = asm_ins->getMthCodeH()->getRegVarEle(fakeregn,
					bool_type, _type_map, false, false);
			return_var->set_original(return_exp);
			//return_var->set_alias_name("v" + Int2Str(fakeregn));
			return_var->set_alias_name(return_var->get_var_name());
			//return_var->set_type(bool_type);

			sail::Constant* smallest_var = new sail::Constant(small_case, true,
					32);
			string lessthan_str = return_var->to_string(true) + "="
					+ op1_var->to_string(true)
					+ il::binop_expression::binop_to_string(il::_LEQ)
					+ smallest_var->to_string(true);
			il::string_const_exp* temp1 = new il::string_const_exp(lessthan_str,
					bool_type, lo);
			sail::Variable* lessthan_var = return_var; ///////////////
			ins_binop = new sail::Binop(return_var, op1_var, smallest_var,
					il::_LT, temp1, line);
			if (SAIL_INS_PRINT) {
				cout << "test switch binop pp ||==>"
						<< ins_binop->to_string(true) << endl;
				cout << "test switch binop ||==>" << ins_binop->to_string(false)
						<< endl;
			}
			_sail_ins->push_back(ins_binop);

			return_exp = new il::string_const_exp("t" + Int2Str(start_size + 1),
					bool_type, lo);
			//return_var = new sail::Variable(return_exp, "t"+Int2Str(start_size+1), bool_type);
			//Destination
			//return_var = asm_ins->getMthCodeH()->getRegVarEle(fakeregn+1, _type_map);
			return_var = asm_ins->getMthCodeH()->getRegVarEle(fakeregn + 1,
					bool_type, _type_map, false, false);
			return_var->set_original(return_exp);
			//return_var->set_alias_name("v" + Int2Str(fakeregn));
			return_var->set_alias_name(return_var->get_var_name());
			//return_var->set_type(bool_type);

			sail::Constant* greatest_var = new sail::Constant(biggest_case,
					true, 32);
			string greaterthan_str = return_var->to_string(true) + "="
					+ op1_var->to_string(true)
					+ il::binop_expression::binop_to_string(il::_GEQ)
					+ greatest_var->to_string(true);
			il::string_const_exp* temp2 = new il::string_const_exp(
					greaterthan_str, bool_type, lo);
			sail::Variable* greaterthan_var = return_var; //////////////////
			ins_binop = new sail::Binop(return_var, op1_var, greatest_var,
					il::_GT, temp2, line);
			if (SAIL_INS_PRINT) {
				cout << "test switch binop pp || ==>"
						<< ins_binop->to_string(true) << endl;
				cout << "test switch binop ||==>" << ins_binop->to_string(false)
						<< endl;
			}
			_sail_ins->push_back(ins_binop);

			//need lessthan_var and greaterthan_var to

			return_exp = new il::string_const_exp("t" + Int2Str(start_size + 2),
					bool_type, lo);
			//return_var = new sail::Variable(return_exp, "t"+Int2Str(start_size+2), bool_type);
			//Destination
			return_var = asm_ins->getMthCodeH()->getRegVarEle(fakeregn + 2,
					bool_type, _type_map, false, false);
			//return_var = asm_ins->getMthCodeH()->getRegVarEle(fakeregn+2, _type_map);
			return_var->set_original(return_exp);
			//return_var->set_alias_name("v"+ Int2Str(fakeregn));
			return_var->set_alias_name(return_var->get_var_name());
			//return_var->set_type(bool_type);

			all_return_var->push_back(return_var);
			string and_str = return_var->to_string(true) + "="
					+ lessthan_var->to_string(true)
					+ il::binop_expression::binop_to_string(il::_LOGICAL_AND)
					+ greatest_var->to_string(true);
			il::string_const_exp* temp3 = new il::string_const_exp(and_str,
					bool_type, lo);
			ins_binop = new sail::Binop(return_var, lessthan_var,
					greaterthan_var, il::_LOGICAL_OR, temp3, line);
			if (SAIL_INS_PRINT) {
				cout << "test switch binop pp ==>" << ins_binop->to_string(true)
						<< endl;
				cout << "test switch binop ==>" << ins_binop->to_string(false)
						<< endl;
			}
			_sail_ins->push_back(ins_binop);
		}
	}

	//now build different labels for the different case;
	vector<unsigned int>* targets = swit->getTarget();
	assert(targets->size() >0);
	vector<sail::Label*>* label_list = new vector<sail::Label*>();
	sail::Label* case_label = NULL;
	for (unsigned int n = 0; n < targets->size(); n++) {
		if (flag_packed) {
			case_label = get_label("pswitch_" + Int2Str(targets->at(n)));
		} else {
			case_label = get_label("sswitch_" + Int2Str(targets->at(n)));
		}
		label_list->push_back(case_label); //prepare for the jump
	}
	//case_label = new sail:: Label("default: ", NULL);
	case_label = get_label("default: " + Int2Str(addr + 2 * 3));
	label_list->push_back(case_label);

	//already has all variable as well as branchs
	assert(all_return_var->size() == branch_size +1);
	assert(label_list->size() == branch_size +1);

	vector<pair<sail::Symbol*, sail::Label*>>* target = new vector<
			pair<sail::Symbol*, sail::Label*>>();
	for (unsigned int u = 0; u < branch_size + 1; u++) {
		pair<sail::Symbol*, sail::Label*> p(all_return_var->at(u),
				label_list->at(u));
		target->push_back(p);
	}

	//il::string_const_exp* origi = new il::string_const_exp();
	sail::Branch* swit_branch = new sail::Branch(target, NULL);
	if (SAIL_INS_PRINT) {
		cout << "test switch ins_branch pp ==>" << swit_branch->to_string(true)
				<< endl;
		cout << "test switch ins_brach ==>" << swit_branch->to_string(false)
				<< endl;
	}
	_sail_ins->push_back(swit_branch);

	sail::Label* nextins_label = label_list->at(branch_size);
	_sail_ins->push_back(nextins_label);
	if (SAIL_INS_PRINT) {
		cout << "test switch next_ins_label pp ==>"
				<< nextins_label->to_string(true) << endl;
		cout << "test switch next_ins_brach ==>"
				<< nextins_label->to_string(false) << endl;
	}
}

void InstructionMapping::InstructionMapping_F31t(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	switch (opcode) {
	case OP_FILL_ARRAY_DATA: {
		fillarray(ins_debug, instruction_debug, asm_ins, addr); //process instructions
	}
		break;
	case OP_PACKED_SWITCH:
	case OP_SPARSE_SWITCH: {
		switch_branch(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	default:
		return;
	}
}


void InstructionMapping::InstructionMapping_F32x(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
}

void InstructionMapping::New_functioncall(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	//cout << endl;
	assert(asm_ins != NULL);
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	bool flag = false;

	//the only register to hold an instance of object/may be an array
	vector<unsigned short>* regs = asm_ins->getRegs();
	assert(regs->size() >= 1);
	unsigned short ret_reg = regs->at(0);
	unsigned short size_reg = 0;
	string ret_reg_name = "v" + Int2Str(ret_reg); //good
	string ret_var_name = ""; //good

	string size_reg_name = "";
	string size_var_name = "";
	if (regs->size() > 1) //new-array
			{
		size_reg = regs->at(1);
		size_reg_name = "v" + Int2Str(size_reg);
	}

	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();
	assert(ins_var_type != NULL);

	//new-instance/array should be the ret_type; and no this pointer here, because it is a static functioncall
	//should infered by instruction itself
	//ClassDefInfo* classdef = (*ins_var_type)[ret_reg];
	string objectname = asm_ins->getOperand2();
	//ClassDefInfo* classdef = _dfr->getClassDef(objectname);
	ClassDefInfo* classdef = (*ins_var_type)[ret_reg];
	il::type* ret_t = getType(0, _dfr, classdef, _type_map);
	if (ret_t->is_void_type()) {
		ret_t = NULL;
	}

	//
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;
	if (instruction_debug->count(asm_ins) > 0) {
		if (SCRATCH_INF)
			cout << "this instruction has debug information" << endl;
		debug = (*instruction_debug)[asm_ins];
	}
	int startline = -1;
	if (debug != NULL) {
		startline = debug->getLine();
		if (startline != -1)
			asm_ins->getMthCodeH()->setLatestLine(startline);

		rvt_list = debug->getRVTList();
		RVT* rvt = NULL;
		if (rvt_list != NULL) {

			for (unsigned int i = 0; i < rvt_list->size(); i++) {
				rvt = rvt_list->at(i);
				if (rvt->getReg() == ret_reg)
					ret_var_name = rvt->getVarName();
				else if (regs->size() > 1) {
					if (rvt->getReg() == size_reg)
						size_var_name = rvt->getVarName();
				}
			}
		}

	} else {
		if (SCRATCH_INF)
			cout << "debug information is NULL " << endl;
	}
	if (startline == -1)
		startline = asm_ins->getMthCodeH()->getLatestLine();

	il::location lo(startline, startline);

	//now prepare for the functioncall right side which x = alloc_b();
	DexOpcode opcode = asm_ins->getOpcode();
	string fn_name = "alloc_";
	//string type_name = classdef->getTypename();

	if (opcode == OP_NEW_INSTANCE) // x = alloc_b(); b is the type
			{
		//make sure build the iltype
		il::type* build_type_for_type_infer = getType(0, _dfr,
				_dfr->getClassDefByName(objectname), _type_map);
		objectname = objectname.substr(0, objectname.length() - 1);
		fn_name += objectname;
	} else //need x = alloc_array_b(int x) where x is the size of array new
	{

		string array_type = asm_ins->getOperand3();
		string ele_type = array_type.substr(1, array_type.length() - 1);

		fn_name += "array_" + ele_type;

		ClassDefInfo* ele_array_type_cdi = _dfr->getClassDefByName(array_type);

		//make sure build the iltype
		il::type* build_il_ele_type = getType(0, _dfr, ele_array_type_cdi,
				_type_map);
	}
	if (SCRATCH_INF)
		cout << "fn_name->" << fn_name << endl;
	//done with the fn_name

	vector<sail::Symbol*>* args_v = new vector<sail::Symbol*>();

	il::type* s_t = il::base_type::make("int", 32, 32, true, il::INTEGER, "");
	if (opcode == OP_NEW_ARRAY) {//build an parameter to hold the length of the array
		flag = true;
		sail::Variable* tmp_arg = NULL;
		if (regs->size() > 1) {

			il::string_const_exp* original = new il::string_const_exp(
					size_var_name, s_t, lo);

			tmp_arg = asm_ins->getMthCodeH()->getRegVarEle(size_reg, s_t,
					_type_map, true, false);

			tmp_arg->set_original(original);
			if (size_var_name != "")
				tmp_arg->set_alias_name(size_var_name);
			else
				tmp_arg->set_alias_name(size_reg_name);

			args_v->push_back(tmp_arg);

			//added by haiyan 5.30
			map<string, long>* const_list =
					asm_ins->getMthCodeH()->getRegConst();
			string arr_size_regn = "v" + Int2Str(size_reg);
			if (const_list->size() > 0) {

				map<string, long>::iterator constRegit = const_list->begin();

				for (; constRegit != const_list->end(); constRegit++) {
					if ((*constRegit).first == arr_size_regn) {
						if (SCRATCH_INF) {
							cout << "Found!!" << endl;
							cout << "size of the array is "
									<< (*constRegit).second << endl;
						}
						asm_ins->getMthCodeH()->updateArrSize(ret_reg_name,
								(*constRegit).second);
						break;
					}
				}
			}
		}

	}
	//done with the vector<sail::Symbol*> args_v

	vector<il::type*>* args = new vector<il::type*>();
	if (regs->size() > 1){ //new-array va,vb, type@ccc
		args->push_back(s_t);
	}

	il::type* fn_signature = il::function_type::make(ret_t, *args, flag);

	if (SCRATCH_INF)
		cout << "fn_sigature : " << fn_signature->to_string() << endl;


	//return var
	il::string_const_exp* ret_exp = new il::string_const_exp(ret_var_name,
			ret_t, lo);

	sail::Variable* ret = asm_ins->getMthCodeH()->getRegVarEle(ret_reg, ret_t,
			_type_map, false, false);

	ret->set_original(ret_exp);
	if (ret_var_name != "")
		ret->set_alias_name(ret_var_name);
	else
		ret->set_alias_name(ret_reg_name);

	//process the instruction
	string ins_str = ret->to_string(false) + "= " + fn_name + "(";
	if (args_v->size() > 0)
		ins_str += args_v->at(0)->to_string(true) + ")";
	else
		ins_str += ")";

	il::string_const_exp* temp = new il::string_const_exp(ins_str, ret_t, lo);
	sail::FunctionCall* inst = new sail::FunctionCall(ret, fn_name,
			il::namespace_context(), fn_signature, args_v, false, true, false,
			false, temp, startline);

	if (PRINT_FUNCALL) {
		_dfr->getFunctionCallfstream().open("/scratch/hzhu/funcall.txt",
				fstream::app);
		_dfr->getFunctionCallfstream() << fn_name << endl;
		_dfr->getFunctionCallfstream() << fn_signature->to_string() << endl
				<< endl;
		_dfr->getFunctionCallfstream().close();
	}

	if (SAIL_INS_PRINT) {
		cout << "test new functioncall pp :: " << inst->to_string(true) << endl;
		cout << "test new functioncall  :: " << inst->to_string() << endl;
	}

	_sail_ins->push_back(inst);

	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)
			&& (!USING_TYPE_INFER)) //boyang will do it if USING_TYPE_INFER is true
		updateAnotherBranchReturnType(addr, ret_reg, ret_t, ret);
}

void InstructionMapping::storeField(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	assert(asm_ins != NULL);
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	//object reference the second register that should be the left size *(v?)=
	sail::Variable* object_var = NULL;
	sail::Variable* field_var = NULL;

	//prepre for the register name and variable name
	vector<unsigned short>* regs = asm_ins->getRegs();
	assert(regs->size() > 1);
	unsigned short field_reg = regs->at(0);
	unsigned short object_reg = regs->at(1);
	string field_reg_name = "v" + Int2Str(field_reg);
	string object_reg_name = "v" + Int2Str(object_reg);
	string field_var_name = "";
	string object_var_name = "";
	int line = -1;
	ClassDefInfo* f_classdef = NULL;
	ClassDefInfo* o_classdef = NULL;
	il::type* field_t = NULL;
	il::type* object_t = NULL;
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;

	//if possible update var_name
	if (instruction_debug != NULL) {
		if (instruction_debug->count(asm_ins) > 0) {
			debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1) {
				asm_ins->getMthCodeH()->setLatestLine(line);
				//cout << "set line " << asm_ins->getMthCodeH()->getLatestLine();
			}
			rvt_list = debug->getRVTList();
		}
		if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
			for (unsigned int i = 0; i < rvt_list->size(); i++) {
				if (rvt_list->at(i)->getReg() == field_reg)
					field_var_name = rvt_list->at(i)->getVarName();
				if (rvt_list->at(i)->getReg() == object_reg)
					object_var_name = rvt_list->at(i)->getVarName();
			}
		}
	}
	if (line == -1)
		line = asm_ins->getMthCodeH()->getLatestLine();
	il::location lo(line, line);
	//prepre for each type
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();
	// two register may be the same and have the same type, so we just hold one register with one type
	if (ins_var_type != NULL) {
		assert(ins_var_type->count(field_reg) >0);

		f_classdef = (*ins_var_type)[field_reg];

		assert(ins_var_type->count(object_reg) >0);
		o_classdef = (*ins_var_type)[object_reg];

		field_t = getType(0, _dfr, f_classdef, _type_map);

		if (SCRATCH_INF) {
			cout << "field_t->" << field_t->to_string() << endl;
			cout << "object type classdef name : " << o_classdef->getTypename()
					<< endl;
		}

		object_t = getType(0, _dfr, o_classdef, _type_map);
		if (SCRATCH_INF) {
			cout << "object_t->" << object_t->to_string() << endl;
		}
	}

	//right
	il::string_const_exp* originalr = new il::string_const_exp(field_var_name,
			field_t, lo);

	field_var = asm_ins->getMthCodeH()->getRegVarEle(field_reg, field_t,
			_type_map, true, false);
	field_var->set_original(originalr);
	if (field_var_name != "")
		field_var->set_alias_name(field_var_name);
	else
		field_var->set_alias_name(field_reg_name);

	//now build each both sides variable //left first
	il::string_const_exp* originall = new il::string_const_exp(object_var_name,
			object_t, lo);

	object_var = asm_ins->getMthCodeH()->getRegVarEle(object_reg, object_t,
			_type_map, false, false);
	object_var->set_original(originall);
	string op3 = asm_ins->getOperand3();

	int pos1 = op3.find("->");

	int pos = op3.find(":");

	string nameinope = op3.substr(pos1 + 2, pos - pos1 - 2);


	string objectname = op3.substr(0, pos1);
	object_var->set_alias_name(objectname);


	//now build the instruction
	//first processing the offset
	string operand3 = asm_ins->getOperand3();

	vector<Field*>* allfields = o_classdef->getField();
	unsigned int offset = 0;
	bool flag = false;
	if ((allfields != NULL) && (allfields->size() != 0)) //search for the field
			{
		for (unsigned int k = 0; k < allfields->size(); k++) {
			if (0) {
				cout << "PAYATTENTION------------" << endl;
				cout << "HZHU :: op3's field name " << nameinope << endl;
				cout << "HZHU:: fields name ->" << allfields->at(k)->getName()
						<< endl;
				cout << "HZHU:: fields offset ->"
						<< allfields->at(k)->getOffset() << endl;
				cout << "HZHU:: fields offset ->"
						<< allfields->at(k)->getOffset() << endl;
				cout << "HZHU:: only check name ------------>"
						<< allfields->at(k)->getName() << endl;
			}
			if (allfields->at(k)->getName() == nameinope) {
				flag = true;
				offset = allfields->at(k)->getOffset();
			}
		}
		if ((!flag) && (SCRATCH_INF)) {
			cout << "poor!, can't find the field !!" << endl;
			assert(false);
		}
	}
	//done with the offset
	//fill for the Store instruction
	string ins_str = string("*") + string(" (");
	ins_str = ins_str + object_var->to_string(true);
	ins_str = ins_str + Int2Str(offset) + ")" + " = "
			+ field_var->to_string(true);

	il::string_const_exp* temp = new il::string_const_exp(ins_str, field_t, lo);

	assert(field_t != NULL);

	sail::Store* inst = new sail::Store(object_var, field_var, offset,
			nameinope, temp, line);
	//cout << "test Store field pp :: " << inst->to_string(true)<<endl;
	if (SAIL_INS_PRINT) {
		cout << "test Store field pp :: " << inst->to_string(true) << endl;
		cout << "test Store field :: " << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);

	//object_reg
	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)&&(!USING_TYPE_INFER)) //boyang will do it if USING_TYPE_INFER is true
		updateAnotherBranchReturnType(addr, object_reg, object_t, object_var);
}

void InstructionMapping::store2array(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	assert(asm_ins != NULL);
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	//parepare for the initialize
	//assert(var_map != NULL);

	sail::Variable* ele_var = NULL; //to the ele_var
	sail::Variable* arr_var = NULL; //from

	vector<unsigned short>* regs = asm_ins->getRegs();
	assert(regs->size() > 2);
	unsigned short ele_reg = regs->at(0);
	unsigned short arr_reg = regs->at(1);
	unsigned short idx_reg = regs->at(2); //for the offset
	//unsigned short idx_reg = getConstant(regs->at(2)); //for the offset

	//cout << "index register:: "<< idx_reg << endl;
	//unsigned int fake_idx = idx_reg; //this value may as a parameter which across multiple methods; //need constant propagation

	string ele_reg_name = "v" + Int2Str(ele_reg);
	string arr_reg_name = "v" + Int2Str(arr_reg);
	string ele_var_name = "";
	string arr_var_name = "";
	string idx_reg_name = "v" + Int2Str(idx_reg);
	string idx_var_name = "";

	int line = -1;
	ClassDefInfo* ele_classdef = NULL;
	ClassDefInfo* arr_classdef = NULL;
	ClassDefInfo* idx_classdef = NULL; /////////////////
	il::type* ele_t = NULL;
	il::type* arr_t = NULL;
	il::type* idx_t = NULL;////////////////////
	DebugInfo* debug = NULL; //instruction level
	vector<RVT*>* rvt_list = NULL;

	if (instruction_debug != NULL) {
		if (instruction_debug->count(asm_ins) > 0) {
			debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1)
				asm_ins->getMthCodeH()->setLatestLine(line);
			rvt_list = debug->getRVTList();
		}
		if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
			for (unsigned int i = 0; i < rvt_list->size(); i++) {
				if (rvt_list->at(i)->getReg() == ele_reg)
					ele_var_name = rvt_list->at(i)->getVarName();
				if (rvt_list->at(i)->getReg() == arr_reg)
					arr_var_name = rvt_list->at(i)->getVarName();
				if (rvt_list->at(i)->getReg() == idx_reg)
					idx_var_name = rvt_list->at(i)->getVarName();
			}
		}
	}
	if (line == -1)
		line = asm_ins->getMthCodeH()->getLatestLine();

	il::location lo(line, line);
	//prepre for each type
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();
	if (ins_var_type != NULL) {
		//assert(ins_var_type->size() >2); it is possible that aput v8, v0, v8, element register and index register are same, so we only save one register
		if (ins_var_type->count(ele_reg) > 0) {
			ele_classdef = (*ins_var_type)[ele_reg];
		}
		if (ins_var_type->count(arr_reg) > 0) {
			assert(arr_reg != ele_reg);
			arr_classdef = (*ins_var_type)[arr_reg];
		}
		if(ins_var_type->count(idx_reg) > 0){/////////////////////////
			if(idx_reg != ele_reg)
				idx_classdef = (*ins_var_type)[idx_reg];
			else
				idx_classdef = (*ins_var_type)[2000];
		}

		ele_t = getType(0, _dfr, ele_classdef, _type_map);
		arr_t = getType(0, _dfr, arr_classdef, _type_map);
		idx_t = getType(0, _dfr, idx_classdef, _type_map);/////////////////
		if (SCRATCH_INF) {
			cout << "ele type ->" << ele_t->to_string() << endl;
			cout << "arr type ->" << arr_t->to_string() << endl;
			cout << "idx type ->" << idx_t->to_string() << endl;////////////////////
		}
	}
	DexOpcode opcode = asm_ins->getOpcode();
	unsigned elesize = 0;
	switch (opcode) {
	case OP_APUT:
		elesize = 4;
		break;
		//case OP_APUT_WIDE: elesize = 8;
	case OP_APUT_WIDE:
		elesize = 4;
		break;
	case OP_APUT_OBJECT:
		elesize = 8; //modified 08/19, the pointer is 8 byte long.
		break;
		//case OP_APUT_BOOLEAN: elesize = 1;
	case OP_APUT_BOOLEAN:
		elesize = 4;
		break;
		//case OP_APUT_BYTE: elesize = 1;
	case OP_APUT_BYTE:
		elesize = 4;
		break;
		//case OP_APUT_CHAR: elesize = 2;
	case OP_APUT_CHAR:
		elesize = 1;
		break;
		//case OP_APUT_SHORT: elesize = 2;
	case OP_APUT_SHORT:
		elesize = 4;
		break;
	default: {
		elesize = 0;
		cout << "no such instruction !" << endl;
		assert(false);
	}
	}
	//int offset = elesize * fake_idx;
	//cout << "offset :: " << offset << endl;

	//first should be the index register
	il::string_const_exp* idx_exp = new il::string_const_exp(idx_var_name,
			/*il::get_integer_type()*/idx_t, lo);
	sail::Variable* idx_var = asm_ins->getMthCodeH()->getRegVarEle(idx_reg,
			/*il::get_integer_type()*/idx_t, _type_map, true, false); //two bool: in_use, cast instruction?
	idx_var->set_original(idx_exp);
	if (idx_var_name != "")
		idx_var->set_alias_name(idx_var_name);
	else
		idx_var->set_alias_name(idx_reg_name);

	//now set up the offset variable  temp = idx_var*4;
	sail::Variable* offset = new sail::Variable("offset",
			il::get_integer_type(), true, false, -1, false);
	offset->set_var_name_as_temp_name();
	offset->set_alias_name(offset->get_var_name());
	sail::Constant* elewidth = new sail::Constant(elesize, false, 32);
	il::binop_type bt = il::_MULTIPLY;
	il::string_const_exp* temp1 = new il::string_const_exp("",
			il::get_integer_type(), lo);
	sail::Binop* bins = new sail::Binop(offset, idx_var, elewidth, bt, temp1,
			line);
	if (SAIL_INS_PRINT) {
		cout << "test aput offset process pp :: " << bins->to_string(true)
				<< endl;
		cout << "test aput offset process  :: " << bins->to_string(false)
				<< endl;
	}
	_sail_ins->push_back(bins);

	//array reference variable
	il::string_const_exp* arr_exp = new il::string_const_exp(arr_var_name,
			arr_t, lo);
	arr_var = asm_ins->getMthCodeH()->getRegVarEle(arr_reg, arr_t, _type_map,
			true, false);
	arr_var->set_original(arr_exp);
	if (arr_var_name != "")
		arr_var->set_alias_name(arr_var_name);
	else
		arr_var->set_alias_name(arr_reg_name);

	sail::Variable* ref = new sail::Variable("location", arr_t, true, false, -1,
			false);
	ref->set_var_name_as_temp_name();
	ref->set_alias_name(ref->get_var_name());
	il::binop_type bt1 = il::_PLUS;
	il::string_const_exp* temp2 = new il::string_const_exp("", arr_t, lo);
	sail::Binop* bins2 = new sail::Binop(ref, arr_var, offset, bt1, temp2,
			line);
	if (SAIL_INS_PRINT) {
		cout << "test aput  object reference  pp :: " << bins2->to_string(true)
				<< endl;
		cout << "test aput object reference  :: " << bins2->to_string(false)
				<< endl;
	}
	_sail_ins->push_back(bins2);

	//ele var
	il::string_const_exp* ele_exp = new il::string_const_exp(ele_var_name,
			ele_t, lo);
	//ele_var = new sail::Variable(ele_exp,ele_reg_name, ele_t);
	ele_var = asm_ins->getMthCodeH()->getRegVarEle(ele_reg, ele_t, _type_map,
			true, false);
	//ele_var = asm_ins->getMthCodeH()->getRegVarEle(ele_reg,_type_map);
	ele_var->set_original(ele_exp);
	if (ele_var_name != "")
		ele_var->set_alias_name(ele_var_name);
	else
		ele_var->set_alias_name(ele_reg_name);

	//string ins_str = string("* (") + arr_var->to_string(true) + Int2Str(offset) + string(")") + string(" = ") + ele_var->to_string(true) ; 
	string ins_str = "";
	il::string_const_exp* temp3 = new il::string_const_exp(ins_str, ele_t, lo);
	assert(ele_t != NULL);
	sail::Store* inst = new sail::Store(ref, ele_var, 0, "", temp3, line);
	if (SAIL_INS_PRINT) {
		cout << "test store array ele pp :: " << inst->to_string(true) << endl;
		cout << "test store array ele :: " << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);
}



void InstructionMapping::fillarray(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {

	assert(asm_ins != NULL);
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	Data* data = NULL;
	vector<ArrayData>* data_list = NULL;
	unsigned int elewidth = 0;
	unsigned int size = 0;
	sail::Variable* arr_ref = NULL;

	//first precess the arr_ref
	vector<unsigned short>* regs = asm_ins->getRegs();
	assert(regs->size() != 0);
	unsigned short arr_reg = regs->at(0);
	string arr_reg_name = "v" + Int2Str(arr_reg);
	string arr_var_name = "";
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();
	ClassDefInfo* arr_t = NULL;
	il::type* t = NULL;
	if ((ins_var_type != NULL) && (ins_var_type->count(arr_reg) > 0)) {
		arr_t = (*ins_var_type)[arr_reg];
		t = getType(0, _dfr, arr_t, _type_map);
	}

	int line = -1;
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;
	if (instruction_debug != NULL) {
		if (instruction_debug->count(asm_ins) > 0) {
			debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1)
				asm_ins->getMthCodeH()->setLatestLine(line);
			rvt_list = debug->getRVTList();
		}
		if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
			arr_var_name = rvt_list->at(0)->getVarName();
		}
	}

	if (line == -1)
		asm_ins->getMthCodeH()->getLatestLine();
	il::location lo(line, line);
	il::string_const_exp* original = new il::string_const_exp(arr_var_name, t,
			lo);

	arr_ref = asm_ins->getMthCodeH()->getRegVarEle(arr_reg, t, _type_map, false,
			false);
	arr_ref->set_original(original);
	if (arr_var_name != "")
		arr_ref->set_alias_name(arr_var_name);
	else
		arr_ref->set_alias_name(arr_reg_name);

	if (SCRATCH_INF)
		cout << "arr_ref->" << arr_ref->to_string(true) << endl;

	sail::Symbol* cons_data = NULL;
	int offset = 0;
	if (asm_ins->hasData()) {
		if (SCRATCH_INF)
			cout << "check has data" << endl;
		data = asm_ins->getData();
		data_list = data->getData();
		assert(data_list != NULL);

		//we map each double, J, short ... to int
		elewidth = 4; // 4 bytes
		short which = data->getWhich();
		if (SCRATCH_INF)
			cout << "ele width->" << elewidth << endl;

		size = data->getSize();

		if (SCRATCH_INF)
			cout << "size of data->" << size << endl;

		for (unsigned int i = 0; i < size; i++) {
			offset = i * elewidth;
			if (arr_t->getTypename() == "INVALID") { //this is for default,may not true
				cons_data = new sail::Constant(data_list->at(i)._uint32_data,
						false, 32);
			} else if (which == ARRAY8)
				cons_data = new sail::Constant(data_list->at(i)._uint8_data,
						false, 32); //one byte
			else if (which == ARRAY16)
				cons_data = new sail::Constant(data_list->at(i)._uint16_data,
						false, 32); //two bytes
			else if (which == ARRAY32) {
				if (arr_t->getTypename() == "[I")
					cons_data = new sail::Constant(
							data_list->at(i)._uint32_data, false, 32); //four bytes
				if (arr_t->getTypename() == "[F") {
					//the array type may be float, so convert it into the float type;
					float value = *((float*) (&(data_list->at(i)._uint32_data)));
					cons_data = new sail::Constant(value, false, 32);
				}
			} else if (which == ARRAY64) {
				if (arr_t->getTypename() == "[J")
					cons_data = new sail::Constant(
							data_list->at(i)._uint64_data, false, 32); //8 bytes
				if (arr_t->getTypename() == "[D") {
					//the array type may be float, so convert it into the float type;
					float value =
							*((double*) (&(data_list->at(i)._uint64_data)));
					cons_data = new sail::Constant(value, false, 32); // 8 bytes
				}

			}

			string ins_str = string("*") + string(" (");
			ins_str = ins_str + arr_ref->to_string(true);
			ins_str = ins_str + Int2Str(offset) + ")" + " = "
					+ cons_data->to_string(true);
			il::string_const_exp* temp = new il::string_const_exp(ins_str, t,
					lo);
			assert(t != NULL);
			sail::Store* inst = new sail::Store(arr_ref, cons_data, offset, "",
					temp, line);
			if (SAIL_INS_PRINT) {
				cout << "test Store pp :: " << inst->to_string(true) << endl;
				cout << "test Store  :: " << inst->to_string(false) << endl;
			}
			_sail_ins->push_back(inst);

		}
	} else {
		if (SCRATCH_INF)
			cout << "check has no data" << endl;
	}

}

void InstructionMapping::loadField(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	assert(asm_ins != NULL);
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	//object reference the second register that should be the left size *(v?)=
	sail::Variable* object_var = NULL;
	sail::Variable* field_var = NULL;
	//prepre for the register name and variable name
	vector<unsigned short>* regs = asm_ins->getRegs();
	assert(regs->size() > 1);
	unsigned short field_reg = regs->at(0);
	unsigned short object_reg = regs->at(1);
	string field_reg_name = "v" + Int2Str(field_reg);
	string object_reg_name = "v" + Int2Str(object_reg);
	string field_var_name = "";
	string object_var_name = "";
	int line = -1;
	ClassDefInfo* f_classdef = NULL;
	ClassDefInfo* o_classdef = NULL;
	il::type* field_t = NULL;
	il::type* object_t = NULL;
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;

	//if possible update var_name
	if (instruction_debug != NULL) {
		if (instruction_debug->count(asm_ins) > 0) {
			debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1)
				asm_ins->getMthCodeH()->setLatestLine(line);
			rvt_list = debug->getRVTList();
		}
		if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
			for (unsigned int i = 0; i < rvt_list->size(); i++) {
				if (rvt_list->at(i)->getReg() == field_reg)
					field_var_name = rvt_list->at(i)->getVarName();
				if (rvt_list->at(i)->getReg() == object_reg)
					object_var_name = rvt_list->at(i)->getVarName();
			}
		}
	}
	if (line == -1) {
		line = asm_ins->getMthCodeH()->getLatestLine();
	}
	il::location lo(line, line);
	//prepre for each typ
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();
	if (ins_var_type != NULL) {
		assert(ins_var_type->size() >1);
		if (ins_var_type->count(field_reg) > 0)
			f_classdef = (*ins_var_type)[field_reg];
		if ((ins_var_type->count(object_reg) > 0) && (object_reg != field_reg)){
			cout << "object_reg " << object_reg << endl;
			o_classdef = (*ins_var_type)[object_reg];
		}
		else{
			o_classdef = (*ins_var_type)[2000];
		}

		field_t = getType(0, _dfr, f_classdef, _type_map);
		object_t = getType(0, _dfr, o_classdef, _type_map);
	}
	//now build each both sides variable //left first
	il::string_const_exp* originall = new il::string_const_exp(object_var_name,
			object_t, lo);

	object_var = asm_ins->getMthCodeH()->getRegVarEle(object_reg, object_t,
			_type_map, true, false);
	object_var->set_original(originall);
	string op3 = asm_ins->getOperand3();
	int pos1 = op3.find("->");
	int pos = op3.find(":");
	string name = op3.substr(pos1 + 2, pos - pos1 - 2);
	string objectname = op3.substr(0, pos1);
	//6.8
	object_var->set_alias_name(objectname);


	//right
	il::string_const_exp* originalr = new il::string_const_exp(field_var_name,
			field_t, lo);

	field_var = asm_ins->getMthCodeH()->getRegVarEle(field_reg, field_t,
			_type_map, false, false);
	//update
	field_var->set_original(originalr);



	if (field_var_name != "")
		field_var->set_alias_name(field_var_name);
	else
		field_var->set_alias_name(field_reg_name);


	assert(o_classdef != NULL);
	vector<Field*>* allfields = o_classdef->getField();

	unsigned int offset = 0;
	bool flag = false;
	if ((allfields != NULL) && (allfields->size() != 0)) //search for the field
			{
		for (unsigned int k = 0; k < allfields->size(); k++) {
			if (0) {
				cout << "PAYATTENTION------------" << endl;
				cout << "HZHU :: op3's field name " << name << endl;
				cout << "HZHU:: fields name ->" << allfields->at(k)->getName()
						<< endl;
				cout << "HZHU:: fields offset ->"
						<< allfields->at(k)->getOffset() << endl;
				cout << "HZHU:: fields offset ->"
						<< allfields->at(k)->getOffset() << endl;
			}
			if (allfields->at(k)->getName() == name) {
				flag = true;
				offset = allfields->at(k)->getOffset();
			}
		}
		if ((!flag) && (SCRATCH_INF)) {
			cout << "poor!, can't find the field" << endl;
			assert(false);
		}
	}

	//fill for the Store instruction
	string ins_str = string("*") + string(" (");
	ins_str = ins_str + object_var->to_string(true);
	ins_str = ins_str + Int2Str(offset) + ")" + " = "
			+ field_var->to_string(true);

	il::string_const_exp* temp = new il::string_const_exp(ins_str, field_t, lo);
	processInvariant(field_var, object_var, NULL, line);
	assert(_tempvar_for_invariant != NULL);
	sail::Load* inst = new sail::Load(field_var, _tempvar_for_invariant, offset,
			name, temp, line);

	if (SAIL_INS_PRINT) {
		cout << "test Load field pp :: " << inst->to_string(true) << endl;
		cout << "test Load field :: " << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);

	//boyang will do it, if UISNG_TYPE_INFER is true
	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)
			&&(!USING_TYPE_INFER))
		updateAnotherBranchReturnType(addr, field_reg, field_t, field_var);

}

void InstructionMapping::loadFromArray(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	assert(asm_ins != NULL);
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);

	//parepare for the initialize

	sail::Variable* ele_var = NULL; //to the ele_var
	sail::Variable* arr_var = NULL; //from

	vector<unsigned short>* regs = asm_ins->getRegs();
	assert(regs->size() > 2);
	unsigned short ele_reg = regs->at(0); //dest
	unsigned short arr_reg = regs->at(1); //source
	unsigned short idx_reg = regs->at(2); //source for the offset

	string ele_reg_name = "v" + Int2Str(ele_reg);
	string arr_reg_name = "v" + Int2Str(arr_reg);
	string ele_var_name = "";
	string arr_var_name = "";
	string idx_reg_name = "v" + Int2Str(idx_reg);
	string idx_var_name = "";

	int line = -1;
	ClassDefInfo* ele_classdef = NULL;
	ClassDefInfo* arr_classdef = NULL;
	ClassDefInfo* idx_classdef = NULL;
	il::type* ele_t = NULL;
	il::type* arr_t = NULL;
	il::type* idx_t = NULL;
	DebugInfo* debug = NULL;
	vector<RVT*>* rvt_list = NULL;

	if (instruction_debug != NULL) {
		if (instruction_debug->count(asm_ins) > 0) {
			debug = (*instruction_debug)[asm_ins];
		}
		if (debug != NULL) {
			line = debug->getLine();
			if (line != -1)
				asm_ins->getMthCodeH()->setLatestLine(line);
			rvt_list = debug->getRVTList();
		}
		if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
			for (unsigned int i = 0; i < rvt_list->size(); i++) {
				if (rvt_list->at(i)->getReg() == ele_reg)
					ele_var_name = rvt_list->at(i)->getVarName();
				if (rvt_list->at(i)->getReg() == arr_reg)
					arr_var_name = rvt_list->at(i)->getVarName();
				if (rvt_list->at(i)->getReg() == idx_reg)
					idx_var_name = rvt_list->at(i)->getVarName();
			}
		}
	}
	if (line == -1)
		line = asm_ins->getMthCodeH()->getLatestLine();

	il::location lo(line, line);
	//prepre for each type
	map<unsigned int, ClassDefInfo*>* ins_var_type = asm_ins->getTypeofVar();

	if (ins_var_type != NULL) {
		assert(ins_var_type->size() >2);
		if (ins_var_type->count(ele_reg) > 0)
			ele_classdef = (*ins_var_type)[ele_reg];
		if (ins_var_type->count(arr_reg) > 0) {
			if (arr_reg != ele_reg)
				arr_classdef = (*ins_var_type)[arr_reg];
			else
				arr_classdef = (*ins_var_type)[2000];
		}
		if (ins_var_type->count(idx_reg) > 0) {
			if (ele_reg != idx_reg)
				idx_classdef = (*ins_var_type)[idx_reg];
			else
				idx_classdef = (*ins_var_type)[2000];
		}

		ele_t = getType(0, _dfr, ele_classdef, _type_map);
		arr_t = getType(0, _dfr, arr_classdef, _type_map);
		idx_t = getType(0, _dfr, idx_classdef, _type_map);
		if (SCRATCH_INF) {
			cout << "ele type ->" << ele_t->to_string() << endl;
			cout << "arr type ->" << arr_t->to_string() << endl;
			cout << "idx  type ->" << idx_t->to_string() << endl;
		}
	}
	DexOpcode opcode = asm_ins->getOpcode();
	unsigned elesize = 0;
	switch (opcode) {
	case OP_AGET:
		elesize = 4;
		break;
		//case OP_AGET_WIDE: elesize = 8;
	case OP_AGET_WIDE:
		elesize = 4;
		break;
	case OP_AGET_OBJECT:
		elesize = 8;
		break;
		//case OP_AGET_BOOLEAN: elesize = 1;
	case OP_AGET_BOOLEAN:
		elesize = 4;
		break;
		//case OP_AGET_BYTE: elesize = 1;
	case OP_AGET_BYTE:
		elesize = 4;
		break;
		//case OP_AGET_CHAR: elesize = 2;
	case OP_AGET_CHAR:
		elesize = 1;
		break;
		//case OP_AGET_SHORT: elesize = 2;
	case OP_AGET_SHORT:
		elesize = 4;
		break;
	default: {
		elesize = 0;
		assert(false);
	}
	}

	//first should be the index register
	il::string_const_exp* idx_exp = new il::string_const_exp(idx_var_name,
	/*il::get_integer_type()*/idx_t, lo);
	sail::Variable* idx_var = asm_ins->getMthCodeH()->getRegVarEle(idx_reg,
	/*il::get_integer_type()*/idx_t, _type_map, true, false); //two bool: in_use, cast instruction?
	idx_var->set_original(idx_exp);
	if (idx_var_name != "")
		idx_var->set_alias_name(idx_var_name);
	else
		idx_var->set_alias_name(idx_reg_name);

	//now set up the offset variable  temp = idx_var*4;
	sail::Variable* offset = new sail::Variable("offset",
			il::get_integer_type(), true, false, -1, false);
	offset->set_var_name_as_temp_name();
	offset->set_alias_name(offset->get_var_name());
	sail::Constant* elewidth = new sail::Constant(elesize, false, 32);
	il::binop_type bt = il::_MULTIPLY;
	il::string_const_exp* temp1 = new il::string_const_exp("",
			il::get_integer_type(), lo);
	sail::Binop* bins = new sail::Binop(offset, idx_var, elewidth, bt, temp1,
			line);
	if (SAIL_INS_PRINT) {
		cout << "test aget offset process pp :: " << bins->to_string(true)
				<< endl;
		cout << "test aget offset process  :: " << bins->to_string(false)
				<< endl;
	}
	_sail_ins->push_back(bins);

	//array reference variable
	il::string_const_exp* arr_exp = new il::string_const_exp(arr_var_name,
			arr_t, lo);
	arr_var = asm_ins->getMthCodeH()->getRegVarEle(arr_reg, arr_t, _type_map,
			true, false);
	arr_var->set_original(arr_exp);
	if (arr_var_name != "")
		arr_var->set_alias_name(arr_var_name);
	else
		arr_var->set_alias_name(arr_reg_name);

	sail::Variable* ref = new sail::Variable("location", arr_t, true, false, -1,
			false);
	ref->set_var_name_as_temp_name();
	ref->set_alias_name(ref->get_var_name());
	il::binop_type bt1 = il::_PLUS;
	il::string_const_exp* temp2 = new il::string_const_exp("", arr_t, lo);
	sail::Binop* bins2 = new sail::Binop(ref, arr_var, offset, bt1, temp2,
			line);
	if (SAIL_INS_PRINT) {
		cout << "test aget  object reference  pp :: " << bins2->to_string(true)
				<< endl;
		cout << "test aget object reference  :: " << bins2->to_string(false)
				<< endl;
	}
	_sail_ins->push_back(bins2);

	//ele var
	il::string_const_exp* ele_exp = new il::string_const_exp(ele_var_name,
			ele_t, lo);
	ele_var = asm_ins->getMthCodeH()->getRegVarEle(ele_reg, ele_t, _type_map,
			false, false); //in_def, not cast

	ele_var->set_original(ele_exp);
	if (ele_var_name != "")
		ele_var->set_alias_name(ele_var_name);
	else
		ele_var->set_alias_name(ele_reg_name);

	string ins_str = "";
	il::string_const_exp* temp3 = new il::string_const_exp(ins_str, ele_t, lo);
	assert(ele_t != NULL);
	//sail::Store* inst = new sail::Store(ref, ele_var, 0, "", temp3, line);
	sail::Load* inst = new sail::Load(ele_var, ref, 0, "", temp3, line);
	if (SAIL_INS_PRINT) {
		cout << "test aget  pp :: " << inst->to_string(true) << endl;
		cout << "test aget   :: " << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);
	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)
			&& (!USING_TYPE_INFER)) //boyang will do it if USING_TYPE_INFER is true
		updateAnotherBranchReturnType(addr, ele_reg, ele_t, ele_var);

}
void InstructionMapping::functioncall(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	assert(asm_ins != NULL);
	if (DEX_INS_PRINT)
		printIns(asm_ins, addr);
	add_labels(asm_ins);
	int curline = -1;
	DebugInfo* funcall_debug = NULL;
	if (instruction_debug->count(asm_ins) > 0) {
		funcall_debug = (*instruction_debug)[asm_ins];
		if (funcall_debug != NULL) {
			curline = funcall_debug->getLine();
			if (curline != -1)
				asm_ins->getMthCodeH()->setLatestLine(curline);
		}
	}
	if (curline == -1)
		curline = asm_ins->getMthCodeH()->getLatestLine();
	//hzhu added 5.25
	unsigned int m_idx = asm_ins->getMethodidx();

	Method* me = _dfr->getMethod(m_idx);
	assert(me != NULL);

	string returnstr = me->getReturnType();

	ClassDefInfo* return_classdef = NULL;

	sail::Variable* ret = NULL;
	il::type* ret_t = NULL;

	il::type* ret_type_for_variable = NULL;
	il::type* ret_type_for_fn_sig = NULL;

	int line = -1;

	//this if precess the sail::Variable ret;
	unsigned int r_reg = -1;
	string ret_reg_name = "";
	string ret_var_name = "";
	DexOpcode opcode = asm_ins->getOpcode();
	if (returnstr != "V") {
		if (!GLOBAL_MAP) {
			if (_type_map->count(returnstr) > 0)
				return_classdef = (*_type_map)[returnstr];
			else
				return_classdef = new ClassDefInfo(returnstr);

		} else
			return_classdef = _dfr->getClassDefByName(returnstr);
		//12/20
		unsigned int typeidx = _dfr->getTypeId(returnstr);
		return_classdef->setTypeIdx(typeidx);
		//12/20

		ret_type_for_fn_sig = getType(0, _dfr, return_classdef, _type_map);

		if (ret_type_for_fn_sig->is_void_type()) {
			ret_type_for_fn_sig = NULL;
		}

		//process the next instruction's register
		//DexOpcode opcode = asm_ins->getOpcode();
		Format format = asm_ins->getOpFormat();
		OpcodeFormat* op_format = new OpcodeFormat(asm_ins->getMthCodeH(),
				opcode);
		unsigned int size = op_format->getSize(format);
		unsigned int next_addr = addr + size * 2;
		map<unsigned int, ASMInstruction*>* add_ins =
				asm_ins->getMthCodeH()->getAdd2InsMap();
		assert(add_ins != NULL);
		ASMInstruction* nextin = NULL;
		if (add_ins->count(next_addr) > 0)
			nextin = (*add_ins)[next_addr];

		//OK, to check if this instruction is move-result, then get its register,
		//we might need the variable name, prepare it, if not move-result ,
		//might be ##return##
		//unsigned int r_reg = 0;
		if (nextin->getOperator().substr(0, 7) == "move-re") {

			//return'value's debug information
			DebugInfo* ret_debug = NULL;
			if (instruction_debug->count(nextin) > 0) {
				ret_debug = (*instruction_debug)[nextin];
			}

			//int next_ins_line;
			vector<RVT*>* next_ins_localv = NULL;
			if (ret_debug != NULL) {
				//next_ins_line = ret_debug->getLine();
				line = ret_debug->getLine();
				if (line != -1)
					asm_ins->getMthCodeH()->setLatestLine(line);
				next_ins_localv = ret_debug->getRVTList();
			}
			RVT* ret_var = NULL;
			vector<unsigned short>* ret_reg = nextin->getRegs();
			assert(ret_reg != 0);
			r_reg = ret_reg->at(0);

			map<unsigned int, ClassDefInfo*>* next_ins_type_map =
					nextin->getTypeofVar();
			assert(next_ins_type_map != NULL);

			assert(next_ins_type_map->count(r_reg) > 0);
			if (next_ins_type_map->count(r_reg) > 0) {
				ret_type_for_variable = getType(0, _dfr,
						(*next_ins_type_map)[r_reg], _type_map);
			}

			ret_reg_name = "v" + Int2Str(r_reg);
			ret_var_name = "";
			if ((next_ins_localv != NULL) && (next_ins_localv->size() != 0)) {
				ret_var = next_ins_localv->at(0);
				ret_var_name = ret_var->getVarName();
			}

			if (line == -1) {
				line = asm_ins->getMthCodeH()->getLatestLine();
			}
			il::location lo1(line, line);

			//ret_var
			il::string_const_exp* ret_exp = new il::string_const_exp(
					ret_var_name, ret_type_for_variable, lo1);

			ret = asm_ins->getMthCodeH()->getRegVarEle(r_reg,
					ret_type_for_variable, _type_map, false, false);
			//update destination reg;
			//hzhu added 5.26
			ret->set_original(ret_exp);
			if (SCRATCH_INF) {
				cout << "ret_var_name " << ret_var_name << endl;
				cout << "ret_reg_name " << ret_reg_name << endl;
			}
			if (ret_var_name != "")
				ret->set_alias_name(ret_var_name);
			else
				ret->set_alias_name(ret_reg_name);

			if (SCRATCH_INF)
				cout << "making the return variable :: " << ret->to_string()
						<< "       " << ret->get_type()->to_string() << endl;

		} else {
			/*This is to capture the situation that, the function has a return type
			 *
			 * but there is no variable to be assigned by this call      */
			ret = NULL;
		}
	} else {
		//If there is no return value, we pass NULL for the return variable
		ret = NULL;
	}

	//now process function name

	// **************************************************************************** //
	/* invoke-virtual   ---> mark as virtual
	 * invoke-super	    ---> do not mark as virtual, mark as super
	 * invoke-direct    ---> virtual if not constructor, non-virtual if constructor
	 * invoke-static    ---> not virtual
	 * invoke-interface ---> virtual
	 * *******************************************************************************/
	il::location loc(curline, curline);
	bool is_call_to_virtual = false;
	bool is_allocator = false; //
	bool is_constructor = false; //
	bool is_super_call = false;
	string fn_name = me->getName();
	if (fn_name.find("init>") != string::npos) {
		//cout << "Yes it is a constructor " << endl;
		is_constructor = true;
	}
	if (SCRATCH_INF)
		cout << "fn_name:: " << fn_name << endl;

	//go back to the own instruction "invoke-"
	vector<unsigned short>*regs_c = asm_ins->getRegs(); ///////////////////yeap
	//cout << "num of registers of this instruction: " << regs_c->size() <<endl;
	//the parameter should prepare from the tail of the regs_c
	//prepare for the arguments
	vector<sail::Symbol*>* args_v = new vector<sail::Symbol*>();
	vector<il::type*> args;
	sail::Variable* tmp_arg = NULL;
	int arg_size = me->getParaSize();
	unsigned int this_reg = 0;
	////////////////////////////////////////////////

	//change is_constructor to see the function name contain
	//get the first register for this functioncal method
	if ((opcode != OP_INVOKE_STATIC) && (opcode != OP_INVOKE_STATIC_RANGE)) {
		assert(regs_c->size() != 0);
		assert((*(asm_ins->getTypeofVar()))[regs_c->at(0)]!= NULL);

		this_reg = regs_c->at(0);

		ClassDefInfo* passed_cdf = (*(asm_ins->getTypeofVar()))[this_reg];
		il::type* passed_this_t = getType(0, _dfr, passed_cdf, _type_map);

		//the fun. sig. use the declared type(for example invoke-virtual {v0} A::b(int)V)
		//sig should be :: [noret](A,int)//not para type v0, for example(v0)

		ClassDefInfo* declared_cdf = type2ClassDef(_dfr,
				me->getClassOwneridx());
		il::type* this_t = getType(0, _dfr, declared_cdf, _type_map); //this type should be the real declared type
		args.push_back(this_t);

		is_call_to_virtual = true;

		string this_var_name = "thisp";
		il::string_const_exp* this_orig = new il::string_const_exp(
				this_var_name, passed_this_t, loc);

		tmp_arg = asm_ins->getMthCodeH()->getRegVarEle(this_reg, passed_this_t,
				_type_map, true, false);
		tmp_arg->set_original(this_orig);

		tmp_arg->set_alias_name(tmp_arg->get_var_name());
		args_v->push_back(tmp_arg); //variable associate to the passed in type

	}
	if ((opcode == OP_INVOKE_SUPER) || (opcode == OP_INVOKE_SUPER_RANGE)) {
		is_super_call = true;
	}

	// arg_size is the size of function definition parameter list,
	//do not include this pointer.
	// starting here from 1, because this pointer has been taken care
	if (arg_size != 0) {
		vector<il::type*>* passed_in_reg_types = new vector<il::type*>();
		vector<unsigned short>* passed_in_reg_numbers = new vector<
				unsigned short>();

		map<unsigned int, ClassDefInfo*>* reg_types = asm_ins->getTypeofVar();
		assert(reg_types != NULL);
		int start = 0;

		if (is_call_to_virtual)
			start = 1;
		else
			// handle static
			start = 0;

		for (unsigned int i = start; i < regs_c->size(); i++) {
			unsigned short passed_in_reg_number = regs_c->at(i);
			// can not assert passed_in_reg_number must be found
			// because, for dealing with Doudle and long type, 2 registers are used
			// e.g. invoke-* {v0, v1}  name(D), only v0 has correct type infomation in the map
			//assert(reg_types->count(passed_in_reg_number) > 0);

			il::type* passed_in_reg_type = NULL;

			if (reg_types->count(passed_in_reg_number) > 0) {
				passed_in_reg_type = getType(0, _dfr,
						(*reg_types)[passed_in_reg_number], _type_map);
				assert(passed_in_reg_type != NULL);
				passed_in_reg_types->push_back(passed_in_reg_type);
				passed_in_reg_numbers->push_back(passed_in_reg_number);
			}
		}

		//keep going to process the vector<sail::Symbol*>* args_v
		//->first prepare for the variable name of parameters and its corresponding register name
		//////this is prepared for the later
		map<string, string>* reg2var = new map<string, string>();
		DebugInfo* debug_c = NULL;
		vector<RVT*>* cur_rvt = NULL;
		//il::location loc(line,line);

		if (instruction_debug->count(asm_ins) > 0) {
			debug_c = (*instruction_debug)[asm_ins];
			if (debug_c != NULL) {
				cur_rvt = debug_c->getRVTList();
				//for the register to see if there is a chance to get the variable name
				for (int r = 0; r < arg_size; r++) {
					unsigned short cur_r = passed_in_reg_numbers->at(r);
					string cur_r_n = "v" + Int2Str(cur_r); //for example reg 3

					(*reg2var)[cur_r_n] = ""; //initial
					if (cur_rvt != NULL) {
						int size_rvt = cur_rvt->size();
						for (int v = 0; v != size_rvt; v++) {
							RVT* temp_rvt = cur_rvt->at(v);
							unsigned short reg_in_rvt = temp_rvt->getReg();
							if (cur_r == reg_in_rvt) {
								(*reg2var)[cur_r_n] = temp_rvt->getVarName(); //override
							}
						}
					}
				}
			}
		}
		//build arg_v which is vector<Symbol*>
		//////////////////////////////////////////////////////////
		unsigned int pos = 0;
		assert(passed_in_reg_numbers -> size() == passed_in_reg_types->size());
		while (pos != passed_in_reg_numbers->size()) {
			unsigned short reg = passed_in_reg_numbers->at(pos);
			string reg_name = "v" + Int2Str(reg);

			string var_name;
			if (reg2var->count(reg_name) > 0)
				var_name = (*reg2var)[reg_name];

			il::type* reg_type = passed_in_reg_types->at(pos);

			il::string_const_exp* original = new il::string_const_exp(var_name,
					reg_type, loc);

			tmp_arg = asm_ins->getMthCodeH()->getRegVarEle(reg, reg_type,
					_type_map, true, false);
			tmp_arg->set_original(original);

			if (var_name != "")
				tmp_arg->set_alias_name(var_name);
			else
				tmp_arg->set_alias_name(reg_name);

			assert(args_v != NULL);
			args_v->push_back(tmp_arg);
			pos++;
		}

	}
	//now process fn_signature ?? is a function type
	//int parasize = me->getParaSize();
	vector<unsigned int>* declared_paras = me->getParaTypeIds();
	//assert(declared_paras != NULL);
	if (declared_paras != NULL) {
		for (int i = 0; i < arg_size; i++) {
			unsigned int paraidx = declared_paras->at(i);
			ClassDefInfo* para_classdef = type2ClassDef(this->_dfr, paraidx);
			assert(para_classdef!= NULL);
			il::type* para_t = getType(0, _dfr, para_classdef, _type_map);
			args.push_back(para_t);
		}
	}

	il::type* fn_signature = il::function_type::make(ret_type_for_fn_sig, args,
			false);
	if (0)
		cout << "fn_sigature ::::::::::::::::::::: "
				<< fn_signature->to_string() << endl;

	//hzhu end 5.26
	string ins_str;
	string ret_str = "";
	if (ret != NULL) {
		ret_str = ret->to_string(true) + "="; //+ fn_name + "(";
	}

	//add namespace into the il::node* original and the namepace might have multiple level of classes 8.15
	StringSplit* sp = new StringSplit(me->getClassOwner(), "$");

	il::namespace_context ns = sp->makeNameSpace();
	if (NAME_SPACE_PRINT)
		cout << "ns == > " << ns.to_string() << endl;

	il::string_const_exp* temp = new il::string_const_exp(ins_str, fn_signature,
			il::location(curline, curline));
	if (CHECK_TYPE) {
		cout << "check before functioncall " << endl;
		if ((ret_type_for_fn_sig != NULL) && (ret != NULL)) {
			cout << "return variable name::" << ret->to_string() << " type :: "
					<< ret->get_type()->to_string() << endl;
		}
		int size = args_v->size();
		int i = 0;
		while (i != size) {
			cout << " arg " << i << " name : " << args_v->at(i)->to_string()
					<< "type: " << args_v->at(i)->get_type()->to_string()
					<< endl;
			i++;
		}
	}
	sail::Instruction* inst;
	if (fn_name == "static_assert") {
		assert(args_v->size() > 0);
		inst = new sail::StaticAssert((*args_v)[0], NULL, curline);
	} else { //cout << "invoke_super ?? " << is_super_call << endl;
		if ((is_constructor) || (is_super_call))
			is_call_to_virtual = false;
		//HZHU add invariant
		processInvariant(ret, NULL, args_v, curline);
		inst = new sail::FunctionCall(ret, fn_name, ns, fn_signature, args_v,
				is_call_to_virtual, is_allocator, is_constructor, is_super_call,
				temp, curline);
		//HZHU end invariant
	}

	if (PRINT_FUNCALL) {
		_dfr->getFunctionCallfstream().open("/scratch/hzhu/funcall.txt",
				fstream::app);
		_dfr->getFunctionCallfstream() << fn_name << endl;
		_dfr->getFunctionCallfstream() << ns.to_string() << endl;
		_dfr->getFunctionCallfstream() << fn_signature->to_string() << endl
				<< endl;
		_dfr->getFunctionCallfstream().close();
	}
	if (SAIL_INS_PRINT) {
		cout << "test functioncall pp :: " << inst->to_string(true) << endl;
		cout << "test functioncall  :: " << inst->to_string(false) << endl;
	}
	_sail_ins->push_back(inst);

	if ((NON_DECLARED_RETURN_TYPE) && (_fun_declared_return_var != NULL)
			&& (!USING_TYPE_INFER)) //boyang will do it if UISNG_TYPE_INFER is true
		updateAnotherBranchReturnType(addr, r_reg, ret_t, ret);

	if (PRINT_PROCESS_EXCEPTION)
		cout << "HZHU before ************* processException " << endl;
	processExceptions(ins_debug, asm_ins, addr, 0);
	if (PRINT_PROCESS_EXCEPTION)
		cout << "HZHU after ***********  processException " << endl;
}

void InstructionMapping::InstructionMapping_F35c(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	switch (opcode) {
	case OP_FILLED_NEW_ARRAY: //this instruction should be followed by move-result-object
	{
		if (DEX_INS_PRINT)
			printIns(asm_ins, addr);
		add_labels(asm_ins);

		//get registers for the filled_new_array
		vector<unsigned short>* regs = asm_ins->getRegs();
		assert( regs->size()>0);

		int array_size = regs->size();

		//get the next instruction which is move-result-object
		unsigned int next_addr = addr + 3 * 2;
		map<unsigned int, ASMInstruction*>* addr_ins =
				asm_ins->getMthCodeH()->getAdd2InsMap();
		assert(addr_ins != NULL);
		ASMInstruction* nextins = NULL;
		if (addr_ins->count(next_addr) > 0)
			nextins = (*addr_ins)[next_addr];
		if (SCRATCH_INF)
			cout << "next instruction is " << nextins->toString() << endl;

		//next ins register which holds the array ref
		assert(nextins->getRegs()->size()>0);
		unsigned short arr_reg = nextins->getRegs()->at(0);
		string arr_reg_name = "v" + Int2Str(arr_reg);
		string arr_var_name = "";
		if (SCRATCH_INF)
			cout << "move-result-object reg is " << arr_reg << endl;

		//debug information for both instructions (filled_new_array && move_result_object)
		DebugInfo* debug = NULL; //instruction level
		vector<RVT*>* rvt_list = NULL;

		//prepare for line and variable name for both instructions
		int firstline = -1;
		int secondline = -1;
		vector<string>* ele_var_name = new vector<string>();
		for (unsigned short i = 0; i < regs->size(); i++) {
			ele_var_name->push_back("");
		}

		if (instruction_debug != NULL) {
			//filled_new_array debug inf for filled_new_array
			if (instruction_debug->count(asm_ins) > 0) {
				debug = (*instruction_debug)[asm_ins];
				if (debug != NULL) {
					firstline = debug->getLine();
					if (firstline != -1)
						asm_ins->getMthCodeH()->setLatestLine(firstline);
					rvt_list = debug->getRVTList();
				}
				if ((rvt_list != NULL) && (rvt_list->size() > 0)) {
					for (unsigned short i = 0; i < regs->size(); i++) {
						bool flag = false;
						unsigned short cur_reg = regs->at(i);
						unsigned int j = 0;
						while (j != rvt_list->size()) {
							if (rvt_list->at(j)->getReg() == cur_reg) {
								flag = true;
								ele_var_name->push_back(
										rvt_list->at(j)->getVarName());
								break;
							}
							j++;
						}
						if (!flag) {
							string empty_str = "";
							ele_var_name->push_back(empty_str);
						}
					}
				}
			}

			// move_result_object debug info
			rvt_list = NULL;
			debug = NULL;
			if (instruction_debug->count(nextins) > 0) {
				debug = (*instruction_debug)[nextins];
				if (debug != NULL) {
					secondline = debug->getLine();
					if (secondline != -1)
						asm_ins->getMthCodeH()->setLatestLine(secondline);
					rvt_list = debug->getRVTList();
					if ((rvt_list != NULL) && (rvt_list->size() > 0))
						arr_var_name = rvt_list->at(0)->getVarName();
				}

			}
		}

		if (firstline == -1)
			asm_ins->getMthCodeH()->getLatestLine();
		if (secondline == -1)
			asm_ins->getMthCodeH()->getLatestLine();

		/////////////////////////////////////////////////////////////
		//type infer here !
		string fn_name = "alloc_";
		string array_type = asm_ins->getOperand2();
		string ele_type = array_type.substr(1, array_type.length() - 1);
		fn_name += "array_" + ele_type;

		ClassDefInfo* ele_classdef = _dfr->getClassDefByName(ele_type);
		ClassDefInfo* arr_classdef = _dfr->getClassDefByName(array_type);

		unsigned int typeidx = _dfr->getTypeId(array_type);
		arr_classdef->setTypeIdx(typeidx);

		unsigned int eletypeid = _dfr->getTypeId(ele_type);
		ele_classdef->setTypeIdx(eletypeid);

		il::type* ele_t = getType(0, _dfr, ele_classdef, _type_map);
		il::type* arr_t = getType(0, _dfr, arr_classdef, _type_map);

		//only used once, it will immediately assigned to the register in move-return instruction;
		sail::Variable* arr_ref = new sail::Variable("array_location", arr_t,
				true, false, -1, false);
		arr_ref->set_var_name_as_temp_name();

		//first alloc an array then fill it with provided register; "[I"
		////////////////////////////////////////////////////////////////
		//the size of array;
		sail::Constant* size_const = new sail::Constant(array_size, true, 32);
		vector<il::type*>* args = new vector<il::type*>();
		args->push_back(il::get_integer_type());

		vector<sail::Symbol*>* args_v = new vector<sail::Symbol*>();
		args_v->push_back(size_const);

		il::type* fn_signature = il::function_type::make(arr_t, *args, false);
		il::string_const_exp* temp_str = new il::string_const_exp("", arr_t,
				il::location(firstline, firstline));

		sail::FunctionCall* alloc_array_inst = new sail::FunctionCall(arr_ref,
				fn_name, il::namespace_context(), fn_signature, args_v, false,
				true, false, false, temp_str, firstline);

		if (SAIL_INS_PRINT) {
			cout << "test Alloc_array pp :: "
					<< alloc_array_inst->to_string(true) << endl;
			cout << "test Alloc_array  :: "
					<< alloc_array_inst->to_string(false) << endl;
		}
		_sail_ins->push_back(alloc_array_inst);

		//a list of regs that hold the content of the array
		vector<sail::Variable*>* ele_list_var = new vector<sail::Variable*>();
		sail::Variable* temp_var = NULL;

		for (unsigned int k = 0; k != regs->size(); k++) {
			temp_var = asm_ins->getMthCodeH()->getRegVarEle(regs->at(k), ele_t,
					_type_map, true, false);

			if (ele_var_name->at(k) != "")
				temp_var->set_alias_name(ele_var_name->at(k));
			else
				temp_var->set_alias_name("v" + Int2Str(regs->at(k)));

			ele_list_var->push_back(temp_var);
		}

		//contructor the instructions
		unsigned int ele_size = 4;
		string t_str = ele_classdef->getTypename();
		//cout << "t_str ==>" << t_str << endl;
		if ((t_str == "Z") || (t_str == "B"))
			ele_size = 4; //ele_size = 1;
		else if ((t_str == "S") || (t_str == "C"))
			ele_size = 4; //ele_size = 2;
		else if ((t_str == "I") || (t_str == "F"))
			ele_size = 4;
		else if ((t_str == "J") || (t_str == "D")) // this one isn't possible, anyway keep it
				{
			assert(false);
			ele_size = 4; //ele_size =4;
		}
		//build the instructions the ele_var inside the ele_list_var
		sail::Variable* ele_var = NULL;
		int offset = 0;
		sail::Store* inst = NULL;
		il::string_const_exp* temp;

		//e.g. v1, v2 hold the content of array;
		for (unsigned int u = 0; u < regs->size(); u++) {
			ele_var = ele_list_var->at(u);
			offset = ele_size * u;
			sail::Constant* offset_const = new sail::Constant(offset, true, 32);

			sail::Variable* ref = new sail::Variable("offset", arr_t, true,
					false, -1, false);
			ref->set_var_name_as_temp_name();

			temp = new il::string_const_exp("", il::get_integer_type(),
					il::location(firstline, firstline));
			//ele_var * u;
			sail::Binop* binop = new sail::Binop(ref, arr_ref, offset_const,
					il::_PLUS, temp, firstline);

			if (SAIL_INS_PRINT) {
				cout << "test binop for array fill pp ::"
						<< binop->to_string(true) << endl;
				cout << "test binop for array fill ::" << binop->to_string()
						<< endl;
			}
			_sail_ins->push_back(binop);

			assert(ele_t != NULL);
			inst = new sail::Store(ref, ele_var, 0, "", temp, firstline);

			if (SAIL_INS_PRINT) {
				cout << "test Store pp :: " << inst->to_string(true) << endl;
				cout << "test Store  :: " << inst->to_string(false) << endl;
			}
			_sail_ins->push_back(inst);
		}


		//move-result-object (next instruction)
		sail::Variable* arr_ref_final = asm_ins->getMthCodeH()->getRegVarEle(
				arr_reg, arr_t, _type_map, false, false);

		if (arr_var_name != "")
			arr_ref_final->set_alias_name(arr_var_name);
		else
			arr_ref_final->set_alias_name(arr_reg_name);

		temp = new il::string_const_exp("", arr_ref_final->get_type(),
				il::location(secondline, secondline));

		sail::Assignment* assi = new sail::Assignment(arr_ref_final, arr_ref,
				temp, secondline);

		if (SAIL_INS_PRINT) {
			cout << "test Assignment pp :: " << assi->to_string(true) << endl;
			cout << "test Assignement  :: " << assi->to_string(false) << endl;
		}
		_sail_ins->push_back(assi);
	}
		break;
	case OP_INVOKE_VIRTUAL:
	case OP_INVOKE_SUPER:
	case OP_INVOKE_DIRECT:
	case OP_INVOKE_STATIC:
	case OP_INVOKE_INTERFACE: {
		if (EXCEPTION_TEST) {
			unsigned int mid = asm_ins->getMethodidx();
			updateFunctionCallExps(mid, asm_ins);
		}
		functioncall(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	default:
		return;
	}
}

void InstructionMapping::InstructionMapping_F3rc(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	DexOpcode opcode = asm_ins->getOpcode();
	switch (opcode) {
	case OP_FILLED_NEW_ARRAY_RANGE: //this instruction should be followed by move-result-object
	{
		//fillarray(ins_debug, instruction_debug, asm_ins);
	}
		break;
	case OP_INVOKE_VIRTUAL_RANGE:
	case OP_INVOKE_SUPER_RANGE:
	case OP_INVOKE_DIRECT_RANGE:
	case OP_INVOKE_STATIC_RANGE:
	case OP_INVOKE_INTERFACE_RANGE: {
		if (EXCEPTION_TEST) {
			unsigned int mid = asm_ins->getMethodidx();
			updateFunctionCallExps(mid, asm_ins);
		}
		functioncall(ins_debug, instruction_debug, asm_ins, addr);
	}
		break;
	default:
		return;
	}
}

void InstructionMapping::InstructionMapping_F51l(Ins2Debug* ins_debug,
		map<ASMInstruction*, DebugInfo*>* instruction_debug,
		ASMInstruction* asm_ins, unsigned int addr) {
	InstructionMapping_constantAssignment(ins_debug, instruction_debug, asm_ins,
			addr);
}

sail::Label* InstructionMapping::get_label(const string& name) {
	if (_str_label->count(name) > 0)
		return (*_str_label)[name];
	sail::Label * l = new sail::Label(name, NULL);
	(*_str_label)[name] = l;
	return l;
}

/*
void InstructionMapping::setConstant(unsigned int reg_n, int value) {
	(*_constant)[reg_n] = value;
}

int InstructionMapping::getConstant(unsigned int reg_n) {
	if (_constant->count(reg_n) > 0)
		return (*_constant)[reg_n];
	return rand() % 20 + 1;
}*/

//this value must greater than 1001 (based on its seed!);
unsigned int InstructionMapping::getRandom() {
	if(_random == 29999){
		cerr << "value start with 30000 should be used as global variable! " << endl;
		assert(false);
	}
	return (++_random);
}

sail::Instruction* InstructionMapping::getLastIns() {
	unsigned int size = _sail_ins->size();
	if (size == 0)
		return NULL;
	else
		return _sail_ins->at(size - 1);
}
void InstructionMapping::add_labels(ASMInstruction* asm_ins) {
	if (asm_ins->is_label()) {
		sail::Label* add_label = NULL;
		string l = asm_ins->getLabel();
		if (SCRATCH_INF)
			cout << "original l is : " << l << endl;
		int len = l.size();
		std::size_t found; // = l.find("&");
		sail::Instruction* last_ins = getLastIns();
		while (l != "") {
			found = l.find("&");
			if (found != string::npos) //found
					{
				if (SCRATCH_INF)
					cout << "label is :" << l.substr(0, found) << endl;
				;
				add_label = get_label(l.substr(0, found));
				l = l.substr(found + 1, len - found - 1); //the substr
				if (SCRATCH_INF)
					cout << "l is : " << l << endl;
				if (last_ins != NULL) {
					if (SCRATCH_INF) {
						cout << "last_ins " << last_ins->to_string() << endl;
						cout << "this_ins " << add_label->to_string() << endl;
					}
					if (last_ins->to_string() != add_label->to_string()) {
						_sail_ins->push_back(add_label);
					}
				} else
					_sail_ins->push_back(add_label);
			} else {
				add_label = get_label(l);
				if (last_ins != NULL) {
					if (SCRATCH_INF) {
						cout << "last_ins " << last_ins->to_string() << endl;
						cout << "this_ins " << add_label->to_string() << endl;
					}
					if (add_label->to_string() != last_ins->to_string()) {
						_sail_ins->push_back(add_label);
					}
				} else
					_sail_ins->push_back(add_label);
				break;
			}
		}
	}
}

//if a self defined functioncall, then based on the annotation inside the dex file, we may be able to get the what type of exceptions we have for this self-definde funcall;
void InstructionMapping::updateFunctionCallExps(unsigned int mid,
		ASMInstruction* asm_ins) {
	map<unsigned int, vector<string>*>* e_m = _dfr->getMethodThrows(); //method id->exception types
	//map<unsigned int, Method*>* id_m = _dfr->getMIdx2Method(); //methodid ->Method(self-defined mapping)
	map<unsigned int, Method*>* id_m = _dfr->getSelfDefinedMethodMap(); //this belongs to self-defined method
	if (id_m->count(mid) > 0)
		asm_ins->setSelfDefinedM(); //set if this functioncall is self-defined method funcall
	if (e_m->count(mid) > 0) {
		vector<string>* e = (*e_m)[mid];
		asm_ins->setExceptionTypes(e); //set the functioncall may throw some type if exceptions(by annotation?)
	}

}

void InstructionMapping::printIns(ASMInstruction* asm_ins, unsigned int addr) {
	if (SAIL_INS_PRINT) {
		cout << "---------------------------------------------" << endl ;
		cout << "Addr: " << addr << endl;
		cout << "asm :" << asm_ins->toString() << endl;
		cout << "hastries " << asm_ins->hasTries() << endl;
		cout << "realTryBegin? " << asm_ins->realTryStart() << endl;
		cout << "realTryEnd? " << asm_ins->realTryEnd() << endl;
		//haiyan added 7.16
		DexOpcode opcode = asm_ins->getOpcode();
		if ((opcode == OP_INVOKE_VIRTUAL) || (opcode == OP_INVOKE_SUPER)
				|| (opcode == OP_INVOKE_DIRECT)
				|| (opcode == OP_INVOKE_INTERFACE)
				|| (opcode == OP_INVOKE_VIRTUAL_RANGE)
				|| (opcode == OP_INVOKE_SUPER_RANGE)
				|| (opcode == OP_INVOKE_DIRECT_RANGE)
				|| (opcode == OP_INVOKE_INTERFACE_RANGE)) {
			cout << "is self defined ? " << asm_ins->isSelfDefinedM() << endl;
			if (asm_ins->hasExceptions()) {
				cout << "size of exception "
						<< asm_ins->getExceptionTypes()->size()
						<< "and its first element is "
						<< asm_ins->getExceptionTypes()->at(0) << endl;
			} else
				cout << "No exceptions for this fun call !" << endl;
		}
		//haiyan end 7.16
		cout << "is label: " << asm_ins->is_label() << endl;
		if (asm_ins->is_label())
			cout << "label is " << asm_ins->getLabel() << endl;
		cout << "inside method: "
				<< asm_ins->getMthCodeH()->getMethod()->toString() << endl;
	}
}

void InstructionMapping::processExceptions(Ins2Debug* ins_debug,
		ASMInstruction* asm_ins, unsigned int addr, unsigned int line) {
	//first want to know if this ins inside any try or sync block
	//if no exception annotation, no syncs and no tries, then no exceptions
	CodeHeader * ch = asm_ins->getMthCodeH();
	//vector<unsigned int>* sync_objs = ch->getSyncObj();
	//vector<vector<pair<unsigned int, unsigned int>>*>* sync_addrs_range = ch->getSyncAddr();
	//vector<pair<unsigned int,unsigned int>>* sync_catchalls = ch->getSyncCatches();
	//actually need to check the addr belongs to which monitor-enter and monitor-exit range that is equvelent to say which index
	if ((!(ch->isThrowExceptions())) && (!(ch->hasRealTries()))) {
		//do nothing
		if(PRINT_PROCESS_EXCEPTION)
		cout << "CASE :: no throw exception annotation and no real tries "
				<< endl;
	}
	if (ch->isThrowExceptions() && (!ch->hasRealTries())) //annotation has exceptions
			{
		if(PRINT_PROCESS_EXCEPTION)
			cout << "CASE :: only has throw annotation has exceptions " << endl;
		if ((asm_ins->isSelfDefinedM()) && (asm_ins->hasExceptions()))
			processAnnotationThrows(ins_debug, asm_ins, addr,
					asm_ins->getMthCodeH()->getLatestLine(), false);
		if (LIB_HAS_EXCEPTION) {

			if ((!(asm_ins->isSelfDefinedM()))
					&& (!(asm_ins->isNewException()))) //if it is the throw new Exception(), followed should be the throw
				processAnnotationThrows(ins_debug, asm_ins, addr,
						asm_ins->getMthCodeH()->getLatestLine(), true);
		}
	}
	if ((!(ch->isThrowExceptions())) && (ch->hasRealTries())) // only has try-catch-finally blocks, this one is good
			{
			if(PRINT_PROCESS_EXCEPTION)
		cout
				<< "CASE ::  has try-catch-finally but without throw annotation!!! "
				<< endl;
		if (ch->isTryFinally()) {
			if (asm_ins->insideTryBlock(addr)) {
				//call something;
				if(PRINT_PROCESS_EXCEPTION)
				cout
						<< "try-finally , match this, process  functioncalls are inside try block!"
						<< endl;
				processTryFinally(ins_debug, asm_ins, addr,
						asm_ins->getMthCodeH()->getLatestLine()); //last false, has_annotation?
			}
		} else {
			if (asm_ins->insideTryBlock(addr)){
				if(PRINT_PROCESS_EXCEPTION)
					cout << "not tryFinally !! " << endl;
				processTryCatch(ins_debug, asm_ins, addr,
						asm_ins->getMthCodeH()->getLatestLine(), false); //last false, has_annotation?
			}
		}
	}
	if ((ch->isThrowExceptions()) && (ch->hasRealTries())) // HZHU :: this one needs check !!!!!!has annotation and try-catch-finally blocks
			{
		if(PRINT_PROCESS_EXCEPTION)
			cout << "CASE :: has throw annotation and try-catch-finally blocks "
				<< endl;
		if (asm_ins->insideTryBlock(addr)) { //inside the try block, nothing related to annotation, the last parameter is false rather than true
			if (!ch->isTryFinally())
				processTryCatch(ins_debug, asm_ins, addr,
						asm_ins->getMthCodeH()->getLatestLine(), false);
			else {
				if(PRINT_PROCESS_EXCEPTION)
					cout << "try-finally, we process it later !" << endl;
				processTryFinally(ins_debug, asm_ins, addr,
						asm_ins->getMthCodeH()->getLatestLine()); //last annotation, has annotation
			}
		} else { //outside try block
			if ((asm_ins->isSelfDefinedM()) && (asm_ins->hasExceptions()))
				processAnnotationThrows(ins_debug, asm_ins, addr,
						asm_ins->getMthCodeH()->getLatestLine(), false); //the last false denotes not a lib functioncall
			if (LIB_HAS_EXCEPTION) {
				if ((!(asm_ins->isSelfDefinedM()))
						&& (!(asm_ins->isNewException()))) // add an additional condition here, should not a new throw exception
					processAnnotationThrows(ins_debug, asm_ins, addr,
							asm_ins->getMthCodeH()->getLatestLine(), true); //true : the lib_functioncall
		}
		}
	}
}

map<string, ClassDefInfo*>* InstructionMapping::getTypeMap() {
	return _type_map;
}

void InstructionMapping::processAnnotationThrows(Ins2Debug* ins_debug,
		ASMInstruction* asm_ins, unsigned int addr, unsigned int line,
		bool is_lib) {

	il::location lo(line, line);
	//il::type* bool_type = il::base_type::make("bool", 8, 8, false, il::BOOL,
	//		"BOOLEEAN");
	il::type* bool_type = il::get_integer_type();
	il::binop_type bt_e = il::_EQ;
	//il::binop_type bt_o = il::_LOGICAL_OR;
	//il::binop_type bt_a = il::_LOGICAL_AND;
	il::unop_type ut = il::_LOGICAL_NOT;
	//common things prepare
	sail::Constant* constant = new sail::Constant(1, true, 32);
	unsigned int nextaddr = addr + 3 * 2; //35c or 3rc
	string else_label_str = "con_" + Int2Str(nextaddr);
	sail::Label* next_ins_label = get_label(else_label_str); //goto next ins

	//prepare for the instanceof Ins
	//sail::Variable* con_var_0 = asm_ins->getMthCodeH()->getRegVarEle(getRandom(),_type_map);
	//first process is_lib or not
	if (is_lib) {
		//if library function call ,first label it such that we can jump to it
		//////////////////////////////haiyan add for simplify
		string lib_check_str = "check LIB_HAS_EXCEPTION" + Int2Str(addr);
		sail::Label* lib_check_l = get_label(lib_check_str);
		_sail_ins->push_back(lib_check_l);

		//sail::Variable* lib_is_excep = asm_ins->getMthCodeH()->getRegVarEle(getRandom(), _type_map);
		sail::Variable* lib_is_excep = asm_ins->getMthCodeH()->getRegVarEle(
				getRandom(), bool_type, _type_map, false, false);
		lib_is_excep->set_alias_name(lib_is_excep->get_var_name());
		//lib_is_excep->set_type(bool_type);
		/*
		 string lib_is_excep_str = lib_is_excep->to_string(true) + " = "
		 + ins_debug->lib_has_exception_var()->to_string(true)
		 + il::binop_expression::binop_to_string(bt_e)
		 + constant->to_string(true);
		 */
		il::string_const_exp * original_1 = new il::string_const_exp("",
				bool_type, lo);

		sail::Binop* lib_is_excep_ins = new sail::Binop(lib_is_excep,
				ins_debug->lib_has_exception_var(), constant, bt_e, original_1,
				line);
		_sail_ins->push_back(lib_is_excep_ins);
		if (SAIL_INS_PRINT) {
			cout << "test library is exception binop pp :: "
					<< lib_is_excep_ins->to_string(true) << endl;
			cout << "test library is exception binop :: "
					<< lib_is_excep_ins->to_string(false) << endl;
		}

		//another ins is unop
		//sail::Variable* lib_non_excep = asm_ins->getMthCodeH()->getRegVarEle(getRandom(), _type_map);
		sail::Variable* lib_non_excep = asm_ins->getMthCodeH()->getRegVarEle(
				getRandom(), bool_type, _type_map, false, false);
		lib_non_excep->set_alias_name(lib_non_excep->get_var_name());
//	lib_non_excep->set_type(bool_type);
		/*string lib_non_excep_str = lib_non_excep->to_string(true) + " != "
		 + lib_is_excep->to_string();*/
		il::string_const_exp* original_2 = new il::string_const_exp("",
				bool_type, lo);
		sail::Unop* lib_non_excep_ins = new sail::Unop(lib_non_excep,
				lib_is_excep, ut, original_2, line);
		_sail_ins->push_back(lib_non_excep_ins);
		if (SAIL_INS_PRINT) {
			cout << "test library is exception binop pp :: "
					<< lib_non_excep_ins->to_string(true) << endl;
			cout << "test library is exception binop :: "
					<< lib_non_excep_ins->to_string(false) << endl;
		}

		//now prepare for the branch for those two variables
		//1) prepare for the then/else label
		string then_label_str_1 = "check CUR_EXCEPTION for "
				+ lib_is_excep->to_string();
		//string else_label_str = "con_" + Int2Str(nextaddr);
		sail::Label* then_label_1 = get_label(then_label_str_1);
		//sail::Label* next_ins_label = get_label(else_label_str);//goto next ins
		sail::Branch* lib_con_ins_branch = new sail::Branch(lib_is_excep,
				lib_non_excep, then_label_1, next_ins_label, NULL);
		_sail_ins->push_back(lib_con_ins_branch);
		_sail_ins->push_back(then_label_1);
		if (SAIL_INS_PRINT) {
			cout << "test library is exception branch pp :: "
					<< lib_con_ins_branch->to_string(true) << endl;
			cout << "test library is exception branch :: "
					<< lib_con_ins_branch->to_string(false) << endl;
			cout << "test CUR_EXCEPTION label pp"
					<< then_label_1->to_string(true) << endl;
			cout << "test CUR_EXCEPTION label "
					<< then_label_1->to_string(false) << endl;
		}
	}
	//next process cur_exception instructions and its branch
	//
	//sail::Variable* cur_has_excep = asm_ins->getMthCodeH()->getRegVarEle(getRandom(), _type_map);
	sail::Variable* cur_has_excep = asm_ins->getMthCodeH()->getRegVarEle(
			getRandom(), bool_type, _type_map, false, false);
	cur_has_excep->set_alias_name(cur_has_excep->get_var_name());
	//cur_has_excep->set_type(bool_type);
	/*string cur_has_excep_str = cur_has_excep->to_string(true) + " = "
	 + ins_debug->cur_exception_var()->to_string(true)
	 + il::binop_expression::binop_to_string(bt_e)
	 + constant->to_string(true);*/
	il::string_const_exp * original_3 = new il::string_const_exp("", bool_type,
			lo);
	sail::Binop* cur_has_excep_ins = new sail::Binop(cur_has_excep,
			ins_debug->cur_exception_var(), constant, bt_e, original_3, line);
	_sail_ins->push_back(cur_has_excep_ins);
	if (SAIL_INS_PRINT) {
		cout << "test cur has exception binop pp :: "
				<< cur_has_excep_ins->to_string(true) << endl;
		cout << "test cur has exception binop :: "
				<< cur_has_excep_ins->to_string(false) << endl;
	}

	//another ins is unop
	//sail::Variable* cur_non_excep = asm_ins->getMthCodeH()->getRegVarEle(getRandom(), _type_map);
	sail::Variable* cur_non_excep = asm_ins->getMthCodeH()->getRegVarEle(
			getRandom(), bool_type, _type_map, false, false);
	cur_non_excep->set_alias_name(cur_non_excep->get_var_name());
	//cur_non_excep->set_type(bool_type);
	/*string cur_non_excep_str = cur_non_excep->to_string(true) + " != "
	 + cur_has_excep->to_string();*/
	il::string_const_exp* original_4 = new il::string_const_exp("", bool_type,
			lo);
	sail::Unop* cur_non_excep_ins = new sail::Unop(cur_non_excep, cur_has_excep,
			ut, original_4, line);
	_sail_ins->push_back(cur_non_excep_ins);
	if (SAIL_INS_PRINT) {
		cout << "test cur is not exception Unop pp :: "
				<< cur_non_excep_ins->to_string(true) << endl;
		cout << "test cur is not exception Unop :: "
				<< cur_non_excep_ins->to_string(false) << endl;
	}

	sail::Label* then_label = NULL;
	bool annotation_set  = _annotation_labelled;
	sail::ExceptionReturn* e_r = NULL;
	//prepare for the then/else label
	if (!_annotation_labelled) {
		string then_label_str = "THROW_EXCEPTION_FOR_ANNOTATION";
		then_label = get_label(then_label_str);
		_label_for_annotation_throw = then_label;
		_annotation_labelled = true;
	}else
		then_label = _label_for_annotation_throw;

	sail::Branch* branch = new sail::Branch(cur_has_excep, cur_non_excep,
			then_label, next_ins_label, NULL);
	_sail_ins->push_back(branch);
	if(!annotation_set){
		_sail_ins->push_back(then_label);
		e_r = new sail::ExceptionReturn();
		_sail_ins->push_back(e_r);
	}
	_sail_ins->push_back(next_ins_label);
	if (SAIL_INS_PRINT) {
		cout << "test Branch for cur_exception pp:::" << branch->to_string(true)
				<< endl;
		cout << "test Branch for cur_exception :::" << branch->to_string(false)
				<< endl;
		if(!annotation_set){
		cout << "test Label for ANNOTION EXCEPTION pp:::"
				<< then_label->to_string(true) << endl;
		cout << "test Label for ANNOTION EXCEPTION :::"
				<< then_label->to_string(false) << endl;

		cout << "test Label for RETURN EXCEPTION pp:::" << e_r->to_string(true)
				<< endl;
		cout << "test Label for RETURN EXCEPTION :::" << e_r->to_string(false)
				<< endl;
		}
		cout << "test Label for next ins pp:::"
				<< next_ins_label->to_string(true) << endl;
		cout << "test Label for next ins :::"
				<< next_ins_label->to_string(false) << endl;
	}
}

void InstructionMapping::processTryCatch(Ins2Debug* ins_debug,
		ASMInstruction* asm_ins, unsigned int addr, unsigned int line,
		bool has_annotation) {
	//instruction inside the try blocks;
	//cout << "in " << endl;
	//cout << "asm_ins->isSelfDefinedM " << asm_ins->isSelfDefinedM() <<endl;
	//cout << "asm_ins->hasExceptions " << asm_ins->hasExceptions() << endl;
	if ((asm_ins->isSelfDefinedM()) && (asm_ins->hasExceptions())){
		//cout << "self defined method and the method itself throw exceptions " << endl;
		processTryThrows(ins_debug, asm_ins, addr,
				asm_ins->getMthCodeH()->getLatestLine(), false, has_annotation);
	}
	if (LIB_HAS_EXCEPTION) {
		if (!asm_ins->isSelfDefinedM()){
			//cout << "coming in " << endl;
			processTryThrows(ins_debug, asm_ins, addr,
					asm_ins->getMthCodeH()->getLatestLine(), true,
					has_annotation);
		}
	}
}

void InstructionMapping::processTryFinally(Ins2Debug* ins_debug,
		ASMInstruction* asm_ins, unsigned int addr, unsigned int line) {
	if ((asm_ins->isSelfDefinedM()) && (asm_ins->hasExceptions()))
		processTryFinallyExceptions(ins_debug, asm_ins, addr,
				asm_ins->getMthCodeH()->getLatestLine(), false);
	if (LIB_HAS_EXCEPTION) {
		if (!asm_ins->isSelfDefinedM())
			processTryFinallyExceptions(ins_debug, asm_ins, addr,
					asm_ins->getMthCodeH()->getLatestLine(), true);
	}

}

//process instruction inside try-block and only has finally
void InstructionMapping::processTryFinallyExceptions(Ins2Debug* ins_debug,
		ASMInstruction* asm_ins, unsigned int addr, unsigned int line,
		bool is_lib) {
	il::location lo(line, line);
	il::type* bool_type = il::get_integer_type();
	il::binop_type bt_e = il::_EQ;
	il::unop_type ut = il::_LOGICAL_NOT;

	sail::Constant* constant = new sail::Constant(1, false, 32);
	//prepare the without exception label; go directly to the next ins;
	unsigned int nextaddr = addr + 3 * 2; //35c or 3rc, the next address of functioncall
	string else_label_str = "con_" + Int2Str(nextaddr);
	sail::Label* next_ins_label = get_label(else_label_str);

	if (is_lib) {
		//temp1 = LIB_HAS_EXCEPTION ==1
		sail::Variable* lib_is_excep = asm_ins->getMthCodeH()->getRegVarEle(
				getRandom(), bool_type, _type_map, false, false);
		lib_is_excep->set_alias_name(lib_is_excep->get_var_name());

		il::string_const_exp * original_1 = new il::string_const_exp("",
				bool_type, lo);
		sail::Binop* lib_is_excep_ins = new sail::Binop(lib_is_excep,
				ins_debug->lib_has_exception_var(), constant, bt_e, original_1,
				line);
		_sail_ins->push_back(lib_is_excep_ins);

		if (SAIL_INS_PRINT) {
			cout << "test library has exception binop pp :: "
					<< lib_is_excep_ins->to_string(true) << endl;
			cout << "test library has exception binop :: "
					<< lib_is_excep_ins->to_string(false) << endl;
		}

		//temp2 != temp1;
		sail::Variable* lib_no_exception = asm_ins->getMthCodeH()->getRegVarEle(
				getRandom(), bool_type, _type_map, false, false);
		lib_no_exception->set_alias_name(lib_no_exception->get_var_name());
		il::string_const_exp * original2 = new il::string_const_exp("",
				bool_type, lo);
		sail::Unop* lib_no_excep_ins = new sail::Unop(lib_no_exception,
				lib_is_excep, ut, original2, line);
		_sail_ins->push_back(lib_no_excep_ins);
		if (SAIL_INS_PRINT) {
			cout << "test library has no exception unop pp :: "
					<< lib_no_excep_ins->to_string(true) << endl;
			cout << "test library has no exception unop :: "
					<< lib_no_excep_ins->to_string(false) << endl;
		}
		//prepare for has  cur_exception label
		string then_label_str = "check CUR_EXCEPTION for "
				+ lib_is_excep->to_string();
		sail::Label* then_label = get_label(then_label_str);
		//sail::Label* next_ins_label = get_label(else_label_str);//goto next ins
		sail::Branch* lib_con_ins_branch = new sail::Branch(lib_is_excep,
				lib_no_exception, then_label, next_ins_label, NULL);
		_sail_ins->push_back(lib_con_ins_branch);
		_sail_ins->push_back(then_label);
		if (SAIL_INS_PRINT) {
			cout << "test library is exception branch pp :: "
					<< lib_con_ins_branch->to_string(true) << endl;
			cout << "test library is exception branch :: "
					<< lib_con_ins_branch->to_string(false) << endl;
			cout << "test CUR_EXCEPTION label pp" << then_label->to_string(true)
					<< endl;
			cout << "test CUR_EXCEPTION label " << then_label->to_string(false)
					<< endl;
		}
	}
	//no matter is_lib or self_defined functioncall, they would do the following
	//then prepare temp1 = CUR_EXCEPTION==1 and temp2 != temp1
	sail::Variable* cur_has_excep = asm_ins->getMthCodeH()->getRegVarEle(
			getRandom(), bool_type, _type_map, false, false);
	cur_has_excep->set_alias_name(cur_has_excep->get_var_name());

	il::string_const_exp * original_3 = new il::string_const_exp("", bool_type,
			lo);
	sail::Binop* cur_has_excep_ins = new sail::Binop(cur_has_excep,
			ins_debug->cur_exception_var(), constant, bt_e, original_3, line);
	_sail_ins->push_back(cur_has_excep_ins);
	if (SAIL_INS_PRINT) {
		cout << "test cur has exception binop pp :: "
				<< cur_has_excep_ins->to_string(true) << endl;
		cout << "test cur has exception binop :: "
				<< cur_has_excep_ins->to_string(false) << endl;
	}

	//another ins is unop
	sail::Variable* cur_non_excep = asm_ins->getMthCodeH()->getRegVarEle(
			getRandom(), bool_type, _type_map, false, false);
	cur_non_excep->set_alias_name(cur_non_excep->get_var_name());
	il::string_const_exp* original_4 = new il::string_const_exp("", bool_type,
			lo);
	sail::Unop* cur_non_excep_ins = new sail::Unop(cur_non_excep, cur_has_excep,
			ut, original_4, line);
	_sail_ins->push_back(cur_non_excep_ins);
	if (SAIL_INS_PRINT) {
		cout << "test cur isn't exception unop pp :: "
				<< cur_non_excep_ins->to_string(true) << endl;
		cout << "test cur isn't exception unop :: "
				<< cur_non_excep_ins->to_string(false) << endl;
	}

	asm_ins->setOwnTry(addr);
	//Tries* tries = asm_ins->getOwnTry();
	vector<unsigned int>* catch_addrs = asm_ins->getCatchAddrs(addr);
	assert(catch_addrs->size() == 1);
	//cout << "catch_addrs finally addr " << catch_addrs->at(0) << endl;

	string catchall_label_str = "catchall_" + Int2Str(catch_addrs->at(0));
	sail::Label* catchall_label = get_label(catchall_label_str);

	//branch used to go to finally or next ins
	sail::Branch* cur_excep_con_ins_branch = new sail::Branch(cur_non_excep,
			cur_has_excep, next_ins_label, catchall_label, NULL);
	_sail_ins->push_back(cur_excep_con_ins_branch);
	if (SAIL_INS_PRINT) {
		cout << "test cur_exception branch pp :: "
				<< cur_excep_con_ins_branch->to_string(true) << endl;
		cout << "test cur_exception branch  :: "
				<< cur_excep_con_ins_branch->to_string(false) << endl;
	}

	_sail_ins->push_back(next_ins_label);
}

void InstructionMapping::processTryThrows(Ins2Debug* ins_debug,
		ASMInstruction* asm_ins, unsigned int addr, unsigned int line,
		bool is_lib, bool has_annotation) {
	il::location lo(line, line);
	il::type* bool_type = il::get_integer_type();
	il::binop_type bt_e = il::_EQ;
	il::binop_type bt_o = il::_LOGICAL_OR;
	//il::binop_type bt_a = il::_LOGICAL_AND;
	il::unop_type ut = il::_LOGICAL_NOT;
	//common things prepare
	sail::Constant* constant = new sail::Constant(1, false, 32);
	unsigned int nextaddr = addr + 3 * 2; //35c or 3rc
	string else_label_str = "con_" + Int2Str(nextaddr);
	sail::Label* next_ins_label = get_label(else_label_str); //goto next ins

	//first process is_lib or not
	if (is_lib) {
		sail::Variable* lib_is_excep = asm_ins->getMthCodeH()->getRegVarEle(
				getRandom(), bool_type, _type_map, false, false);
		lib_is_excep->set_alias_name(lib_is_excep->get_var_name());
		//lib_is_excep->set_type(bool_type);
		string lib_is_excep_str = lib_is_excep->to_string(true) + " = "
				+ ins_debug->lib_has_exception_var()->to_string(true)
				+ il::binop_expression::binop_to_string(bt_e)
				+ constant->to_string(true);
		il::string_const_exp * original_1 = new il::string_const_exp(
				lib_is_excep_str, bool_type, lo);
		sail::Binop* lib_is_excep_ins = new sail::Binop(lib_is_excep,
				ins_debug->lib_has_exception_var(), constant, bt_e, original_1,
				line);
		_sail_ins->push_back(lib_is_excep_ins);
		if (SAIL_INS_PRINT) {
			cout << "test library has exception binop pp :: "
					<< lib_is_excep_ins->to_string(true) << endl;
			cout << "test library has exception binop :: "
					<< lib_is_excep_ins->to_string(false) << endl;
		}

		//another ins is unop
		//sail::Variable* lib_non_excep = asm_ins->getMthCodeH()->getRegVarEle(getRandom(), _type_map);
		sail::Variable* lib_non_excep = asm_ins->getMthCodeH()->getRegVarEle(
				getRandom(), bool_type, _type_map, false, false);
		lib_non_excep->set_alias_name(lib_non_excep->get_var_name());
		string lib_non_excep_str = lib_non_excep->to_string(true) + " != "
				+ lib_is_excep->to_string();
		il::string_const_exp* original_2 = new il::string_const_exp(
				lib_non_excep_str, bool_type, lo);
		sail::Unop* lib_non_excep_ins = new sail::Unop(lib_non_excep,
				lib_is_excep, ut, original_2, line);
		_sail_ins->push_back(lib_non_excep_ins);
		if (SAIL_INS_PRINT) {
			cout << "test library hasn't exception unop pp :: "
					<< lib_non_excep_ins->to_string(true) << endl;
			cout << "test library hasn't exception unop :: "
					<< lib_non_excep_ins->to_string(false) << endl;
		}

		//now prepare for the branch for those two variables
		//1) prepare for the then/else label
		string then_label_str = "check CUR_EXCEPTION for "
				+ lib_is_excep->to_string();
		//string else_label_str = "con_" + Int2Str(nextaddr);
		sail::Label* then_label = get_label(then_label_str);
		//sail::Label* next_ins_label = get_label(else_label_str);//goto next ins
		sail::Branch* lib_con_ins_branch = new sail::Branch(lib_is_excep,
				lib_non_excep, then_label, next_ins_label, NULL);
		_sail_ins->push_back(lib_con_ins_branch);
		_sail_ins->push_back(then_label);
		if (SAIL_INS_PRINT) {
			cout << "test library is exception branch pp :: "
					<< lib_con_ins_branch->to_string(true) << endl;
			cout << "test library is exception branch :: "
					<< lib_con_ins_branch->to_string(false) << endl;
			cout << "test CUR_EXCEPTION label pp" << then_label->to_string(true)
					<< endl;
			cout << "test CUR_EXCEPTION label " << then_label->to_string(false)
					<< endl;
		}
	}
	//next process cur_exception instructions and its branch
	//
	//sail::Variable* cur_has_excep = asm_ins->getMthCodeH()->getRegVarEle(getRandom(), _type_map);
	sail::Variable* cur_has_excep = asm_ins->getMthCodeH()->getRegVarEle(
			getRandom(), bool_type, _type_map, false, false);
	cur_has_excep->set_alias_name(cur_has_excep->get_var_name());
	//cur_has_excep->set_type(bool_type);
	string cur_has_excep_str = cur_has_excep->to_string(true) + " = "
			+ ins_debug->cur_exception_var()->to_string(true)
			+ il::binop_expression::binop_to_string(bt_e)
			+ constant->to_string(true);
	il::string_const_exp * original_3 = new il::string_const_exp(
			cur_has_excep_str, bool_type, lo);
	sail::Binop* cur_has_excep_ins = new sail::Binop(cur_has_excep,
			ins_debug->cur_exception_var(), constant, bt_e, original_3, line);
	_sail_ins->push_back(cur_has_excep_ins);
	if (SAIL_INS_PRINT) {
		cout << "test cur has exception binop pp :: "
				<< cur_has_excep_ins->to_string(true) << endl;
		cout << "test cur has exception binop :: "
				<< cur_has_excep_ins->to_string(false) << endl;
	}

	//another ins is unop
	//sail::Variable* cur_non_excep = asm_ins->getMthCodeH()->getRegVarEle(getRandom(), _type_map);
	sail::Variable* cur_non_excep = asm_ins->getMthCodeH()->getRegVarEle(
			getRandom(), bool_type, _type_map, false, false);
	cur_non_excep->set_alias_name(cur_non_excep->get_var_name());
	//cur_non_excep->set_type(bool_type);
	string cur_non_excep_str = cur_non_excep->to_string(true) + " != "
			+ cur_has_excep->to_string();
	il::string_const_exp* original_4 = new il::string_const_exp(
			cur_non_excep_str, bool_type, lo);
	sail::Unop* cur_non_excep_ins = new sail::Unop(cur_non_excep, cur_has_excep,
			ut, original_4, line);
	_sail_ins->push_back(cur_non_excep_ins);
	if (SAIL_INS_PRINT) {
		cout << "test cur isn't exception binop pp :: "
				<< cur_non_excep_ins->to_string(true) << endl;
		cout << "test cur isn't exception binop :: "
				<< cur_non_excep_ins->to_string(false) << endl;
	}

	//now prepare for the branch for those two variables
	//1) prepare for the then/else label
	string then_label_str = "check TYPE_OF_EXCEPTION for "
			+ cur_has_excep->to_string();
	//string else_label_str = "con_" + Int2Str(nextaddr);
	sail::Label* then_label = get_label(then_label_str);
	//sail::Label* next_ins_label = get_label(else_label_str);//goto next ins
	sail::Branch* cur_con_ins_branch = new sail::Branch(cur_has_excep,
			cur_non_excep, then_label, next_ins_label, NULL);
	_sail_ins->push_back(cur_con_ins_branch);
	_sail_ins->push_back(then_label);
	if (SAIL_INS_PRINT) {
		cout << "test library is exception branch pp :: "
				<< cur_con_ins_branch->to_string(true) << endl;
		cout << "test library is exception branch :: "
				<< cur_con_ins_branch->to_string(false) << endl;
		cout << "test TYPE_OF_EXCEPTION label pp" << then_label->to_string(true)
				<< endl;
		cout << "test TYPE_OF_EXCEPTION label " << then_label->to_string(false)
				<< endl;
	}

	//process different types of exceptions for each catch block
	//////////////////////////////////
	sail::Variable* type_var = ins_debug->type_of_exception_var();
	type_var->set_alias_name(type_var->get_var_name());
	il::type* excp_t = NULL;

	if(!GLOBAL_MAP)
		excp_t = getType(0, _dfr,
			new ClassDefInfo("Ljava/lang/Exception;"), _type_map);
	else
		excp_t = getType(0, _dfr, _dfr->getClassDefByName("Ljava/lang/Exception;"), _type_map);
	type_var->set_type(excp_t);
	//prepare for the different type's exception
	asm_ins->setOwnTry(addr);
	Tries* tries = asm_ins->getOwnTry();
	vector<il::type*>* exception_types = asm_ins->getCatchExceptionType(addr,
			_dfr, _type_map);
	vector<unsigned int>* catch_addrs = asm_ins->getCatchAddrs(addr); //////////////////////////////////////////combine with instance_var_list
	assert(exception_types->size() != 0);
	//cout << "size of exception_types  " << exception_types->size()<<endl;
	vector<sail::Symbol*>* instance_var_list = new vector<sail::Symbol*>();
	for (unsigned int i = 0; i < exception_types->size(); i++) //contain catchall No type
			{
		if (exception_types->at(i) != NULL) {
			//cout << "E " << exception_types->at(i)->to_string() << endl;
			il::type* cur_t = exception_types->at(i);
			type_var->set_type(cur_t);
			//sail::Variable* con_var_0 = asm_ins->getMthCodeH()->getRegVarEle(getRandom(),_type_map);
			sail::Variable* con_var_0 = asm_ins->getMthCodeH()->getRegVarEle(
					getRandom(), cur_t, _type_map, false, false);
			con_var_0->set_alias_name(con_var_0->get_var_name());
			//con_var_0->set_type(cur_t);
			instance_var_list->push_back(con_var_0); //this is prepared for the vector<pair<Symbol*, Label*>>* target (the symbols)
			string instanceof_ins_str = con_var_0->to_string(true) + " = "
					+ "InstanceOf(" + type_var->to_string(true) + ","
					+ cur_t->to_string() + ")";
			il::string_const_exp* original = new il::string_const_exp(
					instanceof_ins_str, cur_t, lo);
			sail::InstanceOf* instanceof = new sail::InstanceOf(con_var_0,
					type_var, cur_t, original, line);
			_sail_ins->push_back(instanceof);
			if (SAIL_INS_PRINT) {
				cout << "test instanceof pp :: " << instanceof->to_string(true)
						<< endl;
				cout << "test instanceof :: " << instanceof->to_string(false)
						<< endl;
			}
		} else if(0)
			cout << "E has no type, which is catchall" << endl;
	}
	if (0)
		cout << "HHHZHU :::::::::::::: first check instance_var_list size "
			<< instance_var_list->size() << endl;
	//process catchcall if possible
	if ((tries->hasFinally()) || (has_annotation)) { //prepare for catchall
		sail::Variable* var = NULL;
		sail::Variable* pre_var = NULL;
		sail::Variable* temp_var = NULL;
		if (instance_var_list->size() > 1) {
			for (unsigned int j = 0; j < instance_var_list->size(); j++) {
				if (j != 0) {
					sail::Variable* cur_var =
							(sail::Variable*) instance_var_list->at(j);
					temp_var = asm_ins->getMthCodeH()->getRegVarEle(getRandom(),
							bool_type, _type_map, false, false);
					temp_var->set_alias_name(temp_var->get_var_name());

					//temp_var = pre_var or cur_var;
					il::string_const_exp* original = new il::string_const_exp(
							"", bool_type, lo);
					sail::Binop* temp_binop = new sail::Binop(temp_var, pre_var,
							cur_var, bt_o, original, line);
					if (SAIL_INS_PRINT) {
						cout << "test temp binop for catchall pp :: "
								<< temp_binop->to_string(true) << endl;
						cout << "test temp binop for catchall :: "
								<< temp_binop->to_string(false) << endl;
					}
					_sail_ins->push_back(temp_binop);
					pre_var = temp_var;

				} else if (j == 0) {//this is the first time
					pre_var = (sail::Variable*) instance_var_list->at(0);
				}
			}
			var = pre_var;
		} else {
			var = (sail::Variable*) instance_var_list->at(0);
		}
		//prepare for the catchall variable(not equals to any of the exception type)
		//sail::Variable* catchall_var = asm_ins->getMthCodeH()->getRegVarEle(getRandom(), _type_map);
		sail::Variable* catchall_var = asm_ins->getMthCodeH()->getRegVarEle(
				getRandom(), bool_type, _type_map, false, false);
		catchall_var->set_alias_name(catchall_var->get_var_name());

		instance_var_list->push_back(catchall_var);
		if (0)
			cout << "HHHZHU :::::::::::::: second check instance_var_list size "
					<< instance_var_list->size() << endl;

		il::string_const_exp* original_5 = new il::string_const_exp("",
				bool_type, lo);
		sail::Unop* catchall_unop_ins = new sail::Unop(catchall_var, var, ut,
				original_5, line);
		_sail_ins->push_back(catchall_unop_ins);
		if (SAIL_INS_PRINT) {
			cout << "test unop for catchall pp :: "
					<< catchall_unop_ins->to_string(true) << endl;
			cout << "test unop for catchall :: "
					<< catchall_unop_ins->to_string(false) << endl;
		}
	} else {
		if (exception_types->size() == 1)
			_sail_ins->pop_back();
	}

	vector<sail::Label*>* l_list = createLabelListForCatch(catch_addrs,
			tries->hasFinally(), has_annotation);
	if(SAIL_INS_PRINT){
		cout << "l_list size " << l_list->size() << endl;
		cout << "instance_var_list size " << instance_var_list->size() << endl;
		for (unsigned int i = 0; i < l_list->size(); i++) {
			cout << "l_list ------>>" << l_list->at(i)->to_string() << endl;
		}
		for (unsigned int i = 0; i < instance_var_list->size(); i++) {
			cout << "instance_var_list " << instance_var_list->at(i)->to_string()
				<< endl;
		}
	}

	assert(l_list->size() == instance_var_list->size());
	vector<pair<sail::Symbol*, sail::Label*>>* targets = createTargets(
			instance_var_list, l_list);
	if (targets->size() > 1) {
		sail::Branch* branch = new sail::Branch(targets, NULL);
		_sail_ins->push_back(branch);
		if (SAIL_INS_PRINT) {
			cout << "test catch branch pp :: " << branch->to_string(true)
					<< endl;
			cout << "test catch branch :: " << branch->to_string(false) << endl;
		}
	} else {
		sail::Jump* jump = new sail::Jump(targets->at(0).second, NULL);
		_sail_ins->push_back(jump);
		if (SAIL_INS_PRINT) {
			cout << "test catch jump pp :: " << jump->to_string(true) << endl;
			cout << "test catch jump :: " << jump->to_string(false) << endl;
		}
	}
	if (has_annotation) {
		sail::Label* throw_label = targets->at(targets->size() - 1).second;
		_sail_ins->push_back(throw_label);
		sail::ExceptionReturn* e_r = new sail::ExceptionReturn();
		_sail_ins->push_back(e_r);
		if (SAIL_INS_PRINT) {
			cout << "exception return pp :: " << e_r->to_string(true) << endl;
			cout << "exception return :: " << e_r->to_string(false) << endl;
		}
	}

	_sail_ins->push_back(next_ins_label);
}

vector<sail::Label*>* InstructionMapping::createLabelListForCatch(
		vector<unsigned int>* addr_list, bool has_finally,
		bool has_annotation) {
	vector<sail::Label*>* labels = new vector<sail::Label*>();
	if (EXCEPTION_LABEL_PRINT)
		cout << "size of addr_list " << addr_list->size() << endl; //possible that there is one less addr than instance var
	if ((!has_finally) && (has_annotation)) {
		extendCatchAddrs(addr_list);
	}
	for (unsigned int i = 0; i < addr_list->size(); i++) {
		if (i == addr_list->size() - 1) {
			if (has_finally) {
				string catchall_str = "catchall_" + Int2Str(addr_list->at(i));
				sail::Label* last_l = get_label(catchall_str);
				labels->push_back(last_l);
			} else if (has_annotation) {
				string throw_str = "Annotation Exception";
				sail::Label* throw_label = get_label(throw_str);
				labels->push_back(throw_label);
			} else if (!(has_finally || has_annotation)) {
				string catch_str = "catch_" + Int2Str(addr_list->at(i));
				sail::Label* l = get_label(catch_str);
				labels->push_back(l);
			}
		} else {
			string catch_str = "catch_" + Int2Str(addr_list->at(i));
			sail::Label* l = get_label(catch_str);
			labels->push_back(l);
		}
	}
	if (EXCEPTION_LABEL_PRINT) {
		cout << "size of labels : " << labels->size() << endl;
		for (unsigned int i = 0; i < labels->size(); i++) {
			cout << "label : " << labels->at(i)->to_string() << endl;
		}

	}
	return labels;
}

vector<pair<sail::Symbol*, sail::Label*>>* InstructionMapping::createTargets(
		vector<sail::Symbol*>* symbols, vector<sail::Label*>* labels) {
	vector<pair<sail::Symbol*, sail::Label*>>* targets = new vector<
			pair<sail::Symbol*, sail::Label*>>();
	if (EXCEPTION_LABEL_PRINT) {
		cout << "symbols->size " << symbols->size() << endl;
		cout << "labels->size " << labels->size() << endl;
	}
	assert(symbols->size() == labels->size());
	unsigned int size = symbols->size();
	for (unsigned int i = 0; i < size; i++) {
		sail::Symbol* sym = symbols->at(i);
		sail::Label* l = labels->at(i);
		pair<sail::Symbol*, sail::Label*> p(sym, l);
		targets->push_back(p);
	}
	return targets;
}

void InstructionMapping::extendCatchAddrs(vector<unsigned int>* orig) {
	orig->push_back(ANNOTATION_ADDR);
}


//for the purpose of line number`
void InstructionMapping::updateSailInstruction() {
	vector<sail::Instruction*>::reverse_iterator rit = _sail_ins->rbegin();
	sail::Instruction* cur_ins = NULL;
	for (; rit != _sail_ins->rend(); rit++) {
		cur_ins = *rit;
		int cur_line_num = cur_ins->get_line_number();
		if (cur_line_num != -1) {
			if (cur_line_num != _backward_latest_line)
				_backward_latest_line = cur_line_num;
		} else {
			cur_ins->set_line_number(_backward_latest_line);
		}
	}
}

//if va = va +1; need to translate to t = va; va = t +1; they designed this because they want to process one edge once in memory graph(no matter if it is invalid type)
void InstructionMapping::processInvariant(sail::Variable* in_def,
		sail::Variable* in_use, vector<sail::Symbol*>* in_uses,
		unsigned int startline) {
	sail::Variable* temp_var = NULL;
	_tempvar_for_invariant = in_use;
	if (in_use != NULL) { //in_def = in_use
		if (in_def->get_var_name() == in_use->get_var_name()) {
			temp_var = in_use->clone();
			temp_var->set_var_name_as_temp_name();
			if(temp_var->is_argument()){
				temp_var->set_as_local();
			}
			string ins_str = "";
			il::string_const_exp* temp = new il::string_const_exp(ins_str,
					temp_var->get_type(), il::location(startline, startline));
			sail::Assignment* inst = new sail::Assignment(temp_var, in_use,
					temp, startline);
			if (0) {
				cout << "variable == " << temp_var->to_string()
						<< "  || type == " << temp_var->get_type()->to_string()
						<< endl;
				cout << "variable == " << in_use->to_string() << "  || type =="
						<< in_use->get_type()->to_string() << endl;
			}
			if (SAIL_INS_PRINT) {
				cout << "[process Invariant] test adding assignment  PP::::"
						<< inst->to_string(true) << endl;
				cout << "[process Invariant] test adding assignment  "
						<< inst->to_string(false) << endl;
			}
			_sail_ins->push_back(inst);
			_tempvar_for_invariant = temp_var;
		} //else
	} else { //need to find the variables that equais to in_def, functioncall
		assert(in_uses != NULL);
		//cout << "in_uses size " << in_uses->size() << endl;
		if (in_def == NULL)
			return;
		for (unsigned int i = 0; i < in_uses->size(); i++) {
			sail::Variable* in_use_var = (sail::Variable*) (in_uses->at(i));
			assert(in_use_var != NULL);
			//if (in_def->to_string() == in_use_var->to_string()) {
			if (in_def->get_var_name() == in_use_var->get_var_name()) {
				temp_var = in_use_var->clone();
				temp_var->set_var_name_as_temp_name();
				if(temp_var->is_argument())
					temp_var->set_as_local();
				string ins_str = "";
				il::string_const_exp* temp = new il::string_const_exp(ins_str,
						temp_var->get_type(),
						il::location(startline, startline));
				sail::Assignment* inst = new sail::Assignment(temp_var,
						in_use_var, temp, startline);
				_sail_ins->push_back(inst);
				if (0) {
					cout << "variable == " << temp_var->to_string()
							<< "  || type == "
							<< temp_var->get_type()->to_string() << endl;
					cout << "variable == " << in_use_var->to_string()
							<< "  || type =="
							<< in_use_var->get_type()->to_string() << endl;
				}
				if (SAIL_INS_PRINT) {
					cout << "test adding assignment  PP::::"
							<< inst->to_string(true) << endl;
					cout << "test adding assignment  " << inst->to_string(false)
							<< endl;
				}
				//in_use_var = temp_var;
				//pay attention, I should double check it to see if it changes as expected.
				in_uses->at(i) = temp_var;

				break;
			}
		}
	}
}


void InstructionMapping::initFields(vector<sail::Instruction*>* init_ins) {
	//cout << "The begin of init Fields init-------------------- " << endl;
	for (unsigned int i = 0; i < init_ins->size(); i++) {
		_sail_ins->push_back(init_ins->at(i));
	}
	//cout << "The end of init Fields init-------------------- " << endl;
}

//call in all instructions
void InstructionMapping::updateAnotherBranchReturnType(unsigned int addr,
		unsigned int reg_num, il::type* type, sail::Variable* var) {
	if ((_return_ins_addr == 0) || (!_return_ins_has_label)
			|| (_fun_declared_return_var == NULL))
		return;
	if (addr > _return_ins_addr) //have the possibility to update the
			{
		if (_fun_declared_return_var->first == reg_num) { //find the same register
				_different_branch_return_type = type;
				_different_branch_return_var = var;
				if(0)
					cout << "HZHU +++++++++++ updating return var !!!!!!!!!!! "
						<< var->to_string() << endl;
		}
	}
}

//call in goto ins;
void InstructionMapping::addingOneAssignmentBeforeReturn(unsigned int jump_addr,
		unsigned int line) {
	if ((this->_return_ins_addr == 0) || (!this->_return_ins_has_label)
			|| (_fun_declared_return_var == NULL))
		return;
	if (jump_addr == this->_return_ins_addr) //the jump to address equals to the return_ins_addr
			{
				//if only if there some branch also contains the same variable as return variable and _differnet_branch_return_var != NULL can gurantee this point.
				if(_different_branch_return_var != NULL){
				assert(_return_ins!= NULL);
				sail::Variable* rhs = (sail::Variable*) _return_ins->get_rhs();
				assert(rhs != NULL);
				if(rhs == _different_branch_return_var)
					return;
				//adding an assignement
				il::string_const_exp* temp = new il::string_const_exp("",
						rhs->get_type(), il::location(line, line));
				sail::Assignment* added_assi = new sail::Assignment(rhs,
						_different_branch_return_var, temp, line);
				if (SAIL_INS_PRINT) {
					cout << "test adding assignment before return PP::::"
							<< added_assi->to_string(true) << endl;
					cout << "test adding assignment before return "
							<< added_assi->to_string(false) << endl;
				}
				_sail_ins->push_back(added_assi);
				}
	}

}
