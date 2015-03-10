//void dex_to_sail(const string & dex_file, const string & out_path);
#include "DexToSail.h"

int main(int argc, char** argv)
{
	if(argc <=2) {
		cout << "Please specify filename of .dex file to parse followed by "
				"the SAIL output directory." << endl;
		exit(1);
	}
	string dex_file = argv[1];
	string out_dir = argv[2];
	sail::TranslationUnit* translation_unit = dex_to_sail(dex_file);
	SerializeToSailFile(translation_unit, out_dir);
}

#if 0
void printCFG(sail::Function* f){
	cout << "CFG :: " << endl
						<< f->get_cfg()->to_dotty(false)
						<< endl;
				cout << "CFG PP ::" << endl
						<< f->get_cfg()->to_dotty(true)
						<< endl;
}
#endif

#if 0
void dex_to_sail(const string & dex_file, const string & out_path)
{
	DataIn* in = new DataIn();
   	in->readDexFile(dex_file);

   	ofstream funsig;
   	if(PRINT_FUNSIG){
   		funsig.open("/scratch/hzhu/funsig.txt");
   	}
   	Ins2Debug* in2debug = new Ins2Debug(in);
   	in2debug->setupAll();

   	vector<CodeHeader*>* codeh = in2debug->getCodeHeaderList(); //build up
   	DexFileReader* dfr = in2debug->getDexFileReader();

   	if(0)
   	{
   		map<string, sail::Variable*>* global_var = dfr->getGlobalVars();
   		map<string, sail::Variable*>::iterator it = global_var->begin();
   		cout<< "PAYATTENTIN global var test begin ========================= " << endl;
   		for(; it != global_var->end(); it++){
   			cout << "global var " << it->second->to_string() << " alias_name :: " << it->first << " type is :: "  << it->second->get_type()->to_string()<< endl;
   		}
   		cout<< "PAYATTENTIN global var test end ========================= " << endl;
   	}
 

   	if(0){
   		//	dfr->setfakeLibClassDefWithFields();
   		dfr->PrintDerivativeSelfDefinedBaseLib();
   	}

   	if(CHECK_SELF_DEFINE_CLASS){
   		cout << endl <<" -------check is self defined class begin! " << endl;
   		for(unsigned int i = 0 ;i < dfr->getTypeIdsSize(); i++)
   		{
   			if(dfr->selfDefinedClass(i))
   				cout << "self defined  ========  T::" << dfr->getType(i) << endl;
   		}
   		cout << "----------check is self defined class end!" << endl;
   	}

   	if(CHECK_CLASSDEF){
   		cout << endl<<"--------------------check classdef begin " << endl;
   		dfr->printClassDef();
   		cout << "check classdef end " << endl;
   	}
   	if(CHECK_CLASS_FIELD){
   		cout << endl<<"------------------------checking inner type offset"<<endl;
   		//dfr->printClassDefInfos();
   		dfr->printClassDefFields();
   		cout << "-----------------------checking inner type offset" << endl<< endl;
   	}


   	//haiyan added for test
   	if(SELF_DEFINED_METHODS){
   		map<unsigned int, Method*>* midx_m = dfr->getMIdx2Method();
   		cout << "number of the set is " << midx_m->size()<<endl;
   		if(midx_m->size() > 0)
   		{
   			map<unsigned int, Method*>::iterator it = midx_m->begin();
   			for(; it != midx_m->end(); it ++){
   				cout << "method id : " << it->first << endl;
   			}
   		}
   	}
   	//haiyan added for test

   	//haiyan added to test the Throws
   	if(METHOD_THROW_EXCEPTIONS)
   	{
   		map<unsigned int, vector<string>*>* throw_map = dfr->getMethodThrows();
   		map<unsigned int, vector<string>*>::iterator throw_it;
   		if(throw_map->size() != 0)
   		{
   			cout << "total size of method inside the map that having throw exceptions " << throw_map->size()<<endl;
   			throw_it = throw_map->begin();
   			for(; throw_it != throw_map->end(); throw_it++)
   			{
   				//unsigned int mid = throw_it->first;
   				vector<string>* throws = throw_it->second;
   				unsigned int throw_size = throws->size();
   				unsigned int i = 0;
   				while( i != throw_size){
   					string throw_type = throws->at(i++);
   					cout << "throw type is " << throw_type << endl;
   				}
   			}
   		}
   	}



   	map<string, ClassDefInfo*>* typemap = in2debug->getTypeMap();
   	map<ASMInstruction*, DebugInfo*>* ins_debug_map = NULL;


   	sail::TranslationUnit* tu_global = NULL;
   	vector<sail::Function*> clinits;
   	sail::Serializer* s_static = NULL;
   	if(USING_STATIC_VALUE){
   		clinits.push_back(in2debug->getSailInitFun());
		if (PRINT_CFG) {
			printCFG(in2debug->getSailInitFun());
		}
   	}


   	////////////////////////////////////////////



	sail::TranslationUnit* tu = NULL;
	vector<sail::Function*> functions ;
	sail::Serializer* s = NULL;
	//HZHU add global init function to functions; ONLY ONE init_static says everything
	/***************************************************************
	 * initialize all instructions inside sorted clinits
	 **************************************************************/
	if (USING_CLINIT) {
		vector<CodeHeader*> sortedcodeh = in2debug->getSortedClinitsVector();
		vector<CodeHeader*>::iterator sorted_clinit_it = sortedcodeh.begin();
		//cout << "the size of clinit " << sortedcodeh.size() << endl;
		int p = 0;
		while (sorted_clinit_it != sortedcodeh.end()) {
			if(CLINIT_PRINT)
				cout << ++p << ":: the clinit is "
					<< (*sorted_clinit_it)->getMethod()->toString()
					<< endl;
			assert((*sorted_clinit_it)->is_clinit());
			//get instructions of each clinit

			string f_name = "init_static_" + Int2Str(p); //function name
			ClassDefInfo* classdefinfo =
					(*sorted_clinit_it)->getClassDefInfo();
			//string file = (*sorted_clinit_it).second->getClassDefInfo()->getFilename();
			assert(classdefinfo != NULL);
			if(CLINIT_PRINT)
				cout << "typename " << classdefinfo->getTypename() << endl;
			string file = classdefinfo->getFilename();
			if(CLINIT_PRINT)
				cout << "file name is " << file << endl;

			StringSplit* sp = new StringSplit(
					(*sorted_clinit_it)->getMethod()->getClassOwner(),
					"$");
			il::namespace_context ns = sp->makeNameSpace();
			vector<il::type*>* para_list = new vector<il::type*>();
			il::function_type* fn_signature = il::function_type::make(NULL,
					*para_list, false);

			//build the init_ins for the Function* f;
			ins_debug_map = (*sorted_clinit_it)->mgetIns2Debug(); //for the efficient, all instructions inside one method share one mapping
			map<unsigned int, ASMInstruction*>* mymap_1 =
					(*sorted_clinit_it)->getAdd2InsMap(); //instructions all inside the first method
			map<unsigned int, ASMInstruction*>::iterator it_static =
					mymap_1->begin();
			InstructionMapping* f_insmap = new InstructionMapping(
					in2debug->getDexFileReader(), typemap, NULL);

			vector<sail::Instruction*>* static_f_ins = NULL;

			for (; it_static != mymap_1->end(); it_static++) {
				unsigned int addr = it_static->first;
				f_insmap->setsailInstruction(in2debug, ins_debug_map,
						it_static->second, addr, false); //each time make some instruction which insert to _sail_ins
			}
			//adding return label to method, which is the last instruction (exit way)
			static_f_ins = f_insmap->getSailInstruction();

			//now we need to check if the static fields are all settled?


			sail::Label* return_l = f_insmap->get_label("__return_label");
			static_f_ins->push_back(return_l);

			sail::Function* f_static = new sail::Function(f_name, file, ns,
					fn_signature, static_f_ins, false, true, false, false);
			clinits.push_back(f_static);
			if (PRINT_CFG) {
				printCFG(f_static);
			}
			sorted_clinit_it++;
		}
	}
   	//the end of serialization clinits

///////////////////////////////////////////////////////////////////////////////////////////////////////
   	if (codeh != NULL)
   	{

		vector<CodeHeader*>::iterator it1 = codeh->begin(); //just test the first method's instruction
		//map<ASMInstruction*, DebugInfo*>* ins_debug_map = NULL;

		int u = 0;
		if (DEBUG_MD)
			cout << "number of methods (including clinit) ::" << codeh->size() << endl;
		int n = 0;

		for (; it1 != codeh->end(); it1++) //for those not clinits
				{
			if ((*it1)->is_clinit()) {// clinit
				continue;
				//3/6/2013
				if(DELETE_CODEH)
					delete *it1;
			}
			u++;

			//if(u != 8975)
			//	continue;
			//if(u = 10125)
			//	continue;


			if (DEBUG_MD) {
				cout
						<< "+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+ begin of method : +_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_+_"
						<< u << endl;
				cout << (*it1)->getMethod()->toString() << endl;
				cout << (*it1)->getMidx() << endl;
				cout << "|||||||||||||||||||||||||||||||||||||||||||||||||||||||||"
									<< endl;
			}
			//(*it1)->printArgs();
			in2debug->processTryBlock(*it1);
			if (DEBUG)
				cout
						<< " ********************************** end of processTryBlock "
						<< endl << endl;

			/*
			 if(PRINT_TRIES)
			 {
			 (*it1)->setRealTries();
			 if((*it1)->hasRealTries())
			 {
			 (*it1)->printRealTries();
			 (*it1)->setRealTryAddrs();
			 //in2debug->extendTryBlock(*it1);
			 }
			 }

			 if(1)
			 {
			 in2debug->extendTryBlock(*it1);
			 cout << "=======================check expand try block begin ===================================" << endl;
			 (*it1)->printTryAddr();
			 cout << endl;
			 cout << "=======================check expand try block end ===================================" << endl;
			 }
			 */

			if (PRINT_SYNC_INFO) {
				cout
						<< "=======================check expand sync block begin ==================================="
						<< endl;
				(*it1)->printSyncAddr();
				cout << endl;
				cout
						<< "=======================check expand sync block end ==================================="
						<< endl;
				cout << "test !!" << endl;
				(*it1)->printSyncCatches();
			}
			//end of test syn blocks

			if (0) { // test
				cout << "TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT"
						<< endl;
				if ((*it1)->isThrowExceptions() && ((*it1)->hasTries()))
					cout << "has throws and has tries ! " << endl;
				else if ((*it1)->isThrowExceptions()) {
					cout << "only has ThrowExceptions!" << endl;
				} else if ((*it1)->hasTries())
					cout << "only has tries! " << endl;
				else
					cout << "nothing to have! " << endl;
			}

			//test exceptions
			if ((*it1)->isThrowExceptions()) {
				n++;
				if (DEBUG)
					cout << "has Exceptions!!!!!!!!!!!!!!!!!!!!!" << n << endl;
				vector<string>* exceptions = (*it1)->getThrowExceptions();
				assert(exceptions->size()> 0);
				int e_s = exceptions->size();
				//cout << "e_s "<< e_s<< endl;
				int v = 0;
				if (DEBUG) {
					while (v != e_s) {
						cout << "v " << v << endl;
						cout << "Exception ::: " << exceptions->at(v++) << endl;
					}
					cout
							<< "______________________________________________________________"
							<< endl;
				}
			}

			//end of testing exceptions
			//test if there is trie/catch and finally
			if (TRIES_CHECK) {
				if ((*it1)->hasTries()) {
					cout << "has tries ! " << endl;
					vector<Tries*>* tries = (*it1)->getTries();
					unsigned int ts = (*it1)->getTriessize();
					cout << "the size of tries is " << ts << endl;
					unsigned int p = 0;
					while (p != ts) {
						Tries* tr = tries->at(p);
						if (tr->hasFinally()) {
							cout << "\t" << "has finally" << endl;
						}
						p++;
					}
				}
			}

			//build the funtion_type

			Method* m = (*it1)->getMethod();
			string return_t = m->getReturnType();
			il::type* ret_t = NULL;
			if (typemap->count(return_t) > 0) {
				ClassDefInfo* ret_classdef = (*typemap)[return_t];
				ret_t = getType(0, dfr, ret_classdef, typemap);
				if (ret_t->is_void_type()) {
					ret_t = NULL;
				}

			}
			vector<unsigned int>* para_ids = m->getParaTypeIds();
			vector<il::type*>* para_list = new vector<il::type*>();
			//prepare for the this pointer if possible;
			//haiyan added 8.13

			bool is_virtual_method = false;
			bool is_abstract_method = false;
			bool is_constructor = false;
			bool is_destructor = false;
			//haiyan changed the condition because we do not maintain a static method table 8.29
			//if((!m->isStatic())&&(!(in2debug->getStaticMIdx()->count(midx)>0))) //non static method, has its this pointer
			if (!m->isStatic()) { //non static method, has its this pointer
				if (0)
					cout << "direct/virutal method !" << endl;
				is_virtual_method = true;
				unsigned int m_owner_idx = m->getClassOwneridx();
				ClassDefInfo* this_cdf = type2ClassDef(dfr, m_owner_idx);
				il::type * this_t = getType(0, dfr, this_cdf, typemap);
				para_list->push_back(this_t);
			} else {
				if(0)
					cout << "Static method! " << endl;
			}

			if (m->getName().find("init>") != string::npos)
				is_constructor = true;

			//haiyan ended 8.13
			int size = 0;
			if (para_ids != NULL)
				size = para_ids->size();
			int k = 0;
			while (k != size) {
				ClassDefInfo* classdef = type2ClassDef(dfr, para_ids->at(k));
				il::type* tmp_t = getType(0, dfr, classdef, typemap);
				para_list->push_back(tmp_t);
				k++;
			}
			//end build the function_type

			ins_debug_map = (*it1)->mgetIns2Debug(); //for the efficient, all instructions inside one method share one mapping
			map<unsigned int, ASMInstruction*>* mymap = (*it1)->getAdd2InsMap(); //instructions all inside the first method
			map<unsigned int, ASMInstruction*>::iterator it = mymap->begin();

			//HZHU
			if (0) {
				dfr->getFileStream()
						<< "HZHU+++++++++++++++++before InstructionMapping !"
						<< endl;
				dfr->printClassDefFields();
				dfr->getFileStream() << endl << endl;

			}

			//HZHU added 9.13
			vector<sail::Instruction*>* init_fields_ins = NULL;
			if (m->getName().find("<init>") != string::npos)
				init_fields_ins = in2debug->buildClassDefInitFieldsIns(*it1);
			//HZHU end 9.13

			InstructionMapping* f_insmap = new InstructionMapping(
					in2debug->getDexFileReader(), typemap, ret_t);
			//HZHU added 9.13(set init fields value as value, because default value is 0 based on jvm specification)
			if (init_fields_ins != NULL)
				f_insmap->initFields(init_fields_ins);
			//HZHU end 9.13

			vector<sail::Instruction*>* f_ins = NULL;
			int i = 0;
			for (; it != mymap->end(); it++) {
				if (DEBUG) {
					cout
							<< "-------------------------------------------------------------------------------------------------------------"
							<< endl;
					cout << "i ==>" << ++i << endl;
				}
				unsigned int addr = it->first;
				f_insmap->setsailInstruction(in2debug, ins_debug_map,
						it->second, addr, false); //each time make some instruction which insert to _sail_ins
			}
			//adding return label to method, which is the last instruction (exit way)
			f_ins = f_insmap->getSailInstruction();

			sail::Label* return_l = f_insmap->get_label("__return_label");
			f_ins->push_back(return_l);
			string fname = (*it1)->getMethod()->getName();
			vector<il::type*> empty;
			//prepare for the namespace;
			//
			StringSplit* sp = new StringSplit(
					(*it1)->getMethod()->getClassOwner(), "$");
			il::namespace_context ns = sp->makeNameSpace();
			if (NAME_SPACE_PRINT)
				cout << "ns " << ns.to_string() << endl;
			if (CLASS_HIERARCHY) {
				if (ret_t != NULL) {
					if (ret_t->is_pointer_type()) {
						if (ret_t->get_inner_type()->is_record_type()) {
							il::record_type* rt =
									static_cast<il::record_type*>(ret_t->get_inner_type());
							map<int, il::record_type*> bases = rt->get_bases();
							map<int, il::record_type*>::iterator it =
									bases.begin();
							for (; it != bases.end(); it++) {
								cout << "bases " << it->first << "type "
										<< it->second->to_string() << endl;
							}
						}
					}
				}
				if (para_list->size() > 0) {
					for (unsigned int j = 0; j < para_list->size(); j++) {
						il::type* t = para_list->at(j);
						//cout << "para: " << j << " == type " << para_list->at(j)->to_string()<<endl;
						if (t->is_pointer_type()) {
							if (t->get_inner_type()->is_record_type()) {
								il::record_type* rt =
										static_cast<il::record_type*>(t->get_inner_type());
								//cout << "para inner record type " << rt->to_string()<<endl;
								map<int, il::record_type*> bases =
										rt->get_bases();
								map<int, il::record_type*>::iterator it =
										bases.begin();
								for (; it != bases.end(); it++) {
									cout << "para inner record type's bases "
											<< it->first << "  type== "
											<< it->second->to_string() << endl;
								}

								vector<il::record_type*> deriv =
										rt->get_derivatives();
								for (unsigned int n = 0; n < deriv.size();
										n++) {
									cout << "para inner record type's deriv "
											<< n << "  type== "
											<< deriv.at(n)->to_string() << endl;
								}
							}
						}
					}
				}
			}

			il::function_type* fn_signature = il::function_type::make(ret_t,
					*para_list, false);
			string file = (*it1)->getClassDefInfo()->getFilename();
			sail::Function* f = new sail::Function(fname, file, ns,
					fn_signature, f_ins, is_virtual_method, is_abstract_method,
					is_constructor, is_destructor);
			if (PRINT_FUNSIG) {
				funsig << fname << endl << ns.to_string() << endl;
				funsig << fn_signature->to_string() << endl;
				funsig << endl;
			}
			//here, we need to add annotations to the function;
			vector<sail::Annotation*>* anno  = dfr->getMethodFlowAnnotation((*it1)->getMidx());
			if(anno != NULL){
				f->add_annotations(*anno);
			}
			delete sp;
			//vector<Annotation*> & get_annotations();

			if (0) {
				vector<sail::Annotation*> get_anno = f->get_annotations();
				for (int i = 0; i < get_anno.size(); i++) {
					cout << "ZZZZZ ;; "
							<< ((sail::FlowAnnotation*) (get_anno.at(i)))->to_string()
							<< endl;
				}
			}
			functions.push_back(f);
			if (PRINT_CFG) {
				printCFG(f);
			}
			if(DELETE_CODEH)
				delete *it1;
		}
	}
   	assert(dfr->getUnresolvedTypes()->size() == 0);
  //serialize static initialization methods
   	tu_global = new sail::TranslationUnit(clinits, NULL, "");
   	s_static = new sail::Serializer(tu_global, out_path);


   	//serialize normal methods;
   	tu = new sail::TranslationUnit(functions, NULL, "");
   	s = new sail::Serializer(tu, out_path);


   	delete dfr;



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	


#if 0
	DataIn* datain = new DataIn();
	//datain->readDexFile("classes.dex");
	Ins2Debug* in2debug = new Ins2Debug(datain);
	DexFileReader* dfr = in2debug->getDexFileReader();
	in2debug->setInsAddrMap();
	map<unsigned int, ASMInstruction*>* mymap = in2debug->getInsAddrMap();
	map<unsigned int, ASMInstruction*>::iterator it;
	CodeHeader* codeh = NULL;
	DebugInfoReader* debuginfr = NULL;
	DebugInfo * debuginf = NULL; // which used to hold every that return back;	
	for (it=mymap->begin(); it!=mymap->end(); it++)
	{
		unsigned int addr = (*it).first;
		printf ("addr: %x \n", addr);
		ASMInstruction* asm_ins = (*it).second;
			
		cout << "Inst: " << asm_ins->toString() << endl; //the instruction
		cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - -1" << endl;
		cout << "Inst test! get fromaddress: " << (in2debug->getInsFromAddr((*it).first)).toString() << endl; //the instruction
		
		//I need the reg information from the instruction; // write a function member that implemented to get the reg?
		
		vector<unsigned short>* regs = (*it).second->getRegs(); // this is important************
		vector<unsigned short>::iterator reg_it;
		/*
		if (regs !=NULL){
			vector<unsigned short>::iterator it2;
			int size = regs->size();
			cout << "register numbers: " << size << endl;
			for (it2=regs->begin(); it2!= regs->end(); it2++)
			{
				cout <<"r: " << *it2 << endl;
			}
		}
		*/
		codeh = (*it).second->getMthCodeH();
		debuginfr = dfr->getDebugInfoReader(codeh); // we have a bunch of info that hold the debuginformation
		if(debuginfr!= NULL)
		{
			//cout << "File name&&&&&&&&&&&&&&&&&&&&&&&&&&&&: " << debuginfr->getFileName() << endl;
			string filename = debuginfr->getFileName(); //important
			cout << "File name: " << debuginfr->getFileName() << endl;
			cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - -2" << endl;
			map<unsigned int, unsigned int>* addr_line = debuginfr->getAddressLine();
			map<unsigned int,unsigned int>::iterator tempit;
			unsigned int line;
			tempit = addr_line->find((*it).first); //from the address
			if(tempit != addr_line->end())
			{
				line = (*tempit).second;
				cout << " line:::: " << line << endl;
			cout << "- - - - - - - - - - - - - - - - - - - - - - - - - - - -3" << endl;
			}
			vector<RVT*>* local_var_list = debuginfr->getRVTList();
			vector<RVT*>* debug_var_list = new vector<RVT*>(); // which is important***********
			//for each register inside the instruction,to check whether it is inside the local variable list
			if(regs!=NULL)
			{
				for(reg_it= regs->begin(); reg_it!= regs->end(); reg_it++)
				{
					if(local_var_list!= NULL)
					{
						for(unsigned int i = 0; i<local_var_list->size();i++)
						{
							RVT* temprvt = local_var_list->at(i);
							if(temprvt->getReg()==(*reg_it))
							{
								debug_var_list->push_back(temprvt);
								cout << "local variable item for the instruction: " << endl;
							//	printf ("addr: %x \n", addr);
							//	cout << "Inst: " << asm_ins->toString() << endl;
							//	cout << "file name : " << filename << endl;
								cout <<"register name: " << temprvt->getRegName()<<endl;
								cout <<"variable name: " << temprvt->getVarName()<<endl;
								cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<endl;
								
							}
						}
						
					}
				}
				
			}
			
			if (debug_var_list->size()==0)
			{
				debug_var_list = NULL;
			}
				
			debuginf = new DebugInfo(line, filename,debug_var_list);			
			//	DebugInfo(line,  filename, vector<RVT*>* rvt);
		}	
		
	//	printf ( "debug: %x \n", (*it).second->getMthCodeH()->getInstructionBeginOff());
	}
#endif

//test read the instruction to check whether it is correct!!!!

#if 0
for(int i = 0xf3; i < 0xfc; i++)
{
	DexOpcode opcode = (DexOpcode) i; //ox01
	printf("opcode : %.2x\n",opcode);
	OpcodeFormat opformat(opcode);
	OpcodeFormat::Format format = opformat.getFormat();
	opformat.printFormat(format);
}
#endif


//check the instruction  and build the instruction and its address mapping
#if 0 
	//the map that hold all instructions and their ab_address
	map<ASMInstruction*, unsigned int>* Inst_addr = new map<ASMInstruction*, unsigned int>();
	vector<CodeHeader*>::iterator it;
	int size = 0;
	vector<CodeHeader*>* cd = DexFileReader::getCodeHeaderList(dfr); //each method has its own code header that means how many method in this dex file
	size = cd->size(); // how many methods in this dex file
	//cout <<"total method size: " <<size <<endl;
	//int count = 1;
	OpcodeFormat* opformat = NULL;
	DexOpcode dexopcode;
	InstructionProcess* insp = NULL;
	//there are total 9 codeheader:
	for(it = cd->begin(); it != cd->end(); it++)
	{	
	#if 0
		cout <<"count:"<<count <<endl;
		int reg = (*it)->getTotalRegisterSize();
		cout << "reg total:" <<reg <<endl;
		int offset =(*it)->getInstructionBeginOff();
		printf("offset: %.2x\n", offset);
		count ++ ;
	#endif	
//		if(count == 1)
//		{      
			//the begin of the first instruction inside the code header
			int inadd_begin = (*it)->getInstructionBeginOff();
		//	cout << "the ins begin off is " << inadd_begin <<endl;
		//	printf("dex address is: %x\n",inadd_begin);
			int instr_count = (*it)->getInstructionSize(); // total number of code unit for this method;
		//	cout << "Total instr units in one method:  " << instr_count << "units!" <<endl;
			int ins_size; // the unit of the instruction
			unsigned int min_data_addr = dfr->getFileSize();
			//process for each inst inside the a codeheader
			for(int i = 0; i < instr_count ; i += ins_size)
			{      
				unsigned int addr = inadd_begin + i*2; //wow I need it
				//printf("instruction address : %x hex\n",addr);
				if(addr >= min_data_addr)
				{
					break;
				}
				DataIn* din;
				din = (*it)->getDataIn();
				din->move(addr); //go to the first instruction address
			//	cout << "current position: " << din->getCurrentPosition()<<endl;
			//	printf("current position : %.4x \n", din->getCurrentPosition());
				unsigned short dexop_read = din->readByte()&0x00ff;
				dexopcode = (DexOpcode)dexop_read; // get the opcode
		//		printf("dexopcode: %.2x\n", dexopcode);
				opformat = new OpcodeFormat((*it),dexopcode); //based on the opcode to map its format
				Format f = opformat->getFormat();
				ins_size = opformat->getSize(f);
		//		cout << "ins_size : " << ins_size <<endl;
				insp = new InstructionProcess((*it), f, dexopcode, addr, NULL); //process instruction and NULL for test
				ASMInstruction* asm_ins = insp->processAll(); //wow I get the Instruction
				if (asm_ins->hasData()|asm_ins->hasSwitch())
				{
					//Operand2 contains the data address;
					string tempaddr = asm_ins->getOperand2();
					int tempint = atoi(tempaddr.c_str());
					if (min_data_addr > tempint)
					{
						min_data_addr = tempint;
			//			cout << "min_data_addr: " <<min_data_addr << endl;
					} 
				}
				string instr = asm_ins->toString();
				(*Inst_addr)[asm_ins] = addr;
				//cout <<"instruction is: " <<instr << endl;
				//cout << endl;
			}
//		}
//		count ++;
	}
	map<ASMInstruction*, unsigned int>::iterator it2;
	for(it2 = Inst_addr->begin(); it2 != Inst_addr->end();it2 ++)
	{
		cout <<"instr: " << (*it2).first->toString() << endl;
		printf("addr: %x\n",(*it2).second);
	}
	
#endif	


#if 0

	//invoke-super{p0,p1},Landroid/app/Activity;->onCreate(Landroid/os/Bundle;)V
	in->move(1560);
	unsigned char opcode = in->readByte();
	//opcode need to map into the invoke-super
	printf("opcode : %.2x\n", opcode);
	//how many register used?
	int psize = in->readByte() >> 4;
	cout << "Register size " << psize << endl;
	//in->skip(1);
	unsigned short midx = in->readShortx();
	printf("midx : %.2x\n", midx);
	int pos = in->getCurrentPosition();
	printf("CurrentPostion : %.2x \n",in->getCurrentPosition());
	//cautious position is changing to the end of file head
	DexFileReader dfr(in);
	
	cout << "Method: " << dfr.getMethod(midx)->toString() <<endl;
	
	//methodid need to map to the "Landroid/app/Activity;->onCreate(Landroid/os/Bundle;)V"
	
	//parameters need to map to {V}
	in->move(pos);	
	unsigned short registers = in->readShortx();
	printf("Registers : %.4x\n", registers);
	vector<unsigned short>* r = new vector<unsigned short>();
	unsigned short base = 0x000f;
	for(int i = 0; i < psize ; i++)
	{
		
		printf("base: %.4x\n",base);
		unsigned short num = registers & base;
		unsigned int dividend = pow(16,i);
		num = num /dividend;
		cout << "register :" << num << endl;
		r->push_back(num);
		base = (base << 4);

	}
	
	vector<unsigned short>::iterator it;
	for( it= r->begin(); it < r->end(); it++)
	{
		cout << "register -> " << *it <<endl;
	}
	
#endif	

//test primitive type
#if 0
	int typesize = dfr.getTypeIdsSize();
	cout << "typesize:  " << typesize<<endl;
	for(int i = 0 ; i < typesize; i++)
	{
		string str = dfr.getType(i);
		cout << "Type: " << str <<endl;
		dfr.isPrimitiveType(i);
		cout << endl;
	}
#endif


//test codeHeader info and debug info
	
#if 0
//	DexFileReader dfr(in);
	int classdefsize = dfr.getClassDefSize(); 
	cout << "test!!!!classdefsize : " <<classdefsize <<endl;
	map<int,int > *m; //this is the map for the methodid and code offset
	map<int,int >::iterator pos;
	ClassDefInfo* tempo;
	CodeHeader* codeh;
	DebugInfoReader* debuginfor;
	// 31 has some problem, the second method has some problem, need to recheck~~~~~~~~~
	for(int i = 0; i < classdefsize ; i++)
	{
			tempo = dfr.getClassDefInfo(i);
			string filename = tempo->getFilename();
			cout << "In the file of :" << filename <<endl;
			string classname = tempo->getTypename();
			cout << "INSIDE=============" << classname <<endl;
			m = tempo->getMethod2Codeoff(); // the methodid and codeoffset map of the ClassDefInfo
			cout <<"+++++++++++++ how many methods inside the classdef?" << m->size() <<endl;
			cout <<"print out the contents of methodid and codehead offset:::::::  " <<i<<endl;
			for(pos = m->begin(); pos!= m->end(); ++pos)
			{
				
				cout << "methodid :" << pos->first<<endl;
				cout << "methodname:  " << dfr.getMethod(pos->first)->toString()<<endl;
				codeh = dfr.getCodeHeader(tempo,pos->first);// return a pointer to a CodeHeader;
				cout <<"code head offset :" <<pos->second<<endl;
				printf("== %x\n",pos->second);
				printf("debug info offset : %x\n",codeh->getDebugInfoOff());
				debuginfor = dfr.getDebugInfoReader(codeh);
				
				cout << endl;
			}
	}
#endif

//test classDefInfo
#if 0
	cout << "_____________________________class def size : " << classdefsize <<endl;
	for(int k = 10; k<53; k++)
	{
        	ClassDefInfo* a = dfr.getClassDefInfo(k);
		//cout << "HHHHH:interface" << a->_interface[0]<<endl;
		//cout << "HHHHH:field" <<*(a->_field[0]).toString()<<endl;
		for(int j =0; j<(a->_field).size();j++)
		{
			cout << "Field_________________" <<j <<endl;
			cout<< a->_field.at(j)->toString()<<endl;
		}
		cout << "method size in this class should be ::::::::::::::" <<a->_method.size() <<endl;
		for(int i=0;i<(a->_method).size(); i++)
		{
			cout <<"method_____________________"<<i<< "________________ "<<endl;
			cout<< a->_method.at(i)->toString()<<endl;
		}
#endif
//	}
#if 0
	map<int,int>::iterator it;
	map<int, int>* maptest;
	maptest = dfr.getClassToTypeidx();
	cout << "print out the class type  map~~~~~~~~~~~~~~~~~~~~" <<endl;
	for(it = (*maptest).begin(); it!=(*maptest).end();it++)
	{
		cout <<"the key value: " << (*it).first <<endl;
		cout << "the mapped value: " <<(*it).second<<endl;
	}
	
       //	cout << *(dfr.getDataIn()) << endl;
#endif
#if 0
	cout << "String IDs size : "<<dfr.getStringIdsSize() <<endl;
	printf(" String IDs Off : %x hex \n", dfr.getStringIdsOff());
	cout << "Type Ids size : " << dfr.getTypeIdsSize() <<endl;
	printf("Type IDs off %x hex \n", dfr.getTypeIdsOff());

	
	cout << dfr.getFieldIdsSize() <<endl;
	printf("Field Ids Off :  %x hex \n", dfr.getFieldIdsOff());
	
	
	cout << "Method Ids size : "<<dfr.getMethodIdsSize() <<endl;
	printf("Method Ids Off %x hex \n", dfr.getMethodIdsOff());


	cout << "class def size : "<< dfr.getClassDefSize() <<endl;
	printf("class def offset %x hex\n", dfr.getClassDefOff());

	
	cout <<"Data size : " <<dfr.getDataSize() <<endl;
	printf("Data off %x hex \n", dfr.getDataOff());
	
#endif
#if 0
	cout << "print out all Methods in the data segment~~~~~~~~~~~~~~~" <<endl;
	
	int methodsize = dfr.getMethodIdsSize();	
	cout << "Method Ids size : "<<methodsize <<endl;
	printf("Method Ids Off %x hex \n", dfr.getMethodIdsOff());
	for (int i = 0; i < 16; i++)
	{
		string tempstr = dfr.getMethod(i)->toString();
		cout <<"Method content ::::::::::::" << i <<": " <<tempstr <<endl;
	}
#endif
#if 0
	int typesize = dfr.getTypeIdsSize();
	cout << "print out all types in the data segment~~~~~~~~~~~~~~`"<<endl;
	for(int i = 0; i< typesize; i++)
	{
		string temps = dfr.getType(i);
		cout << "Type content ******************" << i << ": " <<temps <<endl;
	}
#endif


}
#endif
