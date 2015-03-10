#include "DexToSail.h"
#define TESTING_TYPE true
void printCFG(sail::Function* f){
	cout << "CFG :: " << endl
						<< f->get_cfg()->to_dotty(false)
						<< endl;
//				cout << "CFG PP ::" << endl
//						<< f->get_cfg()->to_dotty(true)
//						<< endl;
}


sail::TranslationUnit* dex_to_sail(const string & dex_file)
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


   	sail::TranslationUnit* tu = NULL;
   	vector<sail::Function*> functions ;
   		//sail::Serializer* s = NULL;

   	//sail::TranslationUnit* tu_global = NULL;
   	//vector<sail::Function*> clinits;
   	//sail::Serializer* s_static = NULL;
   	if(USING_STATIC_VALUE){
   		//clinits.push_back(in2debug->getSailInitFun());
   		//used to test, first skip the functions;

			functions.push_back(in2debug->getSailInitFun());
			if (PRINT_CFG) {
				printCFG(in2debug->getSailInitFun());
			}

   	}


   	////////////////////////////////////////////




	//HZHU add global init function to functions; ONLY ONE init_static says everything
	/***************************************************************
	 * initialize all instructions inside sorted clinits
	 **************************************************************/
	if (USING_CLINIT) {
		vector<CodeHeader*> sortedcodeh = in2debug->getSortedClinitsVector();
		vector<CodeHeader*>::iterator sorted_clinit_it = sortedcodeh.begin();
		cout << "Size of clinits " << sortedcodeh.size() << endl;
		int p = 0;
		int k = 0;
		while (sorted_clinit_it != sortedcodeh.end()) {

			//used to test, skip processing cinit; should be commented out after testing;
//			if(1)
//				break;




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

			cout << endl
					<<"------------------ static_field init function ----------------- "
					<< k++ << endl;
			cout << "total size of instruction " << mymap_1->size() << endl;
			int idx = 0;
			for (; it_static != mymap_1->end(); it_static++) {
				unsigned int addr = it_static->first;
				cout << "idx = " << idx ++ << endl;
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
			//clinits.push_back(f_static);
			functions.push_back(f_static);
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
//
//			if( u != 125)
//				continue;

//			if((u != 3036 )&&(u != 3040) &&(u != 3055))
//				continue;
//			if(u != 114)
//				continue;

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
			cout << "Total instruction size  " << mymap->size() << endl;
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
				//if (DEBUG) {
				if (1){
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
			if(ret_t != NULL)
				cout << "Function return type is " << ret_t->to_string() << endl;
			il::function_type* fn_signature = il::function_type::make(ret_t,
					*para_list, false);
			//cout << "FUNSIG is " << fn_signature->to_string() << endl;
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
   	//tu_global = new sail::TranslationUnit(clinits, NULL, "");
   	//s_static = new sail::Serializer(tu_global, out_path);


   	//serialize normal methods;
   	tu = new sail::TranslationUnit(functions, NULL, "");
   	//s = new sail::Serializer(tu, out_path);

   	delete dfr;

   	/*
	if (TESTING_TYPE) {
		cout << "testing type  === " << endl;
		il::type* t = getType(0, dfr,
				dfr->getClassDefByName(
						"Lcom/gpl/rpg/AndorsTrail/model/item/List;"), typemap);
		if (t->get_inner_type()->is_record_type()) {
			il::record_info * r_info =
					((il::record_type*) (t->get_inner_type()))->get_field_from_name(
							"clothing");
			assert(r_info->t != NULL);
			cout << "r_info === " << r_info->t->to_string() << endl;
		}
	} */
   	return tu;
}


void SerializeToSailFile(sail::TranslationUnit* translation_unit, const string& outpath){
	sail::Serializer* s = new sail::Serializer(translation_unit, outpath);
	delete s;
}
