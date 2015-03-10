#include"Method.h"

//hzhu 4/20/2012


Method::Method(DataIn* in, unsigned int methodid, const string name, const string classowner, unsigned int classowneridx,const string returnval, string* para, const int parasize,vector<unsigned int>* paratypeids, unsigned int returnidx):_datain(in),_midx(methodid),_name(name),_class_owner(classowner),_class_owner_idx(classowneridx),_return_type(returnval),_parameter(para),_para_size(parasize),_para_type_ids(paratypeids),_return_idx(returnidx)
{
	_maddr = NULL;
	_is_virtual = false;
	_is_static = false;
	_is_abstract = false;
}
//hzhu end 4/20/2012
Method::~Method()
{
	delete [] _parameter;
}

string Method::getName()
{
	return _name;
}

string Method::getClassOwner()
{
	return _class_owner;
}

string Method::getReturnType()
{
	return _return_type;
}

int Method::getParaSize()
{
	//cout <<"Virtual Flag :: " << this->isVirtual()<< endl;
	//cout << "Static flag ::" << this->isStatic()<<endl;
	//cout << "method " << this->toString()<< " parameter size is " << _para_size <<endl;
	return _para_size;
}
string* Method::getParas()
{
	return _parameter;
}


string Method::getDesc()
{
	string temp;
	if(_desc == "")
	{
		temp = "(";
		if(_parameter!=NULL)
		{
			for(int i = 0;i< _para_size;i++)
			{
				temp+= _parameter[i];
			}
			
		}
		temp+= ")";
		temp+= _return_type;
		_desc = temp;
	}
	return _desc;
}


string Method::toString()
{
	string str;
	string tempstr1 = getClassOwner();
	string tempstr2 = getName();
//	cout << "name of the method :~~~~~~~~~~~~" <<tempstr2<<endl;
	string tempstr3 = getDesc();
	//hzhu 4/20/2012
	return tempstr1 + "->" + tempstr2 + tempstr3;
	/*
	string mid = Int2Str(_midx);
	str = tempstr1 + "->" + tempstr2 + tempstr3 + " ||idx: {"+mid+"}";
	if(_maddr!= NULL)
		//return tempstr1 + "->" + tempstr2 + tempstr3 + " ||idx: {"+mid+"}";
		str = str + "           ||Addr: ["+ Int2Str(_maddr->getBeginAddr())+ " " + Int2Str(_maddr->getEndAddr())+"]";
	
	if(_para_size == 0)
		return str;
	if(_para_size!= 0)
	{
		//str = str  +  " ||parameter ids: ";
		str = str  + "parameter size: " + Int2Str(_para_size) + " ||parameter ids: ";
		for(vector<unsigned int>::iterator it = _para_type_ids->begin();it!=_para_type_ids->end(); it++)
		{
			str = str + Int2Str((*it))+ ", ";
		}
		str = str + "\n";
	}		
	//hzhu 4/20/2012
	return str;
	*/
}

//added for DebugInfo
void Method::setFlag(string flag)
{ 
	_flag= flag;
}

string Method::getFlag()
{
	return _flag;
}



//hzhu add 4/20/2012
unsigned int Method::getMethodIdx()
{
	return _midx;
}

void Method::setMethodAddr(MethodAddr* maddr)
{	
	_maddr = maddr;
}
MethodAddr* Method::getMethodAddr()
{
	return _maddr;
}
//hzhu end

//hzhu 4/22/2012
DataIn* Method::getDataIn()
{
	return _datain;
}
//hzhu 4/22/2012

//hzhu 4/23/2012
vector<unsigned int>* Method::getParaTypeIds()
{
	return _para_type_ids;
}
//hzhu end 4/23/2012

//hzhu begin 4/23/2012
unsigned int Method::getReturnidx()
{
	return _return_idx; 
}
//hzhu end 4/23/2012

unsigned int Method::getClassOwneridx()
{
	return _class_owner_idx;
}

void Method::setVirtual()
{
    _is_virtual = true;
}

bool Method::isVirtual()
{
    return _is_virtual;
}


bool Method::isStatic()
{
    setAccFlag();
    return _is_static;
}

bool Method::isAbstract()
{
    setAccFlag();
    return _is_abstract;
}
void Method::setAccFlag()
{
	if (_flag == "ACC_STATIC")
		_is_static = true;
	if (_flag == "ACC_ABSTRACT")
		_is_abstract = true;
}

/*il::type* Method::getClassOwnerType(DexFileReader* dfr,map<string,ClassDefInfo*>* typemap)
{
    if(_is_static)
	return NULL;
    ClassDefInfo* this_cdf = type2ClassDef(dfr, _class_owner_idx);
    il::type* this_t = getType(0,dfr,this_cdf,typemap);
    return this_t;
}*/
