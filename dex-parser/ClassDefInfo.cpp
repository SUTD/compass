#include"ClassDefInfo.h"

ClassDefInfo::ClassDefInfo (string classname, 
				string accflag, 
				string super,
				string filename, 
				vector<Field*>* field, 
				vector<Method*>* method,
				vector<string>*interface,
				map<int, int>* midx_codeoff) :  _type_name(classname),_class_acc(accflag), _super_class(super), _file_name(filename), _field(field), _method(method), _interface(interface), _method_codeoff(midx_codeoff)
{
    _super_f = false;
    _inte_f = false;
    //_typeidx = -1;
    _typeidx = 0xffffffff;
    //1/8
    _static_field = NULL;
    //1/8

    //3/7/2013
    _lib_type_fields_updated = false;
    //3/7/2013
}


ClassDefInfo::~ClassDefInfo()
{
	//12/14
	delete _field;
	delete _method;
	delete _interface;
	delete _method_codeoff;
	//12/14
}

string ClassDefInfo::getTypename()
{
	//cout << "_type_name " << _type_name << endl;
	if(_type_name == "Z")
	    _type_name = "I";
	//cout << "before returning _type_name  " << _type_name<<  endl;
	return _type_name;
}

string ClassDefInfo::getClassFlag()
{
	return _class_acc;
}

string ClassDefInfo::getFilename()
{
	return _file_name;
}

string ClassDefInfo::getSuperClass()
{
    return _super_class;
}
bool ClassDefInfo::isPrimitive()
{
	//if((_file_name.size()<= 3) && (_file_name.size() > 1))
	if((_type_name.size()==1)&&(_type_name != "V") &&( _field == NULL)&&( _method == NULL)&&( _interface ==NULL))
        {
                return true;
        }
        return false;
}

bool ClassDefInfo::isDigits(){
	if((_type_name.size() ==1)&&(_type_name != "V")/*&&(_type_name != "C")*/)
		return true;
	return false;
}
vector<Field*> * ClassDefInfo::getField()
{
	return _field;
}
vector<Method*> * ClassDefInfo::getMethod()
{
	return _method;
}
vector<string> * ClassDefInfo::getInterface()
{
	return _interface;
} 
map<int,int>* ClassDefInfo::getMethod2Codeoff()
{
	return _method_codeoff;
}
bool ClassDefInfo::hasMethod()
{
    return((_method != NULL) &&(_method->size()>0));
}

void ClassDefInfo::setSuperClassf(bool sup_f)
{
    _super_f = sup_f;
}
void ClassDefInfo::setInterfacef(bool inte_f)
{
    _inte_f = inte_f;
}
bool ClassDefInfo::hasSuperClass()
{
    return _super_f;
}

bool ClassDefInfo::hasInterface()
{
    return _inte_f;
}
void ClassDefInfo::setTotalOffset()
{
    
    unsigned int field_size = _field->size();
    if(field_size == 0){
	this->_total_offset = 0;
	return;
    }
    //cout << "check why can't minus 1 , field_size " << field_size << endl;
    Field* last_field = _field->at(field_size-1);
    assert(last_field != NULL);
    unsigned int lastfield_off_begin = last_field->getOffset();
    string type_str = last_field->getType();
    unsigned int offsize = last_field->setNextOff(type_str);
    this->_total_offset = lastfield_off_begin + offsize*8;
}

void ClassDefInfo::printFields()
{
	cout << "LLLLLLLLLLLL ::: Inside class ==================== "
			<< getTypename() << endl;
	for (unsigned int i = 0; i < _field->size(); i++) {
		cout << "field name ::::::::::::::::::::::" << _field->at(i)->getName()
				<< endl;
		cout << "field offset ::::::::::::::::::::::"
				<< _field->at(i)->getOffset() << endl;
		cout << "field type:::::::::::::::::::::::" << _field->at(i)->getType()
				<< endl << endl;
	}
}

bool ClassDefInfo::insideOriginalFields(string f_n)
{
	if (_field == NULL)
		return false;
	for (unsigned int i = 0; i < _field->size(); i++) {
		Field* temp_f = _field->at(i);
		if (temp_f->getName() == f_n)
			return true;
	}
	return false;
}


