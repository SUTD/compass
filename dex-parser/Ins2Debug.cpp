#include "Ins2Debug.h"
#define METHOD_CHECK false
#define LABEL_CHECK false
#define INS_CHECK false
//#define EXCEPTION_CHECK false
#define MONITOR_CHECK false
#define PRINT_TRY_ADDR false
#define PRINT_DBG false
#define PRINT_REPLACE_UNSURE false
#define DEBUG_TO_LOCATE_STATIC_METHOD false
#define PARAMETER_CHECK false
#define INSTRUCTION_PROCESS_CHECK false

#define EXTEND_TRY_PRINT false


//Ins2Debug::Ins2Debug()
Ins2Debug::Ins2Debug(DataIn* datain) {
	_ins_addr = new map<unsigned int, ASMInstruction*>();
	//hzhu 4/22/2012
	_datain = datain;
	//hzhu end 4/22/2012
	_dfr = new DexFileReader(_datain);
	if(!GLOBAL_MAP){ //GLOBAL_MAP = TRUE
	_dfr->buildTypeMapping();
	_type_idx_name = _dfr->getTypeMapping();
	//12/12
	_classdef_map = str2Classdef(_dfr); // build str to ClassDef map
	}else{
		_dfr->buildGlobalStr2ClassDefMap();
		_classdef_map = _dfr->getStr2ClassDef();
	}

	//12/15
	cd = DexFileReader::getCodeHeaderList(_dfr);
	//12/15
	//change the way of build _classdef_map, to see if it works

	_lib_has_exception = new sail::Variable("LIB_HAS_EXCEPTION",
			il::base_type::make("bool", 8, 8, false, il::BOOL, "BOOLEAN"),
			true, false, -1, true);
	_type_of_exception = new sail::Variable("EXCEPTION_TYPE_VAR",
			il::get_integer_type(), true, false, -1, true);
	_cur_exception = new sail::Variable("CUR_EXCEPTIONS",
			il::base_type::make("bool", 8, 8, false, il::BOOL, "BOOLEAN"),
			true, false, -1, true);
	_throw_exception = new sail::Variable("THROW_EXCEPTION", il::get_integer_type(), true, false, -1, true);

	//haiyan comment out 8.29 begin, this step is not necessary for static method check
	//_static_midx = new set<unsigned int>();
	//haiyan comment out 8.29 end, this step is not necessary for static method check
	//if(!USING_CLINIT)
	//	_global_var_ins = new map<string, ASMInstruction*>(); //to make sure same instruction would be included more than once
//	_class_field_init_ins = new map<ClassDefInfo*, vector<sail::Instruction*>*>();

	//1/5
	if(USING_CLINIT){
		//_lib_global_initialization = new vector<sail::Instruction*>();
		//_init_static_0 = new sail::Function();
	}
	if(USING_STATIC_VALUE){
		_global_variables = new map<string, sail::Variable*>();
		_fun_initi_static_fields = new vector<sail::Instruction*>();
		_init_statics = new sail::Function();
	}
}

//hzhu added 4/23/2012
vector<ClassDefInfo*>* Ins2Debug::localParaType(Method* method) {
	vector<ClassDefInfo*>* localtypes = new vector<ClassDefInfo*>();
	vector<unsigned int>* localtypeids = method->getParaTypeIds();
	vector<unsigned int>::iterator it;
	unsigned int typeidx;
	ClassDefInfo* type = NULL;
	if (localtypeids != NULL) {
		for (it = localtypeids->begin(); it != localtypeids->end(); it++) {
			typeidx = (*it);
			//haiyan changed the following by calling the  type2ClassDef();
			type = type2ClassDef(_dfr, typeidx);
			/*
			 if(typeid2Classid->count(typeidx) > 0) //can found inside the ClassDefInfo list, return its corresponding ClassDefInfo
			 {
			 unsigned int classtypeid = (*typeid2Classid)[typeidx];
			 type = _dfr->getClassDefInfo(classtypeid);
			 }
			 else //new a ClassDefInfo using its type name
			 {
			 type = new ClassDefInfo(_dfr->getType(typeidx));
			 }
			 */
			localtypes->push_back(type);

		}
	}
	return localtypes;
}
//hzhu end 4/23/2012

void Ins2Debug::updateDeclaredVarMap(unsigned int addr,
		map<unsigned int, vector<RVT*>*>* rvt,
		map<unsigned int, unsigned int>* linemap,
		map<unsigned int, unsigned int>* endmap,
		map<unsigned int, ClassDefInfo*>* declaredvar,
		map<unsigned int, ClassDefInfo*>* backup, //vector<unsigned int>& insAddr,
		vector<struct AddrItem>& insAddr,
		const int insAddridx)

		{
	unsigned int insAddr_idx = insAddridx;
	unsigned int nextOneAddr = 0; // only record the next one instruction addr
	unsigned int curAddr = 0;
	if (insAddr_idx + 1 < insAddr.size()) {
		curAddr = insAddr[insAddr_idx].__address;
		nextOneAddr = insAddr[insAddr_idx + 1].__address;
		//1. lookinto end map to find the killing regs number
		if (endmap->count(nextOneAddr) > 0) { //find current addr
			unsigned int reg = (*endmap)[nextOneAddr];

			//cout << "HZHU: killing register: " << reg << endl;
			assert(declaredvar->count(reg) > 0);
			// has to exist before kill
			//haiyan ignore the assertion first 10.7
			/*
			 * .method protected getCurrentPromptHelper()Lorg/connectbot/service/PromptHelper;
			 .locals 2

			 .prologue
			 .line 252
			 const v1, 0x7f0b0004

			 invoke-virtual {p0, v1}, Lorg/connectbot/ConsoleActivity;->findCurrentView(I)Landroid/view/View;

			 move-result-object v0

			 .line 253
			 .local v0, view:Landroid/view/View;
			 instance-of v1, v0, Lorg/connectbot/TerminalView;

			 if-nez v1, :cond_0

			 const/4 v1, 0x0

			 .line 254
			 .end local v0           #view:Landroid/view/View;
			 :goto_0
			 return-object v1

			 .restart local v0       #view:Landroid/view/View;
			 :cond_0
			 check-cast v0, Lorg/connectbot/TerminalView;

			 .end local v0           #view:Landroid/view/View;
			 iget-object v1, v0, Lorg/connectbot/TerminalView;->bridge:Lorg/connectbot/service/TerminalBridge;

			 iget-object v1, v1, Lorg/connectbot/service/TerminalBridge;->promptHelper:Lorg/connectbot/service/PromptHelper;

			 goto :goto_0
			 .end method
			 * */

			assert(
					((*declaredvar)[reg] != NULL) || ((rvt->count(curAddr) > 0) && (OP_CHECK_CAST == insAddr[insAddr_idx].__opcode)));
			// has to exist before kill
			// erase the reg info from local var
			(*declaredvar)[reg] = NULL;
		}
	}

	//go to first instruction in next line
	unsigned int nextAddr = 0;
	bool hasCheck_cast = false;
	short regCheck_cast = -1;
	do {
		insAddr_idx++;
		if (insAddr_idx >= insAddr.size()) // over the end
			return;

		nextAddr = insAddr[insAddr_idx].__address; // next ins address
		DexOpcode nextOpcode = insAddr[insAddr_idx].__opcode;
		short nextReg = insAddr[insAddr_idx].__firstReg;

		if (OP_CHECK_CAST == nextOpcode) {
			hasCheck_cast = true;
			regCheck_cast = nextReg;
		}

		if (rvt->count(nextAddr) > 0) { // has declaration

			vector<RVT*>* vrvt = (*rvt)[nextAddr];
			for (unsigned int i = 0; i < vrvt->size(); i++) {
				RVT* rvt = vrvt->at(i);
				unsigned int reg = rvt->getReg();
				//cout << "HZHU: find declared var: " << reg << endl;
				ClassDefInfo* type = rvt->getClassType();
				bool isRestart = rvt->getRestartFlag();

				if (linemap->count(addr) == 0) {
					// current instruction do NOT have a line number
					// should be added by its previous instruction
					// but I do a assertion here
					//assert(((declaredvar->count(reg) > 0)&&((*declaredvar)[reg] != NULL)) || (endmap->count(nextOneAddr)>0));
					/*This line is to handle a very uncommon case
					 .line 1009
					 new-instance v3, Lcom/rovio/ka3d/GLSurfaceView$LogWriter;

					 #@cf
					 .end local v3           #log:Ljava/io/Writer;
					 invoke-direct {v3}, Lcom/rovio/ka3d/GLSurfaceView$LogWriter;-><init>()V

					 #@d2
					 .line 1011
					 .restart local v3       #log:Ljava/io/Writer;
					 :cond_d2
					 invoke-static {v2, v0, v3}, Landroid/opengl/GLDebugHelper;->wrap(Ljavax/microedition/khronos/opengles/GL;ILjava/io/Writer;)Ljavax/microedition/khronos/opengles/GL;

					 #@d5
					 move-result-object v2

					 #@d6
					 .end local v0           #configFlags:I
					 .end local v3           #log:Ljava/io/Writer;
					 */

					if ((declaredvar->count(reg) > 0)
							&& ((*declaredvar)[reg] == NULL)
							&& (type != NULL)) {
						//cout << "HZHU: Insering to local map - key: " << reg << ", type: " << type->getTypename() << endl;

						if ((!hasCheck_cast) || (reg != regCheck_cast))
							(*declaredvar)[reg] = type;
					}

					if (isRestart && (type == NULL)) { // restart a parameter
						ClassDefInfo* backup_type = (*backup)[reg];
						assert(backup_type != NULL);
						if ((!hasCheck_cast) || (reg != regCheck_cast))
							(*declaredvar)[reg] = backup_type;
					}
				} else {
					//cout << "HZHU: Insering to local map - key: " << reg << ", type: " << type->getTypename() << endl;
					//if ((declaredvar->count(reg) > 0) && ((*declaredvar)[reg] != NULL))
					//	cout << "HZHU: waring, reg: " << reg << " has type: " << (*declaredvar)[reg]->getTypename() << " in." << endl;
					if (type != NULL) {
						if ((!hasCheck_cast) || (reg != regCheck_cast))
							(*declaredvar)[reg] = type;
					}

					if (isRestart && (type == NULL)) { // restart a parameter
						ClassDefInfo* backup_type = (*backup)[reg];
						assert(backup_type != NULL);
						if ((!hasCheck_cast) || (reg != regCheck_cast))
							(*declaredvar)[reg] = backup_type;
					}
				}
			}
		}
	} while (linemap->count(nextAddr) == 0); // if not have a line number, continue

	//
	/*cout << "HZHU: find first ins in next line: " << (*linemap)[nextAddr] << endl;

	 if (rvt->count(nextAddr) > 0){ // has declaration
	 vector<RVT*>* vrvt = (*rvt)[nextAddr];
	 for (int i = 0; i< vrvt->size(); i++){
	 unsigned int reg = vrvt->at(i)->getReg();
	 ClassDefInfo* type = vrvt->at(i)->getClassType();
	 cout << "HZHU: Insering to local map - key: " << reg << ", type: " << type->getTypename() << endl;
	 if ((declaredvar->count(reg) > 0) && ((*declaredvar)[reg] != NULL))
	 cout << "HZHU: waring, reg: " << reg << " has type: " << (*declaredvar)[reg]->getTypename() << " in." << endl;
	 (*declaredvar)[reg] = type;
	 }
	 }*/

	return;
}

void Ins2Debug::setInsAddrMap() {
	vector<CodeHeader*>::iterator it;
	//unsigned int size = 0;
	//build the fakedlibClassDef;
	vector<pair<unsigned int, unsigned int>>* top_two_levels =
			_dfr->initialBaseClassFiledsToDerivative();
	//haiyan comment them out and add them to the constructor of ins2debug
	//cd = DexFileReader::getCodeHeaderList(_dfr); //each method has its own code header that means how many method in this dex file
	//size = cd->size(); // how many methods defined inside the dex file

	//hzhu add 4/22/2012
	//get the methodid and its address mapping

	_dfr->setMethodidx2AddrsM();
	map<unsigned int, MethodAddr*>* midx_addrs = _dfr->getMethodidx2AddrsM();
	map<unsigned int, vector<string>*>* throw_map = _dfr->getMethodThrows();
	if (EXCEPTION_CHECK) {
		if (throw_map->size() > 0) {
			cout << "number of throw exceptions " << throw_map->size() << endl;
		}
	}
	//hzhu added this label addr into the list;
	//vector<string>* label_list = NULL;
	set<string>* label_list = NULL;
	vector<Tries*>* tries = NULL;

	OpcodeFormat* opformat = NULL;
	DexOpcode dexopcode;
	InstructionProcess* insp = NULL;
	//map<unsigned int, ASMInstruction*>* method_add_ins = new map<unsigned int, ASMInstruction*>();
	unsigned int m = 0; //for print purpose
	unsigned int n = 0;

	for (it = cd->begin(); it != cd->end(); it++) {
		m++;
		if (INSTRUCTION_PROCESS_CHECK)
			cout << endl << "method === > idx " << m << endl;
		//cout << "real method idx in dex file " << (*it)->getMidx()<< endl;
		//unsigned int m = 0;
		//unsigned int* p = new unsigned int(0);
		//cout << "===== Methond: " << (*it)->getMethod()->toString() << endl;
		unsigned int real_args_size = (*it)->getRealArgsSize();
		if (INSTRUCTION_PROCESS_CHECK)
			cout << "================Inside =========== method"
					<< (*it)->getMethod()->toString() << endl;
		//cout << "total regs inside the codeheader " <<(*it)->getTotalRegisterSize()<<endl;
		//cout << "SEEEEEEEEEEEEEEEEEEEEE real args size " << real_args_size << endl;

		map<unsigned int, ASMInstruction*>* method_add_ins = new map<
				unsigned int, ASMInstruction*>();
		map<unsigned int, unsigned int>* args = (*it)->getArgs();
		vector<unsigned int>* argsl = (*it)->getArgsReg(); //get spaces to hold the acutally parameters which containing explicit this pointer
		////////////////////////////////////////////////////////////////
		//haiyan added 7.18
		vector<unsigned int>* sync_obj = (*it)->getSyncObj();
		stack<pair<unsigned int, unsigned int>>* sync_stack =
				(*it)->getSyncStack();
		vector<vector<pair<unsigned int, unsigned int>>*>* sync_addr =
				(*it)->getSyncAddr();
		//vector<pair<string, string>>* sync_catches = (*it)->getSyncCatches();
		//haiyan ended 7.18
		///////////////////////////////////////////////////////////////////
		//	cout << "Inside :: " << (*it)->getMethod()->toString()<<endl;
		//first push back the try list to the map
		label_list = (*it)->getLabelList(); // get the space
		tries = (*it)->getTries();
		if (tries != NULL) {
			vector<Tries*>::iterator tryit = tries->begin();
			for (; tryit != tries->end(); tryit++) {
				string try_start = "try_start_"
						+ Int2Str((*tryit)->getStartAddr());
				//label_list->push_back(try_start);
				label_list->insert(try_start);
				string try_end = "try_end_" + Int2Str((*tryit)->getEndAddr());
				//label_list->push_back(try_end);
				label_list->insert(try_end);
				vector<Handler*>* handler = (*tryit)->getHandlers();
				if (handler != NULL) {
					//cout << "come inside the handler" << endl;
					vector<Handler*>::iterator hit = handler->begin();
					for (; hit != handler->end(); hit++) {
						Handler* ha = *hit;
						string catch_add;
						if (ha->getTypename() != "")
							catch_add = "catch_" + Int2Str(ha->getCatchAddr());
						else
							catch_add = "catchall_"
									+ Int2Str(ha->getCatchAddr());
						label_list->insert(catch_add);
					}
				}
			}

		}
		//end by haiyan 5/3 	
		int inadd_begin = (*it)->getInstructionBeginOff();
		//cout << "method begin addr : " << inadd_begin << endl;

		//******************************
		//hzhu begin 4/23/2012
		Method* method = (*it)->getMethod();
		string flag = method->getFlag();
		bool sta = false;
		if (flag == "ACC_STATIC") {
			sta = true;
		}
		vector<ClassDefInfo*>* para_types = localParaType(method); //now we know the para_types inside the current methods for each instruction
		//now we need to initialize all local variable vector<ClassDefInfo*> inside current method

		//////////////////////////////////////////////////////////////////////
		map<unsigned int, ClassDefInfo*>* localvartype = new map<unsigned int,
				ClassDefInfo*>();
		//hzhu added 5.24
		(*it)->setLocalVarType(localvartype);
		(*it)->setTypeMap(_classdef_map);
		map<unsigned int, ClassDefInfo*>* declared_var =
				(*it)->getDeclaredVar();
		//hzhu 5.24 end
		unsigned int reg_size = (*it)->getTotalRegisterSize();
		//map<unsigned int, ClassDefInfo*>::iterator it1;
		unsigned int i = 0;
		//hzhu added 5.24 process how many registers being used by the parameters
		vector<ClassDefInfo*>::iterator para_it = para_types->begin();
		unsigned int total_para_reg = 0;
		//process if the parameter needs more register than the size of parameter
		for (; para_it != para_types->end(); para_it++) {
			if (((*para_it)->getTypename() == "D")
					|| ((*para_it)->getTypename() == "J"))
				total_para_reg++;
			total_para_reg++;
		}
		if (real_args_size > total_para_reg) {
			assert(real_args_size = total_para_reg +1);
		} else {
			sta = true;
			flag = "ACC_STATIC";
			(*it)->getMethod()->setFlag("ACC_STATIC");
		}

		//cout <<  "total_para_reg ::" << total_para_reg << endl;
		//hzhu end 5.24
		if (METHOD_CHECK) {
			cout << "<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>"
					<< endl;
			cout << method->toString() << endl;
			cout << "static ? " << sta << endl;
			cout << "total register :: " << reg_size << endl;
			cout << "total parameter register ::" << total_para_reg << endl;
		}

		while (i < reg_size) // i= 0  reg_size =2; total_para_reg =2
		{
			unsigned int k = 0;
			if ((i < reg_size - total_para_reg - 1)
					&& (reg_size > total_para_reg)) // 0 < -1 NO //reserve for this pointer position, but if reg_size == total_para_reg, no reserved this pointer
					{
				(*localvartype)[i] = NULL;
				(*declared_var)[i] = NULL;
				i++;
			} else if (flag != "ACC_STATIC") //if the method is not static, then has default P0
					{
				if ((i == reg_size - 1 - total_para_reg)
						&& (reg_size - total_para_reg > 0)) //p0 if possible, this pointer
						{
					ClassDefInfo* this_p = NULL;
					unsigned int classowneridx = method->getClassOwneridx();
					//12/15==
					if (!GLOBAL_MAP) {
						map<int, int>* typeid2Classid =
								_dfr->getClassToTypeidx(); //typeidx
						if (typeid2Classid->count(classowneridx) > 0) {
							unsigned int classdefid =
									(*typeid2Classid)[classowneridx];
							//HZHU
							//this_p = _dfr->getClassDefInfo(classdefid);
							this_p = (*_dfr->getClassDefIdx2Ele())[classdefid];
						} else //new a ClassDefInfo using its type name
						{
							//this_p = new ClassDefInfo(_dfr->getType(classowneridx));
							//using map to unify type name
							this_p = new ClassDefInfo(
									_dfr->getTypename(classowneridx));
						}
					} else {
						string t_name = _dfr->getTypename(classowneridx);
						this_p = _dfr->getClassDefByName(t_name);
					} //12/15==
					  //12/20
					this_p->setTypeIdx(classowneridx);
					//12/20
					(*localvartype)[i] = this_p; //actually it is the type of the method, we need to change it after test
					(*declared_var)[i] = this_p; //actually it is the type of the method, we need to change it after test

					if (METHOD_CHECK) {
						cout << "methodid == " << (*it)->getMidx() << endl;
						cout << "this reg: " << i << " type: "
								<< this_p->getTypename() << endl;
					}
					//add this pointer to the method
					(*args)[i] = 0;
					(*it)->setThisReg(i);
					(*it)->setThisType(this_p);
					//added by haiyan 9.13
					//(*it)->buildThisPointerVar();
					//ended by haiyan 9.13
					argsl->push_back(i);
					(*it)->setFirstArgu(i);
					i++;
				}
				//process p1-pn
				else {
					if (METHOD_CHECK)
						cout << "inside p1-pn" << endl;
					for (para_it = para_types->begin();
							para_it != para_types->end(); para_it++) {
						k++;
						(*localvartype)[i] = *para_it;
						(*declared_var)[i] = *para_it; //actually it is the type of the method, we need to change it after test
						if (METHOD_CHECK)
							cout << "reg: " << i << " type: "
									<< (*para_it)->getTypename() << endl;
						(*args)[i] = k;
						argsl->push_back(i);
						if (((*para_it)->getTypename() == "D")
								|| ((*para_it)->getTypename() == "J"))
							i++;
						i++;
					}
				}
			} else // static method no not have p0,
			{
				if (i == reg_size - total_para_reg) {
					(*it)->setFirstArgu(i);
				} else if (i == reg_size - 1 - total_para_reg) {
					(*localvartype)[i] = NULL;
					(*declared_var)[i] = NULL;
					i++;
					(*it)->setFirstArgu(i);
				}
				int k1 = -1;
				for (para_it = para_types->begin();
						para_it != para_types->end(); para_it++) {
					k1++;
					(*localvartype)[i] = *para_it;
					(*declared_var)[i] = *para_it;
					if (METHOD_CHECK)
						cout << "reg: " << i << " type: "
								<< (*para_it)->getTypename() << endl;
					(*args)[i] = k1;
					argsl->push_back(i);
					if (((*para_it)->getTypename() == "D")
							|| ((*para_it)->getTypename() == "J"))
						i++;
					i++;
				}
			}
		}

		//check parameters
		if (PARAMETER_CHECK) {
			cout << "STATIC ?????????????" << (*it)->getMethod()->isStatic()
					<< endl;
			cout << "First parameter " << (*it)->getFirstArgu() << endl;
			for (unsigned int i = 0; i < argsl->size(); i++) {
				assert(localvartype->count(argsl->at(i)) > 0);
				cout << "p " << i << "===" << argsl->at(i) << "type is "
						<< (*localvartype)[argsl->at(i)]->getTypename() << endl;
			}
		}

		//HZHU added 9.13
		string methodname = method->getName();
		bool constructor_flag = false;
		//cout << "check methodname " << methodname << endl;
		if (methodname.find("<init>") != string::npos) {
			constructor_flag = true;
		}
		//hzhu end 4/23/2012
		bool clinit_flag = false;
		if(USING_CLINIT){
			if(methodname.find("<clinit>") != string::npos){
				_unsorted_clinit_cd[n++] = *it;
				clinit_flag = true;
				(*it)->set_clinit_flag();
			}
		}

		/////////////////////////check if I update correctly

		int instr_count = (*it)->getInstructionSize();
		//cout << "||| total ins size " << instr_count << endl;
		int ins_size; // the unit of the instruction
		unsigned int min_data_addr = _dfr->getFileSize();

		map<unsigned int, vector<RVT*>*>* rvtmap = NULL;
		map<unsigned int, unsigned int>* linemap = NULL;
		map<unsigned int, unsigned int>* endmap = NULL;

		if (this->_dfr->getDebugInfoReader(*it) != NULL) {
			rvtmap = this->_dfr->getDebugInfoReader(*it)->getRVTList();

			linemap = this->_dfr->getDebugInfoReader(*it)->getAddressLine();
			endmap = (*it)->getRegKillMap();
		}

		//Here create a new amp to main only declared variable.
		map<unsigned int, ClassDefInfo*>* declaredvar = (*it)->getDeclaredVar();
		map<unsigned int, ClassDefInfo*> backupmap = (*declaredvar);

		//this loop only used to collect min_data_addr
		int temp_size = 0;
		for (int i = 0; i < instr_count; i += temp_size) {
			unsigned int addr = inadd_begin + i * 2;
			if (addr >= min_data_addr) {
				break;
			}
			DataIn* din;
			din = (*it)->getDataIn();
			din->move(addr); //go to the first instruction address
			unsigned short dexop_read = din->readByte() & 0x00ff;
			dexopcode = (DexOpcode) dexop_read; // get the opcode
			//printf("first check dexopcode::  %x \n", dexopcode);
			opformat = new OpcodeFormat((*it), dexopcode); //based on the opcode to map its format
			Format f = opformat->getFormat();
			temp_size = opformat->getSize(f); //size of current instruction
			///////////////////////
			//prepare for the operand1;
			if ((dexopcode != OP_FILL_ARRAY_DATA)
					&& (dexopcode != OP_PACKED_SWITCH)
					&& (dexopcode != OP_SPARSE_SWITCH))
				continue;
			else {
				din->skip(1); //first operand;
				//second operand;
				unsigned int rela_addr = din->readIntx();
				unsigned int meta_addr = rela_addr * 2 + addr;
				//printf("ab_addr address of content is : %.8x\n", meta_addr);
				if (min_data_addr > meta_addr) {
					min_data_addr = meta_addr;
					//cout << "first min_data_addr: " << min_data_addr << endl;
				}
			}

		}

		vector<struct AddrItem> insAddr;
		// NEED TO READ the instruction first time to build aaddress vector.
		// NOT SURE WHETHER IT WILL CHANGE THE DEX file cursor
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

		for (int i = 0; i < instr_count; i += temp_size) {
			unsigned int addr = inadd_begin + i * 2;
			if (addr >= min_data_addr) {
				break;
			}
			DataIn* din;
			din = (*it)->getDataIn();
			din->move(addr); //go to the first instruction address
			unsigned short dexop_read = din->readByte() & 0x00ff;
			dexopcode = (DexOpcode) dexop_read; // get the opcode
			short reg = -1;
			if (dexopcode == OP_CHECK_CAST) {
				reg = din->readByte();
			}
			//printf("first check dexopcode::  %x \n", dexopcode);
			opformat = new OpcodeFormat((*it), dexopcode); //based on the opcode to map its format
			Format f = opformat->getFormat();
			temp_size = opformat->getSize(f); //size of current instruction

			struct AddrItem addr_item;
			addr_item.__address = addr;
			addr_item.__opcode = dexopcode;
			addr_item.__firstReg = reg;

			insAddr.push_back(addr_item);
			delete opformat;
		}

		//assert(rvtmap != NULL);
		//assert(linemap != NULL);
		//assert(endmap != NULL);
		assert(declaredvar != NULL);
#if 0
		map<unsigned int, vector<RVT*>*>::iterator rvtmap_iter;
		cout << "HZHU: check rvtmap: " << endl;
		for (rvtmap_iter = rvtmap->begin(); rvtmap_iter != rvtmap->end(); rvtmap_iter++) {
			vector<RVT*>* vec = (*rvtmap_iter).second;
			cout << (*rvtmap_iter).first << " ! ";
			for (int i=0; i< vec->size(); i++) {
				cout << vec->at(i)->getRegName();
				if (vec->at(i)->getClassType() != NULL)
				cout << " -- " << vec->at(i)->getClassType()->getTypename() << " ! ";
			}
			cout<< endl;
		}
#endif
		//assert(insAddr.size() != 0);
		int for_declaredmap = 0;
		int ins_count = 0;



		for (int ins_idx = 0; ins_idx < instr_count; ins_idx += ins_size) {
			ins_count++;
			unsigned int addr = inadd_begin + ins_idx * 2; //wow I need it
			if (addr >= min_data_addr) {
				break;
			}
			DataIn* din;
			din = (*it)->getDataIn();
			din->move(addr); //go to the first instruction address
			//      cout << "current position: " << din->getCurrentPosition()<<endl;
			//printf("current position : %.4x \n", din->getCurrentPosition());
			unsigned short dexop_read = din->readByte() & 0x00ff;
			dexopcode = (DexOpcode) dexop_read; // get the opcode
			//printf(" second check dexopcode::  %x \n", dexopcode);

			opformat = new OpcodeFormat((*it), dexopcode); //based on the opcode to map its format
			Format f = opformat->getFormat();
			ins_size = opformat->getSize(f); //size of current instruction
			//cout << "size of ins  inside the fun " << ins_size << endl;
			// cout << "HZHU: ins  addr: " << addr <<endl;
//                                cout << "HZHU: ins  size: " << ins_size <<endl;
			// VERY INPORTANT FUNCTIOIN
			if ((rvtmap != NULL) && (linemap != NULL) && (endmap != NULL))
				updateDeclaredVarMap(addr, rvtmap, linemap, endmap, declaredvar,
						&backupmap, insAddr, for_declaredmap);
			for_declaredmap++;
			//cout << "HZHU: local map: ";
			/*map<unsigned int, ClassDefInfo*>::iterator declaredvarit;
			 for (declaredvarit = declaredvar->begin(); declaredvarit != declaredvar->end(); declaredvarit++){
			 if (declaredvarit->second != NULL)
			 cout << declaredvarit->first << "==" << declaredvarit->second->getTypename()<<", ";
			 }
			 cout << endl;
			 cout << "before calling getting instruction " << endl;*/
			if (0) {
				_dfr->getFileStream() << "before instruction processing check !"
						<< endl;
				_dfr->printClassDefFields();
				_dfr->getFileStream() << "-----------------" << endl;
			}
			insp = new InstructionProcess(_dfr, (*it), f, dexopcode, addr,
					localvartype, declaredvar); //process instruction
			ASMInstruction* asm_ins = insp->processAll();


			if (INSTRUCTION_PROCESS_CHECK)
				cout << "ASMINS ----==== :::" << asm_ins->toString() << endl;

			fakeFieldsForLibClass(asm_ins);


			if (0) {
				_dfr->getFileStream() << "after instruction processing check !"
						<< endl;
				_dfr->printClassDefFields();
				_dfr->getFileStream() << "------------------" << endl;
			}
			assert(asm_ins != NULL);
			//cout<< "after calling getting instruction !!!!!! " << asm_ins->toString()<< endl;
			unsigned int tmp_midx = asm_ins->getMethodidx();
			//if (tmp_midx != -1) // this is a funcall instruction
			if (tmp_midx != UINT_MAX) {
				if (midx_addrs->count(tmp_midx) > 0) {
					//Method* method = _dfr->getMethod(tmp_midx);
					Method* method = (*(_dfr->getMIdx2Method()))[tmp_midx];
					MethodAddr* methodaddr = (*midx_addrs)[tmp_midx];
					method->setMethodAddr(methodaddr); //update method* 4/22/2012
					asm_ins->setMethod(method); //udpate instruction* 4/22/2012
					//haiyan added 7.15
				}
			}

			assert(method_add_ins->count(addr) == 0);
			(*method_add_ins)[addr] = asm_ins; //add by hzhu 4/5/2012
			//cout << "== push to map1 " << addr << " mADDR  " << asm_ins << " == " << asm_ins->toString()<< endl;
			(*_ins_addr)[addr] = asm_ins;
			//cout << "== push to map 2" << addr << " mADDR  " << asm_ins << " == " << asm_ins->toString()<< endl;

			///////////////////////////////let us ignore it first 8.29
			//hzhu added 8.22
			if (!USING_TYPE_INFER) {
				map<unsigned int, unsigned int>* unsure_regs =
						(*it)->getConstRegToInsAddr();
				if ((dexopcode == OP_CONST) || (dexopcode == OP_CONST_4)
						|| (dexopcode == OP_CONST_16)
						|| (dexopcode == OP_CONST_HIGH16)
						|| (dexopcode == OP_CONST_WIDE_16)
						|| (dexopcode == OP_CONST_WIDE_32)
						|| (dexopcode == OP_CONST_WIDE)
						|| (dexopcode == OP_CONST_WIDE_HIGH16)) {
					assert(asm_ins->getRegs()->size() > 0);
					unsigned int reg = asm_ins->getRegs()->at(0);
					(*unsure_regs)[reg] = addr;
				}
				//hzhu ended 8.22
				else if (unsure_regs->size() > 0) //those may update the previous instructions
						{
					//cout <<"asm_ins used to update const " << asm_ins->toString() <<endl;
					vector<unsigned short>* in_def = asm_ins->getInDefRegs();
					bool update_flag = false;
					if (0) {
						if (in_def == NULL)
							cout << "in_def is NULL " << endl;
						else
							cout << "in_def size " << in_def->size() << endl;
					}

					vector<unsigned short>* in_use = asm_ins->getInUseRegs();
					if (in_use != NULL) {
						map<unsigned int, ClassDefInfo*>* ins_reg_type =
								asm_ins->getTypeofVar();
						ClassDefInfo* update_t = NULL;
						//for each instructions contains in_use registers and in_def registers;
						for (unsigned int i = 0; i < in_use->size(); i++) {
							unsigned short cur_in_use = in_use->at(i);
							//cout << "in_use " << cur_in_use << endl;

							//found current instruction in_use register also inside unsure_regs;
							if (unsure_regs->count(cur_in_use) > 0) // current in_use register is in unsure_regs list
									{
								//reg->type map should contain all the register, or 2000 if it is duplicated
								assert(ins_reg_type->count(cur_in_use) > 0);
								// it supposed to hold, acutally need use an assertion
								//first ignore reused regs with different type in same instruction;
								update_t = (*ins_reg_type)[cur_in_use]; //when get the type
								if ((cur_in_use == asm_ins->getRegs()->at(0))
										&& (ins_reg_type->count(2000) > 0))
									update_t = (*ins_reg_type)[2000];
								assert(update_t != NULL);

								//the address that contains the unsure register ;
								unsigned int unsure_addr =
										(*unsure_regs)[cur_in_use];
								assert(method_add_ins->count(unsure_addr) >0);
								//get the unsured instruction which may need update by the current instruction;
								ASMInstruction* update_ins =
										(*method_add_ins)[unsure_addr];
								//get the unsured ins's reg->type mapping
								map<unsigned int, ClassDefInfo*>* type_map =
										update_ins->getTypeofVar();
								if ((type_map->count(cur_in_use) > 0)
										&& (((*type_map)[cur_in_use]) != NULL)) {

									if (((*type_map)[cur_in_use]->getTypename()
											!= update_t->getTypename())
											&& (!update_t->isDigits())) {
										(*type_map)[cur_in_use] = update_t;
										if (0)
											cout
													<< "payattention, cur_in_use::: "
													<< cur_in_use
													<< "in instruction ::; "
													<< update_ins->toString()
													<< "          cast to "
													<< update_t->getTypename()
													<< "by ins::"
													<< asm_ins->toString()
													<< endl;
										//update_ins->setNeedCast();
									}

								}
								//delete unsure
								update_flag = true;
								unsure_regs->erase(cur_in_use);
								if (PRINT_REPLACE_UNSURE) {
									cout << "PAY ATTENTION !!! " << endl;
									cout << "Inside Method "
											<< (*it)->getMethod()->toString()
											<< endl;
									cout << "Instruction addr: " << unsure_addr
											<< "   asm== "
											<< update_ins->toString() << endl;
									cout << "replace type as "
											<< update_t->getTypename() << endl;
									cout << "By instruction inside " << addr
											<< endl;
								}
							}
						}
						delete in_use;
					} //else if((in_def != NULL)&&(in_def->size() > 0)&&(!update_flag))
					if ((in_def != NULL) && (in_def->size() > 0)
							&& (!update_flag)) {
						//cout << "--------asm ins has in_def register " << asm_ins->toString()<< endl;
						for (unsigned int i = 0; i < in_def->size(); i++) {
							unsigned int reg_in_def = in_def->at(i);
							//cout << "REG :: " << reg_in_def<< endl;
							if (unsure_regs->count(reg_in_def) > 0) //found in unsure;
									{
								unsure_regs->erase(reg_in_def); //just remove it from the unsure list;
							}
						}
						delete in_def;
					}
				}
			}
			////////////////let us first igore it first
			//haiyan end 8.29

			//hzhu added 5/3/2012
			string ope = asm_ins->getOperator();
			//check if it is a monitor/syn ins
			processSyncBlock(*it, ope, addr, asm_ins, sync_obj, sync_stack,
					sync_addr);
			//HZHU added for existing fields collecting 9.13
			if (0) {
				if (constructor_flag) //inside constructor method
				{
					if (ope.substr(0, 4) == "iput")
						cout << "collecting fields" << asm_ins->getOperand3()
								<< endl;
				}
			}
			//1/5
			if((USING_CLINIT)&&(clinit_flag)&&(ope.substr(0,4) == "sget")){
				//get class name
				//string mm = (*it)->getMethod()->toString();
				//cout << "methodname " << mm << endl;
				string oprand2 = asm_ins->getOperand2();
				StringSplit ss(oprand2, "->");
				assert(ss.getSubStrs()->size() == 2);
				string str = ss.getSubStrs()->at(0);
				unsigned int typeidx = _dfr->getTypeId(str);
				if(_dfr->isSelfDefinedType(typeidx))
					(*it)->setStaticFieldsInClinit(str);

				/*
				if(!_dfr->isSelfDefinedType(typeidx) )//collect the sget
				{
					//cout << "sget field of lib fields " << oprand2 << endl; //for example Ljava/lang/Integer;->TYPE: Ljava/lang/Class;
					//_sget_from_libs.push_back(oprand2);
					//_sget_from_libs[oprand2] = asm_ins;
				}else { //collect static fields belong to other self defined methods to solve the dependency
					(*it)->setStaticFieldsInClinit(str);
				}*/

				//cout << "class " << (*it)->getClassDefInfo()->getTypename()<<endl;
				//cout << "sget::: from class " << str << endl << "---" << endl;

			}

			/*if(USING_CLINIT){ //collect the rest of lib sget
					if ((ope.substr(0, 4) == "sget")
							|| (ope.substr(0, 4) == "sput")
							|| (ope.substr(0, 11) == "const-class")) {
						bool flag = false;
						string operand2 = asm_ins->getOperand2();
						//cout << "operand2 for sget/sput/const-class " << operand2 << endl;
						if (ope.substr(0, 11) == "const-class") {
							flag = true;
							operand2 += "->TYPE: Ljava/lang/Class;";
						}

						StringSplit ss(operand2, "->");
						assert(ss.getSubStrs()->size() == 2);
						string str = ss.getSubStrs()->at(0);
						unsigned int typeidx = _dfr->getTypeId(str);
						if ((!_dfr->isSelfDefinedType(typeidx)) || (flag)) {
							vector<string>::iterator f = find(
									_sget_from_libs.begin(),
									_sget_from_libs.end(), operand2);
							if (f == _sget_from_libs.end()) //not find
									{
								cout
										<< "didn't find , so push it back to the vector  == "
										<< operand2 << endl;
								_sget_from_libs.push_back(operand2);
							}
						}
					}
			}*/
			//1/5
			//HZHU end for existing fields collecting 9.13

			//HZHU add for global var collection
			/*
			if (!USING_CLINIT) {
				if ((ope.substr(0, 4) == "sget") || (ope.substr(0, 4) == "sput")
						|| (ope.substr(0, 11) == "const-class"))
					collectGlobalVarIns(asm_ins);
			}*/
			//HZHU end for global var collection
			//process those instruction that my hold information of branches and then build the label list
			if ((ope == "goto") || (ope == "goto/16") || (ope == "goto/32")) {
				string operand1 = asm_ins->getOperand1();
				operand1 = operand1.substr(1, operand1.size() - 1);
				string label = "goto_" + operand1;
				label_list->insert(label);
			}
			if ((ope == "if-eq") || (ope == "if-ne") || (ope == "if-lt")
					|| (ope == "if-ge") || (ope == "if-gt")
					|| (ope == "if-le")) {

				string operand3 = asm_ins->getOperand3();
				operand3 = operand3.substr(1, operand3.size() - 1);
				//cout << "operand3:::" << operand3 << endl;
				string label = "con_" + operand3;
				label_list->insert(label);
				//cout << "Lable in if ::" << label << endl;
			}
			if ((ope == "if-eqz") || (ope == "if-nez") || (ope == "if-ltz")
					|| (ope == "if-gez") || (ope == "if-gtz")
					|| (ope == "if-lez")) {
				string operand2 = asm_ins->getOperand2();
				operand2 = operand2.substr(1, operand2.size() - 1);
				string label = "con_" + operand2;
				label_list->insert(label);
				//cout << "Lable in if ::" << label << endl;
			}
			if ((ope == "packed-switch") || (ope == "sparse-switch")) {
				vector<unsigned int>* branches =
						asm_ins->getSwitch()->getTarget();
				vector<unsigned int>::iterator b_it = branches->begin();
				for (; b_it != branches->end(); b_it++) {
					string bra_str = Int2Str(*b_it);
					string label = bra_str;
					//cout << "Branches :" << bra_str << endl;
					if (ope == "packed-switch") {
						label = "pswitch_" + label;
						label_list->insert(label);
					} else {
						label = "sswitch_" + label;
						label_list->insert(label);
					}

				}
			}
			//hzhu added 5/3/2012

		}

		(*it)->resetSyncIdx(); //syn_idx = 0;
		(*it)->setLabelList(label_list);
		(*it)->setAdd2InsMap(method_add_ins);

	}

	//using base classfields to fill the derivative class's fields, which means all subclass should have all the fields from their parent
	_dfr->addBaseClassFieldsToDerivative(top_two_levels);


	//collect all classdefInfo before doing InstructionMapping for completely build all types
	//cout << "before collectAllTypes " << endl;
	this->CollectAllTypes();
	//cout << "After collect All Types  " << endl;


	//print out all the library static fields;
	/*
	for(int i = 0; i < _sget_from_libs.size(); i++){
		cout << "HZHU :: sget or sput from library " << _sget_from_libs.at(i) << endl;
	}*/
	//1/5
	//now we want to build global variables for those static fields;
	if(USING_CLINIT){
		//init_static_forlib(); //to build an method called init_staitc contains all instructions to initialize sth.
		//setInitStaticForGlobalVarFromLib(); //build the init_static0 function
		ProcessClinitOrder();
	}
	//1/5

}

void Ins2Debug::ProcessClinitOrder()
{
	//_unsorted_clinit_cd is map<int, CodeHeader*>
	int size = _unsorted_clinit_cd.size();
	int c = 0;
	while (_unsorted_clinit_cd.size() != 0) {
		if (CHECK_CLINIT_FIELD) {
			cout << "size of unsorted  === " << _unsorted_clinit_cd.size()
					<< endl;
			cout << "size of sorted == " << _sorted_clinit_cd.size() << endl;
		}
		c++;
		if(c == 10000)
			assert(false);
		map<int, CodeHeader*>::iterator it = _unsorted_clinit_cd.begin();
		while (it != _unsorted_clinit_cd.end()) {
			//cout << " -- "<< endl;
			bool shouldmovetosorted = (*it).second->shouldMovetosorted(_unsorted_clinit_cd, _sorted_clinit_cd);
			if(shouldmovetosorted){
				map<int, CodeHeader*>::iterator toErase = it;
				_sorted_clinit_cd[(*it).first] = (*it).second;
				_sorted_clinit_cd_vector.push_back((*it).second);
				it ++;
				//cout << "erase ----" << (*toErase).second->getClassDefInfo()->getTypename() << endl << endl;
				_unsorted_clinit_cd.erase(toErase);
			}else{
				it ++;
			}

		}
	}
	assert(_unsorted_clinit_cd.size() == 0);
	assert(_sorted_clinit_cd.size() == size);
}

/*
void Ins2Debug::processFakedArraySize() {
	sail::Constant* faked_size = new sail::Constant(0, true, 4);
	sail::Variable* faked_size_var = new sail::Variable(
			"v" + Int2Str(_dfr->getGlobalVariableReg()), il::get_integer_type(),
			false, false, -1, true);
	(*_global_variables)[FAKE_ARRAY_SIZE] = faked_size_var;

	//its initialization
	sail::Assignment* assig_fake_array_size = new sail::Assignment(
			faked_size_var, faked_size,
			new il::string_const_exp("", il::get_integer_type(),
					il::location(-1, -1)), -1);
	//if (INS_PRINT) {
	cout << "faked array size PP:: " << assig_fake_array_size->to_string(true)
			<< endl;
	cout << "faked array size :: " << assig_fake_array_size->to_string(false)
			<< endl;
	//}
	_lib_global_initialization->push_back(assig_fake_array_size);

}

void Ins2Debug::init_static_forlib() {
	processFakedArraySize();
	//////////////////////////////////////////////////
	for (unsigned int i = 0; i < _sget_from_libs.size(); i++) {
		string str = _sget_from_libs.at(i);
		StringSplit ss(str, " ");
		vector<string>* sub_strs = ss.getSubStrs();
		string str_key = sub_strs->at(0);
		string str_type = sub_strs->at(1);
		if (!(_global_variables->count(str_key) > 0)) { //if not build, then build
			//first build its variables;
			unsigned int faked_reg_num = _dfr->getGlobalVariableReg();
			string special_temp_name = "v" + Int2Str(faked_reg_num);
			ClassDefInfo* classdef = _dfr->getClassDefByName(str_type);
			cout << "classdef name " << classdef->getTypename() << endl;
			il::type* t = getType(0, _dfr, classdef, _classdef_map);
			cout << "type == " << t->to_string() << endl;
			sail::Variable* global_var = new sail::Variable(special_temp_name,
					t, false, false, -1, true);
			str_key = str_key.substr(0, str_key.length() - 1);
			(*_global_variables)[str_key] = global_var;
			cout << "key == " << str_key << "  and v :: "
					<< global_var->to_string() << endl << endl;
			//then build instruction;
			cout
					<< "build instructions for global variable from library -------- "
					<< str_key << " + " << str_type << endl;
			buildInitInsForGlobalVariableFromLib(str_key, str_type, t,
					global_var);
			cout << endl;

		}
	}
	//exit(1);

}

//set up all instructions for initialize all global variables from library

void Ins2Debug::buildInitInsForGlobalVariableFromLib(string alias, string str_type, il::type* t, sail::Variable* global_var){
	if(str_type.substr(0,1) == "L"){//alloc_type; and init functioncall;
		string fn_name = "alloc_" + str_type;
		vector<il::type*> args;
		il::type* fn_signature = il::function_type::make(t, args, false); //return + parameters type (since it is just alloc_, no parameters)
		vector<sail::Symbol*>* args_v = new vector<sail::Symbol*>();
		//namespace + name == need to split alias_name
		StringSplit* ssp = new StringSplit(alias, "->");
		vector<string>* sub_strs = ssp->getSubStrs();
		assert(sub_strs->size() == 2);
		string name_space = sub_strs->at(0); //class name
		string name = sub_strs->at(1); //Field name


		il::namespace_context ns;
		//il::namespace_context ns = ssp1->makeNameSpace();
		sail::FunctionCall* init_record_type = new sail::FunctionCall(
				global_var, fn_name, ns, fn_signature, args_v, false, true,
				false, false,
				new il::string_const_exp("", fn_signature,
						il::location(-1, -1)), -1);
		//if (INS_PRINT) {
			cout << "init_record_type ins PP:: "
					<< init_record_type->to_string(true) << endl;
			cout << "init_record_type ins :: "
					<< init_record_type->to_string(false) << endl;
		//}
		_lib_global_initialization->push_back(init_record_type);

		//then init functioncall
		StringSplit* ssp1 = new StringSplit(str_type, "$");
		ns = ssp1->makeNameSpace();
		string constructor_name = "<init>";
		vector<sail::Symbol*>* args_v_for_constructor = new vector<
				sail::Symbol*>();
		args_v_for_constructor->push_back(global_var);

		vector<il::type*>* args_constructor = new vector<il::type*>();
		args_constructor->push_back(t);
		il::type* fn_sig_constructor = il::function_type::make(NULL,
				*args_constructor, false);
		sail::FunctionCall* init_record_type_constructor =
				new sail::FunctionCall(NULL, constructor_name, ns,
						fn_sig_constructor, args_v_for_constructor, false,
						false, true, false,
						new il::string_const_exp("", fn_sig_constructor,
								il::location(-1, -1)), -1);

		//if (INS_PRINT) {
			cout << "init_record_type_constructor ins PP:: "
					<< init_record_type_constructor->to_string(true)
					<< endl;
			cout << "init_record_type_constructor ins :: "
					<< init_record_type_constructor->to_string(false)
					<< endl;
		//}
		_lib_global_initialization->push_back(init_record_type_constructor);


	}else if(str_type.substr(0,1) == "["){ //alloc_array_type
		string fn_name = "alloc_array_" + str_type.substr(1,str_type.size()-1);
		vector<il::type*> args;
		args.push_back(il::get_integer_type());
		vector<sail::Symbol*>* args_v = new vector<sail::Symbol*>();
		//get a faked size;
		sail::Variable* faked_size = (*_global_variables)[FAKE_ARRAY_SIZE];
		args_v->push_back(faked_size);
		il::type* fn_signature = il::function_type::make(t, args, false); //return + parameters type (since it is just alloc_, no parameters)

		StringSplit* ssp = new StringSplit(alias, "->");
		vector<string>* sub_strs = ssp->getSubStrs();
		assert(sub_strs->size() == 2);
		string name_space = sub_strs->at(0); //class name
		string name = sub_strs->at(1); //Field name

		StringSplit* ssp1 = new StringSplit(name_space, "$");
		il::namespace_context ns = ssp1->makeNameSpace();
		sail::FunctionCall* init_record_type = new sail::FunctionCall(
						global_var, fn_name, ns, fn_signature, args_v, false, true,
						false, false,
						new il::string_const_exp("", fn_signature,
								il::location(-1, -1)), -1);
				//if (INS_PRINT) {
					cout << "init_record_type ins PP:: "
							<< init_record_type->to_string(true) << endl;
					cout << "init_record_type ins :: "
							<< init_record_type->to_string(false) << endl;
				//}
				_lib_global_initialization->push_back(init_record_type);

	}else{
		sail::Constant* constant = new sail::Constant(0, true, 4);
		sail::Assignment* init_base_type = new sail::Assignment(global_var,
				constant,
				new il::string_const_exp("", t, il::location(-1, -1)), -1);
		//if (INS_PRINT) {
			cout << "init_base_type ins PP:: "
					<< init_base_type->to_string(true) << endl;
			cout << "init_base_type ins :: "
					<< init_base_type->to_string(false) << endl;
		//}
		_lib_global_initialization->push_back(init_base_type);
	}

}

void Ins2Debug::setInitStaticForGlobalVarFromLib() {
	vector<sail::Instruction*>* initi_ins = _lib_global_initialization;
	string f_name = "init_static_0";
	string file = "";
	il::namespace_context ns;
	vector<il::type*>* para_list = new vector<il::type*>();
	il::function_type* fn_signature = il::function_type::make(NULL, *para_list,
			false);
	sail::Function* f = new sail::Function(f_name, file, ns, fn_signature,
			initi_ins, false, true, false, false);
	//_dfr->setsailFunctionforGlobalVarsInit(f); // is this necessary??
	_init_static_0 = f;
	if (0) {
		cout << "global CFG 0 " << endl << f->get_cfg()->to_dotty(false) << endl;
		cout << "global CFG 0 PP" << endl << f->get_cfg()->to_dotty(true) << endl;
	}
}*/

/********************************************************************************
 * for those already sorted clinits,
** use already built global variables to update instructions of sget or const-instruction
** also increase global variables of sput
**********************************************************************************/

//1.5 end
void Ins2Debug::updateInsAddrMap()
{
	setInsAddrMap(); //label settled down
	//HZHU
	if (0) {
		_dfr->getFileStream()
				<< "HZHU+++++++++++++++++after calling setInsAddrMap() !"
				<< endl;
		_dfr->printClassDefFields();
	}

	//HZHU
	//another round of visiting instruction to set label
	//vector<CodeHeader*>* cd = DexFileReader::getCodeHeaderList(_dfr); 
	vector<CodeHeader*>::iterator it = cd->begin();
	//cout<< "size of code header! " << cd->size() << endl;

	std::size_t found;
	set<string>* label_list = NULL;
	set<string>::iterator setit;

	map<unsigned int, ASMInstruction*>* add_ins_map = NULL;
	map<unsigned int, ASMInstruction*>::iterator mapit;

	if (cd == NULL) {
		cout << "NULL pointer" << endl;
		exit(1);
	}

	for (it = cd->begin(); it != cd->end(); it++) //for each method
			{
		//cout << "HZHU check " << (*it)->getClassDefInfo()->getTypename() << endl;
		if ((*it)->getThisType() != NULL)
			(*it)->buildThisPointerVar();
		//haiyan comment out 8.29 begin this is not necessary to static method check
		label_list = (*it)->getLabelList();
		vector<Tries*>* tries = NULL;
		vector<Tries*>::iterator tryite;
		if ((label_list != NULL) && (label_list->size() != 0)) //need to update
				{
			//cout << "label size is :" << label_list->size()<<endl;
			add_ins_map = (*it)->getAdd2InsMap(); // manipulate the map field
			//cout << "instruction size:" << add_ins_map->size() << endl;
			mapit = add_ins_map->begin();
			for (; mapit != add_ins_map->end(); mapit++) //for each instruction inside the method
					{
				unsigned int add = mapit->first; //get the address of instruction and its string format
				string add_str = Int2Str(add);
				//haiyan added for those who
				string labels = "";
				bool label_f = false;
				for (setit = label_list->begin(); setit != label_list->end();
						setit++) //to see if the instruction inside the label_list
						{
					string label = (*setit);
					found = label.find(add_str);
					if (found != string::npos) //update the instruction as a Table
							{
						label_f = true;
						if (labels == "")
							labels += label;
						else {
							labels += "&" + label;
						}
						if (label.substr(0, 7) == "try_end") {
							tries = (*it)->getTries();
							tryite = tries->begin();
							for (; tryite != tries->end(); tryite++) {
								if (add == (*tryite)->getEndAddr()) //haiyan added mapit-- and mapit++;
										{
									mapit--; //back to last ins
									mapit->second->setTries(*tryite);
									(*_ins_addr)[mapit->first] = mapit->second; //update dex map
									mapit++; //to this ins
								}
							}
						}
					}

				}
				if (label_f) {
					ASMInstruction* ins = mapit->second;
					Label* label_ins = NULL;
					label_ins = new Label(ins->getMthCodeH(),
							ins->getOpFormat(), ins->getOpcode(),
							ins->getOperator(), ins->getTypeofVar(),
							ins->getRegs(), ins->getOperand1(),
							ins->getOperand2(), ins->getOperand3(),
							ins->getData(), ins->getSwitch(),
							ins->getMethodidx(), ins->getMethod(),
							ins->getTries()); // a bunch of staff
					if(ins->getCheckCastClassDef() != NULL){
						ClassDefInfo* cdf = ins->getCheckCastClassDef();
						label_ins->setCheckCastClassDef(cdf);
					}
					if (LABEL_CHECK)
						cout << "labels " << labels << endl;
					label_ins->set_label_name(labels);
					mapit->second = label_ins;
					(*_ins_addr)[mapit->first] = label_ins; //update dex map
				}

			}

		}

	}
}

Ins2Debug::~Ins2Debug() {
	delete _ins_addr;
	//added by haiyan 7.22
	delete _lib_has_exception;
	delete _type_of_exception;
	delete _cur_exception;
	delete _throw_exception;
	//ended by haiyan 7.22
	//haiyan 8.20
	//delete _static_midx;
	//if(!USING_CLINIT)
	//	delete _global_var_ins;
	//delete _class_field_init_ins;
}

DexFileReader* Ins2Debug::getDexFileReader() {
	return _dfr;
}

map<unsigned int, ASMInstruction*>* Ins2Debug::getInsAddrMap() {
	return _ins_addr;
}

ASMInstruction* Ins2Debug::getInsFromAddr(unsigned int addr) {
	return (*_ins_addr)[addr];
}

DebugInfo* Ins2Debug::retrieveDebug(unsigned int addr) {

	if (INS_CHECK) {
		cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - -1"
				<< endl;
		printf("addr: %x \t", addr);
		cout << addr << endl;
	}
	//ASMInstruction* asm_ins = getInsFromAddr(addr);
	ASMInstruction* asm_ins = (*_ins_addr)[addr];
	if (INS_CHECK) {
		if (asm_ins->getLabel() == "") {
			cout << "Ins: " << asm_ins->toString() << endl;
		} else {
			cout << asm_ins->getLabel() + "\n" + "Ins:" + asm_ins->toString()
					<< endl;
		}
	}
	vector<unsigned short>* regs = asm_ins->getRegs();
	vector<unsigned short>::iterator reg_it;
	CodeHeader* codeh = asm_ins->getMthCodeH();
	//cout << "Inside method: " << codeh->getMethod()->toString()<< endl;
	//hzhu 4/22/2012 for test
	//cout << "It's method's parameter size is: " << codeh->getMethod()->getParaSize()<<endl;	
	//hzhu 4/22/2012 end

	DebugInfoReader* debuginfr = _dfr->getDebugInfoReader(codeh);
	if (debuginfr == NULL)
		return NULL;
	//assert(debuginfr != NULL);
	//prepare for the extending rvt list
	vector<string>* paras = debuginfr->getParasList(); // the parameter name, which decide if parameter's string name hold and used as debug info
	//cout << "test " <<debuginfr->getRegBeginEndAddr()->size()<< endl;
	vector<unsigned int>* arg_reg = codeh->getArgsReg(); //the parameter register's number
	//cout << "paras size "<< paras->size()<< endl;
	//cout << "arg_reg size "<< arg_reg->size()<< endl;	
	//it is possible that the debuginfo doesn't hold any parameter information
	//assert(paras->size() == arg_reg->size());
	if(0){
	for (unsigned int i = 0; i < arg_reg->size(); i++)
		cout << "ARG: " << arg_reg->at(i) << endl;

	for (unsigned int i = 0; i < paras->size(); i++)
		cout << "PARAS: " << paras->at(i) << endl;
	}

	unsigned int ps = paras->size();
	//unsigned int firstpr;
	if ((ps != 0) && (arg_reg->size() != 0)) {
		if (INS_CHECK)
			cout << "The method has " << arg_reg->size() << " parameters!"
					<< endl;
	//	firstpr = arg_reg->at(0);
	}

	//prepare for the parameter of DebugInfo*
	vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* reg_addrs = codeh->getRegAddrs();
	unsigned int line = -1; //just for initialize
	string filename;

	//for return
	vector<RVT*>* debug_var_list = new vector<RVT*>(); //one instruction may contain multiple vars such that it contains more than one debug items
	//hzhu added 5/17
	//if (debuginfr == NULL)
	//    return NULL;
	//hzhu end 5/17
	if (debuginfr != NULL) {
		filename = debuginfr->getFileName(); //important

		if (INS_CHECK) {
			cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - -2"
					<< endl;
			cout << "File name: " << filename << endl;
		}
		//address to line number mapping
		map<unsigned int, unsigned int>* addr_line =
				debuginfr->getAddressLine();
		//
		map<unsigned int, unsigned int>::iterator tempit;
		//unsigned int line; // this is needed
		tempit = addr_line->find(addr); //from the address
		if (tempit != addr_line->end()) {
			line = (*tempit).second;
			if (INS_CHECK) {
				cout
						<< "- - - - - - - - - - - - - - - - - - - - - - - - - - - -3"
						<< endl;
				cout << " line:::: " << line << endl;
			}
		}
		//haiyan 8.18 changed rvtlist to rvtmap
		//vector<RVT*>* local_var_list = debuginfr->getRVTList();
		//address to list of local varible mapping
		map<unsigned int, vector<RVT*>*>* local_var_list =
				debuginfr->getRVTList();

		map<unsigned int, vector<RVT*>*>::iterator local_var_list_it;

		local_var_list_it = local_var_list->find(addr); //from the address
		if (local_var_list_it != local_var_list->end()) {
			vector<RVT*>* v = (*local_var_list_it).second;
			if (INS_CHECK) {
				cout << "+++RVT+++" << endl;
				for (unsigned int i = 0; i < v->size(); i++){
					cout << "R: " << v->at(i)->getRegName() << ", V: "<< v->at(i)->getVarName() << ", T: ";
					if(v->at(i)->getClassType() != NULL)
						cout << v->at(i)->getClassType()->getTypename() << endl;
					}
			}
		}
		//we want to expand the local_var_list and containing the parameters
		//unsigned int local_size = local_var_list->size();

		/*comment out on 08/28 night for creating <addr, debuginfo> map
		 int p = 0;
		 while(ps-- != 0)
		 {
		 unsigned int reg = arg_reg->at(p);
		 string var = paras->at(p);
		 RVT* para_rvt = new RVT(reg,var,"",NULL);
		 //local_var_list->push_back(para_rvt);
		 p ++;
		 }*/

		//cout << "local_var_list->size()" << local_var_list->size() <<endl;
		//cout << "reg_addrs->size()" << reg_addrs->size()<<endl;
		//assert(local_size == reg_addrs->size());
		//vector<RVT*>* debug_var_list = new vector<RVT*>(); // which is important***********
		//for each register inside the instruction,to check whether it is inside the local variable list
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (regs != NULL) {
			bool breakset = false;
			for (reg_it = regs->begin(); reg_it != regs->end(); reg_it++) {
				unsigned int reg_n = *reg_it;
				if (local_var_list != NULL) {
					RVT* temprvt = NULL;
					for (local_var_list_it = local_var_list->begin();
							local_var_list_it != local_var_list->end();
							local_var_list_it++) { // local_var_list_it is a gobal debug info list
						unsigned int ins_addr = local_var_list_it->first;
						vector<RVT*>* ins_rvt = local_var_list_it->second;
						// need to find all the declaration of reg_n, not all of them are valid, need to check
						// starting  ending address
						for (unsigned int i = 0; i < ins_rvt->size(); i++) {
							//RVT vectors contain reg_n
							if ((ins_rvt->at(i)->getReg() == reg_n)
									&& (addr >= ins_addr)) {
								for (unsigned int j = 0; j < reg_addrs->size(); j++) {
									pair<unsigned int,
											pair<unsigned int, unsigned int>> p =
											reg_addrs->at(j);
									if ((p.first == reg_n)
											&& (p.second.first == ins_addr)) {
										if (addr < p.second.second) {
											temprvt = ins_rvt->at(i);
											breakset = true;
											break;
										}
									}
								}
								if (breakset)
									break;
							}
						}
						if (breakset)
							break;
					}
					if (temprvt != NULL) {
						debug_var_list->push_back(temprvt);

						if (INS_CHECK) {
							cout
									<< "- - - - - - - - - - - - - - - - - - - - - - - - - - - -4"
									<< endl;
							//debug_var_list->push_back(temprvt);
							cout << "local variable item for the instruction: "
									<< endl;
							cout << "register name: " << temprvt->getRegName()
									<< endl;
							cout << "variable name: " << temprvt->getVarName()
									<< endl;
							if(temprvt->getClassType() != NULL)
								cout << "type: "
									<< temprvt->getClassType()->getTypename()
									<< endl;
						}
					}

				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if (debug_var_list->size() == 0) {
			debug_var_list = NULL;
		}
		//return new DebugInfo(line, filename,debug_var_list);
	}
	if (INS_CHECK)
		cout
				<< "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++End of this Ins!!!"
				<< endl;
	return new DebugInfo(line, filename, debug_var_list);
}

//hzhu begin 4/22/2012
DataIn* Ins2Debug::getDataIn() {
	return _datain;
}
//hzhu end 4/22/2012

void Ins2Debug::buildIns2Debug() {
	//map<unsigned int, ASMInstruction*>* _ins_addr;
	_ins_debuginfo = new map<ASMInstruction*, DebugInfo*>(); //prepared for the global
	if (_ins_debuginfo == NULL) {
		cout << "failed to allocate!" << endl;
		exit(1);
	}
	updateInsAddrMap(); //the _ins_addr now is settle down
	if (0) {
		_dfr->getFileStream()
				<< "HZHU+++++++++++++++++after calling setInsAddrMap() !"
				<< endl;
		_dfr->printClassDefFields();
	}
	vector<CodeHeader*>::iterator it = cd->begin();
	//haiyan added for test
	//int p = 0;
	//haiyan end for test
	for (; it != cd->end(); it++) {
		//cout << endl;
		if(0){
		cout << "Inside method ------------------"
				<< (*it)->getMethod()->toString() << endl;
		}
		map<ASMInstruction*, DebugInfo*>* m_ins_debug = new map<ASMInstruction*,
				DebugInfo*>();
		map<unsigned int, ASMInstruction*>* m_addr_ins = (*it)->getAdd2InsMap();
		if (m_addr_ins == NULL) {
			cout << "Empty method's address to instruction mapping" << endl;
			exit(1);
		}
		map<unsigned int, ASMInstruction*>::iterator it1 = m_addr_ins->begin();
		//haiyan added 7.19
		stack<pair<unsigned int, unsigned int>>* mystack =
				(*it)->getSyncStack();
		assert(mystack->empty());
		//haiyan ended 7.19
		//cout << "method is " << (*it)->getMethod()->toString()<<endl;
		//int i = 0;
		for (; it1 != m_addr_ins->end(); it1++) //visitor all add->instructions map
				{
			unsigned int addr = it1->first;
			ASMInstruction* asm_ins = it1->second;
			//cout << "asm_ins " << asm_ins->toString()<<endl;
			DebugInfo* debug = retrieveDebug(addr);
			if (debug != NULL) {
				(*m_ins_debug)[asm_ins] = debug;
				(*_ins_debuginfo)[asm_ins] = debug;
			}

			stack<pair<unsigned int, unsigned int>>* mystack =
					(*it)->getSyncStack();
			//assert(mystack->empty());
			processSyncAttach(*it, asm_ins, mystack, (*it)->getSyncCatches());
		}
		(*it)->msetIns2Debug(m_ins_debug);
		//by the way to set extend the addrs range of sync block as well as the potential catchall
	}
	extendSyncStructures();
}

map<ASMInstruction*, DebugInfo*>* Ins2Debug::getIns2Debug() {
	return _ins_debuginfo;
}

vector<CodeHeader*>* Ins2Debug::getCodeHeaderList() {
	return cd;
}

void Ins2Debug::setupAll() {

	buildIns2Debug();

	if(USING_STATIC_VALUE)
		buildSailInitFun();
	/*
	if(!USING_CLINIT)
		buildInitGlobalVarsMethod();
		*/
	if (0) {
		_dfr->getFileStream()
				<< "HZHU+++++++++++++++++after calling buildIns2Debug() !"
				<< endl;
		_dfr->printClassDefFields();
	}

}

map<string, ClassDefInfo*>* Ins2Debug::getTypeMap() {
	return _classdef_map;
}

void Ins2Debug::processSyncBlock(CodeHeader* codeh, string ope,
		unsigned int addr, ASMInstruction* asm_ins,
		vector<unsigned int>* sync_obj,
		stack<pair<unsigned int, unsigned int>>* sync_stack,
		vector<vector<pair<unsigned int, unsigned int>>*>* sync_addr) {
	if (ope == "monitor-enter") {
		//cout <<  "Monitor-enter " << endl;
		unsigned int obj_r = asm_ins->getRegs()->at(0); //the monitor-enter register number
		sync_obj->push_back(obj_r);
		int idx = codeh->getSyncIdx();
		pair<unsigned int, unsigned int> p_index(idx, obj_r);
		sync_stack->push(p_index);
		//cout << "index: " << p_index.first << " " << p_index.second << endl;
		//pair<unsigned int, unsigned int>* p_addr = new pair<unsigned int, unsigned int>(addr,0);
		pair<unsigned int, unsigned int> p_addr(addr, 0);
		vector<pair<unsigned int, unsigned int>>* addr_list = new vector<
				pair<unsigned int, unsigned int>>();
		addr_list->push_back(p_addr);
		//sync_addr->push_back(p_addr);
		sync_addr->push_back(addr_list);
		codeh->incrSyncIdx();

	}
	if (ope == "monitor-exit") //should based on the corresponding register number and if it is first set
			{
		unsigned int obj_r = asm_ins->getRegs()->at(0);
		if (!sync_stack->empty()) {
			if (sync_stack->top().second == obj_r) {
				//cout <<  "Monitor-exit " << endl;
				unsigned int index = sync_stack->top().first;
				//cout << "idx: " << index << endl;
				//cout << "first addr : " << sync_addr->at(index)->first << endl;
				sync_addr->at(index)->at(0).second = addr;
				//cout << "second addr :" << addr << endl;
				sync_stack->pop();
			}
		}

	}
}

void Ins2Debug::processSyncAttach(CodeHeader* cd, ASMInstruction* asm_ins,
		stack<pair<unsigned int, unsigned int>>* sync_stack,
		vector<pair<unsigned int, unsigned int>>* sync_catches) //this should be called after update** inside build**
		{
	string ope = asm_ins->getOperator();

	if (ope == "monitor-enter") {
		int idx = cd->getSyncIdx();
		unsigned int obj_r = asm_ins->getRegs()->at(0);
		pair<unsigned int, unsigned int> p_index(idx, obj_r);
		sync_stack->push(p_index);
		pair<unsigned int, unsigned int> attach(0, 0);
		if (asm_ins->hasTries()) {
			if (asm_ins->getTries()->hasFinally()) {
				unsigned int catchall_addr =
						asm_ins->getTries()->getHandlers()->at(0)->getCatchAddr();
				attach.first = catchall_addr;
			}
		}
		sync_catches->push_back(attach);
		cd->incrSyncIdx();
	}
	if (ope == "monitor-exit") {
		if (asm_ins->hasTries()) {
			assert(asm_ins->hasTries());
			unsigned int obj_r = asm_ins->getRegs()->at(0);
			if (!sync_stack->empty()) {
				if (sync_stack->top().second == obj_r) {
					unsigned int index = sync_stack->top().first;
					unsigned int catchall_addr =
							asm_ins->getTries()->getHandlers()->at(0)->getCatchAddr();
					sync_catches->at(index).second = catchall_addr;
					sync_stack->pop();
				}
			}
		}
	}

}

void Ins2Debug::extendSyncStructures() {
	vector<CodeHeader*>::iterator it = cd->begin();
	for (; it != cd->end(); it++) {
		//cout << "method " << (*it)->getMethod()->toString()<<endl;
		//vector<pair<unsigned int, unsigned int>*>* addrs = (*it)->getSyncAddr();
		vector<vector<pair<unsigned int, unsigned int>>*>* addrs =
				(*it)->getSyncAddr();
		vector<pair<unsigned int, unsigned int>>* catchalls =
				(*it)->getSyncCatches(); //need a vector of pairs holding the monitered object modify it later
		vector<unsigned int>* sync_obj = (*it)->getSyncObj();
		unsigned int sync_addr_s = addrs->size();
		unsigned int catchalls_s = catchalls->size();
		//cout << "***************sync catchalls_s " << catchalls_s << endl;
		assert(sync_addr_s == catchalls_s);
		//cout << "sync_addr_s " << sync_addr_s << endl;
		//cout << "sync_obj_s " << sync_obj->size()<<endl;
		assert(sync_addr_s == sync_obj->size());
		unsigned int n = 0;
		while (sync_addr_s-- > 0) {
			unsigned int obj = sync_obj->at(n);
			if (MONITOR_CHECK)
				cout << "monitor--------addr------------> v" << obj << endl;
			unsigned int monitor_enter_addr = addrs->at(n)->at(0).first;
			unsigned int monitor_exit_addr = addrs->at(n)->at(0).second;
			if (MONITOR_CHECK) {
				cout << "monitor enter addr : " << monitor_enter_addr << endl;
				cout << "monitor exit addr : " << monitor_exit_addr << endl;
			}

			unsigned int firstcatchall = catchalls->at(n).first;
			unsigned int secondcatchall = catchalls->at(n).second;
			if (MONITOR_CHECK) {
				cout << "monitor--------catchall------------> v" << obj << endl;
				cout << "first catchall : " << firstcatchall << endl;
				cout << "second catchall : " << secondcatchall << endl;
			}
			if ((firstcatchall == 0) && (secondcatchall == 0)) // may have potential update of its monitor exit
					{
				if ((*it)->hasTries()) {
					vector<Tries*>* tries = (*it)->getTries();
					//unsigned int size = tries->size();
					if ((tries->at(0)->hasFinally()) && (catchalls_s == 1)) {
						unsigned int potential_catchall =
								tries->at(0)->getHandlers()->at(0)->getCatchAddr();
						catchalls->at(n).second = potential_catchall;
					}
				}
				if (MONITOR_CHECK) {
					cout << "n is" << n << endl;
					cout << "updated second catchall "
							<< catchalls->at(n).second << endl;
				}
			}
			n++;
		}

	}

}

//haiyan added 7.22
sail::Variable* Ins2Debug::lib_has_exception_var() {
	return _lib_has_exception;
}
sail::Variable* Ins2Debug::type_of_exception_var() {
	return _type_of_exception;
}
sail::Variable* Ins2Debug::cur_exception_var() {
	return _cur_exception;
}

sail::Variable* Ins2Debug::throw_exception_var(){
	return _throw_exception;
}

void Ins2Debug::extendTryBlock(CodeHeader* ch) {
	//vector<vector<pair<unsigned int, unsigned int>>*>* real_try_addrs =
	//		ch->getRealTryAddrs();
	Graph* g = new Graph();
	GraphUtil gu(g, ch);
	gu.createGraph();
	//g->display();
	vector<vector<pair<unsigned int, unsigned int>>*>* addrs =
			ch->getRealTryAddrs();
	if(EXTEND_TRY_PRINT){
		cout << "before extending " << endl;
		ch->printRealTryAddrs();
	}
	unsigned int size = addrs->size();
	if(EXTEND_TRY_PRINT)
		cout << "number of real tries " << size << endl;
	int n = 0;

	while (size-- > 0) {
		if(EXTEND_TRY_PRINT)
			cout << " real tries     " << n << endl;
		vector<pair<unsigned int, unsigned int>>* cur_try_addr = addrs->at(n);
		unsigned int try_start_addr = cur_try_addr->at(0).first; // the begin address
		unsigned int try_end_addr = cur_try_addr->at(0).second;  //the end address
		//cout << "try_start_addr " << try_start_addr << endl;
		//cout << "try_end_addr " << try_end_addr << endl;

		int t_s_idx = gu.findNodeIdx(try_start_addr);
		int t_e_idx = gu.findNodeIdx(try_end_addr);

		gu.findAllPaths(t_s_idx, t_e_idx);

		set<int>* node_set = g->getNodesInAllPaths()->at(n);
		n++;
		if (node_set == NULL)
			continue;
		set<int>::iterator it = node_set->begin();
		for (; it != node_set->end(); it++) {
			if ((*it < t_s_idx) || (*it > t_e_idx)) //find out side sync nodes
					{
				unsigned int s = g->getNodeList()->at(*it)->getData()->getStart(); 
				unsigned int e = g->getNodeList()->at(*it)->getData()->getEnd();
				pair<unsigned int, unsigned int> p(s, e);
				cur_try_addr->push_back(p);
			}
		}
	}
}

void Ins2Debug::extendSyncAddr(CodeHeader* ch) {
	//
	Graph* g = new Graph();
	GraphUtil gu(g, ch);
	gu.createGraph();
	//g->display();
	//
	vector<vector<pair<unsigned int, unsigned int>>*>* addrs =
			ch->getSyncAddr();
	//vector<pair<unsigned int, unsigned int>>* catchalls = ch->getSyncCatches();
	vector<unsigned int>* sync_obj = ch->getSyncObj();

	unsigned int size = sync_obj->size();
	cout << "number of monitors " << size << endl;
	assert(size == addrs->size());
	//assert(size == catchalls->size());
	int n = 0;
	while (size-- > 0) {
		cout << " monitor on =================  v" << sync_obj->at(n) << endl;
		vector<pair<unsigned int, unsigned int>>* cur_m_a = addrs->at(n);
		unsigned int monitor_enter_addr = cur_m_a->at(0).first;
		unsigned int monitor_exit_addr = cur_m_a->at(0).second;

		int m_s_a = gu.findNodeIdx(monitor_enter_addr);
		int m_e_a = gu.findNodeIdx(monitor_exit_addr);
		gu.findAllPaths(m_s_a, m_e_a);
		set<int>* node_set = g->getNodesInAllPaths()->at(n);
		set<int>::iterator it = node_set->begin();
		for (; it != node_set->end(); it++) {
			if ((*it < m_s_a) || (*it > m_e_a)) //find out side sync nodes
					{
				unsigned int s =
						g->getNodeList()->at(*it)->getData()->getStart();
				unsigned int e = g->getNodeList()->at(*it)->getData()->getEnd();
				pair<unsigned int, unsigned int> p(s, e);
				cur_m_a->push_back(p);
			}
		}
		n++;
	}
}



void Ins2Debug::processTryBlock(CodeHeader* ch) {
	ch->setRealTries();
	if (ch->hasRealTries()) {
		if(0)
			ch->printRealTries();
		ch->setRealTryAddrs();
		extendTryBlock(ch);
		if (PRINT_TRY_ADDR) {
			cout
					<< "==================check expand try block begin ======================="
					<< endl;
			ch->printTryAddr();
			//cout << endl;
			cout
					<< "==================check expand try block end ======================="
					<< endl;
		}
	}
}


/*********
 * add used field into Lib class;
 */
void Ins2Debug::fakeFieldsForLibClass(ASMInstruction* asm_ins) {

	string operand3 = asm_ins->getOperand3();
	int size = operand3.length();
	if ((asm_ins->getOperator().substr(0, 4) != "iget")
			&& (asm_ins->getOperator().substr(0, 4) != "iput"))
		return;


	// we only process iget/iput instructions (for example iput-wide v0, p0, Ljava/util/GregorianCalendar;->time:J)

	string find_class = splitFieldStr_getFieldClass(operand3);
	string find_class_field_name = splitFieldStr_getFieldName(operand3);
	string type_str = splitFieldStr_getFieldType(operand3);



	ClassDefInfo* deri_classdef = NULL;
	ClassDefInfo* lib_classdef = NULL;

	Field* f = NULL;
	bool flag = false;
	bool find_class_is_super = false;

	//find the super_class whose fields not defined by its derivatives;
	//not only consider the fields of toppest
	//self-defined class but the lower level self defined's
	while(_dfr->selfDefinedClass(_dfr->getTypeId(find_class))){
		deri_classdef = _dfr->getClassDefByName(find_class);
		if(!(deri_classdef->insideOriginalFields(find_class_field_name))){ //没有找到
			find_class = deri_classdef->getSuperClass();
		}else
			return;
	}
	//after everything, find_class should be lib type


	/*if ((_dfr->insideClassDef(find_class))
			&& (_dfr->insideTopSelfDefinedClass(find_class))) { //the toppest selfdefined class(which needs update)
		deri_classdef = _dfr->getClassDef(find_class);
		//to check if this field name is inside the the field of the original defined
		if (!(deri_classdef->insideOriginalFields(find_class_field_name))){
			find_class = deri_classdef->getSuperClass(); //get the super class
			find_class_is_super = true;
		}
	}*/


	//if ((find_class_is_super &&(_dfr->insideFakedLibClassDefwithFields(find_class))) || ((!find_class_is_super)&&(!(_dfr->selfDefinedClass(_dfr->getTypeId(find_class))))))//fake the fields
	//		{
		//cout << "super class in lib , we want to update its fields" << endl;
		//from str to ClassDefInfo*
		//lib_classdef = _dfr->getFakedLibClassDef(find_class); //this one should unify, because it comes from a map
		//cout << "inside " << endl << endl;
		assert(!_dfr->selfDefinedClass(_dfr->getTypeId(find_class)));

		//added for test!

	if (0) {
		cout << endl<<"ASM_INS"<<asm_ins->toString() << endl;
		cout <<endl<< "find Field used by library! " << endl;
		cout << "find Class is " << find_class << endl;
		cout << "find class field name " << find_class_field_name << endl;
		cout << " find type of field " << type_str << endl;
	}
		lib_classdef = _dfr->getClassDefByName(find_class);
		//cout << "updating lib classdef Address " << lib_classdef << endl;
		assert(lib_classdef != NULL);
		if (!lib_classdef->insideOriginalFields(find_class_field_name)) { //a new met field,so create it
			//cout << "in " << endl;
			flag = true;
			lib_classdef->setLibTypeFieldsUpdated();

			//	cout << "inside lib_classdef, we can not find fields whose name is " << find_class_field_name << ", so adding it!"<<endl;
			unsigned int typeowneridx = _dfr->getTypeIdx(find_class);

			unsigned int typeidx = _dfr->getTypeIdx(type_str);
			//cout << "recheck field name  " << find_class_field_name << endl;
			f = new Field(find_class, typeowneridx, find_class_field_name,
					type_str, typeidx);
			unsigned int latestoff = lib_classdef->getLatestOff(); //byte offset
			f->setCurrOff(latestoff); //set byte offset
			unsigned int addingoff = f->setNextOff(type_str); //also byteoff
			lib_classdef->setLatestOff(latestoff + addingoff);

		}else{
			return;
		}




		//add this new faked field into its classdef's vector<Field*>*
		if (flag) {
			if (lib_classdef->getField() != NULL) {
				//	cout << "already exsting vector<Field>* on the lib class " << endl;
					//cout << "field " << find_class_field_name << "doesnt exist in the vector<Fields>, so adding it" << endl;
					lib_classdef->getField()->push_back(f);
					//cout << "After check, lib classdef has field  ? " << lib_classdef->getField()->size() << endl;
			} else {
				vector<Field*>* fields = new vector<Field*>();
				fields->push_back(f);
				lib_classdef->setFields(fields);
				//cout << "After check, lib classdef has field  ? " << lib_classdef->getField()->size() << endl;
			}
		}
		if (0) {
			cout
					<< "----------------------------check faked fields for lib class begin------------------------"
					<< endl;
			for (unsigned int i = 0; i < lib_classdef->getField()->size(); i++) {
				cout << "faked fields classowner ==>"
						<< lib_classdef->getField()->at(i)->getClassOwner()
						<< endl;
				cout << "faked fields name ==>"
						<< lib_classdef->getField()->at(i)->getName() << endl;
				cout << "faked fields type ==>"
						<< lib_classdef->getField()->at(i)->getType() << endl;
				cout << "faked fields offset ==>"
						<< lib_classdef->getField()->at(i)->getOffset() << "!!!"
						<< endl;
			}
			if (lib_classdef != NULL)
				cout << "The next offset is ==>" << lib_classdef->getLatestOff()
						<< endl;
			cout
					<< "----------------------------check faked fields for lib class end-----------------------"
					<< endl;
		}
	//}

}





vector<sail::Instruction*>* Ins2Debug::buildClassDefInitFieldsIns(
		CodeHeader* ch) {
	sail::Variable* this_var = ch->getThisPointerVar();
	ClassDefInfo* cdf = ch->getClassDefInfo();
	vector<Field*>* f = cdf->getField();

	//prepare for t0 = 0; and t0 used later;
	sail::Constant* zero = new sail::Constant(0, true, 32);
	sail::Variable* zero_var = ch->get_zero_variable();
	sail::Assignment* assi_zero = new sail::Assignment(zero_var, zero,
					new il::string_const_exp("", il::get_integer_type(),
							il::location(-1, -1)), -1);
	vector<sail::Instruction*>* ins = new vector<sail::Instruction*>();
	ins->push_back(assi_zero);

	for (unsigned int i = 0; i != f->size(); i++) {

		Field* fie = f->at(i);
		unsigned int t_id = fie->getTypeIdx();
		ClassDefInfo* f_cdf = type2ClassDef(_dfr, t_id);
		assert(f_cdf != NULL);
		il::type* t = getType(0, _dfr, f_cdf, _classdef_map);

		assert(this_var != NULL);
		assert(zero_var != NULL);

		sail::Store* store = buildClassDefFieldsIput(this_var, zero_var,
				fie->getOffset(), fie->getName(), t);
		if (0) {
			cout << "init store for field PP::" << store->to_string(true)
					<< endl;
			cout << "init store for field :: " << store->to_string(false)
					<< endl;
		}

		ins->push_back(store);
	}
	assert(ins != NULL);
	return ins;
}

sail::Store* Ins2Debug::buildClassDefFieldsIput(sail::Variable* this_var,
		sail::Variable* temp_var, unsigned int offset, string name,
		il::type* t) {
	sail::Store* store = new sail::Store(this_var, temp_var, offset, name,
			new il::string_const_exp("", t, il::location(-1, -1)), -1);
	return store;
}

//to each static fields having values
void Ins2Debug::makeGlobalVariablesandInstructions(string field_name, string field_type, string field_value){
	makeInitializationInstructionsforStaiticFieldHasValue(makeGlobalVariablesForStaticFieldHasValue(field_name, field_type), field_name, field_type);
}

sail::Variable* Ins2Debug::makeGlobalVariablesForStaticFieldHasValue(string field_name, string field_type){
	unsigned int faked_reg_num = _dfr->getGlobalVariableReg();
	string special_temp_name = "v" + Int2Str(faked_reg_num);
	ClassDefInfo* classdef = _dfr->getClassDefByName(field_type);
	il::type* t = getType(0, _dfr, classdef, _classdef_map);
	sail::Variable* global_var = new sail::Variable(special_temp_name,
							t, false, false, -1, true);
	(*_global_variables)[field_name] = global_var;
	assert(global_var != NULL);
	return global_var;
}

void Ins2Debug::makeInitializationInstructionsforStaiticFieldHasValue(sail::Variable* var, string field_name, string field_type){
	struct InitSimpleTypeValue initvalue;
	int which = 0; //int, char ,string
	string init_v = _dfr->getStaticFieldValue(field_name);
	if (0) {
		cout << "**** --------------  ****" << endl;
		cout << "field_name  = [" << field_name;
		cout << "]  field_type  = [" << field_type;
		cout << "]  field_value  = [" << init_v << "]" << endl;
	}
	il::type* t = var->get_type();
	if(field_type == "Ljava/lang/String;"){
		initvalue._stringvalue = init_v;
		which = 2;
	}else if(field_type.size() == 1){ //primitive type
		if(field_type == "C"){
			initvalue._charvalue = (char)Str2Int(init_v);
			which = 1;
		}
		else{
			initvalue._digitalvalue = Str2Double(init_v);
			which = 0;
		}
	}else{ //all other pointer type should be NULL and its initialization should be all in clinit
		assert(init_v == "NULL");
		which = 3;
	}
	//sail::intruction
	sail::Instruction* init_ins = NULL;
	if((which ==0) ||(which == 1)){ //value
		sail::Constant* value = new sail::Constant(initvalue._digitalvalue, true, 32);
		init_ins = new sail::Assignment(var, value,
						new il::string_const_exp("", t, il::location(-1, -1)), -1);
	}else if (which ==2){
		il::string_const_exp* sr = new il::string_const_exp(initvalue._stringvalue,t , il::location(-1,-1));
			 init_ins = new sail::AddressString(var, sr,
					new il::string_const_exp("", t, il::location(-1,-1)), -1);
	}else{
		assert(which == 3);
		sail::Constant* value_zero = new sail::Constant(0, true, 32);
		init_ins = new sail::Assignment(var, value_zero, new il::string_const_exp("", t, il::location(-1,-1)),-1);
	}

	//cout << "init inst ==  [[ " << init_ins->to_string(true)
	//					<< " ]]" << endl;

	_fun_initi_static_fields->push_back(init_ins);
}

void Ins2Debug::setSailInitFun() {
	vector<sail::Instruction*>* initi_ins = _fun_initi_static_fields;
	string f_name = "init_static_field_has_value";
	string file = "";
	il::namespace_context ns;
	vector<il::type*>* para_list = new vector<il::type*>();
	il::function_type* fn_signature = il::function_type::make(NULL, *para_list,
			false);
	sail::Function* f = new sail::Function(f_name, file, ns, fn_signature,
			initi_ins, false, true, false, false);
	_init_statics = f;
}

sail::Function* Ins2Debug::getSailInitFun(){
	return _init_statics;
}

void Ins2Debug::buildSailInitFun() {
	map<string, string>* static_types = _dfr->getStaticFieldTypes();
	map<string, string>* static_values = _dfr->getStaticFieldValues();
	assert(static_types->size() == static_values->size());
	//cout << "static value size " << static_types->size() << endl;
	map<string, string>::iterator it_type = static_types->begin();
	map<string, string>::iterator it_values = static_values->begin();
	while (it_type != static_types->end()) {
		string field_name = it_type->first;
		string field_type = it_type->second;
		assert(field_name == it_values->first);
		string field_value = it_values->second;
		//making global variable
		//making initialization instructions;
		makeGlobalVariablesandInstructions(field_name, field_type,
				field_value);
		it_type++;
		it_values++;
	}
	setSailInitFun();
}

void Ins2Debug::CollectAllTypes() {

	map<string, ClassDefInfo*>* classdef_map = _dfr->getStr2ClassDef();
	assert(classdef_map != NULL);
	assert(classdef_map->size() > 0);
	map<string, ClassDefInfo*>::iterator classdef_map_it =
			classdef_map->begin();
	ClassDefInfo* temp = NULL;
	il::type* t = NULL;

	int i = 0;

	for (; classdef_map_it != classdef_map->end(); classdef_map_it++) {
		temp = classdef_map_it->second;
		t = getType(0, _dfr, temp, _classdef_map);
	}

}

