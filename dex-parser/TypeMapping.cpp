#include "TypeMapping.h"
#include "DataIn.h"
#include "ClassDefInfo.h"
#define TYPE_CHECK false
#define PRINT false
#define PRINT_TYPE false

/*
static void IndentPrint(int level){
    for (int i = 1; i< level; i++)
	cout << "\t";
}*/

/* Input
 * try the cvs/try again
 *
 * */

il::type* makeLibraryType(DexFileReader* dfr, string t_name, map<string, ClassDefInfo*>* typemap ) {
	string name = t_name;
	ClassDefInfo* cdi = NULL;

	cdi = dfr->getClassDefByName(name);
//	if(t_name == "Landroid/content/pm/ApplicationInfo;"){
//		cout << "t_name  " << t_name << endl;
//		cout << "cdi address " << cdi << endl;
//		//exit(1);
//	}


	//we do not need to keep record type, only keep pointer type and primitive type
//	if (NEED_ILTYPE_MAP) {
//		cdi = dfr->getClassDefByName(name);
//		if (dfr->insideBuiltilType(cdi))
//			return dfr->getilType(cdi)->get_inner_type();
//	}
//	//1/3

	vector<il::record_info*>* elem = new vector<il::record_info*>();
	il::record_info* pri = NULL;
	if (!cdi->getLibTypeFieldsUpdateFlag()) { //fields not updated
		pri = new il::record_info();
		pri->fname = "fakefield";
		pri->offset = 0;
		pri->t = il::get_integer_type();
		elem->push_back(pri);
	}else{ //fields updated //and assume that lib types doesn't depends on each other
		vector<Field*>* fields = cdi->getField();
		assert(fields->size() != 0);
		for(int i = 0; i < (int)fields->size(); i++){ //build up each pri
			assert(fields->at(i) != NULL);
			pri = new il::record_info();
			Field* f = fields->at(i);
			pri->fname = f->getName();
			pri->offset = f->getOffset();
			//get the type information;
			string type_name = f->getType();
			ClassDefInfo* f_cdi = dfr->getClassDefByName(type_name);
			//this one is flaw,because lib type may depends on each other
			pri->t = TypeMapping(0, dfr, f_cdi, typemap);
			elem->push_back(pri);
		}
	}

	il::namespace_context ns;
	il::type* type = il::record_type::make(name, ns, 0, 0, false, true, *elem,
			name, false);

	//do not need to keep record type, only keep pointer type
//	if(NEED_ILTYPE_MAP){
//			dfr->setilTypeBuilt(cdi,type);
//	}

	//return il::record_type::make(name, ns, 0, 0, false, true, *elem, name, false);
	return type;
}

il::type* makePrimitiveType(DexFileReader* dfr, string str) {
	//1/3
//	ClassDefInfo* cdi = NULL;
//	if (NEED_ILTYPE_MAP) {
//		cdi = dfr->getClassDefByName(str);
//		if (dfr->insideBuiltilType(cdi))
//			return dfr->getilType(cdi);
//	}
	//1/3
	il::type* type = NULL;
	if (str == "V") {
		type = il::get_void_type();
	}

	if (str == "C") {
		type = il::get_char_type();
		//type = il::get_integer_type();
	}
	if ((str == "I") || (str == "B") || (str == "S") || (str == "J")
			|| (str == "Z")) {
		//il::value_type vt = il::INTEGER;
		type = il::get_integer_type();
		/*
		 if(str == "B")
		 type = il::base_type::make("byte",8,8,true,vt, "BYTE");
		 if(str == "S")
		 type = il::base_type::make("short",16,16,true,vt, "SHORT");
		 if(str == "J")
		 type = il::base_type::make("long int",64,64,true,vt, "LONG INT");
		 */
		//cout << "Type: " <<type->to_string() << endl;
	}
	/*
	 if(str == "Z")
	 {
	 il::value_type vt = il::BOOL;
	 type = il::base_type::make("bool", 8,8, false, vt, "BOOLEAN");
	 //cout << "Type: " <<type->to_string() << endl;
	 }
	 */
	if ((str == "F") || (str == "D")) { //we don't care about the bool type in Compass
		//il::value_type vt = il::INTEGER;
		type = il::get_integer_type();
		/*
		 il::value_type vt = il::IEEE_FLOAT;
		 type = il::base_type::make("ieee_float", 32,32, true, vt, "IEEE_FLOAT");
		 if (str == "D")
		 type = il::base_type::make("ieee_double", 64,64, true, vt, "IEEE_DOUBLE");
		 //cout << "Type: " <<type->to_string() << endl;
		 */
	}
	/*add for invalid type*/
	if (str == "INVALID")
		type = il::invalid_type::make();
	/*end*/
	if (type == NULL) {
		cout << str << " is not a primitive type, check it " << endl;
		assert(false);
	}
//	//1/3
//	if (NEED_ILTYPE_MAP)
//		dfr->setilTypeBuilt(cdi, type);
//	//1/3
	return type;
}







il::type* TypeMapping(unsigned int level, DexFileReader* dfr, ClassDefInfo* t,
		map<string, ClassDefInfo*>* typemap) {
	assert( dfr != NULL);
	assert( t != NULL);
	assert( typemap != NULL);

//	//1/3
//	if (NEED_ILTYPE_MAP) {
//		if (dfr->insideBuiltilType(t))
//			return dfr->getilType(t);
//	}
//	//1/3

	string str = t->getTypename();
	//cout  <<"TypeMapping " << str << endl;
	assert(str != "");

	bool check_flag = false;
//	if(str == "Landroid/content/pm/ApplicationInfo;"){
//		cerr << "yes, Find the type in typemapping! " << endl;
//		check_flag = true;
//	}
	il::type* type = NULL;

	// This index is not used to index the user defined class, but a global table to maintain
	// all the used types like , int, double, library classes, etc...
	int type_idx = t->getTypeIdx();
	// this function call only return user defined class index, for other types it should return -1;

	map<int, vector<il::type**>*>* unresolved_types = dfr->getUnresolvedTypes();
	map<int, il::type*>* resolved_types = dfr->getResolvedTypes();
	if ((str.substr(0, 1) == "L") || (str.substr(0, 1) == "[")
			|| (str.substr(0, 1) == "s")) { //object is pointer;

		il::type* inner = NULL;
		if (str.substr(0, 1) == "s")
			str = "Ljava/lang/String;";


		// this is NOT else if
		if (str.substr(0, 1) == "L") {
			vector<Field*>* field = t->getField();

			/*******
			 * field size is zero can't guarantee that it is library type; because all iget/iput fields are collect for library type;
			 */
			if ((field != NULL) && (field->size() != 0)) {//self-defined type has is own fields
				bool hello = false;
//				if(str == "Landroid/content/pm/ApplicationInfo;"){
//						cerr << "yes, here 2! " << endl;
//						cerr << "size of field " << field->size() << endl;
//						hello = true;
//				}
				vector<Field*>::iterator it_field;
				vector<il::record_info*>* elems =
						new vector<il::record_info*>();
				il::record_info* p_ri = NULL;
				int size = 0;

				for (it_field = field->begin(); it_field != field->end();
						it_field++) {
					p_ri = new il::record_info();
					string name = (*it_field)->getName();
					p_ri->fname = name; // fill record_info with field name
//					if (PRINT){
					if(hello){
								cerr << "field name: " << name << endl;
					}

					int offset = (*it_field)->getOffset();
					p_ri->offset = offset; // fill record_info with offset
					//if (PRINT){
					if(hello){
							cerr << "offset = " << offset << endl;
					}

					unsigned int typeidx = (*it_field)->getTypeIdx();
					ClassDefInfo* typ = type2ClassDef(dfr, typeidx);
					string inner_type_name = typ->getTypename();
					//if (PRINT){
					if(hello){
						cerr << "field type = " << inner_type_name << endl;
					}

					//processing the field type;
					int inner_type_idx = typ->getTypeIdx(); // get inner type index
					assert(inner_type_idx == typeidx);

					bool is_self_defined = dfr->isSelfDefinedType(
							inner_type_idx);

					//if (-1 == inner_type_idx) { // not user defined class, could be primitive or library
					if (!is_self_defined) { // not user defined class, could be primitive or library
						// change "s" to "string"
						//cout << " not userdefined class " << endl;
						if ("s" == inner_type_name.substr(0, 1)) {
							inner_type_name = "Ljava/lang/String";
						}

						if ("L" == inner_type_name.substr(0, 1)) {
							// make the library type
							//il::type* r_type = makeLibraryType(inner_type_name);
							//1/3
							//cout << "1111111111 " << endl;
							il::type* r_type = makeLibraryType(dfr,
									inner_type_name, typemap);
							//1/3
							p_ri->t = il::pointer_type::make(r_type,
									inner_type_name);
							if(check_flag)
								cout << "making the field type is p_ri->t " << p_ri->t->to_string() << endl;
						} else if ("[" == inner_type_name.substr(0, 1)) {

							// note that it is possible to have [[[, not handled yet
							//cout << "inner type name is " << inner_type_name << endl;
							//string name = inner_type_name.substr(1, str.size() - 1); //if [[a, should be [a or L or primitive type
							string name = inner_type_name.substr(1,
									inner_type_name.size() - 1);
							//string name = inner_type_name;
							//cout << "HHHHHHHH ---------------name is " << name << endl;
							ClassDefInfo* inner_type = NULL;

							if (typemap->count(name) > 0) {
								inner_type = (*typemap)[name];
							}
							il::type* temp = NULL;
							if (inner_type != NULL) {
								//cout << " ---inner type is " << inner_type->getTypename() << endl << "and belongs to  ==" << str << endl;

								temp = TypeMapping(0, dfr, inner_type, typemap);
								//temp = TypeMapping(level, dfr, inner_type, typemap);

							} else {
								inner_type = dfr->getClassDefByName(name);

								temp = TypeMapping(0, dfr, inner_type, typemap);
								//temp = TypeMapping(level, dfr, inner_type, typemap);
							}

							type = il::pointer_type::make(temp, name);
							p_ri->t = type;

						} else {
							// make the primitive type
							p_ri->t = makePrimitiveType(dfr, inner_type_name);
						}
					} else { // handle

						/* The idea here is to first set record_info->t to NULL for user defined type.
						 * Then
						 *
						 **/

						assert(unresolved_types != NULL);
						assert(resolved_types != NULL);
						//cout << "user defined class " << endl;
						if (resolved_types->count(inner_type_idx)) { // inner type has been made instances to the pool already
							/*il::type* r_type = (*resolved_types)[inner_type_idx];
							 p_ri->t = il::pointer_type::make(r_type, inner_type_name);*/
							p_ri->t = (*resolved_types)[inner_type_idx];
						} else { // never been made before, insert to a map
							p_ri->t = NULL;
							il::type** unresolved = &(p_ri->t);
							if (unresolved_types->count(inner_type_idx) > 0) {
								//if unresolved_types already have inner_type_idx vector, just push back
								//then push the addr of (p_ri->t)
								(*unresolved_types)[inner_type_idx]->push_back(
										unresolved);
							} else {
								//generate the vector space
								vector<il::type**>* v =
										new vector<il::type**>();
								v->push_back(unresolved);
								(*unresolved_types)[inner_type_idx] = v;
							}
						}
					}

					if(check_flag){
					cout << "** p_ri ->type is " << p_ri->t->to_string() << endl;
					cout << "** p_ri->fn is " << p_ri->fname << endl;
					}
					elems->push_back(p_ri);

					if (it_field == field->end() - 1) {
						size = offset
								+ (*it_field)->setNextOff(typ->getTypename());
					}
				} //end for
				  //if((elems != NULL)&& (elems->size() != 0)){
				assert(elems != NULL);
				int innersize = size; //need to update //I thought it should have total field size of the class;
				int innerasize = size; //need to update
				//cout << "after set field to NULL and Suck here " << str << endl;

				/*******************************************************************************************
				 *** Here a new record_type is newed, we need to check its unresolved_types map and refill
				 *** unfilled blanks
				 *******************************************************************************************/
				inner = il::record_type::make(str, il::namespace_context(),
						innersize, innerasize, false, true, *elems, str, false);
				if(check_flag)
					cout << "making record type " <<inner->to_string() << endl;
				//(*resolved_types)[type_idx] = inner; // update resolved map

			} // end self define type *************


			else { //field size is zero must be library type
				//1/3
				//inner = makeLibraryType(str);
				//cout << "2222222222222 " << endl;
				inner = makeLibraryType(dfr, str, typemap);

				//1/3
			}
		} else { // dealing with [
				 // note that it is possible to have [[[, not handled yet
			string name = str.substr(1, str.size() - 1); //if [[a, should be [a or L or primitive type
			if (PRINT)
				cout << "HHHHHHHHH check name " << name << endl;

			ClassDefInfo* inner_type = NULL;

			if (typemap->count(name) > 0)
				inner_type = (*typemap)[name];

			if (inner_type != NULL) { //self defined?
				//cout << "*** inner type is ----" << inner_type->getTypename() << endl;

				//cout << "inner type is " << inner_type->getTypename() << endl;
				inner = TypeMapping(0, dfr, inner_type, typemap);
			} else {
				inner_type = dfr->getClassDefByName(name);

				inner = TypeMapping(0, dfr, inner_type, typemap);
			}
		} //end dealing with [



		if (inner != NULL) {
			type = il::pointer_type::make(inner, str);
			/*******************************************************************************************
			 *** Here a new record_type is newed, we need to check its unresolved_types map and refill
			 *** unfilled blanks
			 *******************************************************************************************/
			(*resolved_types)[type_idx] = type; // update resolved map
			if (unresolved_types->count(type_idx) > 0) {
				vector<il::type**>* waitings = (*unresolved_types)[type_idx];
				vector<il::type**>::iterator it = waitings->begin();

				for (; it != waitings->end(); it++) {
					il::type** t = *it;
					*t = type;
				}
				unresolved_types->erase(type_idx);
			}
			/*******************************************************************************************
			 *** Type A is handled, need to check the whether there is a need to create array type [A]
			 *** and if it is needed, update the unfilled blanks
			 *******************************************************************************************/
			string array_str = "[" + str;
			unsigned int array_idx = dfr->getTypeIdx(array_str);
			//cout << "array_str  == " << array_str <<  "and idx  == "<< array_idx<<endl;
			//assert(array_idx != NO_INDEX);
			if (array_idx != NO_INDEX) {
				if (unresolved_types->count(array_idx) > 0) {
					il::type* array_type = il::pointer_type::make(type,
							array_str);
					(*resolved_types)[array_idx] = array_type; // update resolved map
					vector<il::type**>* waiting_list =
							(*unresolved_types)[array_idx];
					vector<il::type**>::iterator it = waiting_list->begin();

					for (; it != waiting_list->end(); it++) {
						il::type** t = *it;
						*t = array_type;
					}
					unresolved_types->erase(array_idx);
				}
			}

		}
	} else { //primitive type
		type = makePrimitiveType(dfr, str);
	}


	assert(type!=NULL);

//	//1/3
//	if (NEED_ILTYPE_MAP){
//		if(t->getTypename()== "Leu/domob/angulo/Vector;"){
//			cerr << "first time to build the type ! " << endl;
//			assert(false);
//		}
//		dfr->setilTypeBuilt(t, type);
//	}
//	//1/3
	return type;
}




void updateRecordType(DexFileReader* dfr, il::type* t,
		ClassDefInfo* out_layer_classdef, map<string, ClassDefInfo*>* typemap) {
	if (t == NULL)
		return;
	if (t->is_pointer_type()) {
		if (t->get_inner_type()->is_record_type()) {
			//check it hierarchy
			bool has_sup = false;
			bool has_inteface = false;
			il::record_type* record_type =
					(il::record_type*) (t->get_inner_type());
			// add hierarachy
			if ((out_layer_classdef->hasSuperClass())
					&& (out_layer_classdef->getSuperClass() != "")) {
				has_sup = true;
				//new a super type for it
				//cout << "adding base class to it" << out_layer_classdef->getTypename()<<endl;
				string sup_class = out_layer_classdef->getSuperClass();
				ClassDefInfo* sup_cdi = (*str2Classdef(dfr))[sup_class];

				il::type* sup_t_p = TypeMapping(0, dfr, sup_cdi, typemap);
				//cout << "sup_t_p is " << sup_t_p->to_string() << endl;
				il::record_type* sup_t;
				if (sup_t_p != NULL) {
					if (sup_t_p->is_pointer_type()) {
						if (sup_t_p->get_inner_type()->is_record_type()) {
							sup_t =
									(il::record_type*) (sup_t_p->get_inner_type());
							if (PRINT)
								cout << " ==> " << sup_t->to_string() << endl;
							record_type->add_base(sup_t, 0);
							if (!alreadyInDerivatives(sup_t, record_type))
								sup_t->add_derivative(record_type);
						}
					}
				}
			}
			if ((out_layer_classdef->hasInterface())
					&& (out_layer_classdef->getInterface() != NULL)) {
				has_inteface = true;
				//cout << "adding interface to it " << out_layer_classdef->getTypename()<<endl;
				vector<string>* interface = out_layer_classdef->getInterface();
				//cout << "size is " << interface->size()<<endl;
				for (unsigned int i = 0; i < interface->size(); i++) {
					int fakeoffset = i + 1;
					string cur_interf = interface->at(i);
					ClassDefInfo* inte_cdi = (*str2Classdef(dfr))[cur_interf];

					il::type* inte_t_p = TypeMapping(0, dfr, inte_cdi, typemap);
					il::record_type* inte_t;
					if (inte_t_p != NULL) {
						if (inte_t_p->is_pointer_type()) {
							if (inte_t_p->get_inner_type()->is_record_type()) {
								inte_t =
										(il::record_type*) (inte_t_p->get_inner_type());
								if (PRINT)
									cout << " ==> " << inte_t->to_string()
											<< endl;
								record_type->add_base(inte_t, fakeoffset);
								if (!alreadyInDerivatives(inte_t, record_type))
									inte_t->add_derivative(record_type);
							}
						}
					}
				}

			}
			if ((out_layer_classdef->getTypename() != "Ljava/lang/Object;")
					&& !has_sup && !has_inteface) {
				//cout << "out_layer_classdef :: " << out_layer_classdef->getTypename() << endl;
				il::record_type* record_type =
						(il::record_type*) (t->get_inner_type());
				//cout << "record type of out_layer_classdef " << record_type->to_string() << endl;

				ClassDefInfo* sup_cdi = dfr->getClassDefByName(
						"Ljava/lang/Object;");
				//cout << "classdef is " << sup_cdi->getTypename() << endl;

				il::type* base_type = TypeMapping(0, dfr, sup_cdi, typemap); //pointer type of Ljava/lang/Object;
				assert(base_type->is_pointer_type());
				//cout << "base_type :: " << base_type->to_string() << endl;
				il::record_type* sup_type =
						(il::record_type*) base_type->get_inner_type(); //which is also a record type of Ljava/lang/Object;
				record_type->add_base(sup_type, 0);
				if (!alreadyInDerivatives(sup_type, record_type))
					sup_type->add_derivative(record_type);
			}
			//end of hierarchy
			if (out_layer_classdef->hasMethod()) {
				vector<Method*>* methods = out_layer_classdef->getMethod();
				for (unsigned int i = 0; i < methods->size(); i++) {
					Method* cur_m = methods->at(i);
					//prepare return type
					unsigned int ret_idx = cur_m->getReturnidx();
					ClassDefInfo* cdi = type2ClassDef(dfr, ret_idx);

					il::type* ret_t = TypeMapping(0, dfr, cdi, typemap);
					if (ret_t->is_void_type())
						ret_t = NULL;

					//prepare parameter types
					unsigned int para_size = cur_m->getParaSize();
					vector<il::type*>* arg_type = new vector<il::type*>();
					il::function_type* fn_signature;
					//haiyan add this pointer 8.13
					il::type* this_p_t = NULL;
					if (!cur_m->isStatic()) { //need to add this_p_t;
						//	cout << ">>>>>>>>>>>>>>adding THIS pointer to " << cur_m->toString() << endl;
						unsigned int this_p_idx = cur_m->getClassOwneridx();
						ClassDefInfo* this_p_cdf = type2ClassDef(dfr,
								this_p_idx);
						//	cout << "this_p_cdf" << this_p_cdf->getTypename()<<endl;

						this_p_t = TypeMapping(0, dfr, this_p_cdf, typemap);
						arg_type->push_back(this_p_t);
					}

					//haiyan end this pointer 8.13
					//cout << "para_size " << para_size << endl;
					if (para_size == 0) { //fake a para type which is void
						//cout << "parameter has nothing "  <<  endl;
						//arg_type->push_back(NULL);
						fn_signature = il::function_type::make(ret_t, *arg_type,
								false);
					} else { //prepare for the method's parameters types

						//vector<ClassDefInfo*>* cdi_list = new vector<ClassDefInfo*>();
						//	cout << "has " << para_size << "parameters " << endl;
						//arg_type = new vector<il::type*>();
						vector<unsigned int>* paras_idx =
								cur_m->getParaTypeIds();
						for (unsigned int j = 0; j < paras_idx->size(); j++) {
							unsigned int cur_type_idx = paras_idx->at(j);
							ClassDefInfo* cur_cdi = type2ClassDef(dfr,
									cur_type_idx);

							il::type* cur_para_t = TypeMapping(0, dfr, cur_cdi,
									typemap);
							arg_type->push_back(cur_para_t);
						}
						fn_signature = il::function_type::make(ret_t, *arg_type,
								false);
					}
					//  cout << "total arg size including this  " << arg_type->size()<<endl;
					//cout << "fn_signature when adding methods : " << fn_signature->to_string()<< endl;
					if (PRINT) {
						cout << "m name " << cur_m->getName() << endl;
						cout << "m is virtual " << cur_m->isVirtual() << endl;
						cout << "m is Static " << cur_m->isStatic() << endl;
						cout << "m is Abstract " << cur_m->isAbstract() << endl;
					}
					StringSplit* sp = new StringSplit(cur_m->getClassOwner(),
							"$");
					il::namespace_context ns = sp->makeNameSpace();
					if (NAME_SPACE_PRINT)
						cout << "ns " << ns.to_string() << endl;
					record_type->add_member_function(cur_m->getName(), ns,
							fn_signature, cur_m->isVirtual(), cur_m->isStatic(),
							cur_m->isAbstract());
					delete sp;
				}
			}
		}

	}
}

il::type* getType(unsigned int fieldlevel, DexFileReader* dfr,
		ClassDefInfo* cdi, map<string, ClassDefInfo*>* typemap){
//	cerr << " -- begin type mapping " << endl;
//	if(cdi->getTypename() == "Leu/domob/angulo/Vector;")
//		assert(false);
	//1/3
	if (NEED_ILTYPE_MAP) {
		if (dfr->insideBuiltilType(cdi)){
			//cerr <<cdi->getTypename() <<"  found in built type " << endl;
			return dfr->getilType(cdi);
		}
	}
	//1/3
	//cout << endl <<" == getType " << cdi->getTypename() << "Addr == "<< cdi <<endl;
	il::type* t = TypeMapping(fieldlevel, dfr, cdi, typemap);
	//cout << "-- end type mapping " << endl << endl;
	//cout << "before updating " << t->to_string()<<endl;
	updateRecordType(dfr, t, cdi, typemap);
	if ((PRINT_TYPE) && (t != NULL)) {
		if (t->is_pointer_type()) {
			if (t->get_inner_type()->is_record_type()) {
				il::record_type* rt =
						static_cast<il::record_type*>(t->get_inner_type());
				map<int, il::record_type*> bases = rt->get_bases();
				map<int, il::record_type*>::iterator it = (&bases)->begin();

				if (0) {
					for (; it != bases.end(); it++) {
						  cout << "offset : " << (*it).first << endl;
						  cout << "type :"  << (*it).second << endl;
					}
				}
			}
		}
	}

	if (NEED_ILTYPE_MAP) {
		dfr->setilTypeBuilt(cdi, t);
	}

	return t;
}

bool alreadyInDerivatives(il::record_type* base, il::record_type* deriv)
{
	assert(base != NULL);
	vector<il::record_type*> deriv_types = base->get_derivatives();
	for (unsigned int i = 0; i < deriv_types.size(); i++) {
		if (deriv_types.at(i) == deriv)
			return true;
	}
	return false;
}

