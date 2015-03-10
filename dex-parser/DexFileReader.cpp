#include"DexFileReader.h"

#define ANNOTATION_DEBUG false
#define PARAMETER_CHECK false
#define LOCAL_VAR_CHECK false
#define FIELD_UPDATE_CHECK true

const char DexFileReader::DEX_FILE_MAGIC[3] = { 0x64, 0x65, 0x78 };
const char DexFileReader::VERSION_35[3] = { 0x30, 0x33, 0x35 };
const char DexFileReader::VERSION_36[3] = { 0x30, 0x33, 0x36 };
vector<CodeHeader*>* DexFileReader::_codeheader_list = NULL;
//after read the dex file, current position in the end of the header;
DexFileReader::DexFileReader(DataIn* in) {
	_mid_method = new map<unsigned int, Method*>();
	_selfd_method_map = new map<unsigned int, Method*>();
	_m_throws = new map<unsigned int, vector<string>*>();
	//_selfd_mid = new set<unsigned int>();
	//_class_typeidx = NULL;
	this->_datain = in;
	//char magic;

	_datain->move(0);
	/*
	 for(int i = 0; i < 3; i++)
	 {
	 magic = in->readByte();
	 if( magic == DEX_FILE_MAGIC[i])
	 continue;
	 else{
	 cout << "magic number doesn't match, exist!!" <<endl;
	 break;
	 }
	 }
	 */
	_datain->skip(8 + 4 + 20); //skip info that we do not care about 8 for maigic value, 4 for checksum and 20 for signature
	//_datain->skip(4);//skip length of file in bytes
	//_file_size = _datain->readIntx();	
	_file_size = _datain->readUIntx();
	//_datain->skip(4); //skip size of file	
	_datain->skip(4); //skip length of the headsize
	//int headsize = _datain->readIntx();

	//hzhu comments for check bigendian

	int endian = _datain->readIntx(); // (0x12345678 == little endian) ; (0x78563412 == big endian)i
#ifdef DEBUG
			cout << endl;
			cout << "this is the endian value :" << endian <<endl;
			cout << "              " <<endl;
			printf("%.x\n",endian);
			//cout << endl<<endl;
#endif		
	if (endian == ENDIAN_CONSTANT) {
		this->_little_endian_flag = true;
#ifdef DEBUG
		cout << " little flag :  " << this->_little_endian_flag << endl;
		cout << "yes, it is little endian!" <<endl;
#endif
	}
	//skip unit_link_size
	//skip unit_link_off
	//skip unit map_off
	_datain->skip(4 + 4 + 4);

	if (_little_endian_flag) {
		_string_ids_size = _datain->readIntx();
#ifdef DEBUG
		cout << "string id size is: " << _string_ids_size <<endl;
#endif
		_string_ids_off = _datain->readIntx(); // the table contain all the offset(absolute address) for the acutually string constants in the .dex file

#ifdef DEBUG
		cout << "string id offset is : " << _string_ids_off <<endl;
		printf ("%.2xhex\n", _string_ids_off );
#endif

		_type_ids_size = _datain->readIntx();
#ifdef DEBUG
		cout << "type id size is:  " <<_type_ids_size<<endl;
#endif
		_type_ids_off = _datain->readIntx();

#ifdef DEBUG
		cout << "type ids address is :  " <<_type_ids_off << endl;
		printf("%.2x hex \n", _type_ids_off);
#endif

		//	_datain->skip(4);//skip _proto_ids_size
		//	_datain->skip(4);//skip _proto_ids_off

		_proto_ids_size = _datain->readIntx();
		_proto_ids_off = _datain->readIntx();
#ifdef DEBUG
		cout << "proto size : " << _proto_ids_size <<endl;
		cout << "proto offset : " <<_proto_ids_off <<endl;
		printf("%.2x hex \n", _proto_ids_off);
#endif

		_field_ids_size = _datain->readIntx();
		_field_ids_off = _datain->readIntx();

#ifdef DEBUG
		cout << "field_ids size : " << _field_ids_size <<endl;
		cout << "field_ids  offset : " <<_field_ids_off <<endl;
		printf("%.2x hex \n", _field_ids_off);
#endif

		_method_ids_size = _datain->readIntx();
		_method_ids_off = _datain->readIntx();
#ifdef DEBUG
		cout << "method_id size : " << _method_ids_size <<endl;
		cout << "method_id  offset : " <<_method_ids_off <<endl;
		printf("%.2x hex \n", _method_ids_off);
#endif
		_class_def_size = _datain->readIntx();
		_class_def_off = _datain->readIntx();
		//cout << "class_def size : " << _class_def_size <<endl;
#ifdef DEBUG
		cout << "class_def size : " << _class_def_size <<endl;
		cout << "class_def  offset : " <<_class_def_off <<endl;
		printf("%.2x hex \n", _class_def_off);
#endif

		_data_size = _datain->readIntx();
		_data_off = _datain->readIntx();
#ifdef DEBUG
		cout << "data  size : " << _data_size <<endl;
		cout << "data   offset : " <<_data_off <<endl;
		printf("%.2x hex \n", _data_off);
#endif
	} else {

		_string_ids_size = _datain->bigEndian_readIntx();
#ifdef DEBUG
		cout << "string id size is: " << _string_ids_size <<endl;
#endif
		_string_ids_off = _datain->bigEndian_readIntx(); // the table contain all the offset(absolute address) for the acutually string constants in the .dex file

#ifdef DEBUG
		cout << "string id offset is : " << _string_ids_off <<endl;
		printf ("%.2xhex\n", _string_ids_off );
#endif

		_type_ids_size = _datain->bigEndian_readIntx();
#ifdef DEBUG
		cout << "type id size is:  " <<_type_ids_size<<endl;
#endif
		_type_ids_off = _datain->bigEndian_readIntx();

#ifdef DEBUG
		cout << "type ids address is :  " <<_type_ids_off << endl;
		printf("%.2x hex \n", _type_ids_off);
#endif

		//	_datain->skip(4);//skip _proto_ids_size
		//	_datain->skip(4);//skip _proto_ids_off

		_proto_ids_size = _datain->bigEndian_readIntx();
		_proto_ids_off = _datain->bigEndian_readIntx();
#ifdef DEBUG
		cout << "proto size : " << _proto_ids_size <<endl;
		cout << "proto offset : " <<_proto_ids_off <<endl;
		printf("%.2x hex \n", _proto_ids_off);
#endif

		_field_ids_size = _datain->bigEndian_readIntx();
		_field_ids_off = _datain->bigEndian_readIntx();

#ifdef DEBUG
		cout << "field_ids size : " << _field_ids_size <<endl;
		cout << "field_ids  offset : " <<_field_ids_off <<endl;
		printf("%.2x hex \n", _field_ids_off);
#endif

		_method_ids_size = _datain->bigEndian_readIntx();
		_method_ids_off = _datain->bigEndian_readIntx();
#ifdef DEBUG
		cout << "method_id size : " << _method_ids_size <<endl;
		cout << "method_id  offset : " <<_method_ids_off <<endl;
		printf("%.2x hex \n", _method_ids_off);
#endif
		_class_def_size = _datain->bigEndian_readIntx();
		_class_def_off = _datain->bigEndian_readIntx();
#ifdef DEBUG
		cout << "class_def size : " << _class_def_size <<endl;
		cout << "class_def  offset : " <<_class_def_off <<endl;
		printf("%.2x hex \n", _class_def_off);
#endif

		_data_size = _datain->bigEndian_readIntx();
		_data_off = _datain->bigEndian_readIntx();
#ifdef DEBUG
		cout << "data  size : " << _data_size <<endl;
		cout << "data   offset : " <<_data_off <<endl;
		printf("%.2x hex \n", _data_off);
#endif
	}

	_datain->getCurrentPosition();
	_global_variables = new map<string, sail::Variable*>();
	_global_variables_starting_number = 30000;
	_str_classdef = new map<string, ClassDefInfo*>();
	_ilt_2_classdef = new map<il::type*, ClassDefInfo*>();
	_resolved_types = new map<int, il::type*>();
	_unresolved_types = new map<int, vector<il::type**>*>();
	_classdef_idx_ele = new map<unsigned int, ClassDefInfo*>();
	_base_field_unresolved_selfclassdef =
			new map<unsigned int, ClassDefInfo*>();
	_unresolved_lib_boundary_type_idx = new vector<unsigned int>();
	_classdefidx_typeidx = new map<int, int>(); //flip key and value with _class_typeidx
	_typeidx_classdefidx = new map<int, int>();
	_lib_super_selfdefined_derivative = new vector<
			pair<unsigned int, unsigned int>>();
	_file.open("/scratch/output");
	_faked_lib_classdef_with_fields = new map<unsigned int, ClassDefInfo*>();
	_type_map = new map<unsigned int, string>();
	_top_selfdefined_class = new vector<string>();
	_global_vars = new map<string, sail::Variable*>();
	_initial_global_vars_ins = new vector<sail::Instruction*>();

	//12/15
	_codeh_debuginfo_map = new map<CodeHeader*, DebugInfoReader*>();

	_f_map = new map<unsigned int, Field*>();

	//12/19
	_mid_codeh = new map<unsigned int, CodeHeader*>();
	//12/19

	//12/20
	_name_typeid = new map<string, unsigned int>();
	//12/20

	//1/3
	if (NEED_ILTYPE_MAP)
		_classdef_iltype = new map<ClassDefInfo*, il::type*>();
	//1/3

	//1/5
	if (USING_CLINIT)
		_global_variable = new map<string, sail::Variable*>();
	//1/5
	if(USING_STATIC_VALUE){
		_all_static_fields_type = new map<string, string>();
		_all_static_fields_value = new map<string, string>();
	}

	_method_flow_annotations = new map<unsigned int, vector<sail::Annotation*>*>();
}
DexFileReader::~DexFileReader() {
	delete _mid_method;
	delete _m_throws;
	delete _global_variables;
	delete _str_classdef;
	delete _ilt_2_classdef;
	delete _resolved_types;
	delete _unresolved_types;
	delete _classdef_idx_ele;
	delete _base_field_unresolved_selfclassdef;
	delete _unresolved_lib_boundary_type_idx;
	delete _classdefidx_typeidx;
	delete _typeidx_classdefidx;
	delete _lib_super_selfdefined_derivative;
	_file.close();
	delete _faked_lib_classdef_with_fields;
	delete _type_map;
	delete _top_selfdefined_class;
	delete _global_vars;
	delete _initial_global_vars_ins;

	//12/15
	delete _codeh_debuginfo_map;
	delete _selfd_method_map;

	//12/19
	delete _mid_codeh;
	//12/19

	//12/20
	delete _name_typeid;
	//12/20

	delete _method_flow_annotations;
}

bool DexFileReader::getLEndianF() {
	return this->_little_endian_flag;
}

DataIn* DexFileReader::getDataIn() {
	return this->_datain;
}

unsigned int DexFileReader::getFileSize() {
	return this->_file_size;
}
unsigned int DexFileReader::getStringIdsSize() {
	return this->_string_ids_size;

}
int DexFileReader::getStringIdsOff() {
	return this->_string_ids_off;

}

unsigned int DexFileReader::getTypeIdsSize() {
	return this->_type_ids_size;

}

int DexFileReader::getTypeIdsOff() {
	return this->_type_ids_off;

}

unsigned int DexFileReader::getProtoIdsSize() {
	return this->_proto_ids_size;
}

int DexFileReader::getProtoIdsOff() {
	return this->_proto_ids_off;
}

unsigned int DexFileReader::getFieldIdsSize() {
	return this->_field_ids_size;

}

int DexFileReader::getFieldIdsOff() {
	return this->_field_ids_off;

}

unsigned int DexFileReader::getMethodIdsSize() {
	return this->_method_ids_size;

}

int DexFileReader::getMethodIdsOff() {
	return this->_method_ids_off;

}

unsigned int DexFileReader::getClassDefSize() {
	return _class_def_size;

}

int DexFileReader::getClassDefOff() {
	return this->_class_def_off;

}

unsigned int DexFileReader::getDataSize() {
	return this->_data_size;

}

int DexFileReader::getDataOff() {
	return this->_data_off;

}

string DexFileReader::getString(unsigned int id) {
	if (id == 0xffffffff)
		return "";
	if (id >= _string_ids_size) {
		cout << "Id " << id  << " out of bound in string ID table -----bail out!" << endl;
		exit(1);
	}
	int stridoff = _string_ids_off + id * 4;
#ifdef DEBUG
	cout << "### stridoff:" << stridoff << endl;
	cout << _datain->getCurrentPosition() << endl;
	cout << "stacksize:" << (int)_datain->getStackSize() << endl;
#endif
	_datain->pushMove(stridoff);
	unsigned int absoff; //the absolute address that store the string

	if (_little_endian_flag)
		absoff = _datain->readIntx();
	else
		absoff = _datain->bigEndian_readIntx();
	_datain->pushMove(absoff);
	//absoff :: length + data;
	unsigned int length = _datain->readULeb128();
	unsigned int cur_pos = _datain->getCurrentPosition();

	//unsigned int firstpos = absoff+1; // because the 0 byte stores length?

	//_datain->move(cur_pos);
	/*
	 unsigned short firstchar = (unsigned short)_datain->readByte();
	 cout << "firstchar" <<firstchar <<endl;
	 cout << "firstchar is control char : ? " << iscntrl(firstchar) << endl;
	 if(((firstchar>= 0)&&(firstchar <= 31)) || firstchar ==127)
	 {
	 cout << "yes, in" << endl;
	 cur_pos = cur_pos + 1;
	 length = length -1;

	 }
	 _datain->pop();
	 cout << "good here!" << endl;
	 //string decoded_string( _datain->getByteArray()+absoff+1, length); //from utf-8 to string
	 */
	string decoded_string(_datain->getByteArray() + cur_pos, length); //from utf-8 to string
	//following modified // this should some how to deal with the strcpy of utf-8 string, just leave it alone right now and focus on big problem
	/*
	 cout << "first char " << decoded_string[0] << endl;
	 cout << "first char " <<(unsigned short) decoded_string[0] << endl;
	 char dest_str[1000]; // string copy
	 if((((unsigned short)decoded_string[0] >= 0) && ((unsigned short)decoded_string[0] <= 31)) ||((unsigned short)decoded_string[0] == 127))
	 {
	 cout << "go into here!!" << endl;
	 int len = length -1;
	 int size = decoded_string.size();
	 if (size < 1)
	 {
	 return "";
	 }
	 else // 1 is the position
	 {
	 decoded_string.copy(dest_str,len,1);
	 dest_str[len] = '\0';
	 }
	 }
	 else
	 {
	 cout << "go into else " << endl;
	 cout << "size of the string " << decoded_string.size()<< endl;
	 decoded_string.copy(dest_str, length, 0);
	 }
	 */
#ifdef DEBUG
	cout << "  " << decoded_string << endl;
#endif
	_datain->pop();
	_datain->pop();
	//return (string)dest_str;
	return decoded_string;
}

string DexFileReader::getType(unsigned int id) {
	if (id == 0xffffffff) {
		return "";
	}
	if ((id >= _type_ids_size) || (id < 0)) {
		cout << "Id out of bound in type ID table -----bail out!" << endl;
		assert(false);
		//return"";
	}
	int typeidxoff = _type_ids_off + id * 4;
	_datain->pushMove(typeidxoff);

	int typeabsoff;

	if (_little_endian_flag)
		typeabsoff = _datain->readIntx();
	else
		typeabsoff = _datain->bigEndian_readIntx();

	_datain->pop();
	string restr = getString(typeabsoff);
#ifdef DEBUG
	cout << "_____   " <<restr<< endl;
#endif
	return restr;
}

bool DexFileReader::isPrimitiveType(unsigned int id) {
	//string temp = getType(id);
	//use map to unify type name
	string temp = getTypename(id);
	if ((temp.size() <= 3) && (temp.size() > 1)) {
		//cout << "yes, primitive type!!" <<endl;
		return true;
	}
	return false;
}

Field* DexFileReader::getField(unsigned int id) {
	Field * f = NULL;
	if (_f_map->count(id) > 0)
		return (*_f_map)[id];
	if ((id >= _field_ids_size) || (id < 0)) {
		cout << "Id out of bound in field ID table -----bail out!" << endl;
		exit(1);
	}
	int idexoffset = _field_ids_off + id * 8;
	_datain->pushMove(idexoffset);

	short class_idx;
	short type_idx;
	int name_idx;
	if (_little_endian_flag) {
		class_idx = _datain->readShortx(); // 2bytes
		type_idx = _datain->readShortx(); // 2 bytes
		name_idx = _datain->readIntx(); // 4 bytes
	} else {
		class_idx = _datain->bigEndian_readShortx();
		type_idx = _datain->bigEndian_readShortx();
		name_idx = _datain->bigEndian_readIntx();
	}
	_datain->pop();
	//string tempstr1 = getType(class_idx);
	string tempstr1 = getTypename(class_idx);

	//added by haiyan to make sure that this type is considered
	//as a classdef and should be used to make a il::type;
	getClassDefByName(tempstr1);
	//end by haiyan

	//here, I should add it to ClassDefInfo;

	string tempstr2 = getString(name_idx);
	//string tempstr3 = getType(type_idx);
	string tempstr3 = getTypename(type_idx);
	//return new Field(tempstr1,class_idx,tempstr2,tempstr3,type_idx);
	f = new Field(tempstr1, class_idx, tempstr2, tempstr3, type_idx);
	(*_f_map)[id] = f;
	return f;
}

void DexFileReader::buildMethodMap() {
	for (unsigned int i = 0; i < _method_ids_size; i++) {
		(*_mid_method)[i] = getMethod(i);
	}
}

void DexFileReader::PrintMethod() {
	map<unsigned int, Method*>::iterator it = _mid_method->begin();
	for (; it != _mid_method->end(); it++) {
		cout << "MID == " << it->first << endl;
		cout << "Method Name == " << it->second->toString() << endl;
	}
}
Method* DexFileReader::getMethod(unsigned int method_id) {
	//test
	//cout << "OK here 1" << endl;
	if (method_id >= _method_ids_size || method_id < 0) {
		cout << "Id out of bound in method ID table -----bail out!" << endl;
		exit(1);
	}
	// go to the offset of methodinfo that indicated by method_id;
	//12/12
	if (_mid_method->count(method_id) > 0)
		return (*_mid_method)[method_id];
	//12/12

	int indexoffset = _method_ids_off + method_id * 8;
	_datain->pushMove(indexoffset);

	short owner_type_index;
	unsigned short proto_index;
	int name_index;

	if (_little_endian_flag) {
		owner_type_index = _datain->readShortx(); // 2bytes
		proto_index = _datain->readShortx(); // 2 bytes
		name_index = _datain->readIntx(); // 4 bytes
	} else {
		owner_type_index = _datain->bigEndian_readShortx();
		proto_index = _datain->bigEndian_readShortx();
		name_index = _datain->bigEndian_readIntx();
	}

	//cout << "OK here 2!!!!!!!!!" <<endl;
	if (proto_index >= _proto_ids_size) {
		cout << "Id out of bound in proto ID table------bail out!" << endl;
		exit(1);
	}
	int proto_offset = _proto_ids_off + proto_index * 12;
	_datain->pushMove(proto_offset);
	_datain->skip(4); //skip the descriptor of proto

	int return_index;
	int parameter_offset;

	//cout << "OK here 3 !!!!!!!!!!!!" <<endl;	
	if (_little_endian_flag) {
		return_index = _datain->readIntx(); // 4 bytes- index into the type_ids list
		parameter_offset = _datain->readIntx(); // 4 bytes - offset of parameters list(type_list)
	} else {
		return_index = _datain->bigEndian_readIntx();
		parameter_offset = _datain->bigEndian_readIntx();
	}
	//string return_type = getType(return_index); //get return type for the method
	string return_type = getTypename(return_index);
	int parameter_size = 0;
	string* parameters = NULL;
	//hzhu add 4/23/2012
	vector<unsigned int>* paratypeids = new vector<unsigned int>();
	//hzhu end 4/23/2012
	if (parameter_offset != 0) {
		_datain->pushMove(parameter_offset);
		if (_little_endian_flag) {
			parameter_size = _datain->readIntx();
			//cout << "Parameter size : " << parameter_size << endl;
			parameters = new string[parameter_size];
			for (int i = 0; i < parameter_size; i++) {
				//hzhu 4/23/2012
				//parameters[i] = getType(_datain->readShortx()); // fill the parameter array
				unsigned int type_ids = _datain->readShortx();
				//parameters[i] = getType(type_ids); // fill the parameter array
				//use map to unify type name
				parameters[i] = getTypename(type_ids);
				paratypeids->push_back(type_ids);
				//hzhu 4/23/2012
			}
		} else {
			parameter_size = _datain->bigEndian_readIntx();
			parameters = new string[parameter_size];
			for (int i = 0; i < parameter_size; i++) {
				//parameters[i] =  getType(_datain->bigEndian_readShortx());
				//use map to unify type name
				parameters[i] = getTypename(_datain->bigEndian_readShortx());
			}
		}
		_datain->pop();
	} else {
		parameters = NULL;
		paratypeids = NULL;
	}
	_datain->pop();
	_datain->pop();

	string tempstr1 = getString(name_index);
	//using map to unify type name
	string tempstr2 = getTypename(owner_type_index);
	//hzhu 4/20/2012
	//setMethodidx2AddrsM();
	//if(_midx_addrs_m->count(method_id)>0)
	//{
	//	tmp_m_addr = (*_midx_addrs_m)[method_id];
	//}
	//else
	//{
	//	tmp_m_addr = NULL;
	//}
	Method* m = new Method(_datain, method_id, tempstr1, tempstr2,
			owner_type_index, return_type, parameters, parameter_size,
			paratypeids, return_index);
	(*_mid_method)[method_id] = m;
	return m;
	//end 4/20/2012

}
/**
 * getClassToTypeidx is build all map for selfdefined ClassDefInfo* (typeidx<->classdefidx) (classidx->classdefinfo)
 */
map<int, int>* DexFileReader::getClassToTypeidx() {
	if ((_typeidx_classdefidx != NULL) && (_typeidx_classdefidx->size() != 0)) {
		return _typeidx_classdefidx;
	}
	//	cout << "SHOULD BE HERE ONLY ONCE" << endl;
	_datain->pushMove(_class_def_off); // save the the end of head point
	//_class_typeidx = new map<int,int>();
	if (_class_def_size != 0) {
		int offset = 0;
		int typeidx = 0;
		ClassDefInfo* cur_cdf = NULL;
		if (_little_endian_flag) {
			for (unsigned int i = 0; i < _class_def_size; i++) {

				offset = _class_def_off + 32 * i;
#ifdef DEBUG
				cout << "offset : "<<offset<<endl;
#endif
				_datain->move(offset);
				typeidx = _datain->readIntx();
#ifdef DEBUG
				cout << "test the index of type"<<endl;
#endif
				//cout <<"IMPORTANT "<< typeidx << " ---> " << i << endl;
				// i is corresponding to classdef idx
				(*_typeidx_classdefidx)[typeidx] = i;

				//12/14 classdefindex->typeidex mapping
				(*_classdefidx_typeidx)[i] = typeidx;
				//12/14

				if (!GLOBAL_MAP) {
					cur_cdf = getClassDefInfo(i);
					//use map to unify type name
					(*_str_classdef)[getTypename(typeidx)] = cur_cdf;
					(*_classdef_idx_ele)[i] = cur_cdf;
					(*_base_field_unresolved_selfclassdef)[i] = cur_cdf;
				}

			}
		} else {
			for (unsigned int i = 0; i < _class_def_size; i++) {
				offset = _class_def_off + 32 * i;
				_datain->move(offset);
				typeidx = _datain->bigEndian_readIntx();
				(*_typeidx_classdefidx)[typeidx] = i;
				(*_classdefidx_typeidx)[i] = typeidx;
				if (!GLOBAL_MAP) {
					cur_cdf = getClassDefInfo(i);
					//(*_str_classdef)[getType(typeidx)] = cur_cdf;
					(*_str_classdef)[getTypename(typeidx)] = cur_cdf;
					(*_classdef_idx_ele)[i] = cur_cdf;
					(*_base_field_unresolved_selfclassdef)[i] = cur_cdf;
				}

			}
		}

	} else {
		cout << "no class def, no map" << endl;
		assert(false);
	}
	_datain->pop();
	return _typeidx_classdefidx;
}

//12/15 build up a unique strname->ClassDefInfo map
void DexFileReader::buildGlobalStr2ClassDefMap() {
	this->buildTypeMapping(); // get the typeidx->str mapping for all types existing in dex file
	this->getClassToTypeidx(); //get the typeidx -> classdefidx mapping(for selfdefined class)
	ClassDefInfo* cdf = NULL;
	string str;
	for (unsigned int i = 0; i < this->_type_ids_size; i++) {
		str = (*this->_type_map)[i];
		if (this->_typeidx_classdefidx->count(i) > 0) { //self defined class
			unsigned int cdfidx = (*_typeidx_classdefidx)[i];
			cdf = getClassDefInfo(cdfidx);
			(*_str_classdef)[str] = cdf;
			(*_classdef_idx_ele)[cdfidx] = cdf;
			(*_base_field_unresolved_selfclassdef)[cdfidx] = cdf;
		} else { //lib type class, either primitive type or other lib defined class
			//cdf = new ClassDefInfo(getTypename(i));
			cdf = this->getClassDefByName(getTypename(i));
			cdf->setTypeIdx(i);
			(*_str_classdef)[str] = cdf;
		}
	}
	if (!_str_classdef->count("Ljava/lang/Exception;")) {
		//cdf = new ClassDefInfo("Ljava/lang/Exception;");
		cdf = this->getClassDefByName("Ljava/lang/Exception;");
		//12/20
		unsigned int typeidx = this->getTypeId("Ljava/lang/Exception;");
		cdf->setTypeIdx(typeidx);
		//12/20
		(*_str_classdef)["Ljava/lang/Exception;"] = cdf;

	}
	//if(!(_str_classdef->count("B")>0))
	//	(*_str_classdef)["B"] = new ClassDefInfo("B");
	(*_str_classdef)["INVALID"] = this->getClassDefByName("INVALID");
	(*_str_classdef)["[INVALID"] = this->getClassDefByName("[INVALID");
	(*_str_classdef)["[[INVALID"] = this->getClassDefByName("[[INVALID");
	(*_str_classdef)["[[[INVALID"] = this->getClassDefByName("[[[INVALID");
}
//12/15

string DexFileReader::getAccessFlag(unsigned int value) {
	string flag;

	switch (value) {
	case 0x1:
		flag = "ACC_PUBLIC";
		break;
	case 0x2:
		flag = "ACC_PRIVATE";
		break;
	case 0x4:
		flag = "ACC_PROTECTED";
		break;
	case 0x8:
		flag = "ACC_STATIC";
		break;
	case 0x10:
		flag = "ACC_FINAL";
		break;
	case 0x20:
		flag = "ACC_SYNCHRONIZED";
		break;
	case 0x40:
		flag = "ACC_BRIDGE";
		break;
	case 0x80:
		flag = "ACC_VARARGS";
		break;
	case 0x100:
		flag = "ACC_NATIVE";
		break;
	case 0x200:
		flag = "ACC_INTERFACE";
		break;
	case 0x400:
		flag = "ACC_ABSTRACT";
		break;
	case 0x800:
		flag = "ACC_STRICT";
		break;
	case 0x1000:
		flag = "ACC_SYNTHETIC";
		break;
	case 0x2000:
		flag = "ACC_ANNOTATION";
		break;
	case 0x4000:
		flag = "ACC_ENUM";
		break;
	case 0x10000:
		flag = "ACC_CONSTRUCTOR";
		break;
	case 0x20000:
		flag = "ACC_DECLARED_SYNCHRONIZED";
		break;
	default:
		flag = " NO FLAG whose value is " + Int2Str(value);
	}
	return flag;
}

// the index of
ClassDefInfo* DexFileReader::getClassDefInfo(unsigned int classdefidx) {
	if (classdefidx >= _class_def_size || classdefidx < 0) {
		cout << "Id out of bound in class def table -----bail out!" << endl;
		exit(1);
	}

	int offset = _class_def_off + classdefidx * 32;
	_datain->pushMove(offset); // the end of head be stored in the stack; and move to the offset the correponding class_def_item <<1
	unsigned int type_idx = 0;
	string type_name;
	string acc_flag;
	bool sup_f = false;
	bool inte_f = false;
	string sup_type = "";
	unsigned int sup_t_idx = 0;
	string file_name = "";
	vector<string>* iface = new vector<string>;
	vector<Field*>* fie = new vector<Field*>;
	//1/8
	vector<Field*>* s_fie = new vector<Field*>;
	//1/8
	vector<Method*>* meth = new vector<Method*>;
	map<int, int>* m_idx_codeoff = new map<int, int>;

	//the following variable used as temporal purpose
	int intfa_offset;
	int intfa_size;
	int class_data_off;
	unsigned int static_field_size = 0;
	unsigned int instance_field_size;
	unsigned int direct_method_size;
	unsigned int virtual_method_size;
	Field* temp_field; //need to add the flag information and offset for each element
	Method* temp_method;
	string temp_flag;

	//haiyan those part is used to build the annotation for Throw Exception
	unsigned int annotations_off;

	///////////////////////////////////////////////////////////////////////// 1/8/2013
	//those two should be same, if not same, make up for the leftover with 0;
	unsigned int static_value_size = 0;
	//we set it default valus as NULL, because NULL is recognizable easily, no things can be read from dex file explicitely denoted as NULL
	vector<string>* static_values = NULL;
	unsigned int static_values_off;
	//the following to used to store the information inserted to _all_static_fields_type and _all_static_fields_value
	string field_name;
	string field_type;
	string field_value;
	/////////////////////////////////////////////////////////////////////////////1/8/2013
	//haiyan end
	if (_little_endian_flag) {
		type_idx = _datain->readUIntx();
		//type_name = getType(type_idx);
		//use map to unify type name
		type_name = getTypename(type_idx);
		//type_name = getType(_datain->readUIntx());
		//cout << "DEX::dextypeidx" << dextypeidx << endl;
#ifdef DEBUG
		cout << "~~~~~~~~~~~~~~~~~ " << Int2Str(classdefidx) << "TYPE_NAME: " << type_name << endl;
#endif
		acc_flag = getAccessFlag(_datain->readUIntx());
#ifdef DEBUG
		cout << "~~~~~~~~~~~~~~~~class level Access Flag : " << acc_flag <<endl;
#endif
		sup_t_idx = _datain->readUIntx() & NO_INDEX;
		if (sup_t_idx != NO_INDEX) {
			//sup_type = getType(_datain->readUIntx());
			//sup_type = getType(sup_t_idx);
			//use map to unify type name
			sup_type = getTypename(sup_t_idx);
			sup_f = true;
		} else
			cout << "typeidx " << sup_t_idx << endl;

#ifdef DEBUG
		cout << "~~~~~~~~~~~~~~~~`adding Super class name: " << sup_type <<endl;
#endif
		intfa_offset = _datain->readUIntx(); // the address of get the offset of interface <<< 2

#ifdef DEBUG
		printf("~~~~~~~~~~~~~~~~~~`the offset of interface: %.2x hex \n", intfa_offset);
#endif
		//if the offset is not 0 means there are interface information
		if (intfa_offset != 0) {
			inte_f = true;
			_datain->pushMove(intfa_offset); // save the address of where get the offset and move to the place to create the interface infomation
			intfa_size = _datain->readUIntx();
			//cout << "intfa_size " << intfa_size << endl;
			//cout << "~~~~~~~~~~~~~~~~interface size:  "<< intfa_size<<endl;
			short type_idx;
			string* tempstr = new string();

			for (int i = 0; i < intfa_size; i++) {
				type_idx = _datain->readShortx();
				//*tempstr = getType(type_idx);
				//use map to unify class name
				*tempstr = getTypename(type_idx);
#ifdef DEBUG
				cout << "~~~~~~~~~~~~~~~adding inteface :" << *tempstr <<endl;
#endif
				iface->push_back(*tempstr);
			}
			_datain->pop(); //back to the place store "interface_offset" <<<2
		}
		unsigned int filename_idx = _datain->readUIntx();
		if (filename_idx != NO_INDEX)
			file_name = getString(filename_idx);
		//_datain->skip(4); // annotation field in the class def item
		annotations_off = _datain->readUIntx();
		//if(ANNOTATION_DEBUG)
		//printf("HZHU ---- annotation off : %x\n", annotations_off);// go to the place to build annotation_directory_item
		readMethodThrows(annotations_off); // to build method to Throw exceptions map

		class_data_off = _datain->readUIntx(); //save the place that store information to jump to the class_data item
		if (class_data_off != 0) {
			_datain->pushMove(class_data_off); //<<<<<<<<<<2
			// the place contains all the fields and methods information that belong to the class
			static_field_size = _datain->readULeb128();

			//1/8 based on the real static_field_size, set its initialized value
			static_values = new vector<string>(static_field_size, "EMPTY");
			//1/8
#ifdef DEBUG
			cout << "~~~~~~~~~~~~~~~~~~~the static filed size is : " << static_field_size << endl;
#endif
			instance_field_size = _datain->readULeb128();
#ifdef DEBUG
			cout << "~~~~~~~~~~~~~~~~~~the instance filed size is : " << instance_field_size << endl;
#endif
			//cout << "position:::::::::::::::::::::::::::::" << _datain->getCurrentPosition() <<endl;
			//printf("%.4x\n",_datain->getCurrentPosition());
			direct_method_size = _datain->readULeb128();
#ifdef DEBUG
			cout << "~~~~~~~~~~~~~~~~~~~the direct method size is : " << direct_method_size << endl;
#endif
			virtual_method_size = _datain->readULeb128();
#ifdef DEBUG
			cout << "~~~~~~~~~~~~~~~~~~~~the virtual method size is : " << virtual_method_size<< endl;
#endif
			// field should be sorted by field_idx in increasing order
			unsigned int pre_field_idx_static = 0;
			unsigned int pre_field_idx_instance = 0;
			unsigned int temp_static_field_size = static_field_size; //hzhu
			while (temp_static_field_size != 0) //hzhu
			{
				unsigned int field_idx_diff = _datain->readULeb128();
				unsigned int curr_field_idx = field_idx_diff
						+ pre_field_idx_static;
				pre_field_idx_static = curr_field_idx;
				temp_field = getField(curr_field_idx); //return a Field*
				temp_flag = getAccessFlag(_datain->readULeb128());
				temp_field->setFlag(temp_flag);
#ifdef DEBUG
				cout << "~~~~~~~~~~~~~~~~~the static field information contains : " << temp_field->toString() << "and flag is :"<<temp_field->getFlag()<<endl;
#endif
				//1.8 collect static field
				s_fie->push_back(temp_field);
				//1.8
				temp_static_field_size--; //hzhu
			}
			unsigned int byte_off = 0;
			while (instance_field_size != 0) {

				unsigned int field_idx_diff = _datain->readULeb128();
				unsigned int curr_field_idx = field_idx_diff
						+ pre_field_idx_instance;
				pre_field_idx_instance = curr_field_idx;
				temp_field = getField(curr_field_idx);
				//added for the field offset
				string type = temp_field->getType();
				//cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++type: " << type <<endl;
				//unsigned int b_off = temp_field->setOffset(type);
				unsigned int next_off = temp_field->setNextOff(type);
				//cout << "curr byte off is : " << byte_off << endl;
				//added for the byte offset
				temp_flag = getAccessFlag(_datain->readULeb128());
				temp_field->setFlag(temp_flag);
#ifdef DEBUG
				cout << "~~~~~~~~~~~~~~~~~~~the instance field information contains : " << temp_field->toString() << "and flag is :"<<temp_field->getFlag()<< ", offset: "<< byte_off <<endl;
#endif
				temp_field->setCurrOff(byte_off);
				fie->push_back(temp_field);
				instance_field_size--;
				byte_off += next_off;
				//	cout << "next byte off is: " << next_off << endl;
			}
			//method should be sorted by method_idx in incresing order
			unsigned int pre_method_idx_direct = 0;
			unsigned int pre_method_idx_virtual = 0;
			while (direct_method_size != 0) {
				unsigned int tmpidx = _datain->readULeb128();
				unsigned int curr_method_idx = tmpidx + pre_method_idx_direct;
				pre_method_idx_direct = curr_method_idx;
				//cout << "TTTTTTTTTT direct methodid : " << curr_method_idx <<endl;
				temp_method = getMethod(curr_method_idx); //return a Method*
				temp_flag = getAccessFlag(_datain->readULeb128());
				//cout << "temp_flag ->" << temp_flag << endl;
				temp_method->setFlag(temp_flag);
#ifdef DEBUG
				cout << "~~~~~~~~~~~~~~~~~~~~~the direct method information contains : " << temp_method->toString() << "and flag is :"<<temp_method->getFlag()<<endl;
#endif
				meth->push_back(temp_method);
				//haiyan added 5.24
				(*_mid_method)[curr_method_idx] = temp_method;
				//haiyan end 5.24
				//12/16
				(*_selfd_method_map)[curr_method_idx] = temp_method;
				//12/16
				unsigned int codeoff = _datain->readULeb128();
				if (codeoff != 0) {
					(*m_idx_codeoff)[curr_method_idx] = codeoff;
					//haiyan added for selfdefined method
				}
				direct_method_size--;
			}

			while (virtual_method_size != 0) {
				unsigned int tmpidx = _datain->readULeb128();
				unsigned int curr_method_idx = tmpidx + pre_method_idx_virtual;
				pre_method_idx_virtual = curr_method_idx;
				//	cout << "TTTTTTTTTTTTTTTTT virtual methodid : " << curr_method_idx <<endl;
				temp_method = getMethod(curr_method_idx); //return a Method*
				temp_method->setVirtual();
				temp_flag = getAccessFlag(_datain->readULeb128());
				//cout << "temp_flag ->" << temp_flag << endl;
				temp_method->setFlag(temp_flag);
#ifdef DEBUG
				cout << "~~~~~~~~~~~~~~~~~~~~~the virtual method information contains : " << temp_method->toString() << "and flag is :"<<temp_method->getFlag()<<endl;
#endif
				meth->push_back(temp_method);
				//haiyan added 5.24
				(*_mid_method)[curr_method_idx] = temp_method;
				//haiyan end 5.24
				//12/16
				(*_selfd_method_map)[curr_method_idx] = temp_method;
				//12/16
				unsigned int codeoff = _datain->readULeb128();
				if (codeoff != 0) {
					(*m_idx_codeoff)[curr_method_idx] = codeoff;
					//haiyan added for selfdefined method
				}
				virtual_method_size--;
			}
			_datain->pop(); // back to the place <<<<<<<<<<<<<2
		}

		/////////
		//1.8
		static_values_off = _datain->readUIntx();
		if (static_values_off != 0) {
			_datain->pushMove(static_values_off);
			//cout << endl<< "HHHHHHHHHHH  " << type_name << " has static fields "
			//		<< endl;
			static_value_size = _datain->readULeb128();
			if (0) {
				if (static_field_size != static_value_size)
					cout << endl << "FFFF::: static fields size:: "
							<< static_field_size
							<< " |||and static fields value size "
							<< static_value_size << endl;
				else
					cout << endl << "static fileds has value size "
							<< static_field_size << endl;
			}
			int i = 0;
			while (static_value_size-- != 0) { //read an array called encoded_value[static_value_size]
				unsigned char one_byte = _datain->readByte();
				unsigned int value_type = one_byte & 0x1f; // lower 5 bits
				unsigned int value_arg = (one_byte & (0xff)) >> 5; //higher 3 bits
				unsigned int ubyte_size = value_arg + 1;
				//check the static fields;

				field_value = readEncoded_value(value_type, ubyte_size,NULL,0);
				if (USING_STATIC_VALUE) {
					Field* f = s_fie->at(i);
					field_name = f->getFieldName();
					field_type = f->getType();
					(*_all_static_fields_type)[field_name] = field_type;
					(*_all_static_fields_value)[field_name] = field_value;
					//cout << "HHHHHHHHHHH == { field_name = " << field_name << "}  {field_type == " << field_type << "}  { field_value == " << field_value << "}"<< endl;
				}
				i++;
			}
			_datain->pop();
		}
		//1.8

	} else {
		type_idx = _datain->readUIntx();
		//type_name = getType(_datain->readUIntx());
		//type_name = getType(type_idx);
		//use map to unify type name
		type_name = getTypename(type_idx);
#ifdef DEBUG
		cout << "~~~~~~~~~~~~~~~~~TYPE_NAME: "<< type_name << endl;
#endif
		acc_flag = getAccessFlag(_datain->bigEndian_readIntx());
#ifdef DEBUG
		cout << "~~~~~~~~~~~~~~~~Access Flag : " << acc_flag <<endl;
#endif
		//sup_type = getType(_datain->bigEndian_readIntx());
		//use map to unify type name
		sup_type = getTypename(_datain->bigEndian_readIntx());
#ifdef DEBUG
		cout << "~~~~~~~~~~~~~~~~`Super class name: " << sup_type <<endl;
#endif
		intfa_offset = _datain->bigEndian_readIntx(); // the address of get the offset of interface <<< 2
#ifdef DEBUG
				printf("~~~~~~~~~~~~~~~~~~`the offset of interface: %.2x hex \n", intfa_offset);
#endif
		//if the offset is not 0 means there are interface information
		if (intfa_offset != 0) {
			_datain->pushMove(intfa_offset); // save the address of where get the offset and move to the place to create the interface infomation
			intfa_size = _datain->bigEndian_readIntx();
#ifdef DEBUG
			cout << "~~~~~~~~~~~~~~~~interface size:  "<< intfa_size<<endl;
#endif
			short type_idx;
			string tempstr;

			for (int i = 0; i < intfa_size; i++) {
				type_idx = _datain->bigEndian_readShortx();
				//tempstr = getType(type_idx);
				tempstr = getTypename(type_idx);
#ifdef DEBUG
				cout << "~~~~~~~~~~~~~~~added the string in to the interface vector<string> :" << tempstr <<endl;
#endif
				iface->push_back(tempstr);
			}
			_datain->pop(); //back to the place store "interface_offset" <<<2
		}
		file_name = getString(_datain->bigEndian_readIntx());
		//cout << "in file :" << file_name <<endl;
		_datain->skip(4); //annotation field in the class def item
		class_data_off = _datain->bigEndian_readIntx(); //save the place that store information to jump to the class_data item
		if (class_data_off != 0) {
			_datain->pushMove(class_data_off); //<<<<<<<<<<2
			// the place contains all the fields and methods information that belong to the class
			static_field_size = _datain->readULeb128();
#ifdef DEBUG
			cout << "~~~~~~~~~~~~~~~~~~~the static filed size is : " << static_field_size << endl;
#endif
			instance_field_size = _datain->readULeb128();
#ifdef DEBUG
			cout << "~~~~~~~~~~~~~~~~~~the instance filed size is : " << instance_field_size << endl;
#endif
			direct_method_size = _datain->readULeb128();
#ifdef DEBUG
			cout << "~~~~~~~~~~~~~~~~~~~the direct method size is : " << direct_method_size << endl;
#endif
			virtual_method_size = _datain->readULeb128();
#ifdef DEBUG
			cout << "~~~~~~~~~~~~~~~~~~~~the virtual method size is : " << virtual_method_size<< endl;
#endif
			// field should be sorted by field_idx in increasing order
			unsigned int pre_field_idx_static = 0;
			unsigned int pre_field_idx_instance = 0;

			while (static_field_size != 0) {
				unsigned int field_idx_diff = _datain->readULeb128();
				unsigned int curr_field_idx = field_idx_diff
						+ pre_field_idx_static;
				pre_field_idx_static = curr_field_idx;
				temp_field = getField(curr_field_idx); //return a Field*
				temp_flag = getAccessFlag(_datain->readULeb128());
				temp_field->setFlag(temp_flag);
#ifdef DEBUG
				cout << "~~~~~~~~~~~~~~~~~the static field information contains : " << temp_field->toString() << "and flag is :"<<temp_field->getFlag()<<endl;
#endif
				//hzhu 5/6/2012 comment out the static field, which doesn't belong to the object
				fie->push_back(temp_field);
				static_field_size--;
			}
			unsigned int byte_off = 0;
			while (instance_field_size != 0) {
				unsigned int field_idx_diff = _datain->readULeb128();
				unsigned int curr_field_idx = field_idx_diff
						+ pre_field_idx_instance;
				pre_field_idx_instance = curr_field_idx;
				temp_field = getField(_datain->readULeb128());
				temp_flag = getAccessFlag(_datain->readULeb128());
				temp_field->setFlag(temp_flag);
#ifdef DEBUG
				cout << "~~~~~~~~~~~~~~~~~~~the instance field information contains : " << temp_field->toString() << "and flag is :"<<temp_field->getFlag()<<endl;
#endif
				//added for the field offset
				string type = temp_field->getType();
				//unsigned int b_off = temp_field->setOffset(type);
				unsigned int next_off = temp_field->setNextOff(type);
				cout << "curr byte off is : " << byte_off << endl;
				temp_field->setCurrOff(byte_off);
				//added for the byte offset
				fie->push_back(temp_field);
				instance_field_size--;
				byte_off += next_off; //added for the byte offset
			}
			//method should be sorted by method_idx in incresing order
			unsigned int pre_method_idx_direct = 0;
			unsigned int pre_method_idx_virtual = 0;

			while (direct_method_size != 0) {
				unsigned int tmpidx = _datain->readULeb128();
				unsigned int curr_method_idx = tmpidx + pre_method_idx_direct;
				pre_method_idx_direct = curr_method_idx;
				temp_method = getMethod(curr_method_idx); //return a Method*
				temp_flag = getAccessFlag(_datain->readULeb128());
				temp_method->setFlag(temp_flag);
#ifdef DEBUG
				cout << "~~~~~~~~~~~~~~~~~~~~~the direct method information contains : " << temp_method->toString() << "and flag is :"<<temp_method->getFlag()<<endl;
#endif
				meth->push_back(temp_method);
				unsigned int codeoff = _datain->readULeb128();
				if (codeoff != 0) {
					(*m_idx_codeoff)[curr_method_idx] = codeoff;
					//haiyan added for selfdefined method
				}
				direct_method_size--;
			}

			while (virtual_method_size != 0) {
				unsigned int tmpidx = _datain->readULeb128();
				unsigned int curr_method_idx = tmpidx + pre_method_idx_virtual;
				pre_method_idx_virtual = curr_method_idx;
				temp_method = getMethod(curr_method_idx); //return a Method*
				temp_method->setVirtual();
				temp_flag = getAccessFlag(_datain->readULeb128());
				temp_method->setFlag(temp_flag);
#ifdef DEBUG
				cout << "~~~~~~~~~~~~~~~~~~~~~the virtual method information contains : " << temp_method->toString() << "and flag is :"<<temp_method->getFlag()<<endl;
#endif
				meth->push_back(temp_method);
				unsigned int codeoff = _datain->readULeb128();
				if (codeoff != 0) {
					(*m_idx_codeoff)[curr_method_idx] = codeoff;
					//haiyan added for selfdefined method
				}
				virtual_method_size--;
			}
			_datain->pop(); // back to the place <<<<<<<<<<<<<2

		}

	}
	_datain->pop(); //<<1

	ClassDefInfo* classdefinf = new ClassDefInfo(type_name, acc_flag, sup_type,
			file_name, fie, meth, iface, m_idx_codeoff);
	classdefinf->setSuperClassf(sup_f);
	classdefinf->setSuperClassIdx(sup_t_idx);
	classdefinf->setInterfacef(inte_f);
	classdefinf->setTypeIdx(type_idx);
	//1/8 collect static field
	classdefinf->setStaticFields(s_fie);
	//1/8
	return classdefinf;
}

CodeHeader* DexFileReader::getCodeHeader(ClassDefInfo* classdefinfo,
		unsigned int methodid) {
	if (_mid_codeh->count(methodid) > 0) //found
		return (*_mid_codeh)[methodid];
	//those info used to initialize the CodeHeader;
	int codeheader_off = (*classdefinfo->getMethod2Codeoff())[methodid]; // return corresponding method's code offset(the address of CodeHeader)
	//cout << "codeheader offset :" << codeheader_off<<endl;
	//printf("%x\n", codeheader_off);
	DataIn* in = _datain; //_datain has been influenced
	in->move(codeheader_off);
	int totalreg = 0;
	int args_size = 0;
	int triessize = 0;
	int debugoff = 0;
	int inslength = 0;
	int beginofins = 0;
	//end of the info 
	//hzhu added 4/22/2012
	//Method* method = getMethod(methodid);
	Method* method = NULL;
	if (_mid_method->count(methodid) > 0) {
		method = (*_mid_method)[methodid];

	} else {
		method = getMethod(methodid);
	}
	//cout << "method: " << method->toString();
	//hzhu added 5/3
	vector<Tries*>* tries = new vector<Tries*>();
	//unsigned int beginaddr = 0;
	//unsigned int endaddr = 0;
	//vector<Handler*>* handlerlist = new vector<Handler*>();//for tries
	Tries* tmptries = NULL;
	Handler* tmphandler = NULL;
	bool hastries = false;
	//hzhu 5/3 end
	if (_little_endian_flag) {
		totalreg = in->readShortx();
#ifdef DEBUG
		cout << "total number of registers: " << totalreg<<endl;
#endif
		args_size = in->readShortx();
		in->skip(2);
		//in->skip(2+2); //skip ins_size outs_size and tries_size
		//in->skip(2+2+2); //skip ins_size outs_size and tries_size
		int temptriessize = in->readShortx();
		triessize = temptriessize;
		//	triessize = in->readShortx();
		//	cout << "tries size :" <<triessize <<endl;
		debugoff = in->readIntx(); //debug offset
#ifdef DEBUG
				cout << "degug offset :  " << debugoff << " <-> ";
				printf("%.2x!\n",debugoff);
#endif
		inslength = in->readIntx(); // size of the instructions list(in 16-bit code units)
#ifdef DEBUG
				cout << "total instruction length:  " << inslength<<endl;
				printf("%x\n", inslength);
#endif
		beginofins = in->getCurrentPosition();
#ifdef DEBUG
		//	cout << "First instruction address: " << beginofins<<" <-> ";
		printf("%.2x! \n", beginofins);
#endif		
		unsigned int lasttry = beginofins + 2 * inslength; //even
		if ((inslength % 2) == 1)
			lasttry += 2;
		unsigned int begin_catch_handler_list = 0;
		if (temptriessize != 0) {
			hastries = true; //low 5 bits
			begin_catch_handler_list = lasttry + triessize * 8; //8 bytes per tries element
			//cout << "begin of catch handler list: " << begin_catch_handler_list << endl;
			//printf("%x\n",begin_catch_handler_list);
			in->move(begin_catch_handler_list);
			//int entry_size_handler = in->readULeb128();
			//printf("size of entries: %d\n", entry_size_handler);
		}
		//cout << "entry size of handlers: " <<entry_size_handler <<endl;
		//cout << "Check!!! " << in->getCurrentPosition()<<endl;
		//printf("%x\n",in->getCurrentPosition());
		//hzhu added to process the try and catch thing
		while (temptriessize != 0) {
			vector<Handler*>* handlerlist = new vector<Handler*>(); //for tries
			unsigned int currtry = lasttry; // the last 2 is for padding
			in->move(currtry); //the begin of the tries array
			//cout << "try array start addr:" << currtry <<endl;
			//printf("%x\n",currtry);
			unsigned int tries_startoff = in->readIntx(); //the offset of the try_start
			unsigned int tries_start_ab = tries_startoff * 2 + beginofins;
			//printf("tries start :%x\n", tries_startoff);
			//the end of the try_start
			unsigned int tries_length = in->readShortx();
			unsigned int tries_end_ab = tries_start_ab + tries_length * 2;
			//printf("tries end: %x\n" , //low 5 bitstries_startoff + tries_length);
			//cout << "tries end: " << tries_end_ab <<endl;
			//the bytes from the start of encoded_catch_handler_list to its corresponding handler;
			unsigned int handlerbyteoff = in->readShortx();
			//cout << "tries's handler offset: " <<handlerbyteoff <<endl;
			lasttry = in->getCurrentPosition(); // keep the position of the end of this try ele
			in->move(begin_catch_handler_list + handlerbyteoff);
			//begin to read corresponding catch handler
			int num_catch_type = in->readLeb128(); // could be negative
			//cout << "number of catch_type:" << num_catch_type <<endl;
			unsigned int num = abs(num_catch_type);
			bool has_finally = false;
			while (num != 0) {
				unsigned int typeidx = in->readULeb128();
				//string type = getType(typeidx);
				//use map to unify type name
				string type = getTypename(typeidx);
				in->push();
				ClassDefInfo* classdef = type2ClassDef(this, typeidx);
				in->pop(); //low 5 bits
				//  cout << "type:::" << classdef->getTypename() << endl; //create a ClassDefInfo*
				unsigned int addr = in->readULeb128();
				unsigned int abaddr = addr * 2 + beginofins;
				//  cout << "addr:::" << abaddr <<endl;
				tmphandler = new Handler(classdef, abaddr);
				handlerlist->push_back(tmphandler);
				num--;
			}
			if ((num_catch_type < 0) || (num_catch_type == 0)) {
				has_finally = true;
				int catchall = in->readULeb128();
				unsigned int add = catchall * 2 + beginofins;
				ClassDefInfo* classdef = new ClassDefInfo("");
				tmphandler = new Handler(classdef, add);
				handlerlist->push_back(tmphandler);
			}
			tmptries = new Tries(tries_start_ab, tries_end_ab, handlerlist,
					has_finally); //NULL a list of Handler list
			tries->push_back(tmptries);
			temptriessize--;
		}
		//cout << endl;
	} else {
		totalreg = in->bigEndian_readShortx();
		//in->skip(2+2+2);
		in->skip(2 + 2);
		int temptriessize = in->bigEndian_readShortx();
		triessize = temptriessize;
		//triessize = in->bigEndian_readShortx();
		debugoff = in->bigEndian_readIntx();
		inslength = in->bigEndian_readIntx();
		beginofins = in->getCurrentPosition();
	}
	//prepare for the throw_exceptions
	bool et = false;
	vector<string>* throw_exceptions = new vector<string>();
	if (_m_throws->count(methodid) > 0) //found
			{
		throw_exceptions = (*_m_throws)[methodid];
		et = true;
	}
	//cout << "inside class  == " << classdefinfo->getTypename() << endl;
	//return new CodeHeader(methodid,this,classdefinfo,in,codeheader_off, totalreg, triessize, debugoff, inslength, beginofins, method, tries, throw_exceptions, et, hastries);
	CodeHeader* codeh = new CodeHeader(methodid, this, classdefinfo, in,
			codeheader_off, totalreg, triessize, debugoff, inslength,
			beginofins, method, tries, throw_exceptions, et, hastries);
	codeh->setRealArgsSize(args_size);
	//12/19
	(*_mid_codeh)[methodid] = codeh;
	//12/19
	return codeh;
}

// a list of codeheader [one codeheader corresponding to one method]
vector<CodeHeader*>* DexFileReader::getCodeHeaderList(DexFileReader* dfr) { //low 5 bits
	_codeheader_list = new vector<CodeHeader*>();

	map<int, int> *m = NULL; // field inside the classDefInfo
	map<int, int>::iterator pos; //visitor for m map

	ClassDefInfo* tempo = NULL;
	CodeHeader* codeh = NULL;
	//for each ClassDefInfo there is a field called map<int,int> methodidx_codeoffset;
	//there are total 6 ClassDefInfo item
	for (unsigned int i = 0; i < dfr->_class_def_size; i++) {

		//each of the classDefInfo, get itself
		//tempo = dfr->getClassDefInfo(i);
		tempo = (*dfr->getClassDefIdx2Ele())[i];
		//and get its map of from methodid to codeoffset
		m = tempo->getMethod2Codeoff();
		//cout <<"print out the contents of methodid and codehead offset:::::::  " <<i<<endl;
		for (pos = m->begin(); pos != m->end(); ++pos) {
			//	cout << "methodid :" << pos->first<<endl;;
			//	cout << "methodname:  " <<dfr->getMethod(pos->first)->toString()<<endl;
			//you can get the codeh by given the ClassDefInfo as well as its methodid;
			codeh = dfr->getCodeHeader(tempo, pos->first); // return a pointer to a CodeHeader;
			if (ANNOTATION_DEBUG)
				cout << "??????????????????????" << codeh->isThrowExceptions()
						<< endl;
			_codeheader_list->push_back(codeh);
		}
	}
	return _codeheader_list;
}

DebugInfoReader* DexFileReader::getDebugInfoReader(CodeHeader* codeheader) {
	if (_codeh_debuginfo_map->count(codeheader) > 0)
		return (*_codeh_debuginfo_map)[codeheader];
	//parameters used to create a DebugInfoReader object
	unsigned int baseaddr = codeheader->getInstructionBeginOff();
	map<unsigned int, unsigned int>* regkillmap = codeheader->getRegKillMap();
	string filename = codeheader->getClassDefInfo()->getFilename();
	//cout << "check " << filename << endl;
	map<unsigned int, unsigned int>* addlinemap = new map<unsigned int,
			unsigned int>();
	//haiyan 8.28 changed rvtlist to rvtmap
	//vector<RVT*>* rvtlist = new vector<RVT*>();
	map<unsigned int, vector<RVT*>*>* rvtlist = new map<unsigned int,
			vector<RVT*>*>();
	//the end of the parameters getDebugInfoOff get the debugoff of the codeheader!!!
	int debugoff = codeheader->getDebugInfoOff();
	//haiyan added 6.12
	//temp hold register, begin local addr, end local addr 8.28
	vector<pair<unsigned int, pair<unsigned int, unsigned int>>>* temp = new vector<pair<unsigned int, pair<unsigned int, unsigned int>>>();
	//haiyan added 6.12
//	cout <<"DebugOff: "<<debugoff<<endl;
	DebugInfoReader* dr = NULL;
	if (debugoff == 0) {
		//if(0)// you can changed 0 to 1 to indicate the info
		cout << "DebugOff is 0, there is no debug information for this method!"
				<< endl;
		return NULL;
	} else {
		int pcoffset = 0; //change to unsigned int
		_datain->pushMove(debugoff); //
		unsigned int line = _datain->readULeb128(); //based on the initial value
		vector<string>* parameters = new vector<string>; // the vector carring on the parameters
		{
			unsigned int szpara = _datain->readULeb128();
			//cout << "the method has " << szpara << " parameters" << endl;
			if (szpara != 0) {
				for (unsigned int i = 0; i < szpara; i++) {
					unsigned int string_offset = _datain->readULeb128() - 1;
					if (string_offset == 0xffffffff) //NO_INDEX
							{
						if (PARAMETER_CHECK)
							cout << "No string index saved for parameters : "
									<< i << "NO_INDEX!" << endl;
					} else {
						if (PARAMETER_CHECK)
							cout << "parameters: " << getString(string_offset)
									<< "\n";
						parameters->push_back(getString(string_offset));
					}
				}
			}

		}

		// below is to create corresponding debug information
		//int i = 0;
		//int j = 0;
		//unsigned int lastoffset = 0;
		//haiyan added for debug
		while (true) {
			unsigned char opcode = _datain->readByte();
			switch (opcode) {
			case DBG_END_SEQUENCE: {
				_datain->pop();
				/*
				 for(int k = 0 ; k < temp->size(); k++)
				 {
				 cout << "check -------------------------------------------------------------------" << endl;
				 cout << "register is :" << temp->at(k).first <<endl;
				 cout << "begin addr is " << temp->at(k).second.first << endl;
				 cout << "end addr is " << temp->at(k).second.second<< endl;
				 }
				 */
				dr = new DebugInfoReader(codeheader, filename, addlinemap,
						rvtlist, parameters);
				dr->setRegBeginEndAddr(temp);
				//cout << "size of the vector is " << dr->getRegBeginEndAddr()->size()<<endl;
				//12/15
				(*_codeh_debuginfo_map)[codeheader] = dr;
				//12/15
				return dr;
			}
			case DBG_ADVANCE_PC: {
				//haiyan added for the last ins if possible
				//haiyan roll back 8.28
				//lastoffset = pcoffset;
				unsigned int offset1 = _datain->readULeb128();
				pcoffset += offset1;
			}
				break;
			case DBG_ADVANCE_LINE: {
				int offset2 = (int) _datain->readLeb128();
				line += offset2;
			}
				break;
			case DBG_START_LOCAL: {
				//	cout << "start offset is :: " << pcoffset << endl;
				unsigned int regnum = _datain->readULeb128(); // the number of register in RVT
				//	cout << "start register number is " << regnum << endl;
				/*
				 //haiyan added 6.12
				 pair<unsigned int, pair<unsigned int, unsigned int>> p;
				 p.first = regnum;
				 p.second.first = pcoffset;
				 p.second.second = UINT_MAX;
				 temp->push_back(p);
				 //haiyan end 6.12
				 */

				unsigned int nameidx = _datain->readULeb128() - 1; // the name of the variable in RVT
				unsigned int typeidx = _datain->readULeb128() - 1; // the type of the variable
				string sig = ""; // the signature in RVT
				ClassDefInfo* tempclassdef; // ClassDef Info In RVT
				if ((nameidx >= 0) && (typeidx >= 0)) {
					//haiyan added 6.12
					pair<unsigned int, pair<unsigned int, unsigned int>> p;
					p.first = regnum;
					//haiyan changed pcoffset to lastoffset because of on instruction before start the local var info
					//roll back to the original one 8/28
					p.second.first = pcoffset;
					//p.second.first = lastoffset;
					p.second.second = UINT_MAX;
					temp->push_back(p);
					// cout << "temp->push_back :: " << i++ << endl;
					//haiyan end 6.12
					//check whether the type is primitive or not
					if (isPrimitiveType(typeidx)) {
						string accflag = ""; //needed to build the ClassDefInfo
						string super = ""; //needed to build the ClassDefInfo
						string filename = ""; // needed to build the classDefInfo (primitive type,do not refered to some file)
						vector<Field*>* field = NULL; //needed to build the ClassDefInfo
						vector<Method*>* meth = NULL; //needed to build the ClassDefInfo
						vector<string>* interfa = NULL; //needed to build the ClassDefInfo
						map<int, int>* midx_codeoff = NULL; //needed to build the ClassDefInfo
						//tempclassdef = new ClassDefInfo(getType(typeidx),accflag,super,filename,field, meth,interfa,midx_codeoff); 
						//use map to unify type name
						if (!GLOBAL_MAP)
							tempclassdef = new ClassDefInfo(
									getTypename(typeidx), accflag, super,
									filename, field, meth, interfa,
									midx_codeoff);
						else
							tempclassdef = this->getClassDefByName(
									getTypename(typeidx));
						//12/20
						tempclassdef->setTypeIdx(typeidx);
						//12/20
					} else // get ClassDefInfo item stored in dex file
					{
						map<int, int>* temp_tidx = getClassToTypeidx();
						map<int, int>::iterator it;

						it = temp_tidx->find(typeidx);

						if (it != (temp_tidx->end())) { // found
							if (!GLOBAL_MAP) {
								if (getClassToTypeidx()->count(typeidx) > 0) {
									int idx_class_def =
											(*getClassToTypeidx())[typeidx];
									//cout << "index of class def1 : " << idx_class_def << endl;
									//tempclassdef = getClassDefInfo(idx_class_def);
									tempclassdef =
											(*_classdef_idx_ele)[idx_class_def];
								} else {
									//tempclassdef = new ClassDefInfo(getType(typeidx));
									//use map to unify type name
									tempclassdef = new ClassDefInfo(
											getTypename(typeidx));
								}
							} else {
								tempclassdef = this->getClassDefByName(
										getTypename(typeidx));
							}
							//12/20
							tempclassdef->setTypeIdx(typeidx);
							//12/20
						} else { //not found
							string accflag = ""; //needed to build the ClassDefInfo
							string super = ""; //needed to build the ClassDefInfo
							string filename = ""; // needed to build the classDefInfo (primitive type,do not refered to some file)
							vector<Field*>* field = NULL; //needed to build the ClassDefInfo
							vector<Method*>* meth = NULL; //needed to build the ClassDefInfo
							vector<string>* interfa = NULL; //needed to build the ClassDefInfo
							map<int, int>* midx_codeoff = NULL; //needed to build the ClassDefInfo
							//tempclassdef = new ClassDefInfo(getType(typeidx),accflag,super,filename,field, meth,interfa,midx_codeoff);
							//use name to unify type  name
							if (!GLOBAL_MAP) {
								tempclassdef = new ClassDefInfo(
										getTypename(typeidx), accflag, super,
										filename, field, meth, interfa,
										midx_codeoff);
							} else
								tempclassdef = this->getClassDefByName(
										getTypename(typeidx));
							//12/20
							tempclassdef->setTypeIdx(typeidx);
							//12/20
						}
					}
					// added it in to the RVT(local variable list)
					//rvtlist->push_back(new RVT(regnum, getString(nameidx),sig, tempclassdef));
					RVT* new_rvt = new RVT(regnum, getString(nameidx), sig,
							tempclassdef);
					if ((*rvtlist)[pcoffset * 2 + baseaddr] == NULL)
						(*rvtlist)[pcoffset * 2 + baseaddr] =
								new vector<RVT*>();
					(*rvtlist)[pcoffset * 2 + baseaddr]->push_back(new_rvt);
					//cout << "DBG_START_LOCAL INSERT RVT --  addr: " << pcoffset*2 + baseaddr << ", RTV -- R:" <<new_rvt->getReg() << ", V: " << new_rvt->getVarName() << ", T: " << new_rvt->getClassType()->getTypename() << endl;
					//cout << "rvtlist->push_back :: " << j++ << endl;
				}
			}
				break;
			case DBG_START_LOCAL_EXTENDED: {
				//	cout << "start offset is :: " << pcoffset << endl;
				unsigned int regnum = _datain->readULeb128(); // the number of register in RVT
				//	cout  << "start register number is " << regnum << endl;
				/*
				 //haiyan added 6.12
				 pair<unsigned int, pair<unsigned int, unsigned int>> p;
				 p.first = regnum;
				 p.second.first = pcoffset;
				 p.second.second = UINT_MAX;
				 temp->push_back(p);
				 //haiyan end 6.12
				 */
				unsigned int nameidx = _datain->readULeb128() - 1; // the name of the variable in RVT
				unsigned int typeidx = _datain->readULeb128() - 1; // the type of the variable
				unsigned int sigidx = _datain->readULeb128() - 1; // the signature in RVT
				string sig = getString(sigidx);
				ClassDefInfo* tempclassdef; // ClassDef Info In RVT
				string localname;
				string typena;
				if (nameidx == 0xffffffff) {
					if (LOCAL_VAR_CHECK)
						localname = "No variable name stored";
				}
				if (typeidx == 0xffffffff) {
					if (LOCAL_VAR_CHECK)
						typena = "No typename stored!";
				}
				if ((nameidx >= 0) && (typeidx >= 0)) {
					//haiyan added 6.12
					pair<unsigned int, pair<unsigned int, unsigned int>> p;
					p.first = regnum;
					//haiyan changed pcoffset to lastoffset because of local var info one ins before
					//haiyan roll back 8.28
					p.second.first = pcoffset;
					//p.second.first = lastoffset;
					p.second.second = UINT_MAX;
					temp->push_back(p);
					//cout << "temp->push_back :: " << i++ << endl;
					//haiyan end 6.12
					localname = getString(nameidx);
					//check whether the type is primitive or not
					if (isPrimitiveType(typeidx)) {
						string accflag = ""; //needed to build the ClassDefInfo
						string super = ""; //needed to build the ClassDefInfo
						string filename =
								"Primitive type, no filename define it!"; //needed to build the ClassDefInfo
						vector<Field*>* field = NULL; //needed to build the ClassDefInfo
						vector<Method*>* meth = NULL; //needed to build the ClassDefInfo
						vector<string>* interfa = NULL; //needed to build the ClassDefInfo
						map<int, int>* midx_codeoff = NULL; //needed to build the ClassDefInfo
						//typena = getType(typeidx);
						//using map to unify type name
						typena = getTypename(typeidx);
						if (!GLOBAL_MAP)
							tempclassdef = new ClassDefInfo(typena, accflag,
									super, filename, field, meth, interfa,
									midx_codeoff);
						else
							tempclassdef = this->getClassDefByName(typena);
						//12/20
						tempclassdef->setTypeIdx(typeidx);
						//12/20
					} else // get ClassDefInfo item stored in dex file
					{
						if (!GLOBAL_MAP) {

							if (getClassToTypeidx()->count(typeidx) > 0) //map will insert new element if key is not found(wrong behavior)
									{
								int idx_class_def =
										(*getClassToTypeidx())[typeidx];
								tempclassdef =
										(*_classdef_idx_ele)[idx_class_def];
							} else {
								tempclassdef = new ClassDefInfo(
										getTypename(typeidx));
							}
						} else
							tempclassdef = this->getClassDefByName(
									getTypename(typeidx));
						//12/20
						tempclassdef->setTypeIdx(typeidx);
						//12/20
					}
					// added it in to the RVT(local variable list)
					//rvtlist->push_back(new RVT(regnum, localname,sig, tempclassdef));
					//haiyan changed to rvt map 8.28
					RVT* new_rvt = new RVT(regnum, getString(nameidx), sig,
							tempclassdef);
					if ((*rvtlist)[pcoffset * 2 + baseaddr] == NULL)
						(*rvtlist)[pcoffset * 2 + baseaddr] =
								new vector<RVT*>();
					(*rvtlist)[pcoffset * 2 + baseaddr]->push_back(new_rvt);
					//cout << "EXTENDED INSERT RVT --  addr: " << pcoffset*2 + baseaddr << ", RTV -- R:" <<new_rvt->getReg() << ", V: " << new_rvt->getVarName() << ", T: " << new_rvt->getClassType()->getTypename() << endl;

					//cout << "rvtlist->push_back :: " << j++ << endl;
				}
			}
				//cout << "Exit DBG_START_LOCAL_EXTENDED"<<endl;
				break;
			case DBG_END_LOCAL: {
				if (LOCAL_VAR_CHECK)
					cout << "end local offset is :: " << pcoffset << endl;
				unsigned int endreg = _datain->readULeb128();
				if (LOCAL_VAR_CHECK)
					cout << "end register number is " << endreg << endl;
				//haiyan added 6.12
				//assert(temp->size()>0);
				if (temp->size() > 0) {
					for (unsigned int i = 0; i < temp->size(); i++) {
						if ((temp->at(i).first == endreg)
								&& (temp->at(i).second.first < pcoffset)) {
							temp->at(i).second.second = pcoffset;
							break;
						}
					}
				}
				//haiyan end 6.12
				//haiyan begin 8.29 for prepare the variable declaration with type
				(*regkillmap)[pcoffset * 2 + baseaddr] = endreg;
				//haiyan end 8.29

			}
				break;
			case DBG_RESTART_LOCAL: {
				if (LOCAL_VAR_CHECK)
					cout << "SUCK +++ restart local offset is :: " << pcoffset
							<< endl;
				unsigned int r_reg = _datain->readULeb128();
				if (LOCAL_VAR_CHECK)
					cout << "SUCK +++ restart register is :: " << r_reg << endl;
				// always 4.5.6 , dead locked!
				bool find = false;
				//cout << "rvtlist size " << rvtlist->size() << endl;
				if ((rvtlist != NULL) && (rvtlist->size() > 0)) {
					map<unsigned int, vector<RVT*>*>::reverse_iterator it =
							rvtlist->rbegin();
					bool break_set = false;
					for (; it != rvtlist->rend(); it++) {
						vector<RVT*>* cur_rvt_list = it->second;
						for (unsigned int i = 0; i < cur_rvt_list->size();
								i++) {
							RVT* cur_rvt = cur_rvt_list->at(i); //get each rvt of addr
							unsigned int cur_reg = cur_rvt->getReg(); //get register of  addr
							/* Here want to make check whether the register is defined before.
							 * Because it is .restart, it should have been declared before.
							 * Other wise can not call .restart because do not the type.
							 * */
							if (cur_reg == r_reg) //
									{
								find = true;
								pair<unsigned int,
										pair<unsigned int, unsigned int>> p;
								p.first = r_reg;
								p.second.first = pcoffset;
								p.second.second = UINT_MAX;
								temp->push_back(p);
								//rvtlist->push_back(rvtlist->at(k));
								if ((*rvtlist)[pcoffset * 2 + baseaddr] == NULL)
									(*rvtlist)[pcoffset * 2 + baseaddr] =
											new vector<RVT*>();
								//cout << "cur_rvt push back=========" << cur_rvt->getReg()<< endl;
								cur_rvt->setRestartFlag();
								//cout << "adding ! " << endl;
								(*rvtlist)[pcoffset * 2 + baseaddr]->push_back(
										cur_rvt);
								//cout << "RESTART_LOCAL INSERT RVT --  addr: " << pcoffset*2 + baseaddr << ", RTV -- R:" <<cur_rvt->getReg() << ", V: " << cur_rvt->getVarName() << ", T: " << cur_rvt->getClassType()->getTypename() << endl;
								break_set = true;
								break; //break to A
							}
						} //A
						if (break_set)
							break; //break to B
					} //B
				}
				if (!find) {
					RVT* rvt = new RVT(r_reg);
					rvt->setRestartFlag();
					if ((*rvtlist)[pcoffset * 2 + baseaddr] == NULL)
						(*rvtlist)[pcoffset * 2 + baseaddr] =
								new vector<RVT*>();
					//cout << "adding !!!!! " << endl;
					(*rvtlist)[pcoffset * 2 + baseaddr]->push_back(rvt);
				}
			}
				break;
			case DBG_SET_PROLOGUE_END:
				break;
			case DBG_SET_EPILOGUE_BEGIN:
				break;
			case DBG_SET_FILE: {
				unsigned int sourcefileidx = _datain->readULeb128() - 1;
				filename = getString(sourcefileidx);
			}
				break;
			default: {
				int adjustedOpcode = opcode - DBG_FIRST_SPECIAL;
				line += DBG_LINE_BASE + (adjustedOpcode % DBG_LINE_RANGE);
				if (adjustedOpcode / DBG_LINE_RANGE != 0) {
					pcoffset += (adjustedOpcode / DBG_LINE_RANGE);
				}
				(*addlinemap)[pcoffset * 2 + baseaddr] = line; //addr -> line
			}
			}
		}
	}
}

void DexFileReader::setMethodidx2AddrsM() {
	_midx_addrs_m = new map<unsigned int, MethodAddr*>();
	//prepare for the classdefInfo's one fild to get the _mdix_addrs_m map;
	map<int, int> *m = NULL; // field inside the classDefInfo class
	map<int, int>::iterator pos; //visitor for m map

	ClassDefInfo* tempo = NULL;
	CodeHeader* codeh = NULL;
	//for each ClassDefInfo there is a field called map<int,int> methodidx_codeoffset;
	//there are total 6 ClassDefInfo item
	for (unsigned int i = 0; i < _class_def_size; i++) {
		//each of the classDefInfo, get itself
		//	tempo = getClassDefInfo(i);
		tempo = (*_classdef_idx_ele)[i];
		//and get its map of from methodid to codeoffset
		m = tempo->getMethod2Codeoff();
		//cout <<"print out the contents of methodid and codehead offset:::::::  " <<i<<endl;
		for (pos = m->begin(); pos != m->end(); ++pos) {
			//	cout << "methodid :" << pos->first<<endl;;
			//	cout << "methodname:  " <<dfr->getMethod(pos->first)->toString()<<endl;
			//you can get the codeh by given the ClassDefInfo as well as its methodid;
			unsigned int midx = pos->first;
			codeh = getCodeHeader(tempo, midx); // return a pointer to a CodeHeader;
			unsigned int beginaddr = codeh->getInstructionBeginOff();
			unsigned int endaddr = beginaddr + codeh->getInstructionSize() * 2
					- 1;
			MethodAddr* methodaddrs = new MethodAddr(beginaddr, endaddr);
			(*_midx_addrs_m)[midx] = methodaddrs;
		}
	}
	//return  _midx_addrs_m;
}

map<unsigned int, MethodAddr*>* DexFileReader::getMethodidx2AddrsM() {
	return _midx_addrs_m;
}

map<unsigned int, Method*>* DexFileReader::getMIdx2Method() {
	return _mid_method;
}
int DexFileReader::readBytes(unsigned int size) {
	int t_idx = 0;
	switch (size) {
	case 1: {
		t_idx = _datain->readByte();
	}
		break;
	case 2: {
		t_idx = _datain->readShortx();
	}
		break;
	case 3: {
		unsigned int cur_pos = _datain->getCurrentPosition();
		t_idx = _datain->readIntx();
		t_idx = t_idx & 0x0ffffff;
		_datain->move(cur_pos);
		_datain->skip(3);
	}
		break;
	case 4: {
		t_idx = _datain->readIntx();
	}
		break;
	case 5: {
		unsigned int cur_pos = _datain->getCurrentPosition();
		t_idx = _datain->readLongx();
		t_idx = t_idx & 0x0ffffffffff;
		_datain->move(cur_pos);
		_datain->skip(5);
	}
		break;
	case 6: {
		unsigned int cur_pos = _datain->getCurrentPosition();
		t_idx = _datain->readLongx();
		t_idx = t_idx & 0x0ffffffffffff;
		_datain->move(cur_pos);
		_datain->skip(6);

	}
		break;
	case 7: {
		unsigned int cur_pos = _datain->getCurrentPosition();
		t_idx = _datain->readLongx();
		t_idx = t_idx & 0x0ffffffffffffff;
		_datain->move(cur_pos);
		_datain->skip(7);

	}
		break;
	case 8: {
		t_idx = _datain->readLongx();
	}
		break;
	default:
		cout << "error, value should be 1,2,3,4" << endl;
		assert(false);
	}
	return t_idx;
}
//haiyan added 6.24
void DexFileReader::readMethodThrows(unsigned int annotations_off) // which is complicated
		{
	if (annotations_off != 0) {
		_datain->pushMove(annotations_off); ////
		_datain->skip(4); //skip class annotations_off
		//fields_size annotated
		unsigned int fs = _datain->readUIntx();
		//cout << "number of field annotation size: " << fs<<endl;

		//method_size annotated
		unsigned int ms = _datain->readUIntx();
		if (ANNOTATION_DEBUG)
			cout << "number of method annotation size: " << ms << endl;
		//_m_throws = new map<unsigned int, vector<string>* >();
		unsigned int mid = 0;
		//if the size of method annotated is not equal to zero
		if (ms != 0) //have the chance that some method have throw annotation
				{
			_datain->skip(4); // skip annotated_parameters_off;
			//skip the field annotations;
			if (fs != 0)
				_datain->skip(8 * fs);
			//build method_annotation[ms]
			while (ms-- != 0) {
				//method_id who contains annotations
				mid = _datain->readUIntx(); //map id
				if (ANNOTATION_DEBUG)
					cout << "method has annotation is : "
							<< getMethod(mid)->toString() << endl;
				//prepare for the annonation_set_item
				unsigned int anns_off = _datain->readUIntx();

				_datain->pushMove(anns_off); ////// go to the place store the method annotation information (annotation_set_item)

				//size of annotation_off_item[]
				unsigned int size = _datain->readUIntx();
				if (ANNOTATION_DEBUG)
					cout << "number of annotation_off_item " << size << endl;
				//entries annotation_off_item[size]
				while (size-- != 0) {
					//annotation off
					unsigned int ann_off = _datain->readUIntx();
					if (ANNOTATION_DEBUG)
						cout << "ann_off " << ann_off << endl;
					_datain->pushMove(ann_off); //////// important to track back   aaaaaaaaa

					//for the annotation item
					unsigned int visibility =
							(unsigned int) _datain->readByte();
					if (ANNOTATION_DEBUG)
						cout << "visibility : " << visibility << endl;
					unsigned int type_idx;
					///////////////////////////////added by haiyan for parse annotation; below
					if (ADDING_ANNOATION) {
						if (visibility == VISIBILITY_RUNTIME) {
							type_idx = _datain->readULeb128();
							string annotation_type = getTypename(type_idx);
							//cout << "1) Annotation type is ::  "
							//		<< annotation_type << endl; //for example "STAMP" annotation defined in stamper

							//content of annotation for methods
							if (annotation_type.find("STAMP") != string::npos) { //specialized designed for STAMP annotation
								unsigned int size_annotation =
										_datain->readULeb128();

								while (size_annotation-- != 0) {
									unsigned int name_idx =
											_datain->readULeb128(); // content of name(for example "from" or "to")
									string name = this->getString(name_idx);
									//cout << "annotation name is " << name << endl;
									assert(name.find("flows") != string::npos);
									//this is for e.g flow element flows = {}

									////encoded_value
									unsigned char one_byte =
											_datain->readByte();
									//low 5 bits
									unsigned int value_type_1 = one_byte & 0x1f;

									//high 3 bits;
									unsigned int value_arg_1 = (one_byte
											& (0xff)) >> 5;

									//decode the content of the items frome = "??", we need to decode ??
									// based on the ubyte_size_1 to read the bytes;(how many bytes need to be read)
									//changed it by calling this function;

									vector<vector<string>*>* annotation_content = new vector<vector<string>*>();
									//vector<string>* annotation_content = new vector<string>();
									readEncoded_value(value_type_1,
											value_arg_1 +1, annotation_content,0);
									vector<sail::Annotation*>* anno = makeAnnotation(annotation_content);
									(*_method_flow_annotations)[mid] = anno;

								}
							}
						}
					}
					///////////////////////////////added by haiyan for parse annotation up

					if (visibility == VISIBILITY_SYSTEM) {
						//prepare for it encoded_annotation Format  which is not an array but a triple including type_idx; size; elements
						//unsigned int type_idx = _datain->readULeb128();
						type_idx = _datain->readULeb128();
						if (ANNOTATION_DEBUG)
							cout << "type is : " << getTypename(type_idx)
									<< endl;

						if (getTypename(type_idx)
								== "Ldalvik/annotation/Throws;") {
							if (ANNOTATION_DEBUG)
								cout << "yes inside the Throw annotation"
										<< endl;
							unsigned int size = _datain->readULeb128();
							if (ANNOTATION_DEBUG)
								cout << "size is " << size << endl;
							//annoation_element[size]
							/*************************************************
							 ** value = {
							 **           Ljava/io/IOException;
							 ** 	 	 }
							 **
							 ***********************************************************/
							while (size-- != 0) {
								//unsigned int name_idx = _datain->readULeb128(); // content of name is "value"
								_datain->readULeb128();
								//encoded_value
								unsigned char b = _datain->readByte();
								//printf("b is %x\n", b);
								//cout << "b is " << (unsigned int)b << endl;
								unsigned int value_type = b & 0x1f;
								//cout << "value_type " << value_type << endl;
								/*
								 unsigned int value_arg = (b & 0xff) >> 5;
								 //cout << "value_arg " << value_arg << endl;
								 */
								//because it must be for example some type of Exception, which refere to a type-idx(VALUE_TYPE);
								if (value_type == 28) {

									unsigned int num_ele =
											_datain->readULeb128();
									//cout << "number of ele: " << num_ele << endl;
									//values[num_ele] == encoded_value[]
									vector<string>* exception_t = new vector<
											string>();
									while (num_ele-- != 0) {
										unsigned char b1 = _datain->readByte();
										//  cout << "b1 is "<< (unsigned int)b1 << endl;
										//printf("b1 is %x\n", b1);
										unsigned int value_type = b1 & 0x1f;
										if (value_type == 24) {
											//cout << "value_type1 " << value_type << endl;
											unsigned int value_arg = (b1 & 0xff)
													>> 5;
											//cout << "value_arg1 " << value_arg << endl;
											unsigned int ubyte_size = value_arg
													+ 1;
											switch (ubyte_size) {
											case 1: {
												//cout << "case 1" << endl;
												unsigned char t_idx =
														(unsigned char) _datain->readByte();
												string excpt = getTypename(
														t_idx);
												if (ANNOTATION_DEBUG)
													cout << "Type is " << excpt
															<< endl;
												exception_t->push_back(excpt);
											}
												break;
											case 2: {
												//cout << "case 2" << endl;
												unsigned int t_idx =
														(unsigned int) _datain->readShortx();
												string excpt = getTypename(
														t_idx);
												exception_t->push_back(excpt);
												if (ANNOTATION_DEBUG)
													cout << "Type is "
															<< getTypename(
																	t_idx)
															<< endl;
											}
												break;
											case 3: {
												//cout << "case 3" << endl;
												unsigned int cur_pos =
														_datain->getCurrentPosition();
												unsigned int t_idx =
														(unsigned int) _datain->readUIntx();
												t_idx = t_idx & 0x0ffffff;
												string excpt = getTypename(
														t_idx);
												exception_t->push_back(excpt);
												if (ANNOTATION_DEBUG)
													cout << "Type is "
															<< getTypename(
																	t_idx)
															<< endl;
												_datain->move(cur_pos);
												_datain->skip(3);
											}
												break;
											case 4: {
												//cout << "case 4" << endl;
												unsigned int t_idx =
														(unsigned int) _datain->readUIntx();
												string excpt = getTypename(
														t_idx);
												exception_t->push_back(excpt);
												if (ANNOTATION_DEBUG)
													cout << "Type is "
															<< getTypename(
																	t_idx)
															<< endl;
											}
												break;
											default:
												cout
														<< "error, value should be 1,2,3,4"
														<< endl;
											}
										}
									}
									if (0) {
										cout
												<< "*******************************************************"
												<< endl;
										cout << "method "
												<< getMethod(mid)->toString()
												<< endl;
										int n = exception_t->size();
										int i = 0;
										if (n != 0) {
											while (i != n) {
												cout << "exception type:: "
														<< exception_t->at(i++)
														<< endl;
											}
										}
									}
									//cout << "before adding to _m_throws size" << _m_throws->size()<< endl;
									(*_m_throws)[mid] = exception_t;
									//cout << "after addign to _m_throw size " << _m_throws->size()<<endl;

								}

							}
						}
					}

					_datain->pop(); ////////  aaaaaaaaa

				}
				_datain->pop(); //////
			}

		}
		// _m_throws[mid] = exception_t;
		_datain->pop(); ////
	}

}

map<unsigned int, vector<string>*>* DexFileReader::getMethodThrows() {
	return _m_throws;
}
/*
 set<unsigned int>* DexFileReader::getSelfDefinedMidSet()
 {
 return _selfd_mid;
 }
 */
void DexFileReader::printClassDefInfos() {
	for (unsigned int i = 0; i < _class_def_size; i++) {
		ClassDefInfo* cur_class_info = (*_classdef_idx_ele)[i];
		//cout << endl;
		cout << "cur ClassDef is " << i << "  == "
				<< cur_class_info->getTypename() << endl;
		if (cur_class_info->hasSuperClass()) {
			cout << "cur_class_info  has base type ? "
					<< cur_class_info->getSuperClass() << endl;
		}
		if (cur_class_info->hasInterface()) {
			cout << "cur_class_info  has interface ! "
					<< cur_class_info->hasInterface() << endl;
			int k = 0;
			while (k != (cur_class_info->getInterface()->size())) {
				cout << "interface :) ==> "
						<< cur_class_info->getInterface()->at(k);
				k++;
			}
		}
		cout << endl;
	}
}

ofstream& DexFileReader::getFunctionCallfstream() {
	return _fun_call;
}

void DexFileReader::printClassDefFields() {
	for (unsigned int i = 0; i < _class_def_size; i++) {
		ClassDefInfo* cur_class_info = (*_classdef_idx_ele)[i];
		_file << endl;
		_file << " ================== " << i << "      Type    "
				<< cur_class_info->getTypename() << endl;
		vector<Field*>* fie = cur_class_info->getField();
		if (fie != NULL) {
			for (unsigned int j = 0; j < fie->size(); j++) {

				//ofstream file;
				//file.open("/scratch/output");
				Field* f = fie->at(j);
				_file << "----field idx " << j << "-----" << endl;
				_file << "--- field inside class " << f->getClassOwner()
						<< endl;
				_file << "--- field name " << f->getName() << endl;
				_file << "--- field flag " << f->getFlag() << endl;
				_file << "--- field type " << f->getType() << endl;
				_file << "--- field offset " << f->getOffset() << endl;
				_file << endl;
				//file.close();
			}
		}
	}
}

/*
 void DexFileReader::ilt2ClassDefAdd(il::type* ilt, ClassDefInfo* classdef)
 {
 if (_ilt_2_classdef->count(ilt) > 0)
 return;
 else
 (*_ilt_2_classdef)[ilt] = classdef;
 }


 void DexFileReader::printilt2ClassDef()
 {
 map<il::type*, ClassDefInfo*>::iterator it = _ilt_2_classdef->begin();
 for(; it != _ilt_2_classdef->end(); it ++)
 {
 cout << "key:: il::type" << it->first->to_string()<< endl;
 cout << "value :: classdef " << it->second->getTypename()<< endl;
 }
 }*/
/*
 bool DexFileReader::isInheritedFrom(il::type* inherit,il::type* base)
 {
 ClassDefInfo* base_classdef =NULL;
 if (_ilt_2_classdef->count(base) > 0){//found
 base_classdef = (*_ilt_2_classdef)[base];
 }else // not found
 return false;

 ClassDefInfo* derive_classdef = NULL;
 if (_ilt_2_classdef->count(inherit) > 0){
 derive_classdef = (*_ilt_2_classdef)[inherit];
 }else
 return false;

 while(derive_classdef->hasSuperClass())
 {
 string super_str = derive_classdef->getSuperClass();
 string base_str = base_classdef->getTypename();
 if(super_str == base_str)
 {
 return true;
 }
 if( this->_str_classdef->count(super_str) > 0)//self defined class
 derive_classdef = (*_str_classdef)[super_str];
 else // library class
 return false;
 }
 return false;
 }
 */

bool DexFileReader::selfDefinedClass(unsigned int type_idx) //from the typeidx to check if this is a self defined Class
		{
	bool is_selfdef_class = false;
	////////this one guarantee the typeidx->classdefidx / classdefidx->classdef maps are built.
	map<int, int>* tidx_classdefidx = this->_typeidx_classdefidx;
	if (tidx_classdefidx->count(type_idx) > 0)
		is_selfdef_class = true;
	return is_selfdef_class;
}

bool DexFileReader::isSelfDefinedType(unsigned int type_idx) {
	if (type_idx == NO_INDEX)
		return false;
	//cout << "type_idx " << type_idx << endl;
	string class_name = this->getTypename(type_idx);
	if (class_name.substr(0, 1) != "[")
		return selfDefinedClass(type_idx);

	unsigned int inner_type_idx = NO_INDEX;
	class_name = class_name.substr(1, class_name.size() - 1);
	inner_type_idx = this->getTypeIdx(class_name);
	//assert(inner_type_idx);
	return isSelfDefinedType(inner_type_idx); //recursive call the function itself to handle "[[[["
}

void DexFileReader::printClassDef() {
	map<int, int>* tid_classdefid = this->getClassToTypeidx();
	map<int, int>::iterator it = tid_classdefid->begin();
	for (; it != tid_classdefid->end(); it++) {
		cout << "tid_classdefid == > key (type id ) " << it->first << endl;
		cout << "tid_classdefid == > value (classdef id ) " << it->second
				<< endl;
	}
	assert(
			tid_classdefid->size() == _base_field_unresolved_selfclassdef->size());
	assert(
			_base_field_unresolved_selfclassdef->size() == _classdef_idx_ele->size());
	cout << "ClassDefSize " << tid_classdefid->size() << endl;
	map<unsigned int, ClassDefInfo*>::iterator it1 =
			_base_field_unresolved_selfclassdef->begin();
	for (; it1 != _base_field_unresolved_selfclassdef->end(); it1++) {
		cout << "_base_field_unresolved_selfclassdef ==>key(classdefid )"
				<< it1->first << endl;
		cout << "_base_field_unresolved_selfclassdef ==>value(classdef )"
				<< it1->second->getTypename() << endl;
	}

}
///
vector<pair<unsigned int, unsigned int>>* DexFileReader::initialBaseClassFiledsToDerivative() {
	map<unsigned int, ClassDefInfo*>::iterator it =
			this->_base_field_unresolved_selfclassdef->begin();

	vector<pair<unsigned int, unsigned int>>* top_two_level = new vector<
			pair<unsigned int, unsigned int>>(); //they all in classdef, so directly use classdefIdx

	for (; it != this->_base_field_unresolved_selfclassdef->end(); it++) {
		unsigned int classdefidx = it->first;
		ClassDefInfo* classdef = it->second;
		unsigned int super_t_idx = classdef->getSuperClassIdx();
		if (selfDefinedClass(super_t_idx)) { // super class is self defined
			assert(_typeidx_classdefidx->count(super_t_idx) > 0);
			int super_classdef_idx = (*_typeidx_classdefidx)[super_t_idx]; //find the super' class's ClassDefIn
			assert(_classdef_idx_ele->count(super_classdef_idx) > 0);
			//super class inside classdef, but also super's super shouldn't inside classdef
			ClassDefInfo* super_classdef =
					(*_classdef_idx_ele)[super_classdef_idx];
			unsigned int super_super_t_idx = super_classdef->getSuperClassIdx();
			if (!selfDefinedClass(super_super_t_idx)) {
				pair<unsigned int, unsigned int> p(super_classdef_idx,
						classdefidx); //
				//cout << "find pair < " << super_classdef->getTypename() <<", "<< classdef->getTypename() << ">" << endl;
				top_two_level->push_back(p);
			}
		} else //get the typeid for super_lib and selfdefined classes(super is not inside classdef, so use typeidx)
		{
			//search the classdefidx->typeidx map and get the typeidx for classdefidx
			assert(_classdefidx_typeidx->count(classdefidx) > 0);
			int deriv_t_idx = (*_classdefidx_typeidx)[classdefidx];
			pair<unsigned int, unsigned int> p1(super_t_idx, deriv_t_idx);
			string top_selfdefined_class = getTypename(deriv_t_idx);
			_lib_super_selfdefined_derivative->push_back(p1);
			_top_selfdefined_class->push_back(top_selfdefined_class);
			if (0) {
				cout << "p1. derivative " << p1.first << endl;
				cout << "p1. second " << p1.second << endl;
			}
		}
	}
	setfakeLibClassDefWithFields();
	return top_two_level;

}
void DexFileReader::addBaseClassFieldsToDerivative(
		vector<pair<unsigned int, unsigned int>>* top_two_level) {
	//visit each element of classdef info, we now have _classdef_idx_ele; _base_field_unresolved_selfclassdef, _unresolved_lib_boundary_type_idx built.
	//first process the lib super part, but we havent' get it , make it up later once we finish fake the fields of super class ***Mark here Pay ATTENTION;
	//after build the lib_super classdef, we want it to update the the classdef who is self defined classdef
	if (0) {
		cout << "first check >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> " << endl << endl;
		PrintDerivativeSelfDefinedBaseLib();
	}

	//vector<pair<unsigned int, unsigned int>>::iterator it = this->_lib_super_selfdefined_derivative->begin();
	for (unsigned int i = 0;
			i < this->_lib_super_selfdefined_derivative->size(); i++) {
		pair<unsigned int, unsigned int> cur_p =
				_lib_super_selfdefined_derivative->at(i);
		unsigned int super_class_type_id = cur_p.first;
		unsigned int deriv_class_type_id = cur_p.second;
		assert(_faked_lib_classdef_with_fields->count(super_class_type_id) > 0);

		ClassDefInfo* sup_lib_classdef =
				(*_faked_lib_classdef_with_fields)[super_class_type_id];
		if (sup_lib_classdef->getField() != NULL) //update the derivative class
		{
			//get classdef of derivate
			assert(getClassToTypeidx()->count(deriv_class_type_id) >0);
			unsigned int deriv_classdef_id =
					(*_typeidx_classdefidx)[deriv_class_type_id];
			assert(_classdef_idx_ele->count(deriv_classdef_id) > 0);
			ClassDefInfo* deriv_selfdefined_classdef =
					(*_classdef_idx_ele)[deriv_classdef_id];
			updateDerivativeFields(deriv_selfdefined_classdef,
					sup_lib_classdef);
		}
	}
	if (0) {
		cout << "second check <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl << endl;
		PrintDerivativeSelfDefinedBaseLib();
	}

	//
	//next we process the first two levels that
	//test print
	/*
	 cout << "build top_two_level classdef ==============" << endl;
	 for(int i = 0; i < top_two_level->size(); i++)
	 {
	 cout << "super ::: classidx " << top_two_level->at(i).first << "type :: " << (*_classdef_idx_ele)[top_two_level->at(i).first]->getTypename() << endl;
	 cout << "deriv ::: " << top_two_level->at(i).second << "type :: "  <<(*_classdef_idx_ele)[top_two_level->at(i).second]->getTypename()  <<  endl;
	 }
	 cout << "end of build first round top_two_level classdef ========== " << endl;
	 */
	//test print
	vector<unsigned int>* top_level = new vector<unsigned int>();
	unsigned int k = 0;
	//bool flag = false;
	while ((top_two_level != NULL) && (top_two_level->size() > 0)) {
		//cout << "ROUND: " << k<< endl;
		//proces the first two levels, that is to say, use the
		//check top_two_level
		//cout << k << "       ========  in"  << endl;
		for (unsigned int i = 0; i < top_two_level->size(); i++) {
			pair<unsigned int, unsigned int> p = top_two_level->at(i);
			unsigned int s_classdef_idx = p.first;
			unsigned int d_classdef_idx = p.second;
			assert(_classdef_idx_ele->count(s_classdef_idx) > 0);
			assert(_classdef_idx_ele->count(d_classdef_idx) >0);
			ClassDefInfo* s_classdef = (*_classdef_idx_ele)[s_classdef_idx];
			ClassDefInfo* d_classdef = (*_classdef_idx_ele)[d_classdef_idx];
			updateDerivativeFields(d_classdef, s_classdef);
			//cout << "top_two_level " << top_two_level->size() << endl;
			//cout << "class: " << (*_classdef_idx_ele)[d_classdef_idx]->getTypename() << " --> ";
			//(*_classdef_idx_ele)[d_classdef_idx]->printFields();
		}
		for (unsigned int i = 0; i < top_two_level->size(); i++) {
			pair<unsigned int, unsigned int> p = top_two_level->at(i);
			unsigned int super_classdef_idx = p.first;
			unsigned int deriv_classdef_idx = p.second;
			//delete p.first and save p.second to top_level
			//cout << "before erasing _base_field_unresolved_selfclassdef size: " << _base_field_unresolved_selfclassdef->size()<< ", ";
			if (_base_field_unresolved_selfclassdef->count(super_classdef_idx)
					> 0) {
				_base_field_unresolved_selfclassdef->erase(super_classdef_idx);
				//cout << "erasing: " << (*_classdef_idx_ele)[super_classdef_idx]->getTypename() << ", ";
				//cout << "after erasing _base_field_unresolved_selfclassdef size: " << _base_field_unresolved_selfclassdef->size()<< endl;
			} else {
				//cout << (*_classdef_idx_ele)[super_classdef_idx]->getTypename() << " is already delted" << endl;
			}

			if (_base_field_unresolved_selfclassdef->count(deriv_classdef_idx)
					> 0)
				top_level->push_back(deriv_classdef_idx);
		}

		top_two_level->clear();
		//cout << "-----------------top_two_level size after delete " <<top_two_level->size() << endl;
		//top_two_level->clear();
		//update top_two_level, we have the another round super, we need to find their derivs
		map<unsigned int, ClassDefInfo*>::iterator unresolved_map_it;

		for (unsigned int j = 0; j < top_level->size(); j++) //for each top_level 's classdef idx, we need to find their children
				{ //parent
				  //cout << "using  " << (*_classdef_idx_ele)[top_level->at(j)]->getTypename()<< "( " <<top_level->at(j)<<" )" << " to find children" << endl;
			for (unresolved_map_it =
					_base_field_unresolved_selfclassdef->begin();
					unresolved_map_it
							!= _base_field_unresolved_selfclassdef->end();
					unresolved_map_it++) {
				unsigned int cdf_idx = unresolved_map_it->first; //child cdfidx
				ClassDefInfo* cdf = unresolved_map_it->second; //child cdf
				unsigned int s_t_id = cdf->getSuperClassIdx(); //should map it to the classdef idx //child's parent
				//assert(_typeidx_classdefidx->count(s_t_id) > 0);
				//cout << "Now: checking: " << cdf->getTypename() << ", s_t_id: "<<s_t_id << endl;
				if (selfDefinedClass(s_t_id)) {
					//cout << cdf->getTypename() << "into if" ;
					assert(_typeidx_classdefidx->count(s_t_id) > 0);
					unsigned int s_cdf_idx = (*_typeidx_classdefidx)[s_t_id];
					//cout << ", " << s_cdf_idx << endl;
					if (s_cdf_idx == top_level->at(j)) //save it to the top_two_level
							{
						//cout << "foud child " << (*_classdef_idx_ele)[cdf_idx]->getTypename()<< endl;
						pair<unsigned int, unsigned int> p2(s_cdf_idx, cdf_idx);
						top_two_level->push_back(p2);
					}
				}
			}
		}
		top_level->clear();
		k++;
	}

//	assert(this->_base_field_unresolved_selfClassdef->size()==0);

}

void DexFileReader::updateDerivativeFields(ClassDefInfo* deriv_classdef,
		ClassDefInfo* base_classdef) {
	//assert(_classdef_idx_ele->count(deriv_classdef_idx) > 0);
	//assert(_classdef_idx_ele->count(base_classdef_idx) > 0);
	//ClassDefInfo* deriv_classdef = (*_classdef_idx_ele)[deriv_classdef_idx];
	//if(_classdef_idx_ele->count(base_classdef_idx) > 0)
	//	base_classdef = (*_classdef_idx_ele)[base_classdef_idx];
	//else

	vector<Field*>* deriv_fields = deriv_classdef->getField();
	vector<Field*>* base_fields = base_classdef->getField();
	unsigned int base_f_size = base_fields->size();
	unsigned int deri_f_old_size = deriv_fields->size();
	string deriv_classname = deriv_classdef->getTypename();

	if (0) {
		for (unsigned int i = 0; i < deriv_fields->size(); i++) {
			Field* cur_f = deriv_fields->at(i);
			cout << "o fields offset ==  " << cur_f->getOffset() << endl;
			cout << "o fields classowner ==  " << cur_f->getClassOwner()
					<< endl;
			cout << "o fields name ==" << cur_f->getName() << endl;
			cout << "o fields Type== " << cur_f->getType() << endl;
		}
		cout << "==============================================" << endl;
	}
	//now we need to update deriv_fields with the base_fields;
	//first update all offset of fields inside deriv_class;
	//cout << "base_classdef :: " << base_classdef->getTypename()<< endl;
	//cout << "deriv_classdef :: " << deriv_classdef->getTypename()<< endl;
	unsigned int offset_begin_base = base_classdef->getTotalOffset();
	for (unsigned int i = 0; i < deriv_fields->size(); i++) {
		Field* deri_f = deriv_fields->at(i);
		unsigned int old_offset = deri_f->getOffset();
		unsigned int new_offset = old_offset + offset_begin_base;
		new_offset = (new_offset / 8);
		deri_f->setCurrOff(new_offset);
	} ////next we want to assert basefields to derivative fields;
	  //shift base size of base fields
	for (unsigned int i = 0; i < base_f_size; i++) {
		Field* f = new Field();
		deriv_fields->push_back(f);
	}
	//size should be changed now

	for (int i = deri_f_old_size - 1; i != -1; i--) {
		deriv_fields->at(i + base_f_size) = deriv_fields->at(i);
	} //now insert the number of fields from the base Classdef
	for (unsigned int i = 0; i < base_f_size; i++) {
		//cout << " i " << i<<endl;
		deriv_fields->at(i) = base_fields->at(i);
		deriv_fields->at(i)->setClassOwner(deriv_classname);
	}
	if (0) {
		for (unsigned int i = 0; i < deriv_fields->size(); i++) {
			Field* cur_f = deriv_fields->at(i);
			cout << "fields offset ==  " << cur_f->getOffset() << endl;
			cout << "fields classowner " << cur_f->getClassOwner() << endl;
			cout << "fields name ==" << cur_f->getName() << endl;
			cout << "fields Type== " << cur_f->getType() << endl;
		}
		cout << "==============================================" << endl;
	}
}



void DexFileReader::setfakeLibClassDefWithFields() {
	//for those whose super class is inside lib, we need to fake a classdef with Fields for them
	vector<pair<unsigned int, unsigned int>>::iterator it =
			this->_lib_super_selfdefined_derivative->begin();

	for (; it != this->_lib_super_selfdefined_derivative->end(); it++) {
		unsigned int super_t_idx = it->first;
		//	cout << "super_t_idx " << super_t_idx << "base_t_idx " << it->second << endl;
		if (this->_faked_lib_classdef_with_fields->count(super_t_idx))
			continue;
		//string super_name = getType(super_t_idx);
		string super_name = getTypename(super_t_idx);
		ClassDefInfo* super_classdef = this->getClassDefByName(super_name);
		//12/20
		super_classdef->setTypeIdx(super_t_idx);
		//12/20
		(*this->_faked_lib_classdef_with_fields)[super_t_idx] = super_classdef;
		//	cout << "super name " << getTypename(super_t_idx);

		//12/14 I want to add them into _str_classdef
		//(*this->_str_classdef)[super_name] = super_classdef; //this would be fine, because super_classdef will be updated with fields

		//12/14

	}
	//cout << "total " << _faked_lib_classdef_with_fields->size() << "super classes "  << endl;
}
void DexFileReader::PrintDerivativeSelfDefinedBaseLib() {
	cout << "_faked_lib_classdef_with_fields ->size ()"
			<< _faked_lib_classdef_with_fields->size() << endl;
	cout << "_lib_super_selfdefined_derivative->size()"
			<< _lib_super_selfdefined_derivative->size() << endl;
	//assert(_faked_lib_classdef_with_fields->size() == _lib_super_selfdefined_derivative->size());
	map<unsigned int, ClassDefInfo*>::iterator it =
			this->_faked_lib_classdef_with_fields->begin();
	for (unsigned int i = 0;
			i < this->_lib_super_selfdefined_derivative->size(); i++) {
		pair<unsigned int, unsigned int> p =
				_lib_super_selfdefined_derivative->at(i);
		unsigned int super_t_id = p.first;
		unsigned int derivative_t_id = p.second;
		cout << endl;
		cout << "i " << i << endl;
		cout << "super_t_id " << super_t_id << endl;
		cout << "derivative_t_id " << derivative_t_id << endl;
		cout << "HHH :: derivative inside self defined  type idx ==== "
				<< derivative_t_id << "| type name is == "
				<< getTypename(derivative_t_id) <<endl;
		assert(_typeidx_classdefidx->count(derivative_t_id) > 0);
		unsigned int derivative_classdef_idx =
				(*_typeidx_classdefidx)[derivative_t_id];
		ClassDefInfo* deri_classdef =
				(*_classdef_idx_ele)[derivative_classdef_idx];
		vector<Field*>* deriv_fields = deri_classdef->getField();
		cout << "HHH :: derivative fields  has " << deriv_fields->size()
				<< endl;
		for (unsigned int i = 0; i < deriv_fields->size(); i++) {
			Field* deriv_f = deriv_fields->at(i);
			cout << "original deriv field name ==== " << deriv_f->getName()
					<< endl;
			cout << "original field classowner == " << deriv_f->getClassOwner()
					<< endl;
			cout << "original field offset == " << deriv_f->getOffset() << endl;
			cout << "original field type == " << deriv_f->getType() << endl;
			cout << "origianl field toString == " << deriv_f->toString()
					<< endl;
		}
		cout << "-------------- end of derivative fields infor " << endl
				<< endl;
	}
	for (; it != this->_faked_lib_classdef_with_fields->end(); it++) {
		//pair<unsigned int, unsigned int> p = _lib_super_selfdefined_derivative->at(index);
		unsigned int base_t_id_inlib = it->first;
		ClassDefInfo* base_classdef = it->second;

		//unsigned int super_t_id = p.first;
		//assert(base_t_id_inlib == super_t_id);
		cout << "HHH :: super inside lib type idx === " << base_t_id_inlib
				<< "| type name is ==" << getTypename(base_t_id_inlib) << endl;
		vector<Field*>* fields = base_classdef->getField();
		if (fields == NULL)
			cout << "super class beforing fake " << endl;
		else {
			for (unsigned int i = 0; i < fields->size(); i++) {
				Field* f = fields->at(i);
				cout << "faked field name ===> " << f->getName() << endl;
				cout << "faked field classowner ===> " << f->getClassOwner()
						<< endl;
				cout << "faked offset ===> " << f->getOffset() << endl;
				cout << "faked field type ===>" << f->getType() << endl;
				cout << "faked toString ===> " << f->toString() << endl;
			}
		}
	}
}

bool DexFileReader::insideFakedLibClassDefwithFields(string str) {
	map<unsigned int, ClassDefInfo*>::iterator it =
			this->_faked_lib_classdef_with_fields->begin();
	for (; it != this->_faked_lib_classdef_with_fields->end(); it++) {
		if (getTypename(it->first) == str)
			return true;
	}
	return false;
}

ClassDefInfo* DexFileReader::getFakedLibClassDef(string str) {
	map<unsigned int, ClassDefInfo*>::iterator it =
			this->_faked_lib_classdef_with_fields->begin();
	for (; it != this->_faked_lib_classdef_with_fields->end(); it++) {
		if (getTypename(it->first) == str)
			return it->second;
	}
	assert(false);
}

void DexFileReader::buildTypeMapping() {
	for (unsigned int i = 0; i < _type_ids_size; i++) {
		string typen = getType(i);
		(*_type_map)[i] = typen;
		//12/20
		(*_name_typeid)[typen] = i;
		//12/20
	}
}

unsigned int DexFileReader::getTypeIdx(string typen) //from type nane to type idx
		{
	map<unsigned int, string>::iterator it = this->_type_map->begin();
	for (; it != this->_type_map->end(); it++) {
		if (it->second == typen)
			return it->first;
	}
	return -1;
}

bool DexFileReader::insideTopSelfDefinedClass(string str) {
	//cout << "str to check is top selfdefined?? " << str << endl;
	for (unsigned int i = 0; i < this->_top_selfdefined_class->size(); i++) {
		if (_top_selfdefined_class->at(i) == str) {
			//cout << "yes, this is a top selfdefined! " << endl;
			return true;
		}
	}
	return false;
}

string DexFileReader::readEncoded_value(unsigned int value_type,
		unsigned int value_arg,vector< vector<string>*>* anno_content, unsigned int length) {
	string returnstr;
	unsigned int level = length++;
	//cout << "length " << level << endl;
	//cout << "value_type " << value_type << endl;
	switch (value_type) {

	case VALUE_BYTE: {
		assert(value_arg == 1);
		int value = (int) this->readBytes(1);
		returnstr = Int2Str(value);
	}
		break;

	case VALUE_SHORT: {
		assert((value_arg >0) && (value_arg < 3));
		int value = (int) this->readBytes(value_arg);
		returnstr = Int2Str(value);
	}
		break;
	case VALUE_CHAR: {
		assert((value_arg >0) && (value_arg < 3));
		unsigned int value = (unsigned int) this->readBytes(value_arg);
		if(value_arg == 1)
			value = value_arg & 0x00ff;
		returnstr = Int2Str(value);
	}
		break;
	case VALUE_INT: {
		assert((value_arg >0) && (value_arg < 5));
		int value = (int) this->readBytes(value_arg);
		returnstr = Int2Str(value);
	}
		break;
	case VALUE_LONG: {
		assert((value_arg >0) && (value_arg <9));
		long value = (long) this->readBytes(value_arg);
		returnstr = Long2Str(value);
	}
		break;
	case VALUE_FLOAT: {
		assert((value_arg >0) && (value_arg < 5));
		float value1 = _datain->readVariantFloatx(value_arg);
		returnstr = Float2Str(value1);
	}
		break;
	case VALUE_DOUBLE: {
		assert((value_arg >0) && (value_arg <9));
		double value2 = _datain->readVariantDoublex(value_arg);
		returnstr = Double2Str(value2);
	}
		break;
	case VALUE_STRING: { //last come here !most of the time, the value of annotation is e.g "$getDeviceId"(source,sink,parameter)
		assert((value_arg >0) && (value_arg < 5));
		unsigned int str_idx = (unsigned int) this->readBytes(value_arg);
		str_idx = zeroExtendedto4Bytes(str_idx, value_arg);
		returnstr = this->getString(str_idx);

	}
		break;
	case VALUE_TYPE: {
		assert((value_arg >0) && (value_arg < 5));
		unsigned int type_idx = (unsigned int) this->readBytes(value_arg);
		type_idx = zeroExtendedto4Bytes(type_idx,value_arg);
		returnstr = this->getTypename(type_idx);
	}
		break;
	case VALUE_FIELD: {
		assert((value_arg >0) && (value_arg < 5));
		unsigned int field_idx = (unsigned int) this->readBytes(value_arg);
		field_idx = zeroExtendedto4Bytes(field_idx, value_arg);
		returnstr = this->getField(field_idx)->toString();
	}
		break;
	case VALUE_METHOD: {
		assert((value_arg >0) && (value_arg < 5));
		unsigned int method_ids = (unsigned int) this->readBytes(value_arg);
		method_ids = zeroExtendedto4Bytes(method_ids, value_arg);
		returnstr = this->getMethod(method_ids)->toString();

	}
		break;
	case VALUE_ENUM: {
		assert((value_arg >0) && (value_arg < 5));
		unsigned int field_idx = (unsigned int) this->readBytes(value_arg);
		field_idx = zeroExtendedto4Bytes(field_idx, value_arg);
		returnstr = this->getField(field_idx)->toString();

	}
		break;
	case VALUE_ARRAY: { //first come here
		assert(value_arg == 1);
		unsigned int size = _datain->readULeb128();
		for(int i = 0; i < size; i ++){
			unsigned char one_byte = _datain->readByte();
			unsigned int value_type = one_byte & 0x1f; // lower 5 bits
			unsigned int value_arg = (one_byte & (0xff)) >> 5; //higher 3 bits
			//readEncoded_value(value_type, value_arg +1);
			string value = readEncoded_value(value_type, value_arg +1, anno_content, length);
			returnstr += value + ";; ";
		}
	}
		break;
	case VALUE_ANNOTATION: { //then come here
		assert(value_arg == 1);
		unsigned int type_idx = _datain->readULeb128();
		string type = getType(type_idx);
		type = getTypename(type_idx);
		assert(type.find("Flow") != string::npos); //this is the subannotation e.g. Flow
		//cout << "\t" <<" sub-annotation type " << type << endl;
		unsigned int size = _datain->readULeb128();

		//each time, there is a FlowAnnotation*
		vector<string>* anno = new vector<string>();
		for(int i = 0; i < size; i ++){
			unsigned int name_idx = _datain->readULeb128();
			string name = getString(name_idx); //this is e.g from field inside subannotation(from,through,to)
			//cout << "name == " << name << endl;

			//cout << "sub-annotation name  = [" << name << "]"<< endl;
			unsigned int one_byte = _datain->readByte();
			unsigned int value_type = one_byte & 0x1f;
			unsigned int value_arg = (one_byte & (0xff)) >> 5;
			//assert((name == "from") ||(name == "through") ||(name == "to")); //if name == from, through,or to, seperately put them into sowhere
			//string read_value = readEncoded_value(value_type, value_arg+1);
			string read_value = readEncoded_value(value_type, value_arg+1,NULL,length);
			if((type.find("Flow") != string::npos)&&(anno_content != NULL)){
				anno->push_back(read_value);
				//cout << "save content " << read_value << endl;
			}
			returnstr += name + ", ";
		}
		anno_content->push_back(anno);
	}
		break;
	case VALUE_NULL: {
		assert(value_arg == 1);
		returnstr = "NULL";
	}
		break;
	case VALUE_BOOLEAN: {
		assert((value_arg == 1) ||(value_arg == 2));
		unsigned int value_arg_new = value_arg -1;
		unsigned int flag = 0;
		if(value_arg_new)
			flag = 1;
		returnstr = Int2Str(flag);
	}
		break;
	default: {
		cout << "shouldn't be here no encoding case!" << endl;
		assert(false);
	}

	}
	//cout << "value_type = ["<<value_type << " ]   read encoded value is " << returnstr << " - - - -  - --  - length "<<level << endl << endl;
	return returnstr;
}

vector<sail::Annotation*>* makeAnnotation(vector<vector<string>*>* anno){
	vector<sail::Annotation*>* annota = new vector<sail::Annotation*>();
	sail::FlowAnnotation* flowanno = NULL;
	for(int i = 0;i < anno->size(); i ++){
		assert(anno->at(i) != 0);
		if(anno->at(i)->size() == 3){
			flowanno = new sail::FlowAnnotation(anno->at(i)->at(2), anno->at(i)->at(0), anno->at(i)->at(1));
		}else if(anno->at(i)->size() == 2){
			flowanno = new sail::FlowAnnotation(anno->at(i)->at(1), anno->at(i)->at(0), "");
		}
		annota->push_back(flowanno);
	}
	return annota;
}
